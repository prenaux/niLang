#ifndef __TESTGPUFUNCS_NIL_MODULE__
#define __TESTGPUFUNCS_NIL_MODULE__

// Module: TestGpuFuncs
#include <niCC.h>
struct TestGpuFuncs_TestUniforms;

// TypeMethFwd: TestUniforms
struct TestGpuFuncs_TestUniforms {

  // Constructor
  TestGpuFuncs_TestUniforms();
  TestGpuFuncs_TestUniforms(ni::ain<ni::tF32> a_alphaTest, ni::ain<ni::sVec4f> a_materialColor);

  // Variables
  ni::tF32 alphaTest;
  ni::tF32 padding0;
  ni::tF32 padding1;
  ni::tF32 padding2;
  ni::sVec4f materialColor;
};

// FunctionFwd: TestGpuFuncs

// TypeMeth: TestUniforms
inline TestGpuFuncs_TestUniforms::TestGpuFuncs_TestUniforms() {
  this->alphaTest = (ni::tF32)0.1;
  this->padding0 = (ni::tF32)0.0;
  this->padding1 = (ni::tF32)0.0;
  this->padding2 = (ni::tF32)0.0;
  this->materialColor = ni::sVec4f::White();
}
inline TestGpuFuncs_TestUniforms::TestGpuFuncs_TestUniforms(ni::ain<ni::tF32> a_alphaTest, ni::ain<ni::sVec4f> a_materialColor) {
  this->alphaTest = a_alphaTest;
  this->materialColor = a_materialColor;
}

// Function: TestGpuFuncs
#endif // __TESTGPUFUNCS_NIL_MODULE__
