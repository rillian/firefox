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

OSXVTDecoder::OSXVTDecoder(MediaTaskQueue* aVideoTaskQueue,
                           MediaDataDecoderCallback* aCallback)
  : mTaskQueue(aVideoTaskQueue)
  , mCallback(aCallback)
  , mSession(nullptr)
{
  MOZ_COUNT_CTOR(OSXVTDecoder);
}

OSXVTDecoder::~OSXVTDecoder()
{
  MOZ_COUNT_DTOR(OSXVTDecoder);
}

nsresult
OSXVTDecoder::Init()
{
  NS_WARNING(__func__);
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
  return NS_OK;
}

nsresult
OSXVTDecoder::Input(mp4_demuxer::MP4Sample* aSample)
{
  mp4_demuxer::TrackType type = aSample->type;
  const char* types[] = { "invalid", "video", "audio", "hint" };
  //static_assert(type < ArrayLength(types), "Invalid TrackType");
  // Limit out of bound type values.
  if (type >= ArrayLength(types)) {
    type = mp4_demuxer::TrackType::kInvalid;
  }
  LOG("mp4 input sample %p %s %lld us %lld pts %lld dts%s", aSample, 
      types[type],
      aSample->duration,
      aSample->composition_timestamp,
      aSample->decode_timestamp,
      aSample->is_sync_point ? " keyframe" : "");

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
