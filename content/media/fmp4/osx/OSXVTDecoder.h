/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined(OSXVTDecoder_h_)
#define OSXVTDecoder_h_

#include <VideoToolbox/VideoToolbox.h>
#include "mozilla/RefPtr.h"
#include "mozilla/ReentrantMonitor.h"
#include "nsIThread.h"

namespace mozilla {

class MediaTaskQueue;
class MediaDataDecoderCallback;
namespace layers {
  class ImageContainer;
}

class OSXVTDecoder : public MediaDataDecoder {
public:
  OSXVTDecoder(const mp4_demuxer::VideoDecoderConfig& aConfig,
               MediaTaskQueue* aVideoTaskQueue,
               MediaDataDecoderCallback* aCallback,
               layers::ImageContainer* aImageContainer);
  ~OSXVTDecoder();
  virtual nsresult Init() MOZ_OVERRIDE;
  virtual nsresult Input(mp4_demuxer::MP4Sample* aSample) MOZ_OVERRIDE;
  virtual nsresult Flush() MOZ_OVERRIDE;
  virtual nsresult Drain() MOZ_OVERRIDE;
  virtual nsresult Shutdown() MOZ_OVERRIDE;
  // Return hook for VideoToolbox callback.
  nsresult OutputFrame(CVPixelBufferRef aImage,
                       mp4_demuxer::MP4Sample* aSample);
private:
  const mp4_demuxer::VideoDecoderConfig& mConfig;
  RefPtr<MediaTaskQueue> mTaskQueue;
  MediaDataDecoderCallback* mCallback;
  layers::ImageContainer* mImageContainer;
  CMVideoFormatDescriptionRef mFormat;
  VTDecompressionSessionRef mSession;
};

} // namespace mozilla

#endif // OSXVTDecoder_h_
