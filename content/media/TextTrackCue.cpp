/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "TextTrackCue.h"
#include "mozilla/dom/TextTrackCueBinding.h"
#include "mozilla/dom/HTMLMediaElement.h"
#include "nsIFrame.h"
#include "nsVideoFrame.h"

namespace mozilla {
namespace dom {

NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE_1(TextTrackCue, mGlobal)

NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION_INHERITED(TextTrackCue)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END_INHERITING(nsDOMEventTargetHelper)

NS_IMPL_ADDREF_INHERITED(TextTrackCue, nsDOMEventTargetHelper)
NS_IMPL_RELEASE_INHERITED(TextTrackCue, nsDOMEventTargetHelper)

TextTrackCue::TextTrackCue(nsISupports* aGlobal,
                           const double aStartTime,
                           const double aEndTime,
                           const nsAString& aText)
  : mGlobal(aGlobal)
  , mText(aText)
  , mStartTime(aStartTime)
  , mEndTime(aEndTime)
  , mPosition(50)
  , mSize(100)
  , mPauseOnExit(false)
  , mSnapToLines(true)
{
  MOZ_ASSERT(aGlobal);
  SetIsDOMBinding();
}

TextTrackCue::TextTrackCue(nsISupports* aGlobal,
                           const double aStartTime,
                           const double aEndTime,
                           const nsAString& aText,
                           HTMLTrackElement* aTrackElement,
                           webvtt_node* head)
  : mGlobal(aGlobal)
  , mText(aText)
  , mStartTime(aStartTime)
  , mEndTime(aEndTime)
  , mTrackElement(aTrackElement)
  , mHead(head)
  , mPosition(50)
  , mSize(100)
  , mPauseOnExit(false)
  , mSnapToLines(true)
{
  MOZ_ASSERT(aGlobal);
  SetIsDOMBinding();
}

void
TextTrackCue::RenderCue()
{  
  ErrorResult rv;
  nsRefPtr<DocumentFragment> frag = GetCueAsHTML();
  if (!frag.get() || rv.Failed()) {
    // TODO: Do something with rv.ErrorCode here.
  }

  HTMLMediaElement* parent =
      static_cast<HTMLMediaElement*>(mTrackElement->mMediaParent.get());

  nsIFrame* frame = parent->GetPrimaryFrame();
  if (frame && frame->GetType() == nsGkAtoms::HTMLVideoFrame) {

    nsIContent *overlay =
      static_cast<nsVideoFrame*>(frame)->GetCaptionOverlay();
    nsCOMPtr<nsIDOMNode> div = do_QueryInterface(overlay);

    if (div) {
      nsCOMPtr<nsIDOMNode> resultNode;

      nsCOMPtr<nsIContent> content = do_QueryInterface(div);
      uint32_t childCount = content->GetChildCount();
      for (uint32_t i = 0; i < childCount; ++i) {
        content->RemoveChildAt(i, true);
      }

      div->AppendChild(frag.get(), getter_AddRefs(resultNode));
    }
  }
}
  
already_AddRefed<DocumentFragment>
TextTrackCue::GetCueAsHTML()
{
  ErrorResult rv;
  
  // TODO: Do we need to do something with this error result?
  // TODO: CHANGE ALL ADDREFED TO nsRefPtr
  nsRefPtr<DocumentFragment> frag =
    mTrackElement->OwnerDoc()->CreateDocumentFragment(rv);
    
  // TODO: Should this happen?  
  if (!frag.get()) {
    return nullptr;
  }

  nsCOMPtr<nsIDOMNode> resultNode;
  for (webvtt_uint i = 0; i < mHead->data.internal_data->length; i++) {
    
    nsCOMPtr<nsIContent> cueTextContent = 
      ConvertNodeToCueTextContent(mHead->data.internal_data->children[i]);
    
    nsCOMPtr<nsIDOMNode> node = do_QueryInterface(cueTextContent);
    
    frag.get()->AppendChild(node, getter_AddRefs(resultNode));
  }

  return frag.forget();
}

// TODO: Change to iterative solution instead of recursive
nsCOMPtr<nsIContent>
TextTrackCue::ConvertNodeToCueTextContent(const webvtt_node *aWebVTTNode)
{
  nsCOMPtr<nsIContent> cueTextContent;
  nsINodeInfo* nodeInfo;
  
  if (WEBVTT_IS_VALID_INTERNAL_NODE(aWebVTTNode->kind))
  {   
    nodeInfo = mTrackElement->NodeInfo();
    NS_NewHTMLElement(getter_AddRefs(cueTextContent), nodeInfo, mozilla::dom::NOT_FROM_PARSER);
    
    nsAutoString qualifiedName;
    switch (aWebVTTNode->kind) {
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
      {
        qualifiedName = NS_LITERAL_STRING("span");
        
        nsCOMPtr<nsGenericHTMLElement> htmlElement = 
          do_QueryInterface(cueTextContent);
        
        const char* text = 
          reinterpret_cast<const char *>(
            webvtt_string_text(&aWebVTTNode->data.internal_data->annotation));
        
          htmlElement->SetTitle(NS_ConvertUTF8toUTF16(text));
        break;
      }
      default:
        // TODO: What happens here?
        break;
    }

    nsCOMPtr<nsIDOMHTMLElement> htmlElement = do_QueryInterface(cueTextContent);
    
    // TODO:: Need to concatenate all applicable classes separated by spaces and
    //        set them to the htmlElements class attribute
    
    htmlElement->SetAttributeNS(NS_LITERAL_STRING("html"), qualifiedName, 
                                EmptyString());

    for (webvtt_uint i = 0; i < aWebVTTNode->data.internal_data->length; i++) {
       nsCOMPtr<nsIDOMNode> resultNode, childNode;
       nsCOMPtr<nsIContent> childCueTextContent;
       
       childCueTextContent = ConvertNodeToCueTextContent(
        aWebVTTNode->data.internal_data->children[i]);
       
       childNode = do_QueryInterface(childCueTextContent);
       htmlElement->AppendChild(childNode, getter_AddRefs(resultNode));
    }
  }
  else if (WEBVTT_IS_VALID_LEAF_NODE(aWebVTTNode->kind))
  {
    switch (aWebVTTNode->kind) {
      case WEBVTT_TEXT:
      {
        nodeInfo = mTrackElement->NodeInfo();
        NS_NewTextNode(getter_AddRefs(cueTextContent), nodeInfo->NodeInfoManager());

        if (!cueTextContent) {
          return nullptr;
        }
        {
          const char* text = reinterpret_cast<const char *>(
            webvtt_string_text(&aWebVTTNode->data.text));

          cueTextContent->SetText(NS_ConvertUTF8toUTF16(text), false);
        }
        break;
      }
      case WEBVTT_TIME_STAMP:
        // TODO: Need to create a "ProcessingInstruction?"
        break;
      default:
        // TODO: What happens here?
        break;
    }
  }

  return cueTextContent;
}

JSObject*
TextTrackCue::WrapObject(JSContext* aCx, JSObject* aScope)
{
  return TextTrackCueBinding::Wrap(aCx, aScope, this);
}

void
TextTrackCue::CueChanged()
{
  if (mTrack) {
    mTrack->CueChanged(*this);
  }
}
} // namespace dom
} // namespace mozilla
