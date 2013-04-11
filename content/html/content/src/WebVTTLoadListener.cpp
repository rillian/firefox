/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "WebVTTLoadListener.h"
#include "mozilla/dom/HTMLTrackElement.h"
#include "mozilla/dom/TextTrack.h"
#include "mozilla/dom/TextTrackCue.h"
#include "mozilla/dom/TextTrackCueList.h"

namespace mozilla {
namespace dom {

#ifdef PR_LOGGING
PRLogModuleInfo* gTextTrackLog;
# define LOG(msg) PR_LOG(gTextTrackLog, PR_LOG_DEBUG, (msg))
#else
# define LOG(msg)
#endif

NS_IMPL_ISUPPORTS5(WebVTTLoadListener, nsIRequestObserver,
                   nsIStreamListener, nsIChannelEventSink,
                   nsIInterfaceRequestor, nsIObserver)

WebVTTLoadListener::WebVTTLoadListener(HTMLTrackElement *aElement)
  : mElement(aElement),
    mLoadID(aElement->GetCurrentLoadID())
{
  NS_ABORT_IF_FALSE(mElement, "Must pass an element to the callback");
#ifdef PR_LOGGING
  if (!gTextTrackLog) {
    gTextTrackLog = PR_NewLogModule("TextTrack");
  }
#endif
  LOG("WebVTTLoadListener created.");
}

WebVTTLoadListener::~WebVTTLoadListener()
{
  if (mParser) {
    mParser.reset();
  }
  LOG("WebVTTLoadListener destroyed.");
}

nsresult
WebVTTLoadListener::LoadResource()
{
  webvtt_parser_t *parser = 0;
  webvtt_status status;

  if (!HTMLTrackElement::IsWebVTTEnabled()) {
    NS_WARNING("WebVTT support disabled."
               " See media.webvtt.enabled in about:config. ");
    return NS_ERROR_FAILURE;
  }

  LOG("Loading text track resource.");
  status = webvtt_create_parser(&OnParsedCueWebVTTCallBack, 
                                &OnReportErrorWebVTTCallBack, 
                                this, &parser);

  if (status != WEBVTT_SUCCESS) {
    NS_ENSURE_TRUE(status == WEBVTT_OUT_OF_MEMORY,
                   NS_ERROR_OUT_OF_MEMORY);
    NS_ENSURE_TRUE(status == WEBVTT_INVALID_PARAM,
                   NS_ERROR_INVALID_ARG);
    return NS_ERROR_FAILURE;
  }

  NS_ENSURE_TRUE(parser != nullptr, NS_ERROR_FAILURE);

  mParser.own(parser);
  NS_ENSURE_TRUE(mParser != nullptr, NS_ERROR_FAILURE);

  return NS_OK;
}

NS_IMETHODIMP
WebVTTLoadListener::Observe(nsISupports* aSubject,
                            const char *aTopic,
                            const PRUnichar* aData)
{
  if (strcmp(aTopic, NS_XPCOM_SHUTDOWN_OBSERVER_ID) == 0) {
    nsContentUtils::UnregisterShutdownObserver(this);
  }
 
  // Clear mElement to break cycle so we don't leak on shutdown
  mElement = nullptr;
  return NS_OK;
}

NS_IMETHODIMP
WebVTTLoadListener::OnStartRequest(nsIRequest* aRequest,
                                   nsISupports* aContext)
{
  return NS_OK;
}

NS_IMETHODIMP
WebVTTLoadListener::OnStopRequest(nsIRequest* aRequest,
                                  nsISupports* aContext,
                                  nsresult aStatus)
{
  nsContentUtils::UnregisterShutdownObserver(this);
  return NS_OK;
}

NS_IMETHODIMP
WebVTTLoadListener::OnDataAvailable(nsIRequest* aRequest,
                                    nsISupports *aContext,
                                    nsIInputStream* aStream,
                                    uint64_t aOffset,
                                    uint32_t aCount)
{
  uint32_t count = aCount;
  while (count > 0) {
    uint32_t read;
    nsresult rv = aStream->ReadSegments(ParseChunk, this, count, &read);

    NS_ENSURE_SUCCESS(rv, rv);
    NS_ASSERTION(read > 0, "Read 0 bytes while data was available?" );

    count -= read;
  }

  return NS_OK;
}

NS_IMETHODIMP
WebVTTLoadListener::AsyncOnChannelRedirect(nsIChannel* aOldChannel,
                                           nsIChannel* aNewChannel,
                                           uint32_t aFlags,
                                           nsIAsyncVerifyRedirectCallback* cb)
{
  return NS_OK;
}

NS_IMETHODIMP
WebVTTLoadListener::GetInterface(const nsIID &aIID,
                                 void **aResult)
{
  return QueryInterface(aIID, aResult);
}

NS_METHOD 
WebVTTLoadListener::ParseChunk(nsIInputStream *aInStream, void *aClosure,
                               const char *aFromSegment, uint32_t aToOffset,
                               uint32_t aCount, uint32_t *aWriteCount)
{
  WebVTTLoadListener* loadListener = static_cast<WebVTTLoadListener *>(aClosure);
  
  if (!webvtt_parse_chunk(loadListener->mParser, aFromSegment, aCount)) {
    // TODO: Handle error
  }
  *aWriteCount = aCount;

  return NS_OK;
}

void
WebVTTLoadListener::OnParsedCue(webvtt_cue *aCue)
{
  const char* text = reinterpret_cast<const char *>(webvtt_string_text(&aCue->id));

  nsRefPtr<TextTrackCue> textTrackCue = new TextTrackCue(mElement->OwnerDoc()->GetParentObject(),
                                                         (double)(aCue->from/1000), (double)(aCue->until/1000),
                                                         NS_ConvertUTF8toUTF16(text), mElement,
                                                         aCue->node_head);

  textTrackCue->SetSnapToLines(aCue->snap_to_lines);
  textTrackCue->SetSize(aCue->settings.size);
  textTrackCue->SetPosition(aCue->settings.position);

  //TODO: need to convert webvtt enums to strings
  // textTrackCue.SetVertical();
  // textTrackCue.SetAlign();

  // TODO: Id is the text in the parser so do we need this?
  // textTrackCue.SetId();

  // TODO: Not specified in webvtt so we may not need this.
  // textTrackCue.SetPauseOnExit();

  mElement->mTrack->AddCue(*textTrackCue.get());
  textTrackCue.forget();
}

void 
WebVTTLoadListener::OnReportError(uint32_t aLine, uint32_t aCol, 
                                  webvtt_error aError)
{
  // TODO: Handle error here, been suggessted that we just use PR_LOGGING
  fprintf(stderr, "\nOnReportError aLine=%d aCol=%d aError=%d\n", aLine, aCol, aError);
}

void WEBVTT_CALLBACK
WebVTTLoadListener::OnParsedCueWebVTTCallBack(void *aUserData, webvtt_cue *aCue)
{
  WebVTTLoadListener *self = reinterpret_cast<WebVTTLoadListener *>(aUserData);
  self->OnParsedCue(aCue);
}

int WEBVTT_CALLBACK 
WebVTTLoadListener::OnReportErrorWebVTTCallBack(void *aUserData, uint32_t aLine, 
                            uint32_t aCol, webvtt_error aError)
{
  WebVTTLoadListener *self = reinterpret_cast<WebVTTLoadListener *>(aUserData);
  self->OnReportError(aLine, aCol, aError);
  return WEBVTT_SUCCESS;
}

} // namespace dom
} // namespace mozilla
