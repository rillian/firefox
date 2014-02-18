/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "OSXDecoderModule.h"
#include "mozilla/Preferences.h"
#include "mozilla/DebugOnly.h"
#include "mp4_demuxer/audio_decoder_config.h"

namespace mozilla {

bool OSXDecoderModule::sIsEnabled = false;

OSXDecoderModule::OSXDecoderModule()
{
}

OSXDecoderModule::~OSXDecoderModule()
{
}

/* static */
void
OSXDecoderModule::Init()
{
  MOZ_ASSERT(NS_IsMainThread(), "Must be on main thread.");
  sIsEnabled = Preferences::GetBool("media.mac-platform-codecs.enabled", false);
  if (!sIsEnabled) {
    return;
  }

  // TODO: dlopen VideoToolbox.framework if it's available.
  // If it's not, disable ourselves.
}

nsresult
OSXDecoderModule::Startup()
{
  if (!sIsEnabled) {
    return NS_ERROR_FAILURE;
  }
  // TODO: set up shared decoder state.
  NS_WARNING("OSXDecoderModule::Startup()");
  return NS_OK;
}

nsresult
OSXDecoderModule::Shutdown()
{
  MOZ_ASSERT(NS_IsMainThread(), "Must be on main thread.");

  // TODO: shut down shared state.
  NS_WARNING("OSXDecoderModule::Shutdown()");
  return NS_OK;
}

MediaDataDecoder*
OSXDecoderModule::CreateH264Decoder(const mp4_demuxer::VideoDecoderConfig& aConfig,
                                    mozilla::layers::LayersBackend aLayersBackend,
                                    mozilla::layers::ImageContainer* aImageContainer,
                                    MediaTaskQueue* aVideoTaskQueue,
                                    MediaDataDecoderCallback* aCallback)
{
  NS_WARNING("Creating h264 decoder NYI on OS X");
  return nullptr;
#if 0
  return new OSXVTDecoder(new OSXVideoOutputSource(aLayersBackend,
                                                   aImageContainer,
                                                   sDXVAEnabled),
                          aVideoTaskQueue,
                          aCallback);
#endif
}

MediaDataDecoder*
OSXDecoderModule::CreateAACDecoder(const mp4_demuxer::AudioDecoderConfig& aConfig,
                                   MediaTaskQueue* aAudioTaskQueue,
                                   MediaDataDecoderCallback* aCallback)
{
  // TODO: hook CoreAudio for AAC decoding?
  NS_WARNING("AAC decoder not implemented for OS X.");
  return nullptr;
}

} // namespace mozilla
