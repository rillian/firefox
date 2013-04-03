/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 et tw=78: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "TextTrack.h"
#include "mozilla/dom/TextTrackBinding.h"

namespace mozilla {
namespace dom {

NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE_1(TextTrack, mParent)

NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION_INHERITED(TextTrack)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END_INHERITING(nsDOMEventTargetHelper)

NS_IMPL_ADDREF_INHERITED(TextTrack, nsDOMEventTargetHelper)
NS_IMPL_RELEASE_INHERITED(TextTrack, nsDOMEventTargetHelper)

TextTrack::TextTrack(nsISupports* aParent,
                     const nsAString& aKind,
                     const nsAString& aLabel,
                     const nsAString& aLanguage)
  : mParent(aParent)
  , mKind(aKind)
  , mLabel(aLabel)
  , mLanguage(aLanguage)
  , mType()
  , mMode(TextTrackMode::Hidden)
{
  mCueList = new TextTrackCueList(aParent);
  mActiveCueList = new TextTrackCueList(aParent);

  SetIsDOMBinding();
}

TextTrack::TextTrack(nsISupports* aParent)
  : mParent(aParent)
  , mKind(NS_LITERAL_STRING("subtitles"))
  , mLabel(EmptyString())
  , mLanguage(EmptyString())
  , mType()
  , mMode(TextTrackMode::Disabled)
{
  mCueList = new TextTrackCueList(aParent);
  mActiveCueList = new TextTrackCueList(aParent);

  SetIsDOMBinding();
}

TextTrack::~TextTrack()
{
}

void
TextTrack::Update(double time)
{
fprintf(stderr, "\nTextTrack::Update()\n");
  mCueList->Update(time);
}

JSObject*
TextTrack::WrapObject(JSContext* aCx, JSObject* aScope)
{
  return TextTrackBinding::Wrap(aCx, aScope, this);
}

void
TextTrack::SetMode(TextTrackMode aValue)
{
  mMode = aValue;
}

void
TextTrack::AddCue(TextTrackCue& aCue)
{
  //XXX: if cue exists, remove
  mCueList->AddCue(aCue);
}

void
TextTrack::RemoveCue(TextTrackCue& aCue)
{
  //XXX: if cue does not exists throw
  //a NotFoundError exception
  mCueList->RemoveCue(aCue);
}

void
TextTrack::CueChanged(TextTrackCue& aCue)
{
  //XXX: cue changed handling
}

} // namespace dom
} // namespace mozilla
