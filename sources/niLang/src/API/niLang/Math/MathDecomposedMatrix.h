#ifndef __MATHDECOMPOSEDMATRIX_44024344_H__
#define __MATHDECOMPOSEDMATRIX_44024344_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "MathMatrix.h"
#include "MathQuat.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Math
 * @{
 */

//! Decomposed matrix template.
template <typename T>
struct sDecomposedMatrix
{
  sVec3<T>   translation;
  sQuat<T>  rotation;
  sVec4<T>   scaleH;
};

//! Decomposed matrix tF32.
typedef sDecomposedMatrix<tF32> sDecomposedMatrixf;
//! Decomposed matrix tF64.
typedef sDecomposedMatrix<tF64> sDecomposedMatrixd;

///////////////////////////////////////////////
template <typename T>
sDecomposedMatrix<T>& DecomposedMatrixFromMatrix(sDecomposedMatrix<T>& Out, const sMatrix<T>& In) {
  MatrixDecomposeQ(In,Out.translation,Out.rotation,Out.scaleH);
  return Out;
}

///////////////////////////////////////////////
template <typename T>
sMatrix<T>& DecomposedMatrixToMatrix(sMatrix<T>& Out, const sDecomposedMatrix<T>& In) {
  MatrixComposeQ(Out,In.translation,In.rotation,In.scaleH);
  return Out;
}

///////////////////////////////////////////////
template <typename T>
sDecomposedMatrix<T>& DecomposedMatrixLerp(sDecomposedMatrix<T>& Out, const sDecomposedMatrix<T>& A, const sDecomposedMatrix<T>& B, T fac, eQuatSlerp aSlerp = eQuatSlerp_Short) {
  VecLerp(Out.translation,A.translation,B.translation,fac);
  QuatSlerp(Out.rotation,A.rotation,B.rotation,fac,aSlerp);
  VecLerp(Out.scaleH,A.scaleH,B.scaleH,fac);
  return Out;
}

///////////////////////////////////////////////
template <typename T>
sDecomposedMatrix<T>& DecomposedMatrixMultiply(sDecomposedMatrix<T>& Out, const sDecomposedMatrix<T>& A, const sDecomposedMatrix<T>& B)
{
  sMatrixf mtxA, mtxB, mtxC;
  DecomposedMatrixToMatrix(mtxA,A);
  DecomposedMatrixToMatrix(mtxB,B);
  return DecomposedMatrixFromMatrix(Out,MatrixMultiply(mtxC,mtxA,mtxB));
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __MATHDECOMPOSEDMATRIX_44024344_H__
