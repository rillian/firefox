/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "WebVTTLoadListener.h"
#include "TextTrack.h"
#include "TextTrackCue.h"
#include "TextTrackCueList.h"
#include "nsAutoRefTraits.h"
#include "webvtt/string.h"

// We might want to look into using #if defined(MOZ_WEBVTT)
class nsAutoRefTraits<webvtt_parser> : public nsPointerRefTraits<webvtt_parser>
{
public:
  static void Release(webvtt_parser aParser) { webvtt_delete_parser(aParser); }
};

namespace mozilla {
namespace dom {

NS_IMPL_ISUPPORTS5(WebVTTLoadListener, nsIRequestObserver,
                   nsIStreamListener, nsIChannelEventSink,
                   nsIInterfaceRequestor, nsIObserver)

WebVTTLoadListener(HTMLTrackElement *aElement)
  : mElement(aElement),
    mLoadID(aElement->GetCurrentLoadID())
{
  NS_ABORT_IF_FALSE(mElement, "Must pass an element to the callback");
}

WebVTTLoadListener::~WebVTTLoadListener()
{
  if (mParser) {
    mParser.reset();
  }
}

nsresult
WebVTTLoadListener::LoadResource()
{
  webvtt_parser *parser;
  webvtt_status status;

  status = webvtt_create_parser(&OnParsedCueWebVTTCallBack, 
                                &OnReportErrorWebVTTCallBack, 
                                this, parser);

  if (status != WEBVTT_SUCCESS) {
    NS_ENSURE_TRUE(status == WEBVTT_OUT_OF_MEMORY,
                   NS_ERROR_OUT_OF_MEMORY);
    NS_ENSURE_TRUE(status == WEBVTT_INVALID_ARGUMENT,
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
  // How to determine if this is the final chunk?
  if (!webvtt_parse_chunk(mParser, aFromSegment, aCount, 0)) {
    // TODO: Handle error
  }
  *aWriteCount = aCount;

  return NS_OK;
}

void 
WebVTTLoadListener::OnParsedCue(webvtt_cue *aCue) 
{
  TextTrackCue textTrackCue = ConvertCueToTextTrackCue(aCue);
  mElement.Track.AddCue(textTrackCue);

  ErrorResult rv;
  already_AddRefed<DocumentFragment> frag = ConvertNodeListToDomFragment(aCue, rv);
  if (!frag || rv.Failed()) {
    // TODO: Do something with rv.ErrorCode here.
  }

  nsHTMLMediaElement* parent =
      static_cast<nsHTMLMediaElement*>(mElement->mMediaParent.get());

  nsIFrame* frame = mElement->mMediaParent->GetPrimaryFrame();
  if (frame && frame->GetType() == nsGkAtoms::HTMLVideoFrame) {

    nsIContent *overlay = 
      static_cast<nsVideoFrame*>(frame)->GetCaptionOverlay();
    nsCOMPtr<nsIDOMNode> div = do_QueryInterface(overlay);

    if (div) {
      // TODO: Might need to remove previous children first
      div->appendChild(frag);
    }
  }
}

void 
WebVTTLoadListener::OnReportError(uint32_t aLine, uint32_t aCol, 
                                  webvtt_error aError)
{
  // TODO: Handle error here, been suggessted that we just use PR_LOGGING
}

TextTrackCue 
WebVTTLoadListener::ConvertCueToTextTrackCue(const webvtt_cue *aCue)
{
  // TODO: What to pass in for aGlobal?
  TextTrackCue textTrackCue(/* nsISupports *aGlobal here */, 
                            aCue->from, aCue->until,
                            webvtt_string_text(NS_ConvertUTF8toUTF16(aCue->id)));
  
  textTrackCue.SetSnapToLines(aCue->snap_to_lines);
  textTrackCue.SetSize(aCue->settings.size);
  textTrackCue.SetPosition(aCue->settings.position);
  
  //TODO: need to convert webvtt enums to strings
  textTrackCue.SetVertical();
  textTrackCue.SetAlign();

  // TODO: Id is the text in the parser so do we need this?
  textTrackCue.SetId();

  // TODO: Not specified in webvtt so we may not need this.
  textTrackCue.SetPauseOnExit();

  return textTrackCue;
}

already_AddRefed<DocumentFragment>
WebVTTLoadListener::ConvertNodeListToDocFragment(const webvtt_node *aNode, 
                                                 ErrorResult &rv)
{
  nsCOMPtr<nsIContent> content = do_QueryInterface(mElement);
  if (!content) {
    return nullptr;
  }

  // TODO: Do we need to do something with this error result?
  already_AddRefed<DocumentFragment> frag = content.CreateDocumentFragment(rv);
  if (!frag) {
    return nullptr;
  }

  for (int i = 0; i < aNode->data->internal_data.length; i++) {
    frag.appendChild(ConvertNodeToCueTextContent(
      aNode->data->internal_data.children[i]));
  }

  return frag;
}

nsISupports
WebVTTLoadListener::ConvertNodeToCueTextContent(const webvtt_node *aWebVttNode)
{
  already_AddRefed<nsINodeInfo> nodeInfo;
  nsCOMPtr<nsISupports> cueTextContent;
  
  if (WEBVTT_IS_VALID_INTERNAL_NODE(aWebVttNode->kind))
  {
    // TODO: Change to iterative solution instead of recursive
    nsAString htmlNamespace = NS_LITERAL_STRING("html");

    nsAString qualifiedName;
  
    // TODO: Is this the correct way to be passing in a node info? If we need an 
    //       objects node info, than whose? 
    HTMLElement htmlElement(nodeInfo);
    
    switch (aWebVttNode->kind) {
      case WEBVTT_CLASS:
        qualifiedName = NS_LITERAL_STRING("span");
        break;
      case WEBVTT_ITALIC:
        qualifiedName = NS_LITERAL_STRING("i");
        break;
      case WEBVTT_BOLD:
        qualifiedName = NS_LITERAL_STRING("b");
        break;
      case WEBVTT_UNDERLINE:
        qualifiedName = NS_LITERAL_STRING("u");
        break;
      case WEBVTT_RUBY:
        qualifiedName = NS_LITERAL_STRING("ruby");
        break;
      case WEBVTT_RUBY_TEXT:
        qualifiedName = NS_LITERAL_STRING("rt");
        break;
      case WEBVTT_VOICE:
        qualifiedName = NS_LITERAL_STRING("span");
        htmlElement.SetTitle(
          NS_ConvertUTF8toUTF16(
            webvtt_string_text(aWebVttNode->data.internal_data->annotation));
        break;
    }

    // TODO:: Need to concatenate all applicable classes separated by spaces and
    //        set them to the htmlElements class attribute

    htmlElement.SetAttributeNS(htmlNamespace, qualifiedName, 
                               NS_LITERAL_STRING(""));

    for (int i = 0; i < aWebVttNode->data.internal_data->length; i++) {
      htmlElement.appendChild(
        ConvertNodeToCueTextContent(aWebVttNode->data.internal_data->children[i]);
    }
  }
  else if (WEBVTT_IS_VALID_LEAF_NODE(aWebVttNode->kind))
  {
    switch (aWebVttNode->kind) {
      case WEBVTT_TEXT:
        nsCOMPtr<nsIContent> content;
        
        NS_NewTextNode(content, mElement->GetNodeInfoManager());
        
        if (!content) {
          return nullptr;
        }
        content->SetText(NS_ConvertUTF8toUTF16(aWebVttNode->data.text), false);
        
        cueTextContent = do_QueryInterface(content);
        break;
      case WEBVTT_TIME_STAMP:
        // TODO: Need to create a "ProcessingInstruction?"
        break;
    }
  }

  return cueTextContent;
}

static void WEBVTT_CALLBACK
OnParsedCueWebVTTCallBack(void *aUserData, webvtt_cue *aCue)
{
  WebVTTLoadListener *self = reinterpret_cast<WebVTTLoadListener *>(userdata);
  self->OnParsedCue(aCue);
}

static int WEBVTT_CALLBACK 
OnReportErrorWebVTTCallBack(void *aUserData, uint32_t aLine, 
                            uint32_t aCol, webvtt_error aError)
{
  WebVTTLoadListener *self = reinterpret_cast<WebVTTLoadListener *>(userdata);
  self->OnReportError(aLine, aCol, aError);
}

} // namespace dom
} // namespace mozilla
