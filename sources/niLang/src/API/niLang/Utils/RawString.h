#ifndef __RAWSTRING_H_92970B6C_465A_4C8E_BE02_153F1530C06B__
#define __RAWSTRING_H_92970B6C_465A_4C8E_BE02_153F1530C06B__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../Types.h"
#include "../StringDef.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

struct RawString {
  RawString(ni::tU32 anSrcLine, const achar* const aSrcPath, const cchar* apStr, ni::tSize anStrLen) :
      mnSrcLine(anSrcLine),
      maszSrcPath(aSrcPath),
      mpData(apStr),
      mnSize((anStrLen+1)*sizeof(cchar))
  {
  }
  const ni::achar* GetSrcPath() const {
    return maszSrcPath;
  }
  const ni::tU32 GetSrcLine() const {
    return mnSrcLine;
  }
  const ni::cchar* GetString() const {
    return mpData;
  }
  const ni::cchar* Chars() const {
    return mpData;
  }
  const ni::tSize GetStringLen() const {
    return mnSize/sizeof(ni::cchar);
  }
  const ni::tPtr  GetData() const {
    return (ni::tPtr)mpData;
  }
  const ni::tSize GetSize() const {
    return mnSize;
  }
 private:
  const ni::tU32   mnSrcLine;
  const ni::achar* maszSrcPath;
  const ni::cchar* mpData;
  const ni::tSize  mnSize;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __RAWSTRING_H_92970B6C_465A_4C8E_BE02_153F1530C06B__
