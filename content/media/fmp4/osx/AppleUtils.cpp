/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// Utility functions to help with Apple API calls.

#include <AudioToolbox/AudioToolbox.h>
#include "AppleUtils.h"
#include "prlog.h"

namespace mozilla {

#ifdef PR_LOGGING
  extern PRLogModuleInfo* gMediaDecoderLog;
#define LOGE(...) PR_LOG(gMediaDecoderLog, PR_LOG_ERROR, (__VA_ARGS__))
#define LOGW(...) PR_LOG(gMediaDecoderLog, PR_LOG_WARNING, (__VA_ARGS__))
#define LOGD(...) PR_LOG(gMediaDecoderLog, PR_LOG_DEBUG, (__VA_ARGS__))
#else
#define LOGE(...)
#define LOGW(...)
#define LOGD(...)
#endif

#define PROPERTY_ID_FORMAT "%c%c%c%c"
#define PROPERTY_ID_PRINT(x) ((x) >> 24), \
                             ((x) >> 16) & 0xff, \
                             ((x) >> 8) & 0xff, \
                              (x) & 0xff

nsresult
AppleUtils::GetProperty(AudioFileStreamID aAudioFileStream,
                        AudioFileStreamPropertyID aPropertyID,
                        void *aData)
{
  UInt32 size;
  Boolean writeable;
  OSStatus rv = AudioFileStreamGetPropertyInfo(aAudioFileStream, aPropertyID,
                                                             &size, &writeable);

  if (rv) {
    LOGW("Couldn't get property " PROPERTY_ID_FORMAT "\n",
         PROPERTY_ID_PRINT(aPropertyID));
    return NS_ERROR_FAILURE;
  }

  rv = AudioFileStreamGetProperty(aAudioFileStream, aPropertyID,
                                  &size, aData);

  return NS_OK;
}

} // namespace mozilla
