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
void nish_std_AlphaTest(float aAlphaValue, float aAlphaRef);
// TypeMeth: PixelOutput
nish_std_PixelOutput nish_std_PixelOutput_new(vec4 a_color) {
  nish_std_PixelOutput t;
  t.color = a_color;
  return t;
}
// Function: nish:std
void nish_std_AlphaTest(float aAlphaValue, float aAlphaRef) {
  bool _tmp_0 = (aAlphaValue < aAlphaRef);
  if (_tmp_0) {
    discard;
  }
}
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
nish_std_PixelOutput niUIGpuFuncs_fixed_tex_alphatest_ps(nish_std_VertexOutput aInput, niUIGpuFuncs_FixedUniforms aUniforms, texture2D aTexture, sampler aSS);

// Function: niUIGpuFuncs
nish_std_PixelOutput niUIGpuFuncs_fixed_tex_alphatest_ps(nish_std_VertexOutput aInput, niUIGpuFuncs_FixedUniforms aUniforms, texture2D aTexture, sampler aSS) {
  vec4 texColor = texture(sampler2D(aTexture,aSS),aInput.tex0);
  nish_std_AlphaTest(texColor.w,aUniforms.alphaRef);
  vec4 _tmp_a = (texColor*aInput.color);
  return nish_std_PixelOutput_new(_tmp_a);
}
// MODULE END niUIGpuFuncs

// Pixel Shader main: niUIGpuFuncs_fixed_tex_alphatest_ps
layout(location = 0) in vec4 IN_0_aInput_position;
layout(location = 3) in vec3 IN_3_aInput_normal;
layout(location = 1) in vec4 IN_1_aInput_color;
layout(location = 2) in vec2 IN_2_aInput_tex0;
// type size: 64, underlying: float
layout(set = 0, binding = 0) uniform UBO_niUIGpuFuncs_FixedUniforms { niUIGpuFuncs_FixedUniforms v; } IN_1_aUniforms;
layout(set = 1, binding = 0) uniform texture2D IN_1_aTexture;
layout(set = 5, binding = 0) uniform sampler IN_1_aSS;
layout(location = 0) out vec4 OUT_0_rval_color;
void main(void) {
  nish_std_VertexOutput aInput;
  niUIGpuFuncs_FixedUniforms aUniforms;
  aInput.position = IN_0_aInput_position;
  aInput.normal = IN_3_aInput_normal;
  aInput.color = IN_1_aInput_color;
  aInput.tex0 = IN_2_aInput_tex0;
  aUniforms = IN_1_aUniforms.v;
  nish_std_PixelOutput _rval_ = niUIGpuFuncs_fixed_tex_alphatest_ps(aInput, aUniforms, IN_1_aTexture, IN_1_aSS);
  OUT_0_rval_color = _rval_.color;
}

