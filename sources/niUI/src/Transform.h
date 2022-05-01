#ifndef __TRANSFORM_77651578_H__
#define __TRANSFORM_77651578_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {

struct iTransform;

/** \addtogroup Math
 * @{
 */

//! iTransform is the interface for positionning and orienting.
//! The user can work with local or world coordinnate system.
class cTransform : public cIUnknownImpl<iTransform>, public sTransformDesc
{
 public:
  cTransform(iTransform* pParent = NULL);
  ~cTransform();

  //// iUnkown //////////////////////////////////
  tBool __stdcall IsOK() const;
  void __stdcall Invalidate();
  //// iUnkown //////////////////////////////////

  //// iTransform ///////////////////////////////
  iTransform* __stdcall Clone() const;
  tBool __stdcall Copy(const iTransform* apSrc);

  tPtr __stdcall GetDescStructPtr() const {
    return (tPtr)niStaticCast(const sTransformDesc*,this);
  }

  void __stdcall SetFlags(tU16 anFlags);
  tU16 __stdcall GetFlags() const;

  void __stdcall SetSyncCounter(tU16 anCount);
  tU16 __stdcall GetSyncCounter() const;

  void __stdcall SetParent(iTransform* pParent);
  iTransform* __stdcall GetParent() const;

  void __stdcall Identity();
  tU16 __stdcall SetDirty();

  void __stdcall SetWorldMatrix(const sMatrixf& aMatrix);
  sMatrixf __stdcall GetWorldMatrix() const;
  void __stdcall SetLocalMatrix(const sMatrixf& aMatrix);
  sMatrixf __stdcall GetLocalMatrix() const;
  void __stdcall MultiplyWorldMatrix(const sMatrixf& aMatrix);
  void __stdcall PreMultiplyWorldMatrix(const sMatrixf& aMatrix);
  void __stdcall MultiplyLocalMatrix(const sMatrixf& aMatrix);
  void __stdcall PreMultiplyLocalMatrix(const sMatrixf& aMatrix);
  void __stdcall LookAt(const sVec3f& avLookAt, const sVec3f& avUp);

  void __stdcall SetWorldPosition(const sVec3f& v);
  sVec3f __stdcall GetWorldPosition() const;
  void __stdcall SetLocalPosition(const sVec3f& v);
  sVec3f __stdcall GetLocalPosition() const;
  void __stdcall Translate(const sVec3f& v);
  void __stdcall PreTranslate(const sVec3f& v);

  void __stdcall SetWorldRotation(const sMatrixf& aMatrix);
  void __stdcall SetLocalRotation(const sMatrixf& aMatrix);
  void __stdcall Rotate(const sMatrixf& aMatrix);
  void __stdcall PreRotate(const sMatrixf& aMatrix);

  void __stdcall SetScale(const sVec3f& aScale);
  void __stdcall Scale(const sVec3f& aScale);
  sVec3f __stdcall GetScale() const;

  sVec3f __stdcall GetRight() const;
  sVec3f __stdcall GetUp() const;
  sVec3f __stdcall GetForward() const;
  sVec3f __stdcall GetInvRight() const;
  sVec3f __stdcall GetInvUp() const;
  sVec3f __stdcall GetInvForward() const;

  iTransform* __stdcall CreatePreOffsetTransform();
  iTransform* __stdcall CreatePostOffsetTransform();

 private:
  tU16 mnParentSyncCounter;
  void _UpdateWorldMatrix();
  __forceinline void _SetDirty() {
    if (!niFlagIs(mnFlags,eTransformInternalFlags_Dirty)) {
      ++mnSyncCounter;
    }
    mnFlags |= eTransformInternalFlags_Dirty;
  }
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
};
#endif // __TRANSFORM_77651578_H__
