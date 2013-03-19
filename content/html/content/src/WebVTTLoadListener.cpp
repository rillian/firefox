/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/HTMLMediaElement.h"
#include "nsVideoFrame.h"
#include "nsIFrame.h"
#include "MediaDecoder.h"
#include "WebVTTLoadListener.h"
#include "mozilla/dom/TextTrack.h"
#include "mozilla/dom/TextTrackCue.h"
#include "mozilla/dom/TextTrackCueList.h"
#include "webvtt/string.h"

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

  if (!MediaDecoder::IsWebVTTEnabled()) {
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
  
  // How to determine if this is the final chunk?
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

fprintf(stderr, "WebVTTLoadListener::onParsedCue aCue->from=%llu, aCue->until=%llu\n", aCue->from, aCue->until);

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
WebVTTLoadListener::DisplayCueText(webvtt_node* head) {
fprintf(stderr, "in WebVTTLoadListener::DisplayCueText, should get AppendChild below...\n");
  ErrorResult rv;
  already_AddRefed<DocumentFragment> frag = ConvertNodeListToDocFragment(head, rv);
  if (!frag.get() || rv.Failed()) {
    // TODO: Do something with rv.ErrorCode here.
fprintf(stderr, "WebVTTLoadListener::DisplayCueText failed!!!!!!!!!!!!!!");
  }

  HTMLMediaElement* parent =
      static_cast<HTMLMediaElement*>(mElement->mMediaParent.get());

  nsIFrame* frame = parent->GetPrimaryFrame();
  if (frame && frame->GetType() == nsGkAtoms::HTMLVideoFrame) {

    nsIContent *overlay =
      static_cast<nsVideoFrame*>(frame)->GetCaptionOverlay();
    nsCOMPtr<nsIDOMNode> div = do_QueryInterface(overlay);

    if (div) {
      nsCOMPtr<nsIDOMNode> resultNode;
      // TODO: Might need to remove previous children first
fprintf(stderr, "div->AppendChild()\n");

      nsCOMPtr<nsIContent> content = do_QueryInterface(div);
      uint32_t childCount = content->GetChildCount();
      for (uint32_t i = 0; i < childCount; ++i) {
        content->RemoveChildAt(i, true);
      }

      div->AppendChild(frag.get(), getter_AddRefs(resultNode));
    }
  }
fprintf(stderr, "Done DisplayCueText\n");
}

void 
WebVTTLoadListener::OnReportError(uint32_t aLine, uint32_t aCol, 
                                  webvtt_error aError)
{
  // TODO: Handle error here, been suggessted that we just use PR_LOGGING
fprintf(stderr, "\nOnReportError aLine=%d aCol=%d aError=%d\n", aLine, aCol, aError);
}

/**
TextTrackCue
WebVTTLoadListener::ConvertCueToTextTrackCue(const webvtt_cue *aCue)
{
  const char* text = reinterpret_cast<const char *>(webvtt_string_text(&aCue->id));

fprintf(stderr, "WebVTTLoadListener::ConvertCueToTextTrackCue aCue->from=%llu, aCue->until=%llu\n", aCue->from, aCue->until);

  TextTrackCue textTrackCue(mElement->OwnerDoc()->GetParentObject(),
                            (double)(aCue->from/1000), (double)(aCue->until/1000),
                            NS_ConvertUTF8toUTF16(text), mElement,
                            aCue->node_head);
  
  textTrackCue.SetSnapToLines(aCue->snap_to_lines);
  textTrackCue.SetSize(aCue->settings.size);
  textTrackCue.SetPosition(aCue->settings.position);
  
  //TODO: need to convert webvtt enums to strings
  // textTrackCue.SetVertical();
  // textTrackCue.SetAlign();

  // TODO: Id is the text in the parser so do we need this?
  // textTrackCue.SetId();

  // TODO: Not specified in webvtt so we may not need this.
  // textTrackCue.SetPauseOnExit();

  return textTrackCue;
}
**/

already_AddRefed<DocumentFragment>
WebVTTLoadListener::ConvertNodeListToDocFragment(const webvtt_node *aNode, 
                                                 ErrorResult &rv)
{
  // TODO: Do we need to do something with this error result?
  already_AddRefed<DocumentFragment> frag =
    mElement->OwnerDoc()->CreateDocumentFragment(rv);
  if (!frag.get()) {
    return nullptr;
  }

  nsCOMPtr<nsIDOMNode> resultNode;
  for (webvtt_uint i = 0; i < aNode->data.internal_data->length; i++) {
    
    nsCOMPtr<nsIContent> cueTextContent = 
      ConvertNodeToCueTextContent(aNode->data.internal_data->children[i]);
    
    nsCOMPtr<nsIDOMNode> node = do_QueryInterface(cueTextContent);
    
    frag.get()->AppendChild(node, getter_AddRefs(resultNode));
  }

  return frag;
}

// TODO: Change to iterative solution instead of recursive
nsCOMPtr<nsIContent>
WebVTTLoadListener::ConvertNodeToCueTextContent(const webvtt_node *aWebVttNode)
{
  nsCOMPtr<nsIContent> cueTextContent;
  nsINodeInfo* nodeInfo;
  
  if (WEBVTT_IS_VALID_INTERNAL_NODE(aWebVttNode->kind))
  {   
    nodeInfo = mElement->NodeInfo();
    NS_NewHTMLElement(getter_AddRefs(cueTextContent), nodeInfo, mozilla::dom::NOT_FROM_PARSER);
    
    nsAString *qualifiedName = nullptr;
    switch (aWebVttNode->kind) {
      case WEBVTT_CLASS:
        *qualifiedName = NS_LITERAL_STRING("span");
        break;
      case WEBVTT_ITALIC:
        *qualifiedName = NS_LITERAL_STRING("i");
        break;
      case WEBVTT_BOLD:
        *qualifiedName = NS_LITERAL_STRING("b");
        break;
      case WEBVTT_UNDERLINE:
        *qualifiedName = NS_LITERAL_STRING("u");
        break;
      case WEBVTT_RUBY:
        *qualifiedName = NS_LITERAL_STRING("ruby");
        break;
      case WEBVTT_RUBY_TEXT:
        *qualifiedName = NS_LITERAL_STRING("rt");
        break;
      case WEBVTT_VOICE: 
      {
        *qualifiedName = NS_LITERAL_STRING("span");
        
        nsCOMPtr<nsGenericHTMLElement> htmlElement = 
          do_QueryInterface(cueTextContent);
        
        const char* text = 
          reinterpret_cast<const char *>(
            webvtt_string_text(&aWebVttNode->data.internal_data->annotation));
        
          htmlElement->SetTitle(NS_ConvertUTF8toUTF16(text));
        break;
      }
      default:
        // Nothing for now
        break;
    }

    nsCOMPtr<nsIDOMHTMLElement> htmlElement = do_QueryInterface(cueTextContent);
    
    // TODO:: Need to concatenate all applicable classes separated by spaces and
    //        set them to the htmlElements class attribute

    htmlElement->SetAttributeNS(NS_LITERAL_STRING("html"), *qualifiedName, 
                                NS_LITERAL_STRING(""));

    for (int i = 0; i < aWebVttNode->data.internal_data->length; i++) {
       nsCOMPtr<nsIDOMNode> resultNode, childNode;
       nsCOMPtr<nsIContent> childCueTextContent;
       
       childCueTextContent = ConvertNodeToCueTextContent(
        aWebVttNode->data.internal_data->children[i]);
       
       childNode = do_QueryInterface(childCueTextContent);
       htmlElement->AppendChild(childNode, getter_AddRefs(resultNode));
    }
  }
  else if (WEBVTT_IS_VALID_LEAF_NODE(aWebVttNode->kind))
  {
    switch (aWebVttNode->kind) {
      case WEBVTT_TEXT:
      {
        nodeInfo = mElement->NodeInfo();
        NS_NewTextNode(getter_AddRefs(cueTextContent), nodeInfo->NodeInfoManager());

        if (!cueTextContent) {
          return nullptr;
        }
        {
          const char* text = reinterpret_cast<const char *>(
            webvtt_string_text(&aWebVttNode->data.text));

          cueTextContent->SetText(NS_ConvertUTF8toUTF16(text), false);
        }
        break;
      }
      case WEBVTT_TIME_STAMP:
        // TODO: Need to create a "ProcessingInstruction?"
        break;
      default:
        // Nothing for now
        break;
    }
  }

  return cueTextContent;
}

static void WEBVTT_CALLBACK
OnParsedCueWebVTTCallBack(void *aUserData, webvtt_cue *aCue)
{
  WebVTTLoadListener *self = reinterpret_cast<WebVTTLoadListener *>(aUserData);
  self->OnParsedCue(aCue);
}

static int WEBVTT_CALLBACK 
OnReportErrorWebVTTCallBack(void *aUserData, uint32_t aLine, 
                            uint32_t aCol, webvtt_error aError)
{
  WebVTTLoadListener *self = reinterpret_cast<WebVTTLoadListener *>(aUserData);
  self->OnReportError(aLine, aCol, aError);
  return WEBVTT_SUCCESS;
}

} // namespace dom
} // namespace mozilla
