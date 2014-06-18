/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MP4_DEMUXER_ANNEX_B_H_
#define MP4_DEMUXER_ANNEX_B_H_

#include "mozilla/Vector.h"

namespace mp4_demuxer
{
class ByteReader;
class AnnexB
{
public:
  static mozilla::Vector<uint8_t> ConvertExtraDataToAnnexB(
    mozilla::Vector<uint8_t>& aExtraData);

private:
  static void ConvertSpsOrPsp(ByteReader& aReader, uint8_t aCount,
                              mozilla::Vector<uint8_t>* aAnnexB);
};

} // namespace mp4_demuxer

#endif // MP4_DEMUXER_ANNEX_B_H_
