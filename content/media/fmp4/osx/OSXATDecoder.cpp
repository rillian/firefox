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
  MOZ_ASSERT(mConfig.codec() == mp4_demuxer::kCodecAAC ||
             mConfig.codec() == mp4_demuxer::kCodecMP3);
}

OSXATDecoder::~OSXATDecoder()
{
  MOZ_COUNT_DTOR(OSXATDecoer);
}

nsresult
OSXATDecoder::Init()
{
  NS_WARNING(__func__);
  return NS_ERROR_FAILURE;
}

nsresult
OSXATDecoder::Input(mp4_demuxer::MP4Sample* aSample)
{
  NS_WARNING(__func__);
  return NS_ERROR_FAILURE;
}

nsresult
OSXATDecoder::Flush()
{
  NS_WARNING(__func__);
  return NS_ERROR_FAILURE;
}

nsresult
OSXATDecoder::Drain()
{
  NS_WARNING(__func__);
  return NS_ERROR_FAILURE;
}

nsresult
OSXATDecoder::Shutdown()
{
  NS_WARNING(__func__);
  return NS_ERROR_FAILURE;
}

} // namespace mozilla
