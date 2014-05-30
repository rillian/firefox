/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined(OSXATDecoder_h_)
#define OSXATDecoder_h_

#include <AudioToolbox/AudioToolbox.h>
#include "mozilla/RefPtr.h"
#include "mozilla/ReentrantMonitor.h"
#include "nsIThread.h"

namespace mozilla {

class MediaTaskQueue;
class MediaDataDecoderCallback;

class OSXATDecoder : public MediaDataDecoder {
public:
  OSXATDecoder(const mp4_demuxer::AudioDecoderConfig& aConfig,
               MediaTaskQueue* aVideoTaskQueue,
               MediaDataDecoderCallback* aCallback);
  ~OSXATDecoder();

  virtual nsresult Init() MOZ_OVERRIDE;
  virtual nsresult Input(mp4_demuxer::MP4Sample* aSample) MOZ_OVERRIDE;
  virtual nsresult Flush() MOZ_OVERRIDE;
  virtual nsresult Drain() MOZ_OVERRIDE;
  virtual nsresult Shutdown() MOZ_OVERRIDE;

private:
  const mp4_demuxer::AudioDecoderConfig& mConfig;
  RefPtr<MediaTaskQueue> mTaskQueue;
  MediaDataDecoderCallback* mCallback;
  AudioConverterRef mConverter;
  AudioFileStreamID mStream;
};

} // namespace mozilla

#endif // OSXATDecoder_h_
