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

  TextTrack(nsISupports *aParent,
      const nsAString& aKind,
      const nsAString& aLabel,
      const nsAString& aLanguage);
  ~TextTrack();

  virtual JSObject* WrapObject(JSContext* aCx, JSObject* aScope,
			       bool* aTriedToWrap);

  nsISupports* GetParentObject();

  void GetKind(nsAString& aKind);
  void GetLabel(nsAString& aLabel);
  void GetLanguage(nsAString& aLanguage);
  void GetInBandMetadataTrackDispatchType(nsAString& aType);

  TextTrackCueList* GetCues();
  TextTrackCueList* GetActiveCues();

  void AddCue(TextTrackCue& cue);
  void RemoveCue(TextTrackCue& cue);
  void CueChanged(TextTrackCue& cue);

  IMPL_EVENT_HANDLER(cuechange)

private:
  nsCOMPtr<nsISupports> mParent;

  nsString mKind;
  nsString mLabel;
  nsString mLanguage;
  nsString mType;
  nsString mMode;

  //XXX: TextTrackMode mMode needs to be
  // implemented...
  // spec says its an enum

  nsRefPtr<TextTrackCueList> mCueList;
  nsRefPtr<TextTrackCueList> mActiveCueList;

};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_TextTrack_h
