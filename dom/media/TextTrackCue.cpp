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

TextTrackCue::TextTrackCue(nsISupports *aGlobal,
                           const double aStartTime,
	                         const double aEndTime,
	                         const nsAString& aText)
  : mGlobal(aGlobal),
    mText(aText),
    mStartTime(aStartTime),
    mEndTime(aEndTime),
    mPosition(50),
    mSize(100),
    mPauseOnExit(false),
    mSnapToLines(true)
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

nsISupports*
TextTrackCue::GetParentObject()
{
  return mGlobal;
}

TextTrack*
TextTrackCue::GetTrack() const
{
  return mTrack;
}

void
TextTrackCue::GetId(nsAString& aId) const
{
  aId = mId;
}

void
TextTrackCue::SetId(const nsAString& aId)
{
  if (mId == aId)
    return;

  mId = aId;
  CueChanged();
}

double
TextTrackCue::StartTime() const
{
  return mStartTime;
}

void
TextTrackCue::SetStartTime(const double aStartTime)
{
  //XXXhumph: validate?
  if (mStartTime == aStartTime)
    return;

  mStartTime = aStartTime;
  CueChanged();
}

double
TextTrackCue::EndTime() const
{
  return mEndTime;
}

void
TextTrackCue::SetEndTime(const double aEndTime)
{
  //XXXhumph: validate?
 if (mEndTime == aEndTime)
   return;

  mEndTime = aEndTime;
  CueChanged();
}

bool
TextTrackCue::PauseOnExit()
{
  return mPauseOnExit;
}

void
TextTrackCue::SetPauseOnExit(const bool aPauseOnExit)
{
  if (mPauseOnExit == aPauseOnExit)
    return;

  mPauseOnExit = aPauseOnExit;
  CueChanged();
}

void
TextTrackCue::GetVertical(nsAString& aVertical)
{
  aVertical = mVertical;
}

void
TextTrackCue::SetVertical(const nsAString& aVertical)
{
  if (mVertical == aVertical)
    return;

  mVertical = aVertical;
  CueChanged();
}

bool
TextTrackCue::SnapToLines()
{
  return mSnapToLines;
}

void
TextTrackCue::SetSnapToLines(bool aSnapToLines)
{
  if (mSnapToLines == aSnapToLines)
    return;

  mSnapToLines = aSnapToLines;
  CueChanged();
}

int32_t
TextTrackCue::Position()
{
  return mPosition;
}

void
TextTrackCue::SetPosition(int32_t aPosition)
{
  // XXXhumph: validate?
  if (mPosition == aPosition)
    return;

  mPosition = aPosition;
  CueChanged();
}

int32_t
TextTrackCue::Size()
{
  return mSize;
}

void
TextTrackCue::SetSize(int32_t aSize)
{
  // XXXhumph: validate?
  if (mSize == aSize)
    return;

  mSize = aSize;
  CueChanged();
}

void
TextTrackCue::GetAlign(nsAString& aAlign)
{
  aAlign = mAlign;
}

void
TextTrackCue::SetAlign(const nsAString& aAlign)
{
  // XXXhumph: validate?
  if (mAlign == aAlign)
    return;

  mAlign = aAlign;
  CueChanged();
}

void
TextTrackCue::GetText(nsAString& aText)
{
  aText = mText;
}

void
TextTrackCue::SetText(const nsAString& aText)
{
  // XXXhumph: validate?
  if (mText == aText)
    return;

  mText = aText;
  CueChanged();
}

DocumentFragment*
TextTrackCue::GetCueAsHTML()
{
  // XXXhumph: todo
  return nullptr;
}

} // namespace dom
} // namespace mozilla
