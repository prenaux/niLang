#version 450
// Module: lib:shader
// FunctionFwd: lib:shader
// Function: lib:shader

// Module: TestGpuFuncs

// Type: PixelOutput
struct TestGpuFuncs_PixelOutput {
  vec4 color;
};

// Type: VertexOutput
struct TestGpuFuncs_VertexOutput {
  vec4 position;
  vec4 color;
  vec2 tex0;
};

// TypeMethFwd: PixelOutput
TestGpuFuncs_PixelOutput TestGpuFuncs_PixelOutput_new(vec4 a_color);

// TypeMethFwd: VertexOutput
TestGpuFuncs_PixelOutput TestGpuFuncs_VertexOutput_clear_ps(TestGpuFuncs_VertexOutput aInput);

// FunctionFwd: TestGpuFuncs

// TypeMeth: PixelOutput
TestGpuFuncs_PixelOutput TestGpuFuncs_PixelOutput_new(vec4 a_color) {
  TestGpuFuncs_PixelOutput t;
  t.color = a_color;
  return t;
}

// TypeMeth: VertexOutput
TestGpuFuncs_PixelOutput TestGpuFuncs_VertexOutput_clear_ps(TestGpuFuncs_VertexOutput aInput) {
  vec4 _tmp_2 = aInput.color;
  return TestGpuFuncs_PixelOutput_new(_tmp_2);
}

// Function: TestGpuFuncs

// Pixel Shader main: TestGpuFuncs_VertexOutput_clear_ps
layout(location = 0) in vec4 IN_0_aInput_position;
layout(location = 1) in vec4 IN_1_aInput_color;
layout(location = 2) in vec2 IN_2_aInput_tex0;
layout(location = 0) out vec4 OUT_0_rval_color;
void main(void) {
  TestGpuFuncs_VertexOutput aInput;
  aInput.position = IN_0_aInput_position;
  aInput.color = IN_1_aInput_color;
  aInput.tex0 = IN_2_aInput_tex0;
  TestGpuFuncs_PixelOutput _rval_ = TestGpuFuncs_VertexOutput_clear_ps(aInput);
  OUT_0_rval_color = _rval_.color;
}

