/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// Stub header for CoreMedia framework API.

#ifndef mozilla_CoreMedia_CMBase_h
#define mozilla_CoreMedia_CMBase_h

// CoreMedia is available starting in OS X 10.7,
// so we need to dlopen it as well to run on 10.6,
// but we can depend on the real framework headers at build time.

// We always build on 10.7 or later, but when we build with
// the 10.6 sdk sysroot, the system framework isn't available.
// Pull headers in here instead of providing a duplicate stub.
#include "/System/Library/Frameworks/CoreMedia.framework/Headers/CMBase.h"

#endif // mozilla_CoreMedia_CMBase_h
