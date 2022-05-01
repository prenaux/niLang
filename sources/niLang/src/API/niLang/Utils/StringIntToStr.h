#ifndef __STRINGINTTOSTR_BF3374D8_E989_43E7_AB8C_9CB82B4E91E4_H__
#define __STRINGINTTOSTR_BF3374D8_E989_43E7_AB8C_9CB82B4E91E4_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "../Types.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

const ni::tU32 kMaxCharsIntToStr = 64;

niExportFunc(char*) StringUIntToStr(char* apBuffer, ni::tSize anBufferSize, tU64 i);
niExportFunc(char*) StringIntToStr(char* apBuffer, tSize anBufferSize, tI64 i);

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __STRINGINTTOSTR_BF3374D8_E989_43E7_AB8C_9CB82B4E91E4_H__
