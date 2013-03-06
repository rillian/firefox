/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "TextTrackList.h"
#include "mozilla/dom/TextTrackListBinding.h"

namespace mozilla {
namespace dom {

NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE_1(TextTrackList, mGlobal)

NS_IMPL_CYCLE_COLLECTING_ADDREF(TextTrackList)
NS_IMPL_CYCLE_COLLECTING_RELEASE(TextTrackList)

NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(TextTrackList)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

TextTrackList::TextTrackList(nsISupports* aGlobal) : mGlobal(aGlobal)
{
  SetIsDOMBinding();
}

TextTrackList::~TextTrackList()
{
  mGlobal = nullptr;
}

JSObject*
TextTrackList::WrapObject(JSContext* aCx, JSObject* aScope,
                          bool* aTriedToWrap)
{
  return TextTrackListBinding::Wrap(aCx, aScope, this, aTriedToWrap);
}

TextTrack*
TextTrackList::IndexedGetter(uint32_t aIndex, bool& aFound)
{
  aFound = aIndex < mTextTracks.Length();
  return aFound ? mTextTracks[aIndex] : nullptr;
}

} // namespace dom
} // namespace mozilla
