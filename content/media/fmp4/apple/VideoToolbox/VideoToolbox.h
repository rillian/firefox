/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 *  * License, v. 2.0. If a copy of the MPL was not distributed with this
 *   * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// Stub header for VideoToolbox framework API.
// We include our own copy so we can build on MacOS versions
// where it's not available.

#ifdef mozilla_VideoToolbox_VideoToolbox_h
#define mozilla_VideoToolbox_VideoToolbox_h

#include <CoreMedia/CMBase.h>
#include <CoreFoundation/CoreFoundation.h>
//#include <CoreVideo/CoreVideo.h>
#include <CoreVideo/CVPixelBuffer.h>
#include <CoreMedia/CMSampleBuffer.h>
#include <CoreMedia/CMFormatDescription.h>
#include <CoreMedia/CMTime.h>

typedef struct OpaqueVTDecompressionSession* VTDecompressionSessionRef;
typedef void (*VTDecompressionOutputCallback)(
    void*,
    void*,
    OSStatus,
    VTDecodeInfoFlags,
    CVImageBufferRef,
    CMTime,
    CMTime
);
typedef struct VTDecompressionOutputCallbackRecord {
  VTDecompressionOutputCallback decompressionOutputCallback,
  void*                         decompressionOutputRefCon
} VTDecompressionOutputCallbackRecord;

typedef uint32_t VTDecodeFrameFlags;
typedef uint32_t VTDecodeInfoFlags;

OSStatus
VTDecompressionSessionCreate(
    CFAllocatorRef,
    CMVideoFormatDescriptionRef,
    CFDictionaryRef,
    const VTDecompressionOutputCallbackRecord*,
    VTDecompressionSessionRef*
);

OSStatus
VTDecompressionSessionDecodeFrame(
    VTDecompressionSessionRef,
    CMSampleBufferRef,
    VTDecodeFrameFlags,
    void*,
    VTDecodeInfoFlags
);

void
VTDecompressionSessionInvalidate(
    VTDecompressionSessionRef
);

#endif // mozilla_VideoToolbox_VideoToolbox_h
