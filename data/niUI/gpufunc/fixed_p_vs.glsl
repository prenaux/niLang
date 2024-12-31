#version 450
// DO IMPORTS BEGIN TestGpuFuncs
// MODULE BEGIN lib:shader
// MODULE PROLOGUE BEGIN lib:shader
mat4 nil_Mat4x3ToMat4x4(mat4x3 m) { return mat4(vec4(m[0], 0), vec4(m[1], 0), vec4(m[2], 0), vec4(m[3], 1)); }
mat4 nil_Mat3x4ToMat4x4(mat3x4 m) { return mat4(vec4(m[0]), vec4(m[1]), vec4(m[2]), vec4(0,0,0,1)); }
// MODULE PROLOGUE END lib:shader
// TypeStaticFwd: Vec2
vec2 vec2_Zero;
// FunctionFwd: lib:shader
void vec2_static_initialize() {
  // TypeStatic: Vec2
  vec2_Zero = vec2(0.0,0.0);
}
// Function: lib:shader
// ModuleInitialize: lib_shader
void lib_shader_initialize() {
  vec2_static_initialize();
}
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

// Type: VertexP
struct TestGpuFuncs_VertexP {
  vec3 position;
};

// TypeMethFwd: VertexOutput
TestGpuFuncs_VertexOutput TestGpuFuncs_VertexOutput_new(vec4 a_position, vec4 a_color, vec2 a_tex0);

// TypeMethFwd: VertexP
TestGpuFuncs_VertexOutput TestGpuFuncs_VertexP_fixed_p_vs(TestGpuFuncs_VertexP aInput, TestGpuFuncs_TestUniforms aUniforms);

// FunctionFwd: TestGpuFuncs

// TypeMeth: VertexOutput
TestGpuFuncs_VertexOutput TestGpuFuncs_VertexOutput_new(vec4 a_position, vec4 a_color, vec2 a_tex0) {
  TestGpuFuncs_VertexOutput t;
  t.position = a_position;
  t.color = a_color;
  t.tex0 = a_tex0;
  return t;
}

// TypeMeth: VertexP
TestGpuFuncs_VertexOutput TestGpuFuncs_VertexP_fixed_p_vs(TestGpuFuncs_VertexP aInput, TestGpuFuncs_TestUniforms aUniforms) {
  vec3 _tmp_1 = aInput.position;
  vec4 outPos = vec4(_tmp_1.x,_tmp_1.y,_tmp_1.z,1.0);
  vec4 _tmp_5 = (aUniforms.mtxWVP * outPos);
  vec4 _tmp_9 = aUniforms.materialColor;
  vec2 _tmp_b = vec2_Zero;
  return TestGpuFuncs_VertexOutput_new(_tmp_5, _tmp_9, _tmp_b);
}

// Function: TestGpuFuncs
// MODULE END TestGpuFuncs

// Vertex Shader main: TestGpuFuncs_VertexP_fixed_p_vs
layout(location = 0) in vec3 IN_0_aInput_position;
// type size: 48
layout(set = 0, binding = 0) uniform UBO_TestGpuFuncs_TestUniforms { TestGpuFuncs_TestUniforms v; } IN_1_aUniforms;
// layout() in vec4 gl_Position;
layout(location = 1) out vec4 OUT_1_rval_color;
layout(location = 2) out vec2 OUT_2_rval_tex0;
void main(void) {
  lib_shader_initialize();
  TestGpuFuncs_VertexP aInput;
  TestGpuFuncs_TestUniforms aUniforms;
  aInput.position = IN_0_aInput_position;
  aUniforms = IN_1_aUniforms.v;
  TestGpuFuncs_VertexOutput _rval_ = TestGpuFuncs_VertexP_fixed_p_vs(aInput, aUniforms);
  gl_Position = _rval_.position;
  OUT_1_rval_color = _rval_.color;
  OUT_2_rval_tex0 = _rval_.tex0;
}

