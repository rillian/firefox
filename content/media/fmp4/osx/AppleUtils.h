/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// Utility functions to help with Apple API calls.

#include <AudioToolbox/AudioToolbox.h>
#include "nsError.h"

namespace mozilla {

//typedef uint32_t nsresult;

class AppleUtils {
  public:

    // Helper to retrieve properties from AudioFileStream objects.
    static nsresult GetProperty(AudioFileStreamID aAudioFileStream,
                                AudioFileStreamPropertyID aPropertyID,
                                void *aData);
};

} // namespace mozilla
