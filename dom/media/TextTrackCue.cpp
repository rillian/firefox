/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include "TextTrackCue.h"
#include "mozilla/dom/TextTrackCueBinding.h"

namespace mozilla {
namespace dom {

NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE_1(TextTrackCue, mGlobal)

NS_IMPL_CYCLE_COLLECTING_ADDREF(TextTrackCue)
NS_IMPL_CYCLE_COLLECTING_RELEASE(TextTrackCue)

NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(TextTrackCue)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

TextTrackCue::TextTrackCue(nsISupports* aGlobal,
                           const double aStartTime,
                           const double aEndTime,
                           const nsAString& aText)
  : mGlobal(aGlobal)
  , mText(aText)
  , mStartTime(aStartTime)
  , mEndTime(aEndTime)
  , mPosition(50)
  , mSize(100)
  , mPauseOnExit(false)
  , mSnapToLines(true)
{
  MOZ_ASSERT(aGlobal);
  SetIsDOMBinding();
}

JSObject*
TextTrackCue::WrapObject(JSContext* aCx, JSObject* aScope,
		                     bool* aTriedToWrap)
{
  return TextTrackCueBinding::Wrap(aCx, aScope, this, aTriedToWrap);
}

void
TextTrackCue::CueChanged()
{
  if (mTrack) {
    mTrack->CueChanged(*this);
  }
}
} // namespace dom
} // namespace mozilla
