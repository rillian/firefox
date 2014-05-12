/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <CoreFoundation/CFString.h>
#include <VideoToolbox/Videotoolbox.h>

#include "mozilla/SHA1.h"
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

// Helper to set a string, int32_t pair on a CFMutableDictionaryRef.
// We avoid using the CFSTR macros because there's no way to release those.
void
SetCFDict(CFMutableDictionaryRef dict, const char* key, int32_t value)
{
  CFNumberRef valueRef = CFNumberCreate(NULL, kCFNumberSInt32Type, &value);
  CFStringRef keyRef = CFStringCreateWithCString(NULL, key, kCFStringEncodingUTF8);
  CFDictionarySetValue(dict, keyRef, valueRef);
  CFRelease(keyRef);
  CFRelease(valueRef);
}

// Helper to set a string, string pair on a CFMutableDictionaryRef.
static void
SetCFDict(CFMutableDictionaryRef dict, const char* key, const char* value)
{
  CFStringRef keyRef = CFStringCreateWithCString(NULL, key, kCFStringEncodingUTF8);
  CFDictionarySetValue(dict, keyRef, value);
  CFRelease(keyRef);
}

// Helper to set a string, bool pair on a CFMutableDictionaryRef.
static void
SetCFDict(CFMutableDictionaryRef dict, const char* key, bool value)
{
  CFStringRef keyRef = CFStringCreateWithCString(NULL, key, kCFStringEncodingUTF8);
  CFDictionarySetValue(dict, keyRef, value ? kCFBooleanTrue : kCFBooleanFalse);
  CFRelease(keyRef);
}

nsresult
OSXVTDecoder::Init()
{
  NS_WARNING(__func__);
  OSStatus rv;
  CFMutableDictionaryRef extensions =
    CFDictionaryCreateMutable(NULL, 0,
                              &kCFTypeDictionaryKeyCallBacks,
                              &kCFTypeDictionaryValueCallBacks);
#if 0
  SetCFDict(extensions, "CVImageBufferChromaLocationBottomField", "left");
  SetCFDict(extensions, "CVImageBufferChromaLocationTopField", "left");
#endif
  SetCFDict(extensions, "FullRangeVideo", true);

  CFMutableDictionaryRef atoms =
    CFDictionaryCreateMutable(NULL, 0,
                              &kCFTypeDictionaryKeyCallBacks,
                              &kCFTypeDictionaryValueCallBacks);
  NS_WARNING("HACK: loading AVC Configuration from external file");
  const char* avc_filename = "avcC.dat";
  FILE *avc_file = fopen(avc_filename, "rb");
  if (!avc_file) {
    NS_WARNING("Couldn't open AVC Configuration box");
    return NS_ERROR_FAILURE;
  }
  fseek(avc_file, 0, SEEK_END);
  long avc_size = ftell(avc_file);
  MOZ_ASSERT(avc_size > 0, "Error reading avcC size");
  fseek(avc_file, 0, SEEK_SET);
  nsTArray<uint8_t> avc_buffer;
  avc_buffer.SetLength(avc_size);
  size_t bytes_read = fread(avc_buffer.Elements(), 1, avc_size, avc_file);
  if (bytes_read != static_cast<size_t>(avc_size)) {
    NS_WARNING("Couldn't read AVC Configuration box");
  }
  fclose(avc_file);
  CFDataRef avc_data = CFDataCreate(NULL, avc_buffer.Elements(), avc_size);
  CFDictionarySetValue(atoms, CFSTR("avcC"), avc_data);
  CFRelease(avc_data);
  SHA1Sum avc_hash;
  avc_hash.update(avc_buffer.Elements(), avc_size);
  uint8_t digest_buf[SHA1Sum::HashSize];
  avc_hash.finish(digest_buf);
  nsAutoCString avc_digest;
  for (size_t i = 0; i < sizeof(digest_buf); i++) {
    avc_digest.AppendPrintf("%02x", digest_buf[i]);
  }
  LOG("Read %ld bytes of avcC data from '%s' sha1 %s",
      avc_size, avc_filename, avc_digest.get());

  CFDictionarySetValue(extensions, CFSTR("SampleDescriptionExtensionAtoms"), atoms);
  CFRelease(atoms);
  rv = CMVideoFormatDescriptionCreate(NULL, // Use default allocator.
                                      kCMVideoCodecType_H264,
                                      mConfig.coded_size().width(),
                                      mConfig.coded_size().height(),
                                      extensions,
                                      &mFormat);
  CFRelease(extensions);

  // FIXME: propagate errors to caller.
  NS_ASSERTION(rv == noErr, "Couldn't create format description!");

  // Contruct video decoder selection spec.
  CFMutableDictionaryRef spec =
    CFDictionaryCreateMutable(NULL, 0,
                              &kCFTypeDictionaryKeyCallBacks,
                              &kCFTypeDictionaryValueCallBacks);
  // This key is supported (or ignored) but not declared prior to OSX 10.9.
#if MAC_OS_X_VERSION_MAX_ALLOWED < 1090
  CFStringRef
        kVTVideoDecoderSpecification_EnableHardwareAcceleratedVideoDecoder =
        CFStringCreateWithCString(NULL, "EnableHardwareAcceleratedVideoDecoder",
            kCFStringEncodingUTF8);
#endif
  CFDictionarySetValue(spec,
      kVTVideoDecoderSpecification_EnableHardwareAcceleratedVideoDecoder,
      kCFBooleanTrue);
#if MAC_OS_X_VERSION_MAX_ALLOWED < 1090
  CFRelease(kVTVideoDecoderSpecification_EnableHardwareAcceleratedVideoDecoder);
#endif

  VTDecompressionOutputCallbackRecord cb = { PlatformCallback, this };
  rv = VTDecompressionSessionCreate(NULL, // Allocator.
                                    mFormat,
                                    spec, // Video decoder selection.
                                    NULL, // Output video format.
                                    &cb,
                                    &mSession);
  NS_ASSERTION(rv == noErr, "Couldn't create decompression session!");
  CFRelease(spec);

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
  LOG("mp4 input sample %p %s %lld us %lld pts %lld dts%s %d bytes", aSample,
      track_type_name(aSample->type),
      aSample->duration,
      aSample->composition_timestamp,
      aSample->decode_timestamp,
      aSample->is_sync_point ? " keyframe" : "",
      aSample->data->size());

  CMBlockBufferRef block;
  CMSampleBufferRef sample;
  VTDecodeInfoFlags flags;
  OSStatus rv;
  std::vector<uint8_t>* buffer = aSample->data;

  SHA1Sum hash;
  hash.update(buffer->data(), buffer->size());
  uint8_t digest_buf[SHA1Sum::HashSize];
  hash.finish(digest_buf);
  nsAutoCString digest;
  for (size_t i = 0; i < sizeof(digest_buf); i++) {
    digest.AppendPrintf("%02x", digest_buf[i]);
  }
  LOG("    sha1 %s", digest.get());

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
