#version 460
#extension GL_EXT_nonuniform_qualifier : require
// DO IMPORTS BEGIN TestGpuFuncs
// MODULE BEGIN nish:std
// Type: VertexOutput
struct nish_std_VertexOutput {
  vec4 position;
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

// FunctionFwd: TestGpuFuncs
nish_std_PixelOutput TestGpuFuncs_texture_alphatest_ps(nish_std_VertexOutput aInput, TestGpuFuncs_TestUniforms aUniforms, texture2D aTexture, sampler aSS);

// Function: TestGpuFuncs
nish_std_PixelOutput TestGpuFuncs_texture_alphatest_ps(nish_std_VertexOutput aInput, TestGpuFuncs_TestUniforms aUniforms, texture2D aTexture, sampler aSS) {
  vec4 texColor = texture(sampler2D(aTexture,aSS),aInput.tex0);
  nish_std_AlphaTest(texColor.w,aUniforms.alphaRef);
  vec4 _tmp_a = (texColor*aInput.color);
  return nish_std_PixelOutput_new(_tmp_a);
}
// MODULE END TestGpuFuncs

// Pixel Shader main: TestGpuFuncs_texture_alphatest_ps
layout(location = 0) in vec4 IN_0_aInput_position;
layout(location = 1) in vec4 IN_1_aInput_color;
layout(location = 2) in vec2 IN_2_aInput_tex0;
// type size: 48, underlying: float
layout(set = 0, binding = 0) uniform UBO_TestGpuFuncs_TestUniforms { TestGpuFuncs_TestUniforms v; } IN_1_aUniforms;
layout(set = 1, binding = 0) uniform texture2D IN_1_aTexture;
layout(set = 5, binding = 0) uniform sampler IN_1_aSS;
layout(location = 0) out vec4 OUT_0_rval_color;
void main(void) {
  nish_std_VertexOutput aInput;
  TestGpuFuncs_TestUniforms aUniforms;
  aInput.position = IN_0_aInput_position;
  aInput.color = IN_1_aInput_color;
  aInput.tex0 = IN_2_aInput_tex0;
  aUniforms = IN_1_aUniforms.v;
  nish_std_PixelOutput _rval_ = TestGpuFuncs_texture_alphatest_ps(aInput, aUniforms, IN_1_aTexture, IN_1_aSS);
  OUT_0_rval_color = _rval_.color;
}

