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

TextTrack::TextTrack(nsISupports *aParent) : mParent(aParent)
{
  SetIsDOMBinding();
}

TextTrack::~TextTrack()
{
  mParent = nullptr;
}

JSObject*
TextTrack::WrapObject(JSContext* aCx, JSObject* aScope,
		      bool* aTriedToWrap)
{
  return TextTrackBinding::Wrap(aCx, aScope, this, aTriedToWrap);
}

} // namespace dom
} // namespace mozilla
