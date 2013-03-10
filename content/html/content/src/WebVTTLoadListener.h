/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_WebVTTLoadListener_h
#define mozilla_dom_WebVTTLoadListener_h

#include "HTMLTrackElement.h"
#include "webvtt/parser.h"
#include "webvtt/util.h"
#include "nsAutoRef.h"

namespace mozilla {
namespace dom {

class WEBVTTLoadListener MOZ_FINAL : public nsIStreamListener,
                                     public nsIChannelEventSink,
                                     public nsIInterfaceRequestor,
                                     public nsIObserver
{
  NS_DECL_ISUPPORTS
  NS_DECL_NSIREQUESTOBSERVER
  NS_DECL_NSISTREAMLISTENER
  NS_DECL_NSICHANNELEVENTSINK
  NS_DECL_NSIOBSERVER
  NS_DECL_NSIINTERFACEREQUESTOR

public:
  WebVTTLoadListener(HTMLTrackElement *aElement);
  ~WebVTTLoadListener();
  nsresult LoadResource();
  NS_IMETHODIMP Observe(nsISupports* aSubject, const char *aTopic,
                        const PRUnichar* aData);
  NS_IMETHODIMP OnStartRequest(nsIRequest* aRequest, nsISupports* aContext);
  NS_IMETHODIMP OnStopRequest(nsIRequest* aRequest, nsISupports* aContext,
                              nsresult aStatus);
  NS_IMETHODIMP OnDataAvailable(nsIRequest* aRequest, nsISupports *aContext,
                                nsIInputStream* aStream, uint64_t aOffset,
                                uint32_t aCount);
  NS_IMETHODIMP AsyncOnChannelRedirect(nsIChannel* aOldChannel, 
                                       nsIChannel* aNewChannel,
                                       uint32_t aFlags,
                                       nsIAsyncVerifyRedirectCallback* cb);
  NS_IMETHODIMP GetInterface(const nsIID &aIID, void **aResult);

private:
  NS_METHOD ParseChunk(nsIInputStream *aInStream, void *aClosure,
                     const char *aFromSegment, uint32_t aToOffset,
                     uint32_t aCount, uint32_t *aWriteCount);
  void OnParsedCue(webvtt_cue *cue);
  void OnReportError(uint32_t line, uint32_t col, webvtt_error error);
  TextTrackCue ConvertCueToTextTrackCue(const webvtt_cue *aCue);
  already_AddRefed<DocumentFragment> 
    ConvertNodeListToDocFragment(const webvtt_node *aNode, ErrorResult &rv);
  nsISupports ConvertNodeToCueTextContent(const webvtt_node *aNode);

  nsRefPtr<HTMLTrackElement> mElement;
  nsCOMPtr<nsIStreamListener> mNextListener;
  uint32_t mLoadID;
  nsAutoRef<webvtt_parser> mParser;
};

static void WEBVTT_CALLBACK OnParsedCueWebVTTCallBack(void *aUserData, 
                                                      webvtt_cue *aCue);
static int WEBVTT_CALLBACK OnReportErrorWebVTTCallBack(void *aUserData, 
                                                       uint32_t aLine, 
                                                       uint32_t aCol, 
                                                       webvtt_error aError);

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_WebVTTLoadListener_h