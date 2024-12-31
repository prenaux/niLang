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

// Type: TestUniforms
struct TestGpuFuncs_TestUniforms {
  mat4 mtxWVP;
  float alphaRef;
  float padding0;
  float padding1;
  float padding2;
  vec4 materialColor;
};

// Type: VertexOutput
struct TestGpuFuncs_VertexOutput {
  vec4 position;
  vec4 color;
  vec2 tex0;
};

// Type: VertexPAT1
struct TestGpuFuncs_VertexPAT1 {
  vec3 position;
  vec4 color;
  vec2 tex0;
};

// TypeMethFwd: VertexOutput
TestGpuFuncs_VertexOutput TestGpuFuncs_VertexOutput_new(vec4 a_position, vec4 a_color, vec2 a_tex0);

// TypeMethFwd: VertexPAT1
TestGpuFuncs_VertexOutput TestGpuFuncs_VertexPAT1_fixed_pat1_vs(TestGpuFuncs_VertexPAT1 aInput, TestGpuFuncs_TestUniforms aUniforms);

// FunctionFwd: TestGpuFuncs

// TypeMeth: VertexOutput
TestGpuFuncs_VertexOutput TestGpuFuncs_VertexOutput_new(vec4 a_position, vec4 a_color, vec2 a_tex0) {
  TestGpuFuncs_VertexOutput t;
  t.position = a_position;
  t.color = a_color;
  t.tex0 = a_tex0;
  return t;
}

// TypeMeth: VertexPAT1
TestGpuFuncs_VertexOutput TestGpuFuncs_VertexPAT1_fixed_pat1_vs(TestGpuFuncs_VertexPAT1 aInput, TestGpuFuncs_TestUniforms aUniforms) {
  vec3 _tmp_1 = aInput.position;
  vec4 outPos = vec4(_tmp_1.x,_tmp_1.y,_tmp_1.z,1.0);
  vec4 outColor = aInput.color;
  vec4 _tmp_6 = (aUniforms.mtxWVP * outPos);
  vec4 _tmp_a = (outColor*aUniforms.materialColor);
  vec2 _tmp_e = aInput.tex0;
  return TestGpuFuncs_VertexOutput_new(_tmp_6, _tmp_a, _tmp_e);
}

// Function: TestGpuFuncs
// MODULE END TestGpuFuncs

// Vertex Shader main: TestGpuFuncs_VertexPAT1_fixed_pat1_vs
layout(location = 0) in vec3 IN_0_aInput_position;
layout(location = 4) in vec4 IN_4_aInput_color;
layout(location = 5) in vec2 IN_5_aInput_tex0;
// type size: 48
layout(set = 0, binding = 0) uniform UBO_TestGpuFuncs_TestUniforms { TestGpuFuncs_TestUniforms v; } IN_1_aUniforms;
// layout() in vec4 gl_Position;
layout(location = 1) out vec4 OUT_1_rval_color;
layout(location = 2) out vec2 OUT_2_rval_tex0;
void main(void) {
  TestGpuFuncs_VertexPAT1 aInput;
  TestGpuFuncs_TestUniforms aUniforms;
  aInput.position = IN_0_aInput_position;
  aInput.color = IN_4_aInput_color;
  aInput.tex0 = IN_5_aInput_tex0;
  aUniforms = IN_1_aUniforms.v;
  TestGpuFuncs_VertexOutput _rval_ = TestGpuFuncs_VertexPAT1_fixed_pat1_vs(aInput, aUniforms);
  gl_Position = _rval_.position;
  OUT_1_rval_color = _rval_.color;
  OUT_2_rval_tex0 = _rval_.tex0;
}

