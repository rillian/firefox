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
  void GetKind(nsAString& aKind) const
  {
    GetHTMLAttr(nsGkAtoms::kind, aKind);
  }
  void SetKind(const nsAString& aKind, ErrorResult& aError)
  {
    SetHTMLAttr(nsGkAtoms::kind, aKind, aError);
  }

  void GetSrc(nsAString& aSrc) const
  {
    GetHTMLAttr(nsGkAtoms::src, aSrc);
  }
  void SetSrc(const nsAString& aSrc, ErrorResult& aError)
  {
    SetHTMLAttr(nsGkAtoms::src, aSrc, aError);
  }

  void GetSrclang(nsAString& aSrclang) const
  {
    GetHTMLAttr(nsGkAtoms::srclang, aSrclang);
  }
  void SetSrclang(const nsAString& aSrclang, ErrorResult& aError)
  {
    SetHTMLAttr(nsGkAtoms::srclang, aSrclang, aError);
  }

  void GetLabel(nsAString& aLabel) const
  {
    GetHTMLAttr(nsGkAtoms::label, aLabel);
  }
  void SetLabel(const nsAString& aLabel, ErrorResult& aError)
  {
    SetHTMLAttr(nsGkAtoms::label, aLabel, aError);
  }

  bool Default() const
  {
    return GetBoolAttr(nsGkAtoms::_default);
  }
  void SetDefault(bool aDefault, ErrorResult& aError)
  {
    SetHTMLBoolAttr(nsGkAtoms::_default, aDefault, aError);
  }

  uint16_t ReadyState() const
  {
    return mReadyState;
  }

  TextTrack* Track() const
  {
    // XXXhumph: where to set this?
    return mTrack;
  }

  virtual nsresult SetAcceptHeader(nsIHttpChannel* aChannel);

  // Superclass for Clone() and AsDOMNode() is nsINode
  virtual nsresult Clone(nsINodeInfo* aNodeInfo, nsINode** aResult) const MOZ_OVERRIDE;
  virtual nsIDOMNode* AsDOMNode() MOZ_OVERRIDE { return this; }

  // For Track, ItemValue reflects the src attribute
  virtual void GetItemValueText(nsAString& text) const
  {
    GetSrc(text);
  }
  virtual void SetItemValueText(const nsAString& text) {
    ErrorResult rv;
    SetSrc(text, rv);
  }

  // Override BindToTree() so that we can trigger a load when we add a
  // child track element.
  virtual nsresult BindToTree(nsIDocument* aDocument, nsIContent* aParent,
                              nsIContent* aBindingParent, bool aCompileEventHandlers);

  uint32_t GetCurrentLoadID() { return mCurrentLoadID; }

protected:
  virtual JSObject* WrapNode(JSContext* aCx, JSObject* aScope,
                             bool* aTriedToWrap) MOZ_OVERRIDE;

  class LoadListener;
  uint32_t mCurrentLoadID;
  nsRefPtr<TextTrack> mTrack;
  nsCOMPtr<nsIChannel> mChannel;
  nsCOMPtr<nsIContent> mMediaParent;
  uint16_t mReadyState;

  nsresult LoadResource(nsIURI* aURI);
};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_HTMLTrackElement_h
