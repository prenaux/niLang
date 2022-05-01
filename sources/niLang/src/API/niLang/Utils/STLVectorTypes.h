#ifndef __STLVECTORTYPES_4879846475847_H__
#define __STLVECTORTYPES_4879846475847_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/STL/vector.h>

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

typedef astl::vector<tPtr>        tPtrVec;
typedef tPtrVec::iterator       tPtrVecIt;
typedef tPtrVec::const_iterator     tPtrVecCIt;
typedef tPtrVec::reverse_iterator   tPtrVecRIt;
typedef tPtrVec::const_reverse_iterator tPtrVecCRIt;

typedef astl::vector<tHandle>       tHandleVec;
typedef tHandleVec::iterator        tHandleVecIt;
typedef tHandleVec::const_iterator      tHandleVecCIt;
typedef tHandleVec::reverse_iterator    tHandleVecRIt;
typedef tHandleVec::const_reverse_iterator  tHandleVecCRIt;

typedef astl::vector<tU8>       tU8Vec;
typedef tU8Vec::iterator        tU8VecIt;
typedef tU8Vec::const_iterator      tU8VecCIt;
typedef tU8Vec::reverse_iterator    tU8VecRIt;
typedef tU8Vec::const_reverse_iterator  tU8VecCRIt;

typedef astl::vector<tU16>        tU16Vec;
typedef tU16Vec::iterator       tU16VecIt;
typedef tU16Vec::const_iterator     tU16VecCIt;
typedef tU16Vec::reverse_iterator   tU16VecRIt;
typedef tU16Vec::const_reverse_iterator tU16VecCRIt;

typedef astl::vector<tU32>        tU32Vec;
typedef tU32Vec::iterator       tU32VecIt;
typedef tU32Vec::const_iterator     tU32VecCIt;
typedef tU32Vec::reverse_iterator   tU32VecRIt;
typedef tU32Vec::const_reverse_iterator tU32VecCRIt;

typedef astl::vector<tI32>        tI32Vec;
typedef tI32Vec::iterator       tI32VecIt;
typedef tI32Vec::const_iterator     tI32VecCIt;
typedef tI32Vec::reverse_iterator   tI32VecRIt;
typedef tI32Vec::const_reverse_iterator tI32VecCRIt;

typedef astl::vector<tU32Vec>       tU32VecVec;
typedef tU32VecVec::iterator        tU32VecVecIt;
typedef tU32VecVec::const_iterator      tU32VecVecCIt;
typedef tU32VecVec::reverse_iterator    tU32VecVecRIt;
typedef tU32VecVec::const_reverse_iterator  tU32VecVecCRIt;

typedef astl::vector<tF32>        tF32Vec;
typedef tF32Vec::iterator       tF32VecIt;
typedef tF32Vec::const_iterator     tF32VecCIt;
typedef tF32Vec::reverse_iterator   tF32VecRIt;
typedef tF32Vec::const_reverse_iterator tF32VecCRIt;

//! Index vector.
typedef astl::vector<tIndex>    tIndexVec;
//! Index vector iterator.
typedef tIndexVec::iterator     tIndexVecIt;
//! Index vector constant iterator.
typedef tIndexVec::const_iterator tIndexVecCIt;

/**@}*/
/**@}*/
}
//////////////////////////////////////////////////////////////////////
#endif // __STLVECTORTYPES_4879846475847_H__
