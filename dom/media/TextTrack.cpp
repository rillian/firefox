/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 et tw=78: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "TextTrack.h"
#include "mozilla/dom/TextTrackBinding.h"

namespace mozilla {
namespace dom {

NS_IMPL_CYCLE_COLLECTION_UNLINK_BEGIN(TextTrack)
  NS_IMPL_CYCLE_COLLECTION_UNLINK_PRESERVED_WRAPPER
NS_IMPL_CYCLE_COLLECTION_UNLINK_END

NS_IMPL_CYCLE_COLLECTION_TRAVERSE_BEGIN(TextTrack)
  NS_IMPL_CYCLE_COLLECTION_TRAVERSE(mParent)
  NS_IMPL_CYCLE_COLLECTION_TRAVERSE_SCRIPT_OBJECTS
NS_IMPL_CYCLE_COLLECTION_TRAVERSE_END

NS_IMPL_CYCLE_COLLECTION_TRACE_BEGIN(TextTrack)
  NS_IMPL_CYCLE_COLLECTION_TRACE_PRESERVED_WRAPPER
NS_IMPL_CYCLE_COLLECTION_TRACE_END

NS_IMPL_CYCLE_COLLECTING_ADDREF(TextTrack)
NS_IMPL_CYCLE_COLLECTING_RELEASE(TextTrack)

NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(TextTrack)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

TextTrack::TextTrack(nsISupports *aParent,
    const nsAString& aKind)
    const nsAString& aLabel,
    const nsAString& aLanguage) :
  mParent(aParent),
  mKind(aKind),
  mLabel(aLabel),
  mLanguage(aLanguage)
  //XXX:mMode(WEBVTT_MODE_HIDDEN),
  //XXX:mCueList(new cue list goes here),
  //XXX:mActiveCueList(new active cue list)
{
  //XXX: any asserts done here?
  //XXX: dom spec says to set
  //certain strings as default
  //if they are empty. label
  //and language are optional
  SetIsDOMBinding();
  //XXX: populate both cue lists
}

TextTrack::~TextTrack()
{
  mParent = nullptr;
  //XXX: anything else need to be destroyed?
}

JSObject*
TextTrack::WrapObject(JSContext* aCx, JSObject* aScope,
		      bool* aTriedToWrap)
{
  return TextTrackBinding::Wrap(aCx, aScope, this, aTriedToWrap);
}

nsISupports*
TextTrack::GetParentObject()
{
  return mParent;
}

void
TextTrack::GetKind(nsAString& aKind)
{
  aKind = mKind;
}

void
TextTrack::GetLabel(nsAString& aLabel)
{
  aLabel = mLabel;
}

void
TextTrack::GetLanguage(nsAString& aLanguage)
{
  aLanguage = mLanguage;
}

void
TextTrack::GetInBandMetadataTrackDispatchType(nsAString& aType)
{
  aType = mType;
}

TextTrackCueList*
TextTrack::GetCues()
{
  return mCueList;
}

TextTrackCueList*
TextTrack::GetActiveCues()
{
  return mActiveCueList;
}

void
TextTrack::AddCue(TextTrackCue& cue)
{
  //XXX: if cue exists, remove
  //XXX: add cue
  //mCueList->addCue(cue);
  
}

void
TextTrack::RemoveCue(TextTrackCue& cue)
{
  //XXX: if cue does not exists throw
  //a NotFoundError exception
  //XXX: remove cue
  //mCueList->removeCue(cue);
}

void
TextTrack::CueChanged(TextTrackCue& cue)
{
  //XXX: cue changed handling
}

} // namespace dom
} // namespace mozilla
