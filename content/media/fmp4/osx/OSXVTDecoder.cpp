/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <VideoToolbox/Videotoolbox.h>

#include "MP4Reader.h"
#include "MP4Decoder.h"
#include "nsThreadUtils.h"
#include "OSXVTDecoder.h"
#include "prlog.h"

#ifdef PR_LOGGING
PRLogModuleInfo* GetDemuxerLog();
#define LOG(...) PR_LOG(GetDemuxerLog(), PR_LOG_DEBUG, (__VA_ARGS__))
#else
#define LOG(...)
#endif


namespace mozilla {

OSXVTDecoder::OSXVTDecoder(const mp4_demuxer::VideoDecoderConfig& aConfig,
                           MediaTaskQueue* aVideoTaskQueue,
                           MediaDataDecoderCallback* aCallback)
  : mConfig(aConfig) 
  , mTaskQueue(aVideoTaskQueue)
  , mCallback(aCallback)
  , mFormat(nullptr)
  , mSession(nullptr)
{
  MOZ_COUNT_CTOR(OSXVTDecoder);
  MOZ_ASSERT(mConfig.codec() == mp4_demuxer::kCodecH264);
  LOG("Creating OSXVTDecoder for %dx%d h.264 video",
      mConfig.visible_rect().width(),
      mConfig.visible_rect().height()
     );
  LOG("%s", mConfig.AsHumanReadableString().c_str());
}

OSXVTDecoder::~OSXVTDecoder()
{
  MOZ_COUNT_DTOR(OSXVTDecoder);
}

// Callback passed to the VideoToolbox decoder for returning data.
static void
PlatformCallback(void* decompressionOutputRefCon,
                 void* sourceFrameRefCon,
                 OSStatus status,
                 VTDecodeInfoFlags info,
                 CVImageBufferRef image,
                 CMTime presentationTimeStamp,
                 CMTime presentationDuration)
{
  LOG("OSXVideoDecoder::%s status %d flags %d", __func__, status, info);
  if (status != noErr || !image) {
    NS_WARNING("VideoToolbox decoder returned no data");
    return;
  }
  LOG("  got decoded frame data...");
}

nsresult
OSXVTDecoder::Init()
{
  NS_WARNING(__func__);
  OSStatus rv;
  CFMutableDictionaryRef extensions =
    CFDictionaryCreateMutable(NULL, 3,
                              &kCFTypeDictionaryKeyCallBacks,
                              &kCFTypeDictionaryValueCallBacks);
  if (extensions == NULL) {
    NS_WARNING("Couldn't create OSX VideoToolbox format extensions dict");
    return NS_ERROR_FAILURE;
  }
  rv = CMVideoFormatDescriptionCreate(NULL, // Use default allocator.
                                      kCMVideoCodecType_H264,
                                      mConfig.coded_size().width(),
                                      mConfig.coded_size().height(),
                                      extensions,
                                      &mFormat);
  // FIXME: propagate errors to caller.
  NS_ASSERTION(rv == noErr, "Couldn't create format description!");
  VTDecompressionOutputCallbackRecord cb = { PlatformCallback, this };
  rv = VTDecompressionSessionCreate(NULL, // Allocator.
                                    mFormat,
                                    NULL, // Video decoder selection.
                                    NULL, // Output video format.
                                    &cb,
                                    &mSession);
  NS_ASSERTION(rv == noErr, "Couldn't create decompression session!");

  return NS_OK;
}

nsresult
OSXVTDecoder::Shutdown()
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

static const char* track_type_name(mp4_demuxer::TrackType type)
{
  switch (type) {
    case mp4_demuxer::kVideo:
      return "video";
    case mp4_demuxer::kAudio:
      return "audio";
    case mp4_demuxer::kHint:
      return "hint";
    case mp4_demuxer::kInvalid:
      // Fall through.
      ;;
  }
  return "invalid";
}

nsresult
OSXVTDecoder::Input(mp4_demuxer::MP4Sample* aSample)
{
#if 0
  mp4_demuxer::TrackType type = aSample->type;
  int x = 7;
  type = static_cast<mp4_demuxer::TrackType>(x);
  const char* types[] = { "invalid", "video", "audio", "hint" };
  //static_assert(type < ArrayLength(types), "Invalid TrackType");
  // Limit out of bound type values.
  if (type >= ArrayLength(types)) {
    type = mp4_demuxer::TrackType::kInvalid;
  }
#endif
  LOG("mp4 input sample %p %s %lld us %lld pts %lld dts%s", aSample, 
      track_type_name(aSample->type),
      aSample->duration,
      aSample->composition_timestamp,
      aSample->decode_timestamp,
      aSample->is_sync_point ? " keyframe" : "");

  CMBlockBufferRef block;
  CMSampleBufferRef sample;
  VTDecodeInfoFlags flags;
  OSStatus rv;
  std::vector<uint8_t>* buffer = aSample->data;
  // FIXME: This copies the sample data. I think we can provide
  // a custom block source which reuses the aSample buffer.
  rv = CMBlockBufferCreateWithMemoryBlock(NULL // Struct allocator.
                                         ,buffer->data()
                                         ,buffer->size()
                                         ,NULL // Block allocator.
                                         ,NULL // Block source.
                                         ,0    // Data offset.
                                         ,buffer->size()
                                         ,false
                                         ,&block);
  NS_ASSERTION(rv == noErr, "Couldn't create CMBlockBuffer");
  CMSampleTimingInfo timestamp;
  // FIXME: check units here.
  const int32_t msec_per_sec = 1000000;
  timestamp.duration = CMTimeMake(aSample->duration, msec_per_sec);
  timestamp.presentationTimeStamp = CMTimeMake(aSample->composition_timestamp, msec_per_sec);
  timestamp.decodeTimeStamp = CMTimeMake(aSample->decode_timestamp, msec_per_sec);

  rv = CMSampleBufferCreate(NULL, block, true, 0, 0, mFormat, 1, 1, &timestamp, 0, NULL, &sample);
  NS_ASSERTION(rv == noErr, "Couldn't create CMSampleBuffer");
  rv = VTDecompressionSessionDecodeFrame(mSession, sample, 0, aSample, &flags);
  NS_ASSERTION(rv == noErr, "Couldn't pass frame to decoder");
  // Clean up allocations.
  CFRelease(sample);
  CFRelease(block);
  // We took ownership of aSample so we need to release it.
  delete aSample;
  return NS_OK;
}

nsresult
OSXVTDecoder::Flush()
{
  NS_WARNING(__func__);
  return NS_OK;
}

nsresult
OSXVTDecoder::Drain()
{
  NS_WARNING(__func__);
  return NS_OK;
}

} // namespace mozilla
