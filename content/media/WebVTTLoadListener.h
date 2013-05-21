/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_WebVTTLoadListener_h
#define mozilla_dom_WebVTTLoadListener_h

#include "nsIStreamListener.h"
#include "nsIChannelEventSink.h"
#include "nsIInterfaceRequestor.h"
#include "nsIObserver.h"
#include "nsAutoPtr.h"
#include "nsAutoRef.h"
#include "nsCycleCollectionParticipant.h"
#include "mozilla/dom/HTMLTrackElement.h"
#include "webvtt/parser.h"
#include "webvtt/util.h"

template <>
class nsAutoRefTraits<webvtt_parser_t> : public nsPointerRefTraits<webvtt_parser_t>
{
public:
  static void Release(webvtt_parser_t* aParser) { webvtt_delete_parser(aParser); }
};

namespace mozilla {
namespace dom {

class WebVTTLoadListener MOZ_FINAL : public nsIStreamListener,
                                     public nsIChannelEventSink,
                                     public nsIInterfaceRequestor
{
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_NSIREQUESTOBSERVER
  NS_DECL_NSISTREAMLISTENER
  NS_DECL_NSICHANNELEVENTSINK
  NS_DECL_NSIINTERFACEREQUESTOR

  NS_DECL_CYCLE_COLLECTION_CLASS_AMBIGUOUS(WebVTTLoadListener,
                                           nsIStreamListener)

public:
  WebVTTLoadListener(HTMLTrackElement* aElement);
  ~WebVTTLoadListener();
  void OnParsedCue(webvtt_cue* aCue);
  void OnReportError(uint32_t aLine, uint32_t aCol, webvtt_error aError);
  nsresult LoadResource();

private:
  static NS_METHOD ParseChunk(nsIInputStream* aInStream, void* aClosure,
                              const char* aFromSegment, uint32_t aToOffset,
                              uint32_t aCount, uint32_t* aWriteCount);

  nsRefPtr<HTMLTrackElement> mElement;
  nsAutoRef<webvtt_parser_t> mParser;

  static void WEBVTT_CALLBACK OnParsedCueWebVTTCallBack(void* aUserData,
                                                        webvtt_cue* aCue);
  static int WEBVTT_CALLBACK OnReportErrorWebVTTCallBack(void* aUserData,
                                                         uint32_t aLine,
                                                         uint32_t aCol,
                                                         webvtt_error aError);
};



} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_WebVTTLoadListener_h
