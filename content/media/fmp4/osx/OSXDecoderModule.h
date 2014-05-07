/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#if !defined(OSXPlatformDecoderModule_h_)
#define OSXPlatformDecoderModule_h_

#include "PlatformDecoderModule.h"

namespace mozilla {

class OSXDecoderModule : public PlatformDecoderModule {
public:
  OSXDecoderModule();
  virtual ~OSXDecoderModule();

  // Initializes the module, loads required dynamic libraries, etc.
  // Main thread only.
  nsresult Startup();

  // Called when the decoders have shutdown. Main thread only.
  // Does this really need to be main thread only????
  virtual nsresult Shutdown() MOZ_OVERRIDE;

  // Decode thread.
  virtual MediaDataDecoder*
  CreateH264Decoder(const mp4_demuxer::VideoDecoderConfig& aConfig,
                    mozilla::layers::LayersBackend aLayersBackend,
                    mozilla::layers::ImageContainer* aImageContainer,
                    MediaTaskQueue* aVideoTaskQueue,
                    MediaDataDecoderCallback* aCallback) MOZ_OVERRIDE;

  // Decode thread.
  virtual MediaDataDecoder* CreateAACDecoder(
    const mp4_demuxer::AudioDecoderConfig& aConfig,
    MediaTaskQueue* aAudioTaskQueue,
    MediaDataDecoderCallback* aCallback) MOZ_OVERRIDE;

  static void Init();
private:
  static bool sIsEnabled;
  nsAutoPtr<PlatformDecoderModule> mBlankDecoder;
};

} // namespace mozilla

#endif
