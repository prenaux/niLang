#version 460
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_nonuniform_qualifier : require
// DO IMPORTS BEGIN niUIGpuFuncs
// MODULE BEGIN nish:std
// Type: VertexPAT1
struct nish_std_VertexPAT1 {
  vec3 position;
  vec4 color;
  vec2 tex0;
};
// Type: VertexOutput
struct nish_std_VertexOutput {
  vec4 position;
  vec4 color;
  vec2 tex0;
};
// TypeMethFwd: VertexOutput
nish_std_VertexOutput nish_std_VertexOutput_new(vec4 a_position, vec4 a_color, vec2 a_tex0);
// FunctionFwd: nish:std
// TypeMeth: VertexOutput
nish_std_VertexOutput nish_std_VertexOutput_new(vec4 a_position, vec4 a_color, vec2 a_tex0) {
  nish_std_VertexOutput t;
  t.position = a_position;
  t.color = a_color;
  t.tex0 = a_tex0;
  return t;
}
// Function: nish:std
// MODULE END nish:std
// DO IMPORTS END niUIGpuFuncs

// MODULE BEGIN niUIGpuFuncs

// FunctionFwd: niUIGpuFuncs
nish_std_VertexOutput niUIGpuFuncs_fixed_blit_vs(nish_std_VertexPAT1 aInput);

// Function: niUIGpuFuncs
nish_std_VertexOutput niUIGpuFuncs_fixed_blit_vs(nish_std_VertexPAT1 aInput) {
  vec3 _tmp_1 = aInput.position;
  vec4 outPos = vec4(_tmp_1.x,_tmp_1.y,_tmp_1.z,1.0);
  vec4 outColor = aInput.color;
  vec4 _tmp_6 = outPos;
  vec4 _tmp_7 = outColor;
  vec2 _tmp_8 = aInput.tex0;
  return nish_std_VertexOutput_new(_tmp_6, _tmp_7, _tmp_8);
}
// MODULE END niUIGpuFuncs

// Vertex Shader main: niUIGpuFuncs_fixed_blit_vs
layout(location = 0) in vec3 IN_0_aInput_position;
layout(location = 4) in vec4 IN_4_aInput_color;
layout(location = 5) in vec2 IN_5_aInput_tex0;
// layout() in vec4 gl_Position;
layout(location = 1) out vec4 OUT_1_rval_color;
layout(location = 2) out vec2 OUT_2_rval_tex0;
void main(void) {
  nish_std_VertexPAT1 aInput;
  aInput.position = IN_0_aInput_position;
  aInput.color = IN_4_aInput_color;
  aInput.tex0 = IN_5_aInput_tex0;
  nish_std_VertexOutput _rval_ = niUIGpuFuncs_fixed_blit_vs(aInput);
  gl_Position = _rval_.position;
  OUT_1_rval_color = _rval_.color;
  OUT_2_rval_tex0 = _rval_.tex0;
}

