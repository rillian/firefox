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
  Constructor(GlobalObject& aGlobal,
	      const double aStartTime,
	      const double aEndTime,
	      const nsAString& aText,
	      ErrorResult& aRv)
  {
    nsRefPtr<TextTrackCue> ttcue = new TextTrackCue(aGlobal.Get(), aStartTime,
                                                    aEndTime, aText);
    return ttcue.forget();
  }

  TextTrackCue(nsISupports* aGlobal,  const double aStartTime,
               const double aEndTime, const nsAString& aText);
  ~TextTrackCue()
  {
  }

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
    if (mId == aId) {
      return;
    }

    mId = aId;
    CueChanged();
  }

  double StartTime() const
  {
    return mStartTime;
  }

  void SetStartTime(const double aStartTime)
  {
    //XXXhumph: validate?
    if (mStartTime == aStartTime)
      return;

    mStartTime = aStartTime;
    CueChanged();
  }

  double EndTime() const
  {
    return mEndTime;
  }

  void SetEndTime(const double aEndTime)
  {
    //XXXhumph: validate?
    if (mEndTime == aEndTime)
      return;

    mEndTime = aEndTime;
    CueChanged();
  }

  bool PauseOnExit()
  {
    return mPauseOnExit;
  }

  void SetPauseOnExit(const bool aPauseOnExit)
  {
    if (mPauseOnExit == aPauseOnExit)
      return;

    mPauseOnExit = aPauseOnExit;
    CueChanged();
  }

  void GetVertical(nsAString& aVertical)
  {
    aVertical = mVertical;
  }

  void SetVertical(const nsAString& aVertical)
  {
    if (mVertical == aVertical)
      return;

    mVertical = aVertical;
    CueChanged();
  }

  bool SnapToLines()
  {
    return mSnapToLines;
  }

  void SetSnapToLines(bool aSnapToLines)
  {
    if (mSnapToLines == aSnapToLines)
      return;

    mSnapToLines = aSnapToLines;
    CueChanged();
  }

  double Line()
  {
    return mLine;
  }

  void SetLine(double value)
  {
    //XXX: validate?
    mLine = value;
  }

  int32_t Position()
  {
    return mPosition;
  }

  void SetPosition(int32_t aPosition)
  {
    // XXXhumph: validate?
    if (mPosition == aPosition)
      return;

    mPosition = aPosition;
    CueChanged();
  }

  int32_t Size()
  {
    return mSize;
  }

  void SetSize(int32_t aSize)
  {
    if (mSize == aSize) {
      return;
    }

    if (aSize < 0 || aSize > 100) {
      //XXX:throw IndexSizeError;
    }

    mSize = aSize;
    CueChanged();
  }

  TextTrackCueAlign Align()
  {
    return mAlign;
  }

  void SetAlign(TextTrackCueAlign& aAlign)
  {
    mAlign = aAlign;
    CueChanged();
  }

  void GetText(nsAString& aText)
  {
    aText = mText;
  }

  void SetText(const nsAString& aText)
  {
    // XXXhumph: validate?
    if (mText == aText)
      return;

    mText = aText;
    CueChanged();
  }

  DocumentFragment* GetCueAsHTML()
  {
    // XXXhumph: todo
    return nullptr;
  }

  bool
  operator==(const TextTrackCue& rhs) const
  {
    return (mId.Equals(rhs.mId));
  }


  IMPL_EVENT_HANDLER(enter)
  IMPL_EVENT_HANDLER(exit)

private:
  void CueChanged();

  nsCOMPtr<nsISupports> mGlobal;

  nsRefPtr<TextTrack> mTrack;
  nsString mId;
  double mStartTime;
  double mEndTime;
  bool mPauseOnExit;
  nsString mVertical;
  bool mSnapToLines;
  double mLine;
  int32_t mPosition;
  int32_t mSize;
  TextTrackCueAlign mAlign;
  nsString mText;
};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_TextTrackCue_h
