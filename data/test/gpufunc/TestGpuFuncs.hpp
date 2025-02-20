#ifndef __TESTGPUFUNCS_NIL_MODULE__
#define __TESTGPUFUNCS_NIL_MODULE__
#include <niCC.h>

// Module: TestGpuFuncs
struct TestGpuFuncs_TestUniforms;
struct TestGpuFuncs_RayUniforms;
struct TestGpuFuncs_InstanceData;

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
  TestGpuFuncs_InstanceData(ni::ain<ni::sMatrixf> a_mtxWorld, ni::ain<ni::tU32> a_tex0, ni::ain<ni::tU32> a_tex1, ni::ain<ni::tU32> a_tex2, ni::ain<ni::tU32> a_tex3);

  // Variables
  ni::sMatrixf mtxWorld;
  ni::tU32 tex0;
  ni::tU32 tex1;
  ni::tU32 tex2;
  ni::tU32 tex3;
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
  this->tex0 = 0;
  this->tex1 = 0;
  this->tex2 = 0;
  this->tex3 = 0;
}
inline TestGpuFuncs_InstanceData::TestGpuFuncs_InstanceData(ni::ain<ni::sMatrixf> a_mtxWorld, ni::ain<ni::tU32> a_tex0, ni::ain<ni::tU32> a_tex1, ni::ain<ni::tU32> a_tex2, ni::ain<ni::tU32> a_tex3) {
  this->mtxWorld = a_mtxWorld;
  this->tex0 = a_tex0;
  this->tex1 = a_tex1;
  this->tex2 = a_tex2;
  this->tex3 = a_tex3;
}

// Function: TestGpuFuncs
#endif // __TESTGPUFUNCS_NIL_MODULE__
