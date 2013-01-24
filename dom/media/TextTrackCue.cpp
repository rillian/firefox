/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "TextTrackCue.h"
#include "mozilla/dom/TextTrackCueBinding.h"

namespace mozilla {
namespace dom {

NS_IMPL_CYCLE_COLLECTION_UNLINK_BEGIN(TextTrackCue)
  NS_IMPL_CYCLE_COLLECTION_UNLINK_PRESERVED_WRAPPER
NS_IMPL_CYCLE_COLLECTION_UNLINK_END

NS_IMPL_CYCLE_COLLECTION_TRAVERSE_BEGIN(TextTrackCue)
  NS_IMPL_CYCLE_COLLECTION_TRAVERSE(mGlobal)
  NS_IMPL_CYCLE_COLLECTION_TRAVERSE_SCRIPT_OBJECTS
NS_IMPL_CYCLE_COLLECTION_TRAVERSE_END

NS_IMPL_CYCLE_COLLECTION_TRACE_BEGIN(TextTrackCue)
  NS_IMPL_CYCLE_COLLECTION_TRACE_PRESERVED_WRAPPER
NS_IMPL_CYCLE_COLLECTION_TRACE_END

NS_IMPL_CYCLE_COLLECTING_ADDREF(TextTrackCue)
NS_IMPL_CYCLE_COLLECTING_RELEASE(TextTrackCue)

NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(TextTrackCue)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

TextTrackCue::TextTrackCue(nsISupports *aGlobal) : mGlobal(aGlobal)
{
  MOZ_ASSERT(aGlobal);
  SetIsDOMBinding();
}

TextTrackCue::~TextTrackCue()
{
  mGlobal = nullptr;
  mTrack = nullptr;
}

JSObject*
TextTrackCue::WrapObject(JSContext* aCx, JSObject* aScope,
		      bool* aTriedToWrap)
{
  return TextTrackCueBinding::Wrap(aCx, aScope, this, aTriedToWrap);
}

void
TextTrackCue::Init(const double aStartTime, const double aEndTime,
		   const nsAString& aText, ErrorResult& aRv)
{
  // XXXhumph: validation...
  mStartTime = aStartTime;
  mEndTime = aEndTime;
  mText = aText;
}

} // namespace dom
} // namespace mozilla
