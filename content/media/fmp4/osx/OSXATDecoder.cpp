/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <AudioToolbox/AudioToolbox.h>
#include "MP4Reader.h"
#include "MP4Decoder.h"
#include "mozilla/RefPtr.h"
#include "mozilla/ReentrantMonitor.h"
#include "nsIThread.h"
#include "OSXATDecoder.h"
#include "prlog.h"

#ifdef PR_LOGGING
PRLogModuleInfo* GetDemuxerLog();
#define LOG(...) PR_LOG(GetDemuxerLog(), PR_LOG_DEBUG, (__VA_ARGS__))
#else
#define LOG(...)
#endif

namespace mozilla {

OSXATDecoder::OSXATDecoder(const mp4_demuxer::AudioDecoderConfig& aConfig,
                           MediaTaskQueue* anAudioTaskQueue,
                           MediaDataDecoderCallback* aCallback)
  : mConfig(aConfig)
  , mTaskQueue(anAudioTaskQueue)
  , mCallback(aCallback)
{
  MOZ_COUNT_CTOR(OSXATDecoder);
  LOG("Creating Apple AudioToolbox AAC decoder");
#if 0
  LOG("Audio Decoder configuration: %s %d Hz %d channels %d bits per channel",
      mConfig.mime_type,
      mConfig.samples_per_second,
      mConfig.channel_count,
      mConfig.bits_per_sample);
#endif
  MOZ_ASSERT(mConfig.codec() == mp4_demuxer::kCodecAAC ||
             mConfig.codec() == mp4_demuxer::kCodecMP3);
}

OSXATDecoder::~OSXATDecoder()
{
  MOZ_COUNT_DTOR(OSXATDecoer);
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
OSXATDecoder::Init()
{
  LOG("Initializing Apple AudioToolbox AAC decoder");
  AudioFileTypeID fileType = kAudioFileAAC_ADTSType; // or kAudioFileMPEG4Type, kAudioFileM4AType
  OSStatus rv = AudioFileStreamOpen(this,
                                    _MetadataCallback,
                                    _SampleCallback,
                                    fileType,
                                    &mStream);
  if (rv) {
    return NS_ERROR_FAILURE;
  }
  return NS_OK;
}

nsresult
OSXATDecoder::Input(mp4_demuxer::MP4Sample* aSample)
{
  LOG("mp4 input sample %p %s %lld us %lld pts %lld dts%s %d bytes", aSample,
      MP4Reader::TrackTypeToStr(aSample->type),
      aSample->duration,
      aSample->composition_timestamp,
      aSample->decode_timestamp,
      aSample->is_sync_point ? " keyframe" : "",
      aSample->data->size());
  return NS_OK;
}

nsresult
OSXATDecoder::Flush()
{
  NS_WARNING(__func__);
  return NS_OK;
}

nsresult
OSXATDecoder::Drain()
{
  NS_WARNING(__func__);
  return NS_OK;
}

nsresult
OSXATDecoder::Shutdown()
{
  NS_WARNING(__func__);
  return NS_OK;
}

} // namespace mozilla
