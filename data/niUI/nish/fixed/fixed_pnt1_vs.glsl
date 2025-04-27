#version 460
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_nonuniform_qualifier : require
// DO IMPORTS BEGIN niUIGpuFuncs
// MODULE BEGIN nish:std
// Type: VertexPNT1
struct nish_std_VertexPNT1 {
  vec3 position;
  vec3 normal;
  vec2 tex0;
};
// Type: VertexOutput
struct nish_std_VertexOutput {
  vec4 position;
  vec3 normal;
  vec4 color;
  vec2 tex0;
};
// TypeMethFwd: VertexOutput
nish_std_VertexOutput nish_std_VertexOutput_new(vec4 a_position, vec3 a_normal, vec4 a_color, vec2 a_tex0);
// FunctionFwd: nish:std
// TypeMeth: VertexOutput
nish_std_VertexOutput nish_std_VertexOutput_new(vec4 a_position, vec3 a_normal, vec4 a_color, vec2 a_tex0) {
  nish_std_VertexOutput t;
  t.position = a_position;
  t.normal = a_normal;
  t.color = a_color;
  t.tex0 = a_tex0;
  return t;
}
// Function: nish:std
// MODULE END nish:std
// DO IMPORTS END niUIGpuFuncs

// MODULE BEGIN niUIGpuFuncs

// Type: FixedUniforms
struct niUIGpuFuncs_FixedUniforms {
  mat4 mtxWVP;
  mat4 mtxW;
  float alphaRef;
  float padding0;
  float padding1;
  float padding2;
  vec4 materialColor;
};

// FunctionFwd: niUIGpuFuncs
nish_std_VertexOutput niUIGpuFuncs_fixed_pnt1_vs(nish_std_VertexPNT1 aInput, niUIGpuFuncs_FixedUniforms aUniforms);

// Function: niUIGpuFuncs
nish_std_VertexOutput niUIGpuFuncs_fixed_pnt1_vs(nish_std_VertexPNT1 aInput, niUIGpuFuncs_FixedUniforms aUniforms) {
  vec3 _tmp_1 = aInput.position;
  vec4 outPos = vec4(_tmp_1.x,_tmp_1.y,_tmp_1.z,1.0);
  vec4 _tmp_5 = (aUniforms.mtxWVP * outPos);
  vec3 _tmp_g = aInput.normal;
  vec3 _tmp_9 = normalize(((aUniforms.mtxW * vec4(_tmp_g.x,_tmp_g.y,_tmp_g.z,0.0)).xyz));
  vec4 _tmp_i = aUniforms.materialColor;
  vec2 _tmp_k = aInput.tex0;
  return nish_std_VertexOutput_new(_tmp_5, _tmp_9, _tmp_i, _tmp_k);
}
// MODULE END niUIGpuFuncs

// Vertex Shader main: niUIGpuFuncs_fixed_pnt1_vs
layout(location = 0) in vec3 IN_0_aInput_position;
layout(location = 3) in vec3 IN_3_aInput_normal;
layout(location = 5) in vec2 IN_5_aInput_tex0;
// type size: 64, underlying: float
layout(set = 0, binding = 0) uniform UBO_niUIGpuFuncs_FixedUniforms { niUIGpuFuncs_FixedUniforms v; } IN_1_aUniforms;
// layout() in vec4 gl_Position;
layout(location = 3) out vec3 OUT_3_rval_normal;
layout(location = 1) out vec4 OUT_1_rval_color;
layout(location = 2) out vec2 OUT_2_rval_tex0;
void main(void) {
  nish_std_VertexPNT1 aInput;
  niUIGpuFuncs_FixedUniforms aUniforms;
  aInput.position = IN_0_aInput_position;
  aInput.normal = IN_3_aInput_normal;
  aInput.tex0 = IN_5_aInput_tex0;
  aUniforms = IN_1_aUniforms.v;
  nish_std_VertexOutput _rval_ = niUIGpuFuncs_fixed_pnt1_vs(aInput, aUniforms);
  gl_Position = _rval_.position;
  OUT_3_rval_normal = _rval_.normal;
  OUT_1_rval_color = _rval_.color;
  OUT_2_rval_tex0 = _rval_.tex0;
}

