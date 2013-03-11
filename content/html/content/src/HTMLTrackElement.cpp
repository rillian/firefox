/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "HTMLTrackElement.h"
#include "mozilla/dom/HTMLTrackElementBinding.h"

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
#include "nsHTMLMediaElement.h"
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

NS_IMPL_NS_NEW_HTML_ELEMENT(Track)

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
HTMLTrackElement::WrapNode(JSContext* aCx, JSObject* aScope, bool* aTriedToWrap)
{
  return HTMLTrackElementBinding::Wrap(aCx, aScope, this, aTriedToWrap);
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

  nsRefPtr<WebVTTLoadListener> listener = new WebVTTLoadListener(this);
  channel->SetNotificationCallbacks(listener);

  LOG(PR_LOG_DEBUG, ("opening webvtt channel"));
  rv = channel->AsyncOpen(listener, nullptr);
  NS_ENSURE_SUCCESS(rv, rv);

  mChannel = channel;

  nsContentUtils::RegisterShutdownObserver(listener);

  return NS_OK;
}

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

  LOG(PR_LOG_DEBUG, ("Track Element bound to tree."));
  if (!aParent || !aParent->IsNodeOfType(nsINode::eMEDIA)) {
    return NS_OK;
  }

  // Store our parent so we can look up its frame for display
  mMediaParent = do_QueryInterface(aParent);

  nsHTMLMediaElement* media = static_cast<nsHTMLMediaElement*>(aParent);
  // TODO: separate notification for 'alternate' tracks?
  media->NotifyAddedSource();
  LOG(PR_LOG_DEBUG, ("Track element sent notification to parent."));

  // Find our 'src' url
  nsAutoString src;

  if (GetAttr(kNameSpaceID_None, nsGkAtoms::src, src)) {
    nsCOMPtr<nsIURI> uri;
    nsresult rvTwo = NewURIFromString(src, getter_AddRefs(uri));
    if (NS_SUCCEEDED(rvTwo)) {
      LOG(PR_LOG_ALWAYS, ("%p Trying to load from src=%s", this,
	     NS_ConvertUTF16toUTF8(src).get()));
      LoadResource(uri);
    }
  }

  return NS_OK;
}

} // namespace dom
} // namespace mozilla
