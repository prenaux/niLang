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
TestGpuFuncs_VertexOutput TestGpuFuncs_VertexPA_clear_vs(TestGpuFuncs_VertexPA aInput);

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
TestGpuFuncs_VertexOutput TestGpuFuncs_VertexPA_clear_vs(TestGpuFuncs_VertexPA aInput) {
  vec3 _tmp_1 = aInput.position;
  vec4 outPos = vec4(_tmp_1.x,_tmp_1.y,_tmp_1.z,1.0);
  vec4 outColor = aInput.color;
  vec4 _tmp_6 = outPos;
  vec4 _tmp_7 = outColor;
  vec2 _tmp_8 = vec2_Zero;
  return TestGpuFuncs_VertexOutput_new(_tmp_6, _tmp_7, _tmp_8);
}

// Function: TestGpuFuncs

// Vertex Shader main: TestGpuFuncs_VertexPA_clear_vs
layout(location = 0) in vec3 IN_0_aInput_position;
layout(location = 4) in vec4 IN_4_aInput_color;
// layout() in vec4 gl_Position;
layout(location = 1) out vec4 OUT_1_rval_color;
layout(location = 2) out vec2 OUT_2_rval_tex0;
void main(void) {
  lib_shader_initialize();
  TestGpuFuncs_VertexPA aInput;
  aInput.position = IN_0_aInput_position;
  aInput.color = IN_4_aInput_color;
  TestGpuFuncs_VertexOutput _rval_ = TestGpuFuncs_VertexPA_clear_vs(aInput);
  gl_Position = _rval_.position;
  OUT_1_rval_color = _rval_.color;
  OUT_2_rval_tex0 = _rval_.tex0;
}

