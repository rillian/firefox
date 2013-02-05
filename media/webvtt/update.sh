#!/bin/sh
# script to update the webvtt library source

URL=https://github.com/mozilla/webvtt.git
BRANCH=dev
REPO=gunk
SRCDIR=.

rm -rf include src

if [ -d ${REPO}/.git ]; then
  echo "Updating existing checkout..."
  cd ${REPO} && git fetch && git checkout ${BRANCH} -f && git clean -x -f -d && cd ..
else
  echo "Downloading source from ${URL}"
  git clone ${URL} ${REPO} -b ${BRANCH}
fi

#Create directories
mkdir include include/webvtt src

#Copy C headers
find ${REPO}/include/webvtt -type f -name '[^w]*.h' -exec cp '{}' ${SRCDIR}/include/webvtt/ \;
#Copy C sources
find ${REPO}/src/libwebvtt -type f -name '*.[ch]' -exec cp '{}' ${SRCDIR}/src/ \;

rm -rf ${REPO}

patch -p 0 -r . < patch1.diff