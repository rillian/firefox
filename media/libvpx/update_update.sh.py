#!/usr/bin/env python2
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
import os
import re
import sys

if len(sys.argv) == 2:
    prefix = sys.argv[1]
else:
    print "usage: %s /path/to/libvpx" % sys.argv[0]
    sys.exit(1)

if not prefix.endswith('/'):
    prefix += '/'

extensions = [
    'asm',
    'c',
    'h',
]

common_files = [
    'build/make/ads2gas.pl',
    'LICENSE',
    'PATENTS',
]

ignore_files = [
    'vp8/common/context.c',
    'vp8/common/textblit.c',
    'vp8/encoder/ssim.c',
    'vpx_mem/vpx_mem_tracker.c',
    'vpx_scale/generic/bicubic_scaler.c',
    'vpx_scale/win32/scaleopt.c',
    'vpx_scale/win32/scalesystemdependent.c',
    'vp8/encoder/x86/ssim_opt.asm',
]
ignore_folders = [
    'examples/',
    'googletest/',
    'libmkv/',
    'mips/',
    'nestegg/',
    'objdir/',
    'ppc/',
    'test/',
    'libyuv/',
    'vpx_mem/memory_manager/',
]

for root, folders, files in os.walk(prefix):
    for f in files:
        f = os.path.join(root, f)[len(prefix):]
        if f.split('.')[-1] in extensions \
          and '/' in f \
          and f not in ignore_files \
          and not filter(lambda folder: folder in f, ignore_folders):
            common_files.append(f)

current_files = []
for root, folders, files in os.walk('./'):
    for f in files:
        f = os.path.join(root, f)[len('./'):]
        if f.split('.')[-1] in extensions \
          and '/' in f \
          and f not in ignore_files \
          and not filter(lambda folder: folder in f, ignore_folders):
            current_files.append(f)

common_files_s = "commonFiles=("
for f in sorted(common_files):
    common_files_s += "\n  " + f
common_files_s += "\n)\n"

with open('update.sh') as f:
    update_sh = f.read()

update_sh_new = re.sub(re.compile('commonFiles=\(.*?\)\n', re.DOTALL), common_files_s, update_sh)
if update_sh != update_sh_new:
    with open('update.sh', 'w') as f:
        f.write(update_sh_new)
    print 'update.sh updated'
else:
    print 'update.sh is up to date'

with open('moz.build') as f:
    moz_build = f.read()
with open('Makefile.in') as f:
    Makefile_in = f.read()

missing = []
for f in sorted(common_files):
    if f.split('.')[-1] in ['c', 'asm']:
        name = os.path.basename(f)
        if name not in moz_build and name not in Makefile_in:
            missing.append(f)

if missing:
    print 'WARNING! .c/.asm files missing from moz.build and Makefile.in'
    print '  ' + '\n  '.join(missing)
    print ''

removed_files = [f for f in current_files if f not in common_files]
if removed_files:
    print 'The following files are no longer in libvpx and should be removed:'
    print '  ' + '  '.join(removed_files)
