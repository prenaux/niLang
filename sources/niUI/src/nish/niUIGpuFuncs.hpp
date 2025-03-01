#ifndef __NIUIGPUFUNCS_NIL_MODULE__
#define __NIUIGPUFUNCS_NIL_MODULE__
#include <niCC.h>

// Module: niUIGpuFuncs
struct niUIGpuFuncs_FixedUniforms;

// TypeMethFwd: FixedUniforms
struct niUIGpuFuncs_FixedUniforms {

  // Constructor
  niUIGpuFuncs_FixedUniforms();
  niUIGpuFuncs_FixedUniforms(ni::ain<ni::sMatrixf> a_mtxWVP, ni::ain<ni::tF32> a_alphaRef, ni::ain<ni::sVec4f> a_materialColor);

  // Variables
  ni::sMatrixf mtxWVP;
  ni::tF32 alphaRef;
  ni::tF32 padding0;
  ni::tF32 padding1;
  ni::tF32 padding2;
  ni::sVec4f materialColor;
};

// FunctionFwd: niUIGpuFuncs

// TypeMeth: FixedUniforms
inline niUIGpuFuncs_FixedUniforms::niUIGpuFuncs_FixedUniforms() {
  this->mtxWVP = ni::sMatrixf::Identity();
  this->alphaRef = (ni::tF32)0.1;
  this->padding0 = (ni::tF32)0.0;
  this->padding1 = (ni::tF32)0.0;
  this->padding2 = (ni::tF32)0.0;
  this->materialColor = ni::sVec4f::White();
}
inline niUIGpuFuncs_FixedUniforms::niUIGpuFuncs_FixedUniforms(ni::ain<ni::sMatrixf> a_mtxWVP, ni::ain<ni::tF32> a_alphaRef, ni::ain<ni::sVec4f> a_materialColor) {
  this->mtxWVP = a_mtxWVP;
  this->alphaRef = a_alphaRef;
  this->materialColor = a_materialColor;
}

// Function: niUIGpuFuncs
#endif // __NIUIGPUFUNCS_NIL_MODULE__
