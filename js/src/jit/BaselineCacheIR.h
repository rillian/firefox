/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=8 sts=4 et sw=4 tw=99:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef jit_BaselineCacheIR_h
#define jit_BaselineCacheIR_h

#include "gc/Barrier.h"
#include "jit/CacheIR.h"
#include "jit/CacheIRCompiler.h"

namespace js {
namespace jit {

class ICFallbackStub;
class ICStub;

void TraceBaselineCacheIRStub(JSTracer* trc, ICStub* stub, const CacheIRStubInfo* stubInfo);

ICStub* AttachBaselineCacheIRStub(JSContext* cx, const CacheIRWriter& writer,
                                  CacheKind kind, ICStubEngine engine, JSScript* outerScript,
                                  ICFallbackStub* stub);

} // namespace jit
} // namespace js

#endif /* jit_BaselineCacheIR_h */
