#version 450
// Module: lib:shader
// FunctionFwd: lib:shader
// Function: lib:shader

// Module: TestGpuFuncs

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
TestGpuFuncs_VertexOutput TestGpuFuncs_VertexPAT1_texture_vs(TestGpuFuncs_VertexPAT1 aInput);

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
TestGpuFuncs_VertexOutput TestGpuFuncs_VertexPAT1_texture_vs(TestGpuFuncs_VertexPAT1 aInput) {
  vec3 _tmp_1 = aInput.position;
  vec4 outPos = vec4(_tmp_1.x,_tmp_1.y,_tmp_1.z,1.0);
  vec4 outColor = aInput.color;
  vec4 _tmp_6 = outPos;
  vec4 _tmp_7 = outColor;
  vec2 _tmp_8 = aInput.tex0;
  return TestGpuFuncs_VertexOutput_new(_tmp_6, _tmp_7, _tmp_8);
}

// Function: TestGpuFuncs

// Vertex Shader main: TestGpuFuncs_VertexPAT1_texture_vs
layout(location = 0) in vec3 IN_0_aInput_position;
layout(location = 1) in vec4 IN_1_aInput_color;
layout(location = 2) in vec2 IN_2_aInput_tex0;
// layout() in vec4 gl_Position;
layout(location = 1) out vec4 OUT_1_rval_color;
layout(location = 2) out vec2 OUT_2_rval_tex0;
void main(void) {
  TestGpuFuncs_VertexPAT1 aInput;
  aInput.position = IN_0_aInput_position;
  aInput.color = IN_1_aInput_color;
  aInput.tex0 = IN_2_aInput_tex0;
  TestGpuFuncs_VertexOutput _rval_ = TestGpuFuncs_VertexPAT1_texture_vs(aInput);
  gl_Position = _rval_.position;
  OUT_1_rval_color = _rval_.color;
  OUT_2_rval_tex0 = _rval_.tex0;
}

