/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <AudioToolbox/AudioToolbox.h>
#include "AppleUtils.h"
#include "MP4Reader.h"
#include "MP4Decoder.h"
#include "mozilla/RefPtr.h"
#include "mozilla/ReentrantMonitor.h"
#include "nsIThread.h"
#include "AppleATDecoder.h"
#include "prlog.h"

#ifdef PR_LOGGING
PRLogModuleInfo* GetDemuxerLog();
#define LOG(...) PR_LOG(GetDemuxerLog(), PR_LOG_DEBUG, (__VA_ARGS__))
#else
#define LOG(...)
#endif

namespace mozilla {

AppleATDecoder::AppleATDecoder(const mp4_demuxer::AudioDecoderConfig& aConfig,
                           MediaTaskQueue* anAudioTaskQueue,
                           MediaDataDecoderCallback* aCallback)
  : mConfig(aConfig)
  , mTaskQueue(anAudioTaskQueue)
  , mCallback(aCallback)
{
  MOZ_COUNT_CTOR(AppleATDecoder);
  LOG("Creating Apple AudioToolbox AAC decoder");
  LOG("Audio Decoder configuration: %s %d Hz %d channels %d bits per channel",
      mConfig.mime_type,
      mConfig.samples_per_second,
      mConfig.channel_count,
      mConfig.bits_per_sample);
  // TODO: Verify aConfig.mime_type.
}

AppleATDecoder::~AppleATDecoder()
{
  MOZ_COUNT_DTOR(AppleATDecoer);
}

static void
_MetadataCallback(void *aDecoder,
                  AudioFileStreamID aStream,
                  AudioFileStreamPropertyID aProperty,
                  UInt32 *aFlags)
{
  LOG("Metadata callback");
}

static void
_SampleCallback(void *aDecoder,
                UInt32 aNumBytes, UInt32 aNumPackets,
                const void *aData,
                AudioStreamPacketDescription *aPackets)
{
  LOG("Sample callback");
}

nsresult
AppleATDecoder::Init()
{
  LOG("Initializing Apple AudioToolbox AAC decoder");
  AudioFileTypeID fileType = kAudioFileAAC_ADTSType;
  // or kAudioFileMPEG4Type, kAudioFileM4AType
  OSStatus rv = AudioFileStreamOpen(this,
                                    _MetadataCallback,
                                    _SampleCallback,
                                    fileType,
                                    &mStream);
  if (rv) {
    return NS_ERROR_FAILURE;
  }

  AudioStreamBasicDescription inputFormat, outputFormat;
  // Fill in the input format description from the stream.
  AppleUtils::GetProperty(mStream,
      kAudioFileStreamProperty_DataFormat, &inputFormat);

  // Fill in the output format manually.
  PodZero(&outputFormat);
  outputFormat.mFormatID = kAudioFormatLinearPCM;
#if defined(MOZ_SAMPLE_TYPE_FLOAT32)
  outputFormat.mBitsPerChannel = 32;
  outputFormat.mFormatFlags =
    kLinearPCMFormatFlagIsFloat |
    0;
#else
# error Unknown audio sample type
#endif
  // Set up the decoder so it gives us one sample per frame
  outputFormat.mFramesPerPacket = 1;
  outputFormat.mBytesPerPacket = outputFormat.mBytesPerFrame
        = outputFormat.mChannelsPerFrame * outputFormat.mBitsPerChannel / 8;

  rv = AudioConverterNew(&inputFormat, &outputFormat, &mConverter);
  if (rv) {
    LOG("Error %d constructing AudioConverter", rv);
    mConverter = nullptr;
    return NS_ERROR_FAILURE;
  }

  return NS_OK;
}

nsresult
AppleATDecoder::Input(mp4_demuxer::MP4Sample* aSample)
{
  LOG("mp4 input sample %p %lld us %lld pts%s %llu bytes", aSample,
      aSample->duration,
      aSample->composition_timestamp,
      aSample->is_sync_point ? " keyframe" : "",
      (unsigned long long)aSample->size);
  return NS_OK;
}

nsresult
AppleATDecoder::Flush()
{
  NS_WARNING(__func__);
  return NS_OK;
}

nsresult
AppleATDecoder::Drain()
{
  NS_WARNING(__func__);
  return NS_OK;
}

nsresult
AppleATDecoder::Shutdown()
{
  NS_WARNING(__func__);
  OSStatus rv = AudioConverterDispose(mConverter);
  if (rv) {
    LOG("error %d disposing of AudioConverter", rv);
    return NS_ERROR_FAILURE;
  }
  return NS_OK;
}

} // namespace mozilla
