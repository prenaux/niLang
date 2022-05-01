#ifndef __UUID_83193888_H__
#define __UUID_83193888_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../Types.h"
#include "../ITime.h"
#include "../StringDef.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

//! UUID structure
struct sUUID
{
  union {
    tUUID    nUUID;
    ni::tU8  nData[knUUIDSize];
    struct {
      tU32 nData1;
      tU16 nData2;
      tU16 nData3;
      tU8  nData4[8];
    };
    struct {
      tU64 nDataHi;
      tU64 nDataLo;
    };
    struct {
      tU32  nTimeLow;
      tU16  nTimeMid;
      tU16  nTimeHiAndVersion;
      tU8   nClockReserved;
      tU8   nClockLow;
      tU8     nNodes[6];
    };
  };

  sUUID() {
    niCAssert(sizeof(*this) == sizeof(tUUID));
    Reset();
  }

  sUUID(const sUUID& aUUID) {
    Copy(aUUID);
  }

  sUUID(const tUUID& aUUID) {
    nUUID = aUUID;
  }

  sUUID(ni::tU32  anData1, ni::tU16 anData2, ni::tU16 anData3,
        ni::tU8 anData4, ni::tU8 anData5, ni::tU8 anData6, ni::tU8 anData7,
        ni::tU8 anData8, ni::tU8 anData9, ni::tU8 anData10, ni::tU8 anData11)
  {
    nData1 = anData1;
    nData2 = anData2;
    nData3 = anData3;
    nData4[0] = anData4;
    nData4[1] = anData5;
    nData4[2] = anData6;
    nData4[3] = anData7;
    nData4[4] = anData8;
    nData4[5] = anData9;
    nData4[6] = anData10;
    nData4[7] = anData11;
  }

  explicit sUUID(const achar* aaszUUID) {
    FromString(aaszUUID);
  }

  void __stdcall Reset() {
    nUUID = kuuidZero;
  }

  void __stdcall Copy(const sUUID& aRight) {
    nUUID = aRight.nUUID;
  }

  sUUID& __stdcall operator = (const sUUID& aRight) {
    Copy(aRight);
    return *this;
  }

  //! tUUID cast operator.
  operator const tUUID& () const {
    return nUUID;
  }

  ni::tI32 __stdcall Compare(const tUUID& aRight) const {
    return UUIDCmp(nUUID,aRight);
  }

  ni::tBool __stdcall operator == (const tUUID& aRight) const {
    return UUIDEq(nUUID,aRight);
  }
  ni::tBool __stdcall operator != (const tUUID& aRight) const {
    return !UUIDEq(nUUID,aRight);
  }
  ni::tBool __stdcall operator > (const tUUID& aRight) const {
    return Compare(aRight) > 0;
  }
  ni::tBool __stdcall operator < (const tUUID& aRight) {
    return Compare(aRight) < 0;
  }
  ni::tBool __stdcall operator >= (const tUUID& aRight) {
    return Compare(aRight) >= 0;
  }
  ni::tBool __stdcall operator <= (const tUUID& aRight) {
    return Compare(aRight) <= 0;
  }

  void __stdcall FromString(const achar* aaszUUID) {
    nUUID = cString::_ToUUID(aaszUUID,ni::StrLen(aaszUUID));
  }
  ni::cString __stdcall ToString() const {
    return ni::cString(nUUID);
  }
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __UUID_83193888_H__
