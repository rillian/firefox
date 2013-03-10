/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 et tw=78: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_TextTrackList_h
#define mozilla_dom_TextTrackList_h

#include "TextTrack.h"
#include "nsCycleCollectionParticipant.h"
#include "nsDOMEventTargetHelper.h"

namespace mozilla {
namespace dom {

class TextTrack;

class TextTrackList MOZ_FINAL : public nsDOMEventTargetHelper
{
public:
  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS(TextTrackList)
  NS_FORWARD_NSIDOMEVENTTARGET(nsDOMEventTargetHelper::)

  // TextTrackList
  TextTrackList(nsISupports* aGlobal);
  ~TextTrackList();

  virtual JSObject* WrapObject(JSContext* aCx, JSObject* aScope,
      bool* aTriedToWrap);

  nsISupports* GetParentObject()
  {
    return mGlobal;
  }

  uint32_t
  Length()
  {
    return mTextTracks.Length();
  }

  TextTrack* IndexedGetter(uint32_t aIndex, bool& aFound);

  already_AddRefed<TextTrack> AddTextTrack(const nsAString& aKind,
                                           const nsAString& aLabel,
                                           const nsAString& aLanguage);
  void RemoveTextTrack(const TextTrack& aTrack);

  IMPL_EVENT_HANDLER(addtrack)
  IMPL_EVENT_HANDLER(removetrack)

private:
  nsCOMPtr<nsISupports> mGlobal;
  nsTArray<nsRefPtr<TextTrack>> mTextTracks;
};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_TextTrackList_h
