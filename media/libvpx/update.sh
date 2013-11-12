#!/bin/bash
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

version=1.2.0
curl https://webm.googlecode.com/files/libvpx-v${version}.tar.bz2 > libvpx-v${version}.tar.bz2
tar xjf libvpx-v${version}.tar.bz2
test -e src && mv src src_old
mv libvpx-v${version} src
rm -rf src/examples src/third_party/googletest
rm libvpx-v${version}.tar.bz2

cd src
patch -p3 < ../stdint.patch
