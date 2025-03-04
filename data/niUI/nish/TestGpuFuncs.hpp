#ifndef __TESTGPUFUNCS_NIL_MODULE__
#define __TESTGPUFUNCS_NIL_MODULE__
#include <niCC.h>

// Module: TestGpuFuncs
struct TestGpuFuncs_TestUniforms;
struct TestGpuFuncs_RayUniforms;
struct TestGpuFuncs_InstanceData;
struct TestGpuFuncs_RayInstanceData;

// TypeMethFwd: TestUniforms
struct TestGpuFuncs_TestUniforms {

  // Constructor
  TestGpuFuncs_TestUniforms();
  TestGpuFuncs_TestUniforms(ni::ain<ni::sMatrixf> a_mtxWVP, ni::ain<ni::tF32> a_alphaRef, ni::ain<ni::sVec4f> a_materialColor);

  // Variables
  ni::sMatrixf mtxWVP;
  ni::tF32 alphaRef;
  ni::tF32 padding0;
  ni::tF32 padding1;
  ni::tF32 padding2;
  ni::sVec4f materialColor;
};

// TypeMethFwd: RayUniforms
struct TestGpuFuncs_RayUniforms {

  // Constructor
  TestGpuFuncs_RayUniforms();
  TestGpuFuncs_RayUniforms(ni::ain<ni::tF32> a_rtWidth, ni::ain<ni::tF32> a_rtHeight, ni::ain<ni::tF32> a_cameraFarClipPlane, ni::ain<ni::sMatrixf> a_cameraInvView, ni::ain<ni::sMatrixf> a_cameraInvViewProj);

  // Variables
  ni::tF32 rtWidth;
  ni::tF32 rtHeight;
  ni::tF32 cameraFarClipPlane;
  ni::tF32 padding;
  ni::sMatrixf cameraInvView;
  ni::sMatrixf cameraInvViewProj;
};

// TypeMethFwd: InstanceData
struct TestGpuFuncs_InstanceData {

  // Constructor
  TestGpuFuncs_InstanceData();
  TestGpuFuncs_InstanceData(ni::ain<ni::sMatrixf> a_mtxWorld, ni::ain<ni::tU32> a_texIndex0, ni::ain<ni::tU32> a_texIndex1, ni::ain<ni::tU32> a_texIndex2, ni::ain<ni::tU32> a_texIndex3);

  // Variables
  ni::sMatrixf mtxWorld;
  ni::tU32 texIndex0;
  ni::tU32 texIndex1;
  ni::tU32 texIndex2;
  ni::tU32 texIndex3;
};

// TypeMethFwd: RayInstanceData
struct TestGpuFuncs_RayInstanceData {

  // Constructor
  TestGpuFuncs_RayInstanceData();
  TestGpuFuncs_RayInstanceData(ni::ain<ni::tU32> a_ibIndex, ni::ain<ni::tU32> a_vbIndex, ni::ain<ni::tU32> a_texIndex);

  // Variables
  ni::tU32 ibIndex;
  ni::tU32 vbIndex;
  ni::tU32 texIndex;
};

// FunctionFwd: TestGpuFuncs

// TypeMeth: TestUniforms
inline TestGpuFuncs_TestUniforms::TestGpuFuncs_TestUniforms() {
  this->mtxWVP = ni::sMatrixf::Identity();
  this->alphaRef = (ni::tF32)0.1;
  this->padding0 = (ni::tF32)0.0;
  this->padding1 = (ni::tF32)0.0;
  this->padding2 = (ni::tF32)0.0;
  this->materialColor = ni::sVec4f::White();
}
inline TestGpuFuncs_TestUniforms::TestGpuFuncs_TestUniforms(ni::ain<ni::sMatrixf> a_mtxWVP, ni::ain<ni::tF32> a_alphaRef, ni::ain<ni::sVec4f> a_materialColor) {
  this->mtxWVP = a_mtxWVP;
  this->alphaRef = a_alphaRef;
  this->materialColor = a_materialColor;
}

// TypeMeth: RayUniforms
inline TestGpuFuncs_RayUniforms::TestGpuFuncs_RayUniforms() {
  this->rtWidth = (ni::tF32)0.0;
  this->rtHeight = (ni::tF32)0.0;
  this->cameraFarClipPlane = (ni::tF32)10000.0;
  this->padding = (ni::tF32)0.0;
  this->cameraInvView = ni::sMatrixf::Identity();
  this->cameraInvViewProj = ni::sMatrixf::Identity();
}
inline TestGpuFuncs_RayUniforms::TestGpuFuncs_RayUniforms(ni::ain<ni::tF32> a_rtWidth, ni::ain<ni::tF32> a_rtHeight, ni::ain<ni::tF32> a_cameraFarClipPlane, ni::ain<ni::sMatrixf> a_cameraInvView, ni::ain<ni::sMatrixf> a_cameraInvViewProj) {
  this->rtWidth = a_rtWidth;
  this->rtHeight = a_rtHeight;
  this->cameraFarClipPlane = a_cameraFarClipPlane;
  this->cameraInvView = a_cameraInvView;
  this->cameraInvViewProj = a_cameraInvViewProj;
}

// TypeMeth: InstanceData
inline TestGpuFuncs_InstanceData::TestGpuFuncs_InstanceData() {
  this->mtxWorld = ni::sMatrixf::Identity();
  this->texIndex0 = 0;
  this->texIndex1 = 0;
  this->texIndex2 = 0;
  this->texIndex3 = 0;
}
inline TestGpuFuncs_InstanceData::TestGpuFuncs_InstanceData(ni::ain<ni::sMatrixf> a_mtxWorld, ni::ain<ni::tU32> a_texIndex0, ni::ain<ni::tU32> a_texIndex1, ni::ain<ni::tU32> a_texIndex2, ni::ain<ni::tU32> a_texIndex3) {
  this->mtxWorld = a_mtxWorld;
  this->texIndex0 = a_texIndex0;
  this->texIndex1 = a_texIndex1;
  this->texIndex2 = a_texIndex2;
  this->texIndex3 = a_texIndex3;
}

// TypeMeth: RayInstanceData
inline TestGpuFuncs_RayInstanceData::TestGpuFuncs_RayInstanceData() {
  this->ibIndex = 0;
  this->vbIndex = 0;
  this->texIndex = 0;
}
inline TestGpuFuncs_RayInstanceData::TestGpuFuncs_RayInstanceData(ni::ain<ni::tU32> a_ibIndex, ni::ain<ni::tU32> a_vbIndex, ni::ain<ni::tU32> a_texIndex) {
  this->ibIndex = a_ibIndex;
  this->vbIndex = a_vbIndex;
  this->texIndex = a_texIndex;
}

// Function: TestGpuFuncs
#endif // __TESTGPUFUNCS_NIL_MODULE__
