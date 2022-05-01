// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"

#if niMinFeatures(20)
#include "Graphics.h"
#include "VG.h"

//////////////////////////////////////////////////////////////////////////////////////////////
class cVGTransform : public cIUnknownImpl<iVGTransform>
{
 public:
  cVGTransform() {
    mTransform.push(agg::trans_affine()); // identity
  }
  cVGTransform(tF32 a0, tF32 a1, tF32 a2, tF32 a3, tF32 a4, tF32 a5) {
    mTransform.push(agg::trans_affine(a0,a1,a2,a3,a4,a5));
  }
  ~cVGTransform() {
  }

  tBool __stdcall Copy(const iVGTransform* apTransform) {
    niCheckSilent(niIsOK(apTransform),ni::eFalse);
    GetTransform() = niStaticCast(const cVGTransform*,apTransform)->GetTransform();
    return eTrue;
  }
  cVGTransform* __stdcall Clone() const {
    cVGTransform* pTransform = niNew cVGTransform();
    pTransform->Copy(this);
    return pTransform;
  }

  tBool __stdcall Push() {
    mTransform.push(mTransform.top());
    return eTrue;
  }
  tBool __stdcall Pop() {
    if (mTransform.size() > 1) {
      mTransform.pop();
      return eTrue;
    }
    else {
      return eFalse;
    }
  }

  void __stdcall SetMatrix(const sMatrixf& aMatrix) {
    agg_real vals[6];
    vals[0] = aMatrix._11;
    vals[1] = aMatrix._12;
    vals[2] = aMatrix._21;
    vals[3] = aMatrix._22;
    vals[4] = aMatrix._41;
    vals[5] = aMatrix._42;
    GetTransform().load_from(vals);
  }
  sMatrixf __stdcall GetMatrix() const {
    agg_real vals[6];
    GetTransform().store_to(vals);
    sMatrixf mtx = sMatrixf::Identity();
    mtx._11 = tF32(vals[0]); mtx._12 = tF32(vals[1]); mtx._41 = tF32(vals[4]);
    mtx._21 = tF32(vals[2]); mtx._22 = tF32(vals[3]); mtx._42 = tF32(vals[5]);
    return mtx;
  }

  virtual void __stdcall SetValue(eVGTransformValue aType, tF32 afValue) {
    if (aType >= 6) return;
    GetTransform().value(aType) = afValue;
  }
  virtual tF32 __stdcall GetValue(eVGTransformValue aType) const {
    if (aType >= 6) return 0.0f;
    return GetTransform().value((unsigned)aType);
  }

  virtual void __stdcall SetValues(tF32 m0, tF32 m1, tF32 m2, tF32 m3, tF32 m4, tF32 m5) {
    GetTransform() = agg::trans_affine(m0,m1,m2,m3,m4,m5);
  }

  void __stdcall Identity() {
    GetTransform().reset();
  }
  void __stdcall Invert() {
    GetTransform().invert();
  }

  tBool __stdcall GetIsIdentity() const {
    return GetTransform().is_identity();
  }
  tBool __stdcall IsEqual(const iVGTransform* apTransform) {
    niCheckSilent(niIsOK(apTransform),ni::eFalse);
    return GetTransform().is_equal(niStaticCast(const cVGTransform*,apTransform)->GetTransform());
  }

  tF32 __stdcall GetDeterminant() const {
    return (tF32)GetTransform().determinant();
  }

  void __stdcall FlipX() {
    GetTransform().flip_x();
  }
  void __stdcall FlipY() {
    GetTransform().flip_y();
  }

  sVec2f __stdcall Transform(const sVec2f& avV) {
    agg_real x = avV.x, y = avV.y;
    GetTransform().transform(&x,&y);
    return Vec2f((tF32)x,(tF32)y);
  }

  sVec2f __stdcall TransformRotate(const sVec2f& avV) {
    agg_real x = avV.x, y = avV.y;
    GetTransform().transform_2x2(&x,&y);
    return Vec2f((tF32)x,(tF32)y);
  }

  void __stdcall Multiply(const iVGTransform* apTransform) {
    niCheckSilent(niIsOK(apTransform),;);
    GetTransform().multiply(niStaticCast(const cVGTransform*,apTransform)->GetTransform());
  }
  void __stdcall PreMultiply(const iVGTransform* apTransform) {
    niCheckSilent(niIsOK(apTransform),;);
    GetTransform().premultiply(niStaticCast(const cVGTransform*,apTransform)->GetTransform());
  }

  virtual void __stdcall MultiplyMatrix(const sMatrixf& aMatrix) {
    GetTransform().multiply(agg::trans_affine(aMatrix._11,aMatrix._12,aMatrix._21,aMatrix._22,aMatrix._41,aMatrix._42));
  }
  virtual void __stdcall PreMultiplyMatrix(const sMatrixf& aMatrix) {
    GetTransform().premultiply(agg::trans_affine(aMatrix._11,aMatrix._12,aMatrix._21,aMatrix._22,aMatrix._41,aMatrix._42));
  }

  virtual void __stdcall MultiplyValues(tF32 m0, tF32 m1, tF32 m2, tF32 m3, tF32 m4, tF32 m5) {
    GetTransform().multiply(agg::trans_affine(m0,m1,m2,m3,m4,m5));
  }
  virtual void __stdcall PreMultiplyValues(tF32 m0, tF32 m1, tF32 m2, tF32 m3, tF32 m4, tF32 m5) {
    GetTransform().premultiply(agg::trans_affine(m0,m1,m2,m3,m4,m5));
  }

  void __stdcall Rotate(tF32 afRadians) {
    GetTransform().multiply(agg::trans_affine_rotation(afRadians));
  }
  void __stdcall PreRotate(tF32 afRadians) {
    GetTransform().premultiply(agg::trans_affine_rotation(afRadians));
  }

  void __stdcall RotateAround(const sVec2f& aV, tF32 afRadians) {
    GetTransform().multiply(agg::trans_affine_translation(-aV.x,-aV.y) *
                            agg::trans_affine_rotation(afRadians) *
                            agg::trans_affine_translation(aV.x,aV.y));
  }
  void __stdcall PreRotateAround(const sVec2f& aV, tF32 afRadians) {
    GetTransform().premultiply(agg::trans_affine_translation(-aV.x,-aV.y) *
                               agg::trans_affine_rotation(afRadians) *
                               agg::trans_affine_translation(aV.x,aV.y));
  }

  void __stdcall Translate(const sVec2f& aV) {
    GetTransform().multiply(agg::trans_affine_translation(aV.x,aV.y));
  }
  void __stdcall PreTranslate(const sVec2f& aV) {
    GetTransform().premultiply(agg::trans_affine_translation(aV.x,aV.y));
  }

  void __stdcall Scaling(const sVec2f& aV) {
    GetTransform().multiply(agg::trans_affine_scaling(aV.x,aV.y));
  }
  void __stdcall PreScaling(const sVec2f& aV) {
    GetTransform().premultiply(agg::trans_affine_scaling(aV.x,aV.y));
  }

  void __stdcall Skew(const sVec2f& aV) {
    GetTransform().multiply(agg::trans_affine_skewing(aV.x,aV.y));
  }
  void __stdcall PreSkew(const sVec2f& aV) {
    GetTransform().premultiply(agg::trans_affine_skewing(aV.x,aV.y));
  }

  void __stdcall LineSegment(const sVec2f& aStart, const sVec2f& aEnd, tF32 afDist) {
    GetTransform().multiply(agg::trans_affine_line_segment(aStart.x,aStart.y,aEnd.x,aEnd.y,afDist));
  }
  void __stdcall PreLineSegment(const sVec2f& aStart, const sVec2f& aEnd, tF32 afDist) {
    GetTransform().premultiply(agg::trans_affine_line_segment(aStart.x,aStart.y,aEnd.x,aEnd.y,afDist));
  }

  inline agg::trans_affine& GetTransform() { return mTransform.top(); }
  inline const agg::trans_affine& GetTransform() const { return mTransform.top(); }

  tF32 __stdcall GetRotation() const {
    return (tF32)GetTransform().rotation();
  }
  tF32 __stdcall GetScale() const {
    return (tF32)GetTransform().scale();
  }
  sVec2f __stdcall GetTranslation() const {
    agg_real x = 0, y = 0;
    GetTransform().translation(&x,&y);
    return Vec2f((tF32)x,(tF32)y);
  }
  sVec2f __stdcall GetScaling() const {
    agg_real x = 0, y = 0;
    GetTransform().scaling(&x,&y);
    return Vec2f((tF32)x,(tF32)y);
  }

 private:
  astl::stack<agg::trans_affine>  mTransform;
};

Ptr<iVGTransform> __stdcall cGraphics::CreateVGTransform() {
  return niNew cVGTransform();
}

///////////////////////////////////////////////
niExportFuncCPP(agg::trans_affine&) AGGGetTransform(iVGTransform* apTransform)
{
  return niStaticCast(cVGTransform*,apTransform)->GetTransform();
}
niExportFuncCPP(agg::trans_affine&) AGGGetTransform(const iVGTransform* apTransform)
{
  return niConstStaticCast(iVGTransform*,cVGTransform*,apTransform)->GetTransform();
}

///////////////////////////////////////////////
niExportFunc(iVGTransform*) CreateVGTransform() {
  return niNew cVGTransform();
}
#endif // niMinFeatures
