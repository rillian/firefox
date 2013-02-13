/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_HTMLTrackElement_h
#define mozilla_dom_HTMLTrackElement_h

#include "nsIDOMHTMLElement.h"
#include "nsIDOMEventTarget.h"
#include "nsGenericHTMLElement.h"
#include "nsIContent.h"
#include "nsIDocument.h"
#include "nsIHttpChannel.h"
#include "nsGkAtoms.h"
#include "mozilla/dom/TextTrack.h"

namespace mozilla {
namespace dom {

class TextTrack;

class HTMLTrackElement MOZ_FINAL : public nsGenericHTMLElement,
                                   public nsIDOMHTMLElement
{
public:
  HTMLTrackElement(already_AddRefed<nsINodeInfo> aNodeInfo);
  virtual ~HTMLTrackElement();

  // nsISupports
  NS_DECL_ISUPPORTS_INHERITED

  // nsIDOMNode
  NS_FORWARD_NSIDOMNODE_TO_NSINODE

  // nsIDOMElement
  NS_FORWARD_NSIDOMELEMENT_TO_GENERIC

  // nsIDOMHTMLElement
  NS_FORWARD_NSIDOMHTMLELEMENT_TO_GENERIC

  // HTMLTrackElement WebIDL
  void GetKind(nsString& aKind)
  {
    GetHTMLAttr(nsGkAtoms::kind, aKind);
  }
  void SetKind(const nsAString& aKind, ErrorResult& aError)
  {
    SetHTMLAttr(nsGkAtoms::kind, aKind);
  }

  void GetSrc(nsString& aSrc)
  {
    GetHTMLAttr(nsGkAtoms::src, aSrc);
  }
  void SetSrc(const nsAString& aSrc, ErrorResult& aError)
  {
    SetHTMLAttr(nsGkAtoms::src, aSrc, aError);
  }

  void GetSrclang(nsString& aSrclang)
  {
    GetHTMLAttr(nsGkAtoms::srclang, aSrclang);
  }
  void SetSrclang(const nsAString& aSrclang, ErrorResult& aError)
  {
    SetHTMLAttr(nsGkAtoms::srclang, aSrclang, aError);
  }

  void GetLabel(nsString& aLabel)
  {
    GetHTMLAttr(nsGkAtoms::label, aLabel);
  }
  void SetLabel(const nsAString& aLabel, ErrorResult& aError)
  {
    SetHTMLAttr(nsGkAtoms::label, aLabel, aError);
  }

  bool Default()
  {
    return GetBoolAttr(nsGkAtoms::_default);
  }
  void SetDefault(bool aDefault, ErrorResult& aError)
  {
    SetHTMLBoolAttr(nsGkAtoms::_default, aDefault, aError);
  }

  uint16_t ReadyState()
  {
    return mReadyState;
  }

  TextTrack* Track()
  {
    // XXXhumph: where to set this?
    return mTrack;
  }

  virtual nsresult Clone(nsINodeInfo* aNodeInfo, nsINode** aResult) const;
  virtual nsresult SetAcceptHeader(nsIHttpChannel* aChannel);
  virtual nsIDOMNode* AsDOMNode() { return this; }

  // Override BindToTree() so that we can trigger a load when we add a
  // child track element.
  virtual nsresult BindToTree(nsIDocument* aDocument, nsIContent* aParent,
                              nsIContent* aBindingParent, bool aCompileEventHandlers);

  PRUint32 GetCurrentLoadID() { return mCurrentLoadID; }

protected:
  virtual JSObject* WrapNode(JSContext* aCx, JSObject* aScope,
                             bool* aTriedToWrap) MOZ_OVERRIDE;

  class LoadListener;
  PRUint32 mCurrentLoadID;
  nsRefPtr<TextTrack> mTrack;
  nsCOMPtr<nsIChannel> mChannel;
  nsCOMPtr<nsIContent> mMediaParent;
  uint16_t mReadyState;

  nsresult NewURIFromString(const nsAutoString& aURISpec, nsIURI** aURI);
  nsresult LoadResource(nsIURI* aURI);
};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_HTMLTrackElement_h
