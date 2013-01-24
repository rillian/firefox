/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 et tw=78: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_TextTrack_h
#define mozilla_dom_TextTrack_h

//#include "mozilla/dom/TextTrackBinding.h"
#include "TextTrackCue.h"
#include "TextTrackCueList.h"
#include "nsWrapperCache.h"
#include "nsCycleCollectionParticipant.h"
#include "nsDOMEventTargetHelper.h"
#include "nsString.h"
#include "nsCOMPtr.h"

namespace mozilla {
namespace dom {

class TextTrackCue;
class TextTrackCueList;

class TextTrack MOZ_FINAL : public nsDOMEventTargetHelper
{
public:
  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS(TextTrack)
  NS_FORWARD_NSIDOMEVENTTARGET(nsDOMEventTargetHelper::)

  TextTrack(nsISupports *aParent);
  ~TextTrack();

  virtual JSObject* WrapObject(JSContext* aCx, JSObject* aScope,
			       bool* aTriedToWrap);

  nsISupports* GetParentObject()
  {
    return mParent;
  }

  void GetKind(nsAString& aKind)
  {
    aKind = mKind;
  }

  void GetLabel(nsAString& aLabel)
  {
    aLabel = mLabel;
  }

  void GetLanguage(nsAString& aLanguage)
  {
    aLanguage = mLanguage;
  }

  void GetInBandMetadataTrackDispatchType(nsAString& aType)
  {
    aType = mType;
  }

  TextTrackCueList*
  GetCues()
  {
    // XXXhumph: todo
    return nullptr;
  }

  TextTrackCueList*
  GetActiveCues()
  {
    // XXXhumph: todo
    return nullptr;
  }

  void AddCue(TextTrackCue& cue)
  {
    // XXXhumph: todo
  }

  void RemoveCue(TextTrackCue& cue)
  {
    // XXXhumph: todo
  }

  IMPL_EVENT_HANDLER(cuechange)

private:
  nsCOMPtr<nsISupports> mParent;

  nsString mKind;
  nsString mLabel;
  nsString mLanguage;
  nsString mType;

  // XXXhumph: need list of cues, active cues...
};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_TextTrack_h
