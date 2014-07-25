#!/usr/bin/env python

# Script to generate boilerplate wrappers including the system
# version of framework headers.

SYSTEM_PATH = '/System/Library/Frameworks/'
FRAMEWORK = 'CoreMedia'
HEADERS = [
    'CoreMedia.h',
    'CMAttachment.h',
    'CMBlockBuffer.h',
    'CMBufferQueue.h',
    'CMFormatDescription.h',
    'CMSampleBuffer.h',
    'CMSimpleQueue.h',
    'CMTime.h',
    'CMTimeRange.h',
]

HEADER_TEMPLATE = '''/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// Stub header for the %(framework)s framework API.

#ifndef %(guard)s
#define %(guard)s

// CoreMedia is available starting in OS X 10.7,
// so we need to dlopen it as well to run on 10.6,
// but we can depend on the real framework headers at build time.

// We always build on 10.7 or later, but when we build with
// the 10.6 sdk sysroot, the system framework isn't available.
// Pull headers in here instead of providing a duplicate stub.
#include "%(path)s"

#endif // %(guard)s
'''

from os import path
print('Generating headers...')
for header in HEADERS:
  f = open(header, 'w')
  print('  %s' % header)
  f.write(HEADER_TEMPLATE % {
    'framework': FRAMEWORK,
    'guard': '_'.join(['mozilla', FRAMEWORK, header[:-2], 'h']),
    'path': path.join(SYSTEM_PATH, FRAMEWORK+'.framework', 'Headers', header),
  })
  f.close()
