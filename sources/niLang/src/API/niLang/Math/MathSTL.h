#ifndef __MATHSTL_61092392_H__
#define __MATHSTL_61092392_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "MathVec2.h"
#include "MathVec3.h"
#include "MathVec4.h"
#include "MathMatrix.h"
#include "../STL/vector.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Math
 * @{
 */

typedef astl::vector<sVec2f> tVec2fVec;
typedef tVec2fVec::iterator  tVec2fVecIt;
typedef tVec2fVec::const_iterator  tVec2fVecCIt;

typedef astl::vector<sVec2i>     tVec2iVec;
typedef tVec2iVec::iterator      tVec2iVecIt;
typedef tVec2iVec::const_iterator  tVec2iVecCIt;

typedef astl::vector<sVec3f> tVec3fVec;
typedef tVec3fVec::iterator  tVec3fVecIt;
typedef tVec3fVec::const_iterator  tVec3fVecCIt;

typedef astl::vector<sVec4f> tVec4fVec;
typedef tVec4fVec::iterator  tVec4fVecIt;
typedef tVec4fVec::const_iterator  tVec4fVecCIt;

typedef astl::vector<sPlanef> tPlanefVec;
typedef tPlanefVec::iterator  tPlanefVecIt;
typedef tPlanefVec::const_iterator  tPlanefVecCIt;

//! sMatrixf vector.
typedef astl::vector<sMatrixf>        tMatrixfVec;
//! sMatrixf vector iterator.
typedef tMatrixfVec::iterator       tMatrixfVecIt;
//! sMatrixf vector constant iterator.
typedef tMatrixfVec::const_iterator     tMatrixfVecCIt;
//! sMatrixf vector reverse iterator.
typedef tMatrixfVec::reverse_iterator   tMatrixfVecRIt;
//! sMatrixf vector constant reverse iterator.
typedef tMatrixfVec::const_reverse_iterator tMatrixfVecCRIt;


//! tF64 matrix.
typedef sMatrix<tF64> sMatrixd;
//! sMatrixd vector.
typedef astl::vector<sMatrixd>        tMatrixdVec;
//! sMatrixd vector iterator.
typedef tMatrixdVec::iterator       tMatrixdVecIt;
//! sMatrixd vector constant iterator.
typedef tMatrixdVec::const_iterator     tMatrixdVecCIt;
//! sMatrixd vector reverse iterator.
typedef tMatrixdVec::reverse_iterator   tMatrixdVecRIt;
//! sMatrixd vector constant reverse iterator.
typedef tMatrixdVec::const_reverse_iterator tMatrixdVecCRIt;

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __MATHSTL_61092392_H__
