/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "HTMLTrackElement.h"
#include "mozilla/dom/HTMLTrackElementBinding.h"
#include "HTMLUnknownElement.h"
#include "nsIDOMHTMLMediaElement.h"
#include "mozilla/dom/HTMLMediaElement.h"
#include "nsIDOMEventTarget.h"
#include "nsGenericHTMLElement.h"
#include "nsGkAtoms.h"
#include "nsStyleConsts.h"
#include "nsMappedAttributes.h"
#include "nsRuleData.h"
#include "nsContentUtils.h"
#include "nsIContentPolicy.h"
#include "nsContentPolicyUtils.h"
#include "nsILoadGroup.h"
#include "nsIDocument.h"
#include "nsICachingChannel.h"
#include "nsIHttpChannel.h"
#include "nsNetUtil.h"
#include "nsIStreamListener.h"
#include "nsIChannelEventSink.h"
#include "nsIChannelPolicy.h"
#include "nsIAsyncVerifyRedirectCallback.h"
#include "nsIContentSecurityPolicy.h"
#include "nsIInterfaceRequestor.h"
#include "nsIObserver.h"
#include "nsCOMPtr.h"
#include "nsThreadUtils.h"
#include "nsIFrame.h"
#include "nsVideoFrame.h"
#include "nsISupportsImpl.h"
#include "WebVTTLoadListener.h"
#include "webvtt/parser.h"

#ifdef PR_LOGGING
static PRLogModuleInfo* gTrackElementLog;
#define LOG(type, msg) PR_LOG(gTrackElementLog, type, msg)
#else
#define LOG(type, msg)
#endif

// Replace the usual NS_IMPL_NS_NEW_HTML_ELEMENT(Track) so
// we can return an UnknownElement instead when pref'd off.
nsGenericHTMLElement*
NS_NewHTMLTrackElement(already_AddRefed<nsINodeInfo> aNodeInfo,
                       mozilla::dom::FromParser aFromParser)
{
  if (!mozilla::dom::HTMLTrackElementBinding::PrefEnabled()) {
    return mozilla::dom::NewHTMLElementHelper::Create<nsHTMLUnknownElement,
           mozilla::dom::HTMLUnknownElement>(aNodeInfo);
  }

  return mozilla::dom::NewHTMLElementHelper::Create<nsHTMLTrackElement,
         mozilla::dom::HTMLTrackElement>(aNodeInfo);
}

namespace mozilla {
namespace dom {

//NS_IMPL_ISUPPORTS5(HTMLTrackElement, nsIRequestObserver, nsIStreamListener,
//    nsIChannelEventSink, nsIInterfaceRequestor, nsIObserver)

/** HTMLTrackElement */
HTMLTrackElement::HTMLTrackElement(already_AddRefed<nsINodeInfo> aNodeInfo)
  : nsGenericHTMLElement(aNodeInfo)
{
#ifdef PR_LOGGING
  if (!gTrackElementLog) {
    gTrackElementLog = PR_NewLogModule("nsTrackElement");
  }
#endif

  SetIsDOMBinding();
}

HTMLTrackElement::~HTMLTrackElement()
{
}

NS_IMPL_ADDREF_INHERITED(HTMLTrackElement, Element)
NS_IMPL_RELEASE_INHERITED(HTMLTrackElement, Element)

NS_INTERFACE_TABLE_HEAD(HTMLTrackElement)
  NS_HTML_CONTENT_INTERFACE_TABLE1(HTMLTrackElement,
				   nsIDOMHTMLElement)
  NS_HTML_CONTENT_INTERFACE_TABLE_TO_MAP_SEGUE(HTMLTrackElement,
					       nsGenericHTMLElement)
NS_HTML_CONTENT_INTERFACE_MAP_END

NS_IMPL_ELEMENT_CLONE(HTMLTrackElement)

JSObject*
HTMLTrackElement::WrapNode(JSContext* aCx, JSObject* aScope)
{
  return HTMLTrackElementBinding::Wrap(aCx, aScope, this);
}

TextTrack*
HTMLTrackElement::Track()
{
  if (!mTrack) {
    // We're expected to always have an internal TextTrack so create
    // an empty object to return if we don't already have one.
    mTrack = new TextTrack(OwnerDoc()->GetParentObject());
  }

  return mTrack;
}

nsresult
HTMLTrackElement::SetAcceptHeader(nsIHttpChannel* aChannel)
{
#ifdef MOZ_WEBVTT
  NS_NAMED_LITERAL_CSTRING(value, "text/webvtt");

  return aChannel->SetRequestHeader(NS_LITERAL_CSTRING("Accept"),
				    value,
				    false);
#else
  return NS_ERROR_NOT_IMPLEMENTED;
#endif
}

nsresult
HTMLTrackElement::LoadResource(nsIURI* aURI)
{
  if (mChannel) {
    mChannel->Cancel(NS_BINDING_ABORTED);
    mChannel = nullptr;
  }

  int16_t shouldLoad = nsIContentPolicy::ACCEPT;
  nsresult rv;
  rv = NS_CheckContentLoadPolicy(nsIContentPolicy::TYPE_MEDIA,
				 aURI,
				 NodePrincipal(),
				 static_cast<nsGenericHTMLElement*>(this),
				 EmptyCString(), // mime type
				 nullptr, // extra
				 &shouldLoad,
				 nsContentUtils::GetContentPolicy(),
				 nsContentUtils::GetSecurityManager());
  NS_ENSURE_SUCCESS(rv, rv);
  if (NS_CP_REJECTED(shouldLoad)) {
    return NS_ERROR_FAILURE;
  }

  nsCOMPtr<nsILoadGroup> loadGroup = OwnerDoc()->GetDocumentLoadGroup();

  CreateTextTrack();

  // check for a Content Security Policy to pass down to the channel
  // created to load the media content
  nsCOMPtr<nsIChannelPolicy> channelPolicy;
  nsCOMPtr<nsIContentSecurityPolicy> csp;
  rv = NodePrincipal()->GetCsp(getter_AddRefs(csp));
  NS_ENSURE_SUCCESS(rv, rv);
  if (csp) {
    channelPolicy = do_CreateInstance("@mozilla.org/nschannelpolicy;1");
    if (!channelPolicy) {
      return NS_ERROR_FAILURE;
    }
    channelPolicy->SetContentSecurityPolicy(csp);
    channelPolicy->SetLoadType(nsIContentPolicy::TYPE_MEDIA);
  }
  nsCOMPtr<nsIChannel> channel;
  rv = NS_NewChannel(getter_AddRefs(channel),
		     aURI,
		     nullptr,
		     loadGroup,
		     nullptr,
		     nsICachingChannel::LOAD_BYPASS_LOCAL_CACHE_IF_BUSY,
		     channelPolicy);
  NS_ENSURE_SUCCESS(rv, rv);

  mLoadListener = new WebVTTLoadListener(this);
  mLoadListener->LoadResource();
  channel->SetNotificationCallbacks(mLoadListener);

  LOG(PR_LOG_DEBUG, ("opening webvtt channel"));
  rv = channel->AsyncOpen(mLoadListener, nullptr);
  NS_ENSURE_SUCCESS(rv, rv);

  mChannel = channel;

  nsContentUtils::RegisterShutdownObserver(mLoadListener);

  return NS_OK;
}

void
HTMLTrackElement::DisplayCueText(webvtt_node* head)
{
fprintf(stderr, "calling HTMLTrackElement::DisplayCueText()\n");
  mLoadListener->DisplayCueText(head);
}

void
HTMLTrackElement::CreateTextTrack()
{
  nsString kind, label, srcLang;
  GetKind(kind);
  GetSrclang(srcLang);
  GetLabel(label);
  mTrack = new TextTrack(OwnerDoc()->GetParentObject(),
                         kind,
                         label,
                         srcLang);

fprintf(stderr, "...Trying to add mTrack to media element's TextTrackList...\n");
  nsCOMPtr<nsIDOMHTMLMediaElement> domMediaElem(do_QueryInterface(mMediaParent));
  if (domMediaElem) {
    HTMLMediaElement* mediaElem = static_cast<HTMLMediaElement*>(mMediaParent.get());
    if (mediaElem) {
      mediaElem->AddTextTrack(mTrack);
    }
  }
fprintf(stderr, "Done\n");
}

/** XXX: this is the right way to do it, but we have a timing bug on getting the media element
nsresult
HTMLTrackElement::SetAttr(int32_t aNameSpaceID, nsIAtom* aName,
                          nsIAtom* aPrefix, const nsAString& aValue,
                          bool aNotify)
{
  nsresult rv =
    nsGenericHTMLElement::SetAttr(aNameSpaceID, aName, aPrefix, aValue,
                                  aNotify);
  if (NS_FAILED(rv)) {
    return rv;
  }

  if (aNameSpaceID == kNameSpaceID_None && aName == nsGkAtoms::src) {
    nsAutoString src(aValue);
    nsCOMPtr<nsIURI> uri;
    nsresult rvTwo = NewURIFromString(src, getter_AddRefs(uri));
    if (NS_SUCCEEDED(rvTwo)) {
      LOG(PR_LOG_ALWAYS, ("%p Trying to load from src=%s", this,
	     NS_ConvertUTF16toUTF8(src).get()));
      LoadResource(uri);
    }
  }

  return rv;
}
**/

nsresult
HTMLTrackElement::BindToTree(nsIDocument* aDocument,
			     nsIContent* aParent,
			     nsIContent* aBindingParent,
			     bool aCompileEventHandlers)
{
  nsresult rv = nsGenericHTMLElement::BindToTree(aDocument,
						 aParent,
						 aBindingParent,
						 aCompileEventHandlers);
  NS_ENSURE_SUCCESS(rv, rv);

  if (!aDocument) {
    return NS_OK;
  }

  LOG(PR_LOG_DEBUG, ("Track Element bound to tree."));
  if (!aParent || !aParent->IsNodeOfType(nsINode::eMEDIA)) {
    return NS_OK;
  }

  // Store our parent so we can look up its frame for display
  if (!mMediaParent) {
    mMediaParent = do_QueryInterface(aParent);

    HTMLMediaElement* media = static_cast<HTMLMediaElement*>(aParent);
    // TODO: separate notification for 'alternate' tracks?
    media->NotifyAddedSource();
    LOG(PR_LOG_DEBUG, ("Track element sent notification to parent."));

    // Find our 'src' url
    nsAutoString src;

    // TODO: we might want to instead call LoadResource() in a
    // SetAttr override, like we do in media element.
    if (GetAttr(kNameSpaceID_None, nsGkAtoms::src, src)) {
      nsCOMPtr<nsIURI> uri;
      nsresult rvTwo = NewURIFromString(src, getter_AddRefs(uri));
      if (NS_SUCCEEDED(rvTwo)) {
        LOG(PR_LOG_ALWAYS, ("%p Trying to load from src=%s", this,
        NS_ConvertUTF16toUTF8(src).get()));
        LoadResource(uri);
      }
    }
  }

  return NS_OK;
}

} // namespace dom
} // namespace mozilla
