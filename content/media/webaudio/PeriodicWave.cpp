/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "PeriodicWave.h"
#include "AudioContext.h"
#include "AudioNodeEngine.h"
#include "mozilla/dom/PeriodicWaveBinding.h"

namespace mozilla {
namespace dom {

NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE_1(PeriodicWave, mContext)

NS_IMPL_CYCLE_COLLECTION_ROOT_NATIVE(PeriodicWave, AddRef)
NS_IMPL_CYCLE_COLLECTION_UNROOT_NATIVE(PeriodicWave, Release)

PeriodicWave::PeriodicWave(AudioContext* aContext,
                           const float* aRealData,
                           const float* aImagData,
                           uint32_t aLength)
  : mContext(aContext)
{
  MOZ_ASSERT(aContext);
  SetIsDOMBinding();

  /* Caller should have checked this and thrown. */
  MOZ_ASSERT(aLength > 0);
  MOZ_ASSERT(aLength <= 4096);

  /* Copy frequency-domain data. */
  mRealData = new float[aLength];
  memcpy(mRealData, aRealData, aLength*sizeof(float));
  mImagData = new float[aLength];
  memcpy(mImagData, aImagData, aLength*sizeof(float));
  mLength = static_cast<int32_t>(aLength);
}

already_AddRefed<ThreadSharedFloatArrayBufferList> 
  PeriodicWave::GetThreadSharedBuffer()
{
  nsRefPtr<ThreadSharedFloatArrayBufferList> data =
    new ThreadSharedFloatArrayBufferList(2);
  /* FIXME: copy again? See the neutered buffer stuff. */
  float *real = new float[mLength];
  memcpy(real, mRealData, mLength*sizeof(float));
  data->SetData(0, real, real);
  float *imag = new float[mLength];
  memcpy(imag, mImagData, mLength*sizeof(float));
  data->SetData(1, imag, imag);
  return data.forget();
}

JSObject*
PeriodicWave::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aScope)
{
  return PeriodicWaveBinding::Wrap(aCx, aScope, this);
}

}
}

