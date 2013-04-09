/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "TextTrackCueList.h"
#include "mozilla/dom/TextTrackCueListBinding.h"
#include "mozilla/dom/TextTrackCue.h"
 
namespace mozilla {
namespace dom {

NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE_1(TextTrackCueList, mParent)

NS_IMPL_CYCLE_COLLECTING_ADDREF(TextTrackCueList)
NS_IMPL_CYCLE_COLLECTING_RELEASE(TextTrackCueList)

NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(TextTrackCueList)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

TextTrackCueList::TextTrackCueList(nsISupports* aParent) : mParent(aParent)
{
  SetIsDOMBinding();
}

TextTrackCueList::~TextTrackCueList()
{
  mParent = nullptr;
}

void
TextTrackCueList::Update(double time)
{
  uint32_t i, length = mList.Length();
  for (i = 0; i < length; i++) {
    if (time > mList[i]->StartTime() && time < mList[i]->EndTime()) {
      mList[i]->RenderCue();
    }
  }
}

JSObject*
TextTrackCueList::WrapObject(JSContext* aCx, JSObject* aScope)
{
  return TextTrackCueListBinding::Wrap(aCx, aScope, this);
}

TextTrackCue*
TextTrackCueList::IndexedGetter(uint32_t aIndex, bool& aFound)
{
  aFound = aIndex < mList.Length();
  return aFound ? mList[aIndex] : nullptr;
}

TextTrackCue*
TextTrackCueList::GetCueById(const nsAString& id)
{
  if(id.EqualsLiteral("")) {
    return nullptr;
  }

  for (uint32_t i = 0; i < mList.Length(); i++) {
    nsString tid;
    mList[i]->GetId(tid);
    if (id.Equals(tid)) {
      return mList[i];
    }
  }
  return nullptr;
}

void
TextTrackCueList::AddCue(TextTrackCue& cue)
{
  mList.AppendElement(&cue);
}

void
TextTrackCueList::RemoveCue(TextTrackCue& cue)
{
  mList.RemoveElement(&cue);
}

} // namespace dom
} // namespace mozilla
