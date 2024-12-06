#version 450
// Module: lib:shader
// FunctionFwd: lib:shader
// TypeMeth: Vec2
vec2 vec2_Zero;
void vec2_static_initialize() {
  vec2_Zero = vec2(0.0,0.0);
}
// Function: lib:shader
// ModuleInitialize: lib_shader
void lib_shader_initialize() {
  vec2_static_initialize();
}

// Module: TestGpuFuncs

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

// Type: VertexPA
struct TestGpuFuncs_VertexPA {
  vec3 position;
  vec4 color;
};

// TypeMethFwd: VertexOutput
TestGpuFuncs_VertexOutput TestGpuFuncs_VertexOutput_new(vec4 a_position, vec4 a_color, vec2 a_tex0);

// TypeMethFwd: VertexPA
TestGpuFuncs_VertexOutput TestGpuFuncs_VertexPA_fixed_pa_vs(TestGpuFuncs_VertexPA aInput, TestGpuFuncs_TestUniforms aUniforms);

// FunctionFwd: TestGpuFuncs

// TypeMeth: VertexOutput
TestGpuFuncs_VertexOutput TestGpuFuncs_VertexOutput_new(vec4 a_position, vec4 a_color, vec2 a_tex0) {
  TestGpuFuncs_VertexOutput t;
  t.position = a_position;
  t.color = a_color;
  t.tex0 = a_tex0;
  return t;
}

// TypeMeth: VertexPA
TestGpuFuncs_VertexOutput TestGpuFuncs_VertexPA_fixed_pa_vs(TestGpuFuncs_VertexPA aInput, TestGpuFuncs_TestUniforms aUniforms) {
  vec3 _tmp_1 = aInput.position;
  vec4 outPos = vec4(_tmp_1.x,_tmp_1.y,_tmp_1.z,1.0);
  vec4 _tmp_5 = (aUniforms.mtxWVP * outPos);
  vec4 _tmp_9 = (aInput.color*aUniforms.materialColor);
  vec2 _tmp_e = vec2_Zero;
  return TestGpuFuncs_VertexOutput_new(_tmp_5, _tmp_9, _tmp_e);
}

// Function: TestGpuFuncs

// Vertex Shader main: TestGpuFuncs_VertexPA_fixed_pa_vs
layout(location = 0) in vec3 IN_0_aInput_position;
layout(location = 4) in vec4 IN_4_aInput_color;
// type size: 48
layout(set = 0, binding = 0) uniform UBO_TestGpuFuncs_TestUniforms { TestGpuFuncs_TestUniforms v; } IN_1_aUniforms;
// layout() in vec4 gl_Position;
layout(location = 1) out vec4 OUT_1_rval_color;
layout(location = 2) out vec2 OUT_2_rval_tex0;
void main(void) {
  lib_shader_initialize();
  TestGpuFuncs_VertexPA aInput;
  TestGpuFuncs_TestUniforms aUniforms;
  aInput.position = IN_0_aInput_position;
  aInput.color = IN_4_aInput_color;
  aUniforms = IN_1_aUniforms.v;
  TestGpuFuncs_VertexOutput _rval_ = TestGpuFuncs_VertexPA_fixed_pa_vs(aInput, aUniforms);
  gl_Position = _rval_.position;
  OUT_1_rval_color = _rval_.color;
  OUT_2_rval_tex0 = _rval_.tex0;
}

