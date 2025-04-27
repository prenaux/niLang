#version 460
#extension GL_EXT_nonuniform_qualifier : require
// DO IMPORTS BEGIN niUIGpuFuncs
// MODULE BEGIN nish:std
// Type: VertexOutput
struct nish_std_VertexOutput {
  vec4 position;
  vec3 normal;
  vec4 color;
  vec2 tex0;
};
// Type: PixelOutput
struct nish_std_PixelOutput {
  vec4 color;
};
// TypeMethFwd: PixelOutput
nish_std_PixelOutput nish_std_PixelOutput_new(vec4 a_color);
// FunctionFwd: nish:std
// TypeMeth: PixelOutput
nish_std_PixelOutput nish_std_PixelOutput_new(vec4 a_color) {
  nish_std_PixelOutput t;
  t.color = a_color;
  return t;
}
// Function: nish:std
// MODULE END nish:std
// DO IMPORTS END niUIGpuFuncs

// MODULE BEGIN niUIGpuFuncs

// FunctionFwd: niUIGpuFuncs
nish_std_PixelOutput niUIGpuFuncs_fixed_clear_ps(nish_std_VertexOutput aInput);

// Function: niUIGpuFuncs
nish_std_PixelOutput niUIGpuFuncs_fixed_clear_ps(nish_std_VertexOutput aInput) {
  vec4 _tmp_2 = aInput.color;
  return nish_std_PixelOutput_new(_tmp_2);
}
// MODULE END niUIGpuFuncs

// Pixel Shader main: niUIGpuFuncs_fixed_clear_ps
layout(location = 0) in vec4 IN_0_aInput_position;
layout(location = 3) in vec3 IN_3_aInput_normal;
layout(location = 1) in vec4 IN_1_aInput_color;
layout(location = 2) in vec2 IN_2_aInput_tex0;
layout(location = 0) out vec4 OUT_0_rval_color;
void main(void) {
  nish_std_VertexOutput aInput;
  aInput.position = IN_0_aInput_position;
  aInput.normal = IN_3_aInput_normal;
  aInput.color = IN_1_aInput_color;
  aInput.tex0 = IN_2_aInput_tex0;
  nish_std_PixelOutput _rval_ = niUIGpuFuncs_fixed_clear_ps(aInput);
  OUT_0_rval_color = _rval_.color;
}

