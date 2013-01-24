/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 et tw=78: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_TextTrackCueList_h
#define mozilla_dom_TextTrackCueList_h

#include "nsWrapperCache.h"
#include "nsCycleCollectionParticipant.h"
#include "TextTrackCue.h"
#include "nsCOMPtr.h"

namespace mozilla {
namespace dom {

class TextTrackCue;

// XXXhumph: should this follow what MediaStreamList does, and use NonRefcountedDOMObject?
class TextTrackCueList MOZ_FINAL : public nsISupports,
				   public nsWrapperCache
{
public:
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS(TextTrackCueList)

  // TextTrackCueList WebIDL
  TextTrackCueList(nsISupports *aParent);
  ~TextTrackCueList();
  virtual JSObject* WrapObject(JSContext* aCx, JSObject* aScope,
			       bool* aTriedToWrap);

  nsISupports* GetParentObject()
  {
    return mParent;
  }

  uint32_t Length()
  {
    return mLength;
  }

  TextTrackCue* IndexedGetter(int32_t aIndex, bool& aFound)
  {
    // XXXhumph: todo
    return nullptr;
  }

  TextTrackCue* GetCueById(const nsAString& id)
  {
    // XXXhumph: todo
    return nullptr;
  }

private:
  nsCOMPtr<nsISupports> mParent;

  uint32_t mLength;
};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_TextTrackCueList_h
