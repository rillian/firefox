/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <CoreFoundation/CFString.h>
#include "VideoToolbox/Videotoolbox.h"

#include "AppleUtils.h"
#include "mozilla/SHA1.h"
#include "mp4_demuxer/DecoderData.h"
#include "MP4Reader.h"
#include "MP4Decoder.h"
#include "nsAutoPtr.h"
#include "nsThreadUtils.h"
#include "AppleVTDecoder.h"
#include "AppleVTLinker.h"
#include "prlog.h"
#include "MediaData.h"

#ifdef PR_LOGGING
PRLogModuleInfo* GetDemuxerLog();
#define LOG(...) PR_LOG(GetDemuxerLog(), PR_LOG_DEBUG, (__VA_ARGS__))
#define LOG_MEDIA_SHA1
#else
#define LOG(...)
#endif

namespace mozilla {

AppleVTDecoder::AppleVTDecoder(const mp4_demuxer::VideoDecoderConfig& aConfig,
                           MediaTaskQueue* aVideoTaskQueue,
                           MediaDataDecoderCallback* aCallback,
                           layers::ImageContainer* aImageContainer)
  : mConfig(aConfig)
  , mTaskQueue(aVideoTaskQueue)
  , mCallback(aCallback)
  , mImageContainer(aImageContainer)
  , mFormat(nullptr)
  , mSession(nullptr)
{
  MOZ_COUNT_CTOR(AppleVTDecoder);
  // TODO: Verify aConfig.mime_type.
  LOG("Creating AppleVTDecoder for %dx%d h.264 video",
      mConfig.display_width,
      mConfig.display_height
     );
}

AppleVTDecoder::~AppleVTDecoder()
{
  MOZ_COUNT_DTOR(AppleVTDecoder);
}

nsresult
AppleVTDecoder::Init()
{
  NS_WARNING(__func__);
  nsresult rv = InitializeSession();
  return rv;
}

nsresult
AppleVTDecoder::Shutdown()
{
  NS_WARNING(__func__);
  if (mSession) {
    LOG("%s: cleaning up session %p", __func__, mSession);
    VTDecompressionSessionInvalidate(mSession);
    CFRelease(mSession);
    mSession = nullptr;
  }
  if (mFormat) {
    LOG("%s: releasing format %p", __func__, mFormat);
    CFRelease(mFormat);
    mFormat = nullptr;
  }
  return NS_OK;
}

nsresult
AppleVTDecoder::Input(mp4_demuxer::MP4Sample* aSample)
{
  LOG("mp4 input sample %p %lld us %lld pts%s %d bytes dispatched",
      aSample,
      aSample->duration,
      aSample->composition_timestamp,
      aSample->is_sync_point ? " keyframe" : "",
      aSample->size);

#ifdef LOG_MEDIA_SHA1
  SHA1Sum hash;
  hash.update(aSample->data, aSample->size);
  uint8_t digest_buf[SHA1Sum::HashSize];
  hash.finish(digest_buf);
  nsAutoCString digest;
  for (size_t i = 0; i < sizeof(digest_buf); i++) {
    digest.AppendPrintf("%02x", digest_buf[i]);
  }
  LOG("    sha1 %s", digest.get());
#endif // LOG_MEDIA_SHA1

  mTaskQueue->Dispatch(
      NS_NewRunnableMethodWithArg<nsAutoPtr<mp4_demuxer::MP4Sample>>(
          this,
          &AppleVTDecoder::SubmitFrame,
          nsAutoPtr<mp4_demuxer::MP4Sample>(aSample)));
  return NS_OK;
}
nsresult
AppleVTDecoder::Flush()
{
  NS_WARNING(__func__);
  return NS_OK;
}

nsresult
AppleVTDecoder::Drain()
{
  NS_WARNING(__func__);
  return NS_OK;
}

//
// Implementation details.
//

// Callback passed to the VideoToolbox decoder for returning data.
// This needs to be static because the API takes a C-style pair of
// function and userdata pointers.
// This validates parameters and
// forwards the decoded image back to an object method.
static void
PlatformCallback(void* decompressionOutputRefCon,
                 void* sourceFrameRefCon,
                 OSStatus status,
                 VTDecodeInfoFlags flags,
                 CVImageBufferRef image,
                 CMTime presentationTimeStamp,
                 CMTime presentationDuration)
{
  AppleVTDecoder* decoder = static_cast<AppleVTDecoder*>(decompressionOutputRefCon);
  mp4_demuxer::MP4Sample* sample = static_cast<mp4_demuxer::MP4Sample*>(sourceFrameRefCon);

  LOG("AppleVideoDecoder %s status %d flags %d", __func__, status, flags);

  // Validate our arguments.
  if (status != noErr || !image) {
    NS_WARNING("VideoToolbox decoder returned no data");
    return;
  }
  if (flags & kVTDecodeInfo_FrameDropped) {
    NS_WARNING("  ...frame dropped...");
  }
  MOZ_ASSERT(CFGetTypeID(image) == CVPixelBufferGetTypeID(),
    "VideoToolbox returned an unexpected image type");

  // Forward the data back to an object method which can access
  // the correct MP4Reader callback.
  decoder->OutputFrame(image, sample);
}

// Copy and return a decoded frame.
// FIXME: probably better to queue this as a new task
// to avoid this thread-unsafe public member function.
nsresult
AppleVTDecoder::OutputFrame(CVPixelBufferRef aImage,
                          mp4_demuxer::MP4Sample* aSample)
{
  size_t width = CVPixelBufferGetWidth(aImage);
  size_t height = CVPixelBufferGetHeight(aImage);
  LOG("  got decoded frame data... %ux%u %s", width, height,
      CVPixelBufferIsPlanar(aImage) ? "planar" : "chunked");
  size_t planes = CVPixelBufferGetPlaneCount(aImage);
  for (size_t i = 0; i < planes; ++i) {
    size_t stride = CVPixelBufferGetBytesPerRowOfPlane(aImage, i);
    LOG("     plane %u %ux%u rowbytes %u",
        (unsigned)i,
        CVPixelBufferGetWidthOfPlane(aImage, i),
        CVPixelBufferGetHeightOfPlane(aImage, i),
        (unsigned)stride);
  }

  MOZ_ASSERT(planes == 2);
  uint8_t* frame = new uint8_t[width * height * 3 / 2];
  PodZero(frame);
  VideoData::YCbCrBuffer buffer;

  // Y plane.
  buffer.mPlanes[0].mData = frame;
  buffer.mPlanes[0].mStride = CVPixelBufferGetBytesPerRowOfPlane(aImage, 0);
  buffer.mPlanes[0].mWidth = width;
  buffer.mPlanes[0].mHeight = height;
  buffer.mPlanes[0].mOffset = 0;
  buffer.mPlanes[0].mSkip = 0;
  // Cb plane.
  buffer.mPlanes[1].mData = frame + width*height;
  buffer.mPlanes[1].mStride = CVPixelBufferGetBytesPerRowOfPlane(aImage, 1);
  buffer.mPlanes[1].mWidth = (width+1) / 2;
  buffer.mPlanes[1].mHeight = (height+1) / 2;
  buffer.mPlanes[1].mOffset = 0;
  buffer.mPlanes[1].mSkip = 1;
  // Cr plane.
  buffer.mPlanes[2].mData = frame + width*height;
  buffer.mPlanes[2].mStride = CVPixelBufferGetBytesPerRowOfPlane(aImage, 1);
  buffer.mPlanes[2].mWidth = (width+1) / 2;
  buffer.mPlanes[2].mHeight = (height+1) / 2;
  buffer.mPlanes[2].mOffset = 1;
  buffer.mPlanes[2].mSkip = 1;

  CVReturn rv = CVPixelBufferLockBaseAddress(aImage, kCVPixelBufferLock_ReadOnly);
  MOZ_ASSERT(rv == kCVReturnSuccess, "error locking pixel data");
  memcpy(frame, CVPixelBufferGetBaseAddressOfPlane(aImage, 0), width*height);
  memcpy(frame + width*height, CVPixelBufferGetBaseAddressOfPlane(aImage, 1), width*height/2);
  CVPixelBufferUnlockBaseAddress(aImage, kCVPixelBufferLock_ReadOnly);

  VideoInfo info;
  info.mDisplay = nsIntSize(width, height);
  info.mHasVideo = true;
  gfx::IntRect visible = gfx::IntRect(0,
                                      0,
                                      mConfig.display_width,
                                      mConfig.display_height);
  nsAutoPtr<VideoData> data;
  data =
    VideoData::Create(info,
                      mImageContainer,
                      nullptr,
                      aSample->byte_offset,
                      aSample->composition_timestamp,
                      aSample->duration,
                      buffer,
                      aSample->is_sync_point,
                      aSample->composition_timestamp,
                      visible);
  mCallback->Output(data.forget());
  return NS_OK;
}

// Helper to fill in a timestamp structure.
static CMSampleTimingInfo
TimingInfoFromSample(mp4_demuxer::MP4Sample* aSample)
{
  CMSampleTimingInfo timestamp;

  // FIXME: check units here.
  const int32_t msec_per_sec = 1000000;

  timestamp.duration = CMTimeMake(aSample->duration, msec_per_sec);
  timestamp.presentationTimeStamp =
    CMTimeMake(aSample->composition_timestamp, msec_per_sec);
  // No DTS value available from libstagefright.
  timestamp.decodeTimeStamp =
    CMTimeMake(aSample->composition_timestamp, msec_per_sec);

  return timestamp;
}

nsresult
AppleVTDecoder::SubmitFrame(mp4_demuxer::MP4Sample* aSample)
{
  // For some reason this gives me a double-free error with stagefright.
  AutoCFRelease<CMBlockBufferRef> block = nullptr;
  AutoCFRelease<CMSampleBufferRef> sample = nullptr;
  VTDecodeInfoFlags flags;
  OSStatus rv;

  // FIXME: This copies the sample data. I think we can provide
  // a custom block source which reuses the aSample buffer.
  // But note that there may be a problem keeping the samples
  // alive over multiple frames.
  rv = CMBlockBufferCreateWithMemoryBlock(NULL // Struct allocator.
                                         ,aSample->data
                                         ,aSample->size
                                         ,kCFAllocatorNull // Block allocator.
                                         ,NULL // Block source.
                                         ,0    // Data offset.
                                         ,aSample->size
                                         ,false
                                         ,block.receive());
  NS_ASSERTION(rv == noErr, "Couldn't create CMBlockBuffer");
  CMSampleTimingInfo timestamp = TimingInfoFromSample(aSample);
  rv = CMSampleBufferCreate(NULL, block, true, 0, 0, mFormat, 1, 1, &timestamp, 0, NULL, sample.receive());
  NS_ASSERTION(rv == noErr, "Couldn't create CMSampleBuffer");
  rv = VTDecompressionSessionDecodeFrame(mSession, sample, 0, aSample, &flags);
  NS_ASSERTION(rv == noErr, "Couldn't pass frame to decoder");

  // Ask for more data.
  if (mTaskQueue->IsEmpty()) {
    LOG("AppleVTDecoder task queue empty; requesting more data");
    mCallback->InputExhausted();
  }

  return NS_OK;
}

nsresult
AppleVTDecoder::InitializeSession()
{
  OSStatus rv;
  AutoCFRelease<CFMutableDictionaryRef> extensions =
    CFDictionaryCreateMutable(NULL, 0,
                              &kCFTypeDictionaryKeyCallBacks,
                              &kCFTypeDictionaryValueCallBacks);
#if 1
  AppleUtils::SetCFDict(extensions, "CVImageBufferChromaLocationBottomField", "left");
  AppleUtils::SetCFDict(extensions, "CVImageBufferChromaLocationTopField", "left");
#endif
  AppleUtils::SetCFDict(extensions, "FullRangeVideo", true);

  AutoCFRelease<CFMutableDictionaryRef> atoms =
    CFDictionaryCreateMutable(NULL, 0,
                              &kCFTypeDictionaryKeyCallBacks,
                              &kCFTypeDictionaryValueCallBacks);
  AutoCFRelease<CFDataRef> avc_data = CFDataCreate(NULL,
      mConfig.extra_data.begin(), mConfig.extra_data.length());

#ifdef LOG_MEDIA_SHA1
  SHA1Sum avc_hash;
  avc_hash.update(mConfig.extra_data.begin(), mConfig.extra_data.length());
  uint8_t digest_buf[SHA1Sum::HashSize];
  avc_hash.finish(digest_buf);
  nsAutoCString avc_digest;
  for (size_t i = 0; i < sizeof(digest_buf); i++) {
    avc_digest.AppendPrintf("%02x", digest_buf[i]);
  }
  LOG("AVCDecoderConfig %ld bytes sha1 %s",
      mConfig.extra_data.length(), avc_digest.get());
#endif // LOG_MEDIA_SHA1

  CFDictionarySetValue(atoms, CFSTR("avcC"), avc_data);
  CFDictionarySetValue(extensions, CFSTR("SampleDescriptionExtensionAtoms"), atoms);
  rv = CMVideoFormatDescriptionCreate(NULL, // Use default allocator.
                                      kCMVideoCodecType_H264,
                                      mConfig.display_width,
                                      mConfig.display_height,
                                      extensions,
                                      &mFormat);
  // FIXME: propagate errors to caller.
  NS_ASSERTION(rv == noErr, "Couldn't create format description!");

  // Contruct video decoder selection spec.
  AutoCFRelease<CFMutableDictionaryRef> spec =
    CFDictionaryCreateMutable(NULL, 0,
                              &kCFTypeDictionaryKeyCallBacks,
                              &kCFTypeDictionaryValueCallBacks);
  // This key is supported (or ignored) but not declared prior to OSX 10.9.
#if MAC_OS_X_VERSION_MAX_ALLOWED < 1090
  AutoCFRelease<CFStringRef>
        kVTVideoDecoderSpecification_EnableHardwareAcceleratedVideoDecoder =
        CFStringCreateWithCString(NULL, "EnableHardwareAcceleratedVideoDecoder",
            kCFStringEncodingUTF8);
#endif
  CFDictionarySetValue(spec,
      kVTVideoDecoderSpecification_EnableHardwareAcceleratedVideoDecoder,
      kCFBooleanTrue);

  VTDecompressionOutputCallbackRecord cb = { PlatformCallback, this };
  rv = VTDecompressionSessionCreate(NULL, // Allocator.
                                    mFormat,
                                    spec, // Video decoder selection.
                                    NULL, // Output video format.
                                    &cb,
                                    &mSession);
  NS_ASSERTION(rv == noErr, "Couldn't create decompression session!");

  return NS_OK;
}

} // namespace mozilla
