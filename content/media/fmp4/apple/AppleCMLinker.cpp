/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <dlfcn.h>

#include "AppleCMLinker.h"
#include "nsDebug.h"

#ifdef PR_LOGGING
extern PRLogModuleInfo* gMediaDecoderLog;
#define LOG(...) PR_LOG(gMediaDecoderLog, PR_LOG_DEBUG, (__VA_ARGS__))
#else
#define LOG(...)
#endif

namespace mozilla
{


AppleCMLinker::LinkStatus
AppleCMLinker::sLinkStatus = LinkStatus_INIT;

void* AppleCMLinker::sLink = nullptr;

#define LINK_FUNC(func) typeof(func) func;
#include "AppleCMFunctions.h"
#undef LINK_FUNC

/* static */ bool
AppleCMLinker::Link()
{
  if (sLinkStatus) {
    return sLinkStatus == LinkStatus_SUCCEEDED;
  }


  const char* dlname =
    "/System/Library/Frameworks/CoreMedia.framework/CoreMedia";
  if (!(sLink = dlopen(dlname, RTLD_NOW | RTLD_LOCAL))) {
    NS_WARNING("Couldn't link CoreMedia framework.");
    goto fail;
  }

#define LINK_FUNC(func)                                        \
  func = (typeof(func))dlsym(sLink, #func);                    \
  if (!func) {                                                 \
    NS_WARNING("Couldn't load CoreMedia function " #func ); \
    goto fail;                                                 \
  }
#include "AppleCMFunctions.h"
#undef LINK_FUNC

  sLinkStatus = LinkStatus_SUCCEEDED;
  return true;

fail:
  Unlink();

  sLinkStatus = LinkStatus_FAILED;
  return false;
}

/* static */ void
AppleCMLinker::Unlink()
{
  LOG("Unlinking CoreMedia framework.");
  if (sLink) {
    dlclose(sLink);
    sLink = nullptr;
  }
}

} // namespace mozilla
