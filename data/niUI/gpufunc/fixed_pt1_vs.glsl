#version 450
// Module: lib:shader
// FunctionFwd: lib:shader
// Function: lib:shader

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

// Type: VertexPT1
struct TestGpuFuncs_VertexPT1 {
  vec3 position;
  vec2 tex0;
};

// TypeMethFwd: VertexOutput
TestGpuFuncs_VertexOutput TestGpuFuncs_VertexOutput_new(vec4 a_position, vec4 a_color, vec2 a_tex0);

// TypeMethFwd: VertexPT1
TestGpuFuncs_VertexOutput TestGpuFuncs_VertexPT1_fixed_pt1_vs(TestGpuFuncs_VertexPT1 aInput, TestGpuFuncs_TestUniforms aUniforms);

// FunctionFwd: TestGpuFuncs

// TypeMeth: VertexOutput
TestGpuFuncs_VertexOutput TestGpuFuncs_VertexOutput_new(vec4 a_position, vec4 a_color, vec2 a_tex0) {
  TestGpuFuncs_VertexOutput t;
  t.position = a_position;
  t.color = a_color;
  t.tex0 = a_tex0;
  return t;
}

// TypeMeth: VertexPT1
TestGpuFuncs_VertexOutput TestGpuFuncs_VertexPT1_fixed_pt1_vs(TestGpuFuncs_VertexPT1 aInput, TestGpuFuncs_TestUniforms aUniforms) {
  vec3 _tmp_1 = aInput.position;
  vec4 outPos = vec4(_tmp_1.x,_tmp_1.y,_tmp_1.z,1.0);
  vec4 _tmp_5 = (aUniforms.mtxWVP * outPos);
  vec4 _tmp_9 = aUniforms.materialColor;
  vec2 _tmp_b = aInput.tex0;
  return TestGpuFuncs_VertexOutput_new(_tmp_5, _tmp_9, _tmp_b);
}

// Function: TestGpuFuncs

// Vertex Shader main: TestGpuFuncs_VertexPT1_fixed_pt1_vs
layout(location = 0) in vec3 IN_0_aInput_position;
layout(location = 2) in vec2 IN_2_aInput_tex0;
// type size: 48
layout(set = 0, binding = 0) uniform UBO_TestGpuFuncs_TestUniforms { TestGpuFuncs_TestUniforms v; } IN_1_aUniforms;
// layout() in vec4 gl_Position;
layout(location = 1) out vec4 OUT_1_rval_color;
layout(location = 2) out vec2 OUT_2_rval_tex0;
void main(void) {
  TestGpuFuncs_VertexPT1 aInput;
  TestGpuFuncs_TestUniforms aUniforms;
  aInput.position = IN_0_aInput_position;
  aInput.tex0 = IN_2_aInput_tex0;
  aUniforms = IN_1_aUniforms.v;
  TestGpuFuncs_VertexOutput _rval_ = TestGpuFuncs_VertexPT1_fixed_pt1_vs(aInput, aUniforms);
  gl_Position = _rval_.position;
  OUT_1_rval_color = _rval_.color;
  OUT_2_rval_tex0 = _rval_.tex0;
}

