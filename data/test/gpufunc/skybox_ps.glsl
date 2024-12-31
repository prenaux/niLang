#version 450
// DO IMPORTS BEGIN TestGpuFuncs
// MODULE BEGIN lib:shader
// MODULE PROLOGUE BEGIN lib:shader
mat4 nil_Mat4x3ToMat4x4(mat4x3 m) { return mat4(vec4(m[0], 0), vec4(m[1], 0), vec4(m[2], 0), vec4(m[3], 1)); }
mat4 nil_Mat3x4ToMat4x4(mat3x4 m) { return mat4(vec4(m[0]), vec4(m[1]), vec4(m[2]), vec4(0,0,0,1)); }
// MODULE PROLOGUE END lib:shader
// FunctionFwd: lib:shader
// Function: lib:shader
// MODULE END lib:shader
// DO IMPORTS END TestGpuFuncs

// MODULE BEGIN TestGpuFuncs

// Type: PixelOutput
struct TestGpuFuncs_PixelOutput {
  vec4 color;
};

// Type: VertexOutputPN
struct TestGpuFuncs_VertexOutputPN {
  vec4 position;
  vec3 normal;
};

// TypeMethFwd: PixelOutput
TestGpuFuncs_PixelOutput TestGpuFuncs_PixelOutput_new(vec4 a_color);

// TypeMethFwd: VertexOutputPN
TestGpuFuncs_PixelOutput TestGpuFuncs_VertexOutputPN_skybox_ps(TestGpuFuncs_VertexOutputPN aInput, textureCube aTexture, sampler aSS);

// FunctionFwd: TestGpuFuncs

// TypeMeth: PixelOutput
TestGpuFuncs_PixelOutput TestGpuFuncs_PixelOutput_new(vec4 a_color) {
  TestGpuFuncs_PixelOutput t;
  t.color = a_color;
  return t;
}

// TypeMeth: VertexOutputPN
TestGpuFuncs_PixelOutput TestGpuFuncs_VertexOutputPN_skybox_ps(TestGpuFuncs_VertexOutputPN aInput, textureCube aTexture, sampler aSS) {
  vec4 texColor = texture(samplerCube(aTexture,aSS),aInput.normal);
  vec4 _tmp_6 = texColor;
  return TestGpuFuncs_PixelOutput_new(_tmp_6);
}

// Function: TestGpuFuncs
// MODULE END TestGpuFuncs

// Pixel Shader main: TestGpuFuncs_VertexOutputPN_skybox_ps
layout(location = 0) in vec4 IN_0_aInput_position;
layout(location = 3) in vec3 IN_3_aInput_normal;
layout(set = 2, binding = 0) uniform textureCube IN_1_aTexture;
layout(set = 5, binding = 0) uniform sampler IN_1_aSS;
layout(location = 0) out vec4 OUT_0_rval_color;
void main(void) {
  TestGpuFuncs_VertexOutputPN aInput;
  aInput.position = IN_0_aInput_position;
  aInput.normal = IN_3_aInput_normal;
  TestGpuFuncs_PixelOutput _rval_ = TestGpuFuncs_VertexOutputPN_skybox_ps(aInput, IN_1_aTexture, IN_1_aSS);
  OUT_0_rval_color = _rval_.color;
}

