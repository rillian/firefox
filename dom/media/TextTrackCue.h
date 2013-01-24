/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 et tw=78: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_TextTrackCue_h
#define mozilla_dom_TextTrackCue_h

#include "TextTrack.h"
#include "nsCycleCollectionParticipant.h"
#include "mozilla/dom/DocumentFragment.h"
#include "nsDOMEventTargetHelper.h"

namespace mozilla {
namespace dom {

class TextTrack;

class TextTrackCue MOZ_FINAL : public nsDOMEventTargetHelper
{
public:
  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS(TextTrackCue)
  NS_FORWARD_NSIDOMEVENTTARGET(nsDOMEventTargetHelper::)

  // TextTrackCue WebIDL
  static already_AddRefed<TextTrackCue>
  Constructor(nsISupports* aGlobal,
	      const double aStartTime,
	      const double aEndTime,
	      const nsAString& aText,
	      ErrorResult& aRv)
  {
    nsRefPtr<TextTrackCue> ttcue = new TextTrackCue(aGlobal);
    ttcue->Init(aStartTime, aEndTime, aText, aRv);
    if (aRv.Failed()) {
      return nullptr;
    }
    return ttcue.forget();
  }

  TextTrackCue(nsISupports* aGlobal);
  ~TextTrackCue();

  void Init(const double aStartTime, const double aEndTime,
	    const nsAString& aText, ErrorResult& aRv);

  virtual JSObject* WrapObject(JSContext* aCx, JSObject* aScope,
			       bool* aTriedToWrap);

  nsISupports*
  GetParentObject()
  {
    return mGlobal;
  }

  TextTrack* GetTrack() const
  {
    return mTrack;
  }

  void GetId(nsAString& aId) const
  {
    aId = mId;
  }

  void SetId(const nsAString& aId)
  {
    mId = aId;
  }

  double StartTime() const
  {
    return mStartTime;
  }

  void SetStartTime(const double aStartTime)
  {
    //XXXhumph: validate?
    mStartTime = aStartTime;
  }

  double EndTime() const
  {
    return mEndTime;
  }

  void SetEndTime(const double aEndTime)
  {
    //XXXhumph: validate?
    mEndTime = aEndTime;
  }

  bool PauseOnExit()
  {
    return mPauseOnExit;
  }

  void SetPauseOnExit(const bool aPauseOnExit)
  {
    mPauseOnExit = aPauseOnExit;
  }

  void GetVertical(nsAString& aVertical)
  {
    aVertical = mVertical;
  }

  void SetVertical(const nsAString& aVertical)
  {
    mVertical = aVertical;
  }

  bool SnapToLines()
  {
    return mSnapToLines;
  }

  void SetSnapToLines(bool aSnapToLines)
  {
    mSnapToLines = aSnapToLines;
  }

  int32_t Position()
  {
    return mPosition;
  }

  void SetPosition(int32_t aPosition)
  {
    // XXXhumph: validate?
    mPosition = aPosition;
  }

  int32_t Size()
  {
    return mSize;
  }

  void SetSize(int32_t aSize)
  {
    // XXXhumph: validate?
    mSize = aSize;
  }

  void GetAlign(nsAString& aAlign)
  {
    aAlign = mAlign;
  }

  void SetAlign(const nsAString& aAlign)
  {
    // XXXhumph: validate?
    mAlign = aAlign;
  }

  void GetText(nsAString& aText)
  {
    aText = mText;
  }

  void SetText(const nsAString& aText)
  {
    // XXXhumph: validate?
    mText = aText;
  }

  DocumentFragment* GetCueAsHTML()
  {
    // XXXhumph: todo
    return nullptr;
  }

  IMPL_EVENT_HANDLER(enter)
  IMPL_EVENT_HANDLER(exit)

private:
  nsCOMPtr<nsISupports> mGlobal;

  nsRefPtr<TextTrack> mTrack;
  nsString mId;
  double mStartTime;
  double mEndTime;
  bool mPauseOnExit;
  nsString mVertical;
  bool mSnapToLines;
  int32_t mPosition;
  int32_t mSize;
  nsString mAlign;
  nsString mText;
};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_TextTrackCue_h
