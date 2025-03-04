#ifndef __NIUIGPUFUNCS_NIL_MODULE__
#define __NIUIGPUFUNCS_NIL_MODULE__
#include <niCC.h>

// Module: niUIGpuFuncs
struct niUIGpuFuncs_FixedUniforms;
struct niUIGpuFuncs_RayInstanceData;
struct niUIGpuFuncs_RayUniforms;

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

// TypeMethFwd: RayInstanceData
struct niUIGpuFuncs_RayInstanceData {

  // Constructor
  niUIGpuFuncs_RayInstanceData();
  niUIGpuFuncs_RayInstanceData(ni::ain<ni::tU32> a_ibIndex, ni::ain<ni::tU32> a_vbIndex, ni::ain<ni::tU32> a_texIndex);

  // Variables
  ni::tU32 ibIndex;
  ni::tU32 vbIndex;
  ni::tU32 texIndex;
};

// TypeMethFwd: RayUniforms
struct niUIGpuFuncs_RayUniforms {

  // Constructor
  niUIGpuFuncs_RayUniforms();
  niUIGpuFuncs_RayUniforms(ni::ain<ni::tF32> a_rtWidth, ni::ain<ni::tF32> a_rtHeight, ni::ain<ni::tF32> a_cameraFarClipPlane, ni::ain<ni::sMatrixf> a_cameraInvView, ni::ain<ni::sMatrixf> a_cameraInvViewProj);

  // Variables
  ni::tF32 rtWidth;
  ni::tF32 rtHeight;
  ni::tF32 cameraFarClipPlane;
  ni::tF32 padding;
  ni::sMatrixf cameraInvView;
  ni::sMatrixf cameraInvViewProj;
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

// TypeMeth: RayInstanceData
inline niUIGpuFuncs_RayInstanceData::niUIGpuFuncs_RayInstanceData() {
  this->ibIndex = 0;
  this->vbIndex = 0;
  this->texIndex = 0;
}
inline niUIGpuFuncs_RayInstanceData::niUIGpuFuncs_RayInstanceData(ni::ain<ni::tU32> a_ibIndex, ni::ain<ni::tU32> a_vbIndex, ni::ain<ni::tU32> a_texIndex) {
  this->ibIndex = a_ibIndex;
  this->vbIndex = a_vbIndex;
  this->texIndex = a_texIndex;
}

// TypeMeth: RayUniforms
inline niUIGpuFuncs_RayUniforms::niUIGpuFuncs_RayUniforms() {
  this->rtWidth = (ni::tF32)0.0;
  this->rtHeight = (ni::tF32)0.0;
  this->cameraFarClipPlane = (ni::tF32)10000.0;
  this->padding = (ni::tF32)0.0;
  this->cameraInvView = ni::sMatrixf::Identity();
  this->cameraInvViewProj = ni::sMatrixf::Identity();
}
inline niUIGpuFuncs_RayUniforms::niUIGpuFuncs_RayUniforms(ni::ain<ni::tF32> a_rtWidth, ni::ain<ni::tF32> a_rtHeight, ni::ain<ni::tF32> a_cameraFarClipPlane, ni::ain<ni::sMatrixf> a_cameraInvView, ni::ain<ni::sMatrixf> a_cameraInvViewProj) {
  this->rtWidth = a_rtWidth;
  this->rtHeight = a_rtHeight;
  this->cameraFarClipPlane = a_cameraFarClipPlane;
  this->cameraInvView = a_cameraInvView;
  this->cameraInvViewProj = a_cameraInvViewProj;
}

// Function: niUIGpuFuncs
#endif // __NIUIGPUFUNCS_NIL_MODULE__
