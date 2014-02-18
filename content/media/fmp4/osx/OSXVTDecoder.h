/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined(OSXVTDecoder_h_)
#define OSXVTDecoder_h_

#include "mozilla/RefPtr.h"
#include "mozilla/ReentrantMonitor.h"
#include "nsIThread.h"

namespace mozilla {

class MediaTaskQueue;
class MediaDataDecoderCallback;

class OSXVTDecoder : public AtomicRefCounted<OSXVTDecoder> {
public:
  OSXVTDecoder(MediaTaskQueue* aVideoTaskQueue,
               MediaDataDecoderCallback* aCallback);
  ~OSXVTDecoder();
private:
  RefPtr<MediaTaskQueue> mTaskQueue;
  MediaDataDecoderCallback* mCallback;
};

} // namespace mozilla

#endif // OSXVTDecoder_h_
