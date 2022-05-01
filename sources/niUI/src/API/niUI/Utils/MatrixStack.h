#ifndef __MATRIXSTACK_H_BA0B244B_C443_4117_AFB7_A400AA47A2A1__
#define __MATRIXSTACK_H_BA0B244B_C443_4117_AFB7_A400AA47A2A1__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/Math/MathMatrix.h>

namespace ni {
/** \addtogroup niUI
 * @{
 */
/** \addtogroup niUI_Utils
 * @{
 */

//////////////////////////////////////////////////////////////////////////////////////////////
template <typename T, typename B = cUnknown0>
class cMatrixStack : public B
{
 public:
  // Default contructor, initialise the stack.
  cMatrixStack() {
    mStack.push(sMatrix<T>::Identity());
    mpTopMatrix = &mStack.top();
  }
  // Default destructor.
  ~cMatrixStack() {
  }

  //! Determines the product of the current matrix and the given
  //! matrix. This method right-multiplies the given matrix to the
  //! current matrix (transformation is about the current world
  //! origin).
  void __stdcall MultMatrix(const sMatrix<T>& Mat) {
    MatrixMultiply(*mpTopMatrix, *mpTopMatrix, Mat);
  }

  //! Determines the product of the given matrix and the current matrix.
  //! This method left-multiplies the given matrix to the current matrix
  //! (transformation is about the local origin of the object).
  void __stdcall MultMatrixLocal(const sMatrix<T>& Mat) {
    MatrixMultiply(*mpTopMatrix, Mat, *mpTopMatrix);
  }

  //! Retrieves the current matrix at the top of the stack.
  sMatrix<T>& __stdcall GetTop() {
    return *mpTopMatrix;
  }
  //! Retrieves the current matrix at the top of the stack.
  const sMatrix<T>& __stdcall GetTop() const {
    return *mpTopMatrix;
  }
  //! Set the value of the matrix at the top of the stack.
  void __stdcall SetTop(const sMatrixf& aMtx) {
    *mpTopMatrix = aMtx;
  }
  //! Loads identity in the current matrix.
  void __stdcall LoadIdentity() {
    MatrixIdentity(*mpTopMatrix);
  }
  //! Loads the given matrix into the current matrix.
  void __stdcall LoadMatrix(const sMatrix<T>& Mat) {
    memcpy(mpTopMatrix, &Mat, sizeof(sMatrix<T>));
  }
  //! Removes the current matrix from the top of the stack.
  void __stdcall Pop() {
    mStack.pop();
    mpTopMatrix = &mStack.top();
  }
  //! Adds a the current to the stack.
  void __stdcall Push() {
    mStack.push(*mpTopMatrix);
    mpTopMatrix = &mStack.top();
  }

  //! Determines the product of the current matrix and the computed
  //! rotation matrix. This method right-multiplies the current
  //! matrix with the computed rotation matrix, counterclockwise
  //! about the given axis with the given angle (rotation is about
  //! the current world origin).
  void  __stdcall RotateAxis(const sVec3<T>& V, T Angle) {
    sMatrix<T> tmp;
    MatrixRotationAxis(tmp, V, Angle);
    *mpTopMatrix = (*mpTopMatrix) * tmp;
  }
  //! Determines the product of the computed rotation matrix and the
  //! current matrix. This method left-multiplies the current matrix
  //! with the computed rotation matrix, counterclockwise about the
  //! given axis with the given angle (rotation is about the local
  //! origin of the object).
  void  __stdcall RotateAxisLocal(const sVec3<T>& V, T Angle) {
    sMatrix<T> tmp;
    MatrixRotationAxis(tmp, V, Angle);
    *mpTopMatrix = tmp * (*mpTopMatrix);
  }
  //! Determines the product of the current matrix and the computed
  //! rotation matrix (composed of a given yaw, pitch, and roll).
  //! This method right-multiplies the current matrix with the
  //! computed rotation matrix. All angles are counterclockwise and
  //! rotation is about the current world origin.
  void  __stdcall RotateYawPitchRoll(T Yaw, T Pitch, T Roll) {
    sMatrix<T> tmp;
    MatrixRotationYawPitchRoll(tmp, Yaw, Pitch, Roll);
    *mpTopMatrix = (*mpTopMatrix) * tmp;
  }
  //! Determines the product of the computed rotation matrix
  //! (composed of a given yaw, pitch, and roll) and the current
  //! matrix. This method left-multiplies the current matrix with
  //! the computed rotation matrix. All angles are counterclockwise
  //! and rotation is about the local origin of the object.
  void  __stdcall RotateYawPitchRollLocal(T Yaw, T Pitch, T Roll) {
    sMatrix<T> tmp;
    MatrixRotationYawPitchRoll(tmp, Yaw, Pitch, Roll);
    *mpTopMatrix = tmp * (*mpTopMatrix);
  }

  //! Determines the product of the current matrix and the computed
  //! scale matrix composed from the given point (x, y, and z). This
  //! method right-multiplies the current matrix with the computed
  //! scale matrix (transformation is about the current world
  //! origin).
  void  __stdcall Scale(T x, T y, T z) {
    sMatrix<T> tmp;
    MatrixScaling(tmp, x, y, z);
    *mpTopMatrix = (*mpTopMatrix) * tmp;
  }
  //! Determines the product of the computed scale matrix composed
  //! from the given point (x, y, and z) and the current
  //! matrix. This method left-multiplies the current matrix with
  //! the computed scale matrix (transformation is about the local
  //! origin of the object).
  void  __stdcall ScaleLocal(T x, T y, T z) {
    sMatrix<T> tmp;
    MatrixScaling(tmp, x, y, z);
    *mpTopMatrix = tmp * (*mpTopMatrix);
  }

  //! Determines the product of the current matrix and the computed
  //! translation matrix determined by the given factors (x, y, and
  //! z). This method right-multiplies the current matrix with the
  //! computed translation matrix (transformation is about the
  //! current world origin).
  void  __stdcall Translate(T x, T y, T z) {
    sMatrix<T> tmp;
    MatrixTranslation(tmp, x, y, z);
    *mpTopMatrix = (*mpTopMatrix) * tmp;
  }

  //! Determines the product of the computed translation matrix
  //! determined by the given factors (x, y, and z) and the current
  //! matrix. This method left-multiplies the current matrix with
  //! the computed translation matrix (transformation is about the
  //! local origin of the object).
  void  __stdcall TranslateLocal(T x, T y, T z) {
    sMatrix<T> tmp;
    MatrixTranslation(tmp, x, y, z);
    *mpTopMatrix = tmp * (*mpTopMatrix);
  }

 private:
  astl::stack< sMatrix<T> > mStack;
  sMatrix<T>*         mpTopMatrix;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//! tF32 matrix stack.
typedef cMatrixStack<tF32> cMatrixStackf;
//! tF64 matrix stack.
typedef cMatrixStack<tF64> cMatrixStackd;

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}; // End of ni
#endif // __MATRIXSTACK_H_BA0B244B_C443_4117_AFB7_A400AA47A2A1__
