/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <dlfcn.h>

#include "AppleVTLinker.h"
#include "nsDebug.h"

#ifdef PR_LOGGING
PRLogModuleInfo* GetDemuxerLog();
#define LOG(...) PR_LOG(GetDemuxerLog(), PR_LOG_DEBUG, (__VA_ARGS__))
#else
#define LOG(...)
#endif

namespace mozilla
{

static const char *
dlname = "/System/Library/Frameworks/VideoToolbox.framework/VideoToolbox";

AppleVTLinker::LinkStatus
AppleVTLinker::sLinkStatus = LinkStatus_INIT;

void* AppleVTLinker::sLink = nullptr;

#define LINK_FUNC(func) typeof(func) func;
#include "AppleVTFunctions.h"
#undef LINK_FUNC

/* static */ bool
AppleVTLinker::Link()
{
  if (sLinkStatus) {
    return sLinkStatus == LinkStatus_SUCCEEDED;
  }

  if (!(sLink = dlopen(dlname, RTLD_NOW | RTLD_LOCAL))) {
    NS_WARNING("Couldn't link VideoToolbox framework.");
    goto fail;
  }

#define LINK_FUNC(func)                                        \
  func = (typeof(func))dlsym(sLink, #func);                    \
  if (!func) {                                                 \
    NS_WARNING("Couldn't load VideoToolbox function " #func ); \
    goto fail;                                                 \
  }
#include "AppleVTFunctions.h"
#undef LINK_FUNC

  sLinkStatus = LinkStatus_SUCCEEDED;
  return true;

fail:
  Unlink();

  sLinkStatus = LinkStatus_FAILED;
  return false;
}

/* static */ void
AppleVTLinker::Unlink()
{
  LOG("Unlinking VideoToolbox framework.");
  if (sLink) {
    dlclose(sLink);
    sLink = nullptr;
  }
}

} // namespace mozilla
