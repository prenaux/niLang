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
// TypeStaticFwd: FloatConsts
float nish_std_FloatConsts_niPi2;
float nish_std_FloatConsts_ni2Pi;
// TypeMethFwd: PixelOutput
nish_std_PixelOutput nish_std_PixelOutput_new(vec4 a_color);
// FunctionFwd: nish:std
layout(set = 10, binding = 0) uniform texture2D nil_builtin_GetTexture2D[];
void nish_std_FloatConsts_static_initialize() {
  // TypeStatic: FloatConsts
  nish_std_FloatConsts_niPi2 = 1.5707963267949;
  nish_std_FloatConsts_ni2Pi = 6.2831853071796;
}
// TypeMeth: PixelOutput
nish_std_PixelOutput nish_std_PixelOutput_new(vec4 a_color) {
  nish_std_PixelOutput t;
  t.color = a_color;
  return t;
}
// Function: nish:std
// ModuleInitialize: nish_std
void nish_std_initialize() {
  nish_std_FloatConsts_static_initialize();
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
float TestGpuFuncs_ComputePieQuadrant(vec2 pos, float numQuadrants);
nish_std_PixelOutput TestGpuFuncs_texture_bindless_ps(nish_std_VertexOutput aInput, TestGpuFuncs_TestUniforms aUniforms, sampler aSS);

// Function: TestGpuFuncs
float TestGpuFuncs_ComputePieQuadrant(vec2 pos, float numQuadrants) {
  float centered_x = (pos.x - 0.5);
  float centered_y = (pos.y - 0.5);
  float angle = (atan(centered_y,centered_x) + nish_std_FloatConsts_niPi2);
  float angle_normalized;
  bool _tmp_a = (angle < 0.0);
  if (_tmp_a) {
    angle_normalized = (angle + nish_std_FloatConsts_ni2Pi);
  }
  else {
    {
      angle_normalized = angle;
    }
  }
  float quadrantF = ((angle_normalized / nish_std_FloatConsts_ni2Pi) * numQuadrants);
  return floor(quadrantF);
}
nish_std_PixelOutput TestGpuFuncs_texture_bindless_ps(nish_std_VertexOutput aInput, TestGpuFuncs_TestUniforms aUniforms, sampler aSS) {
  vec2 texCoo = aInput.tex0;
  float q = TestGpuFuncs_ComputePieQuadrant(texCoo,3.0);
  vec4 texColor;
  bool _tmp_p = (q < 1.0);
  if (_tmp_p) {
    texColor = texture(sampler2D(nil_builtin_GetTexture2D[nonuniformEXT(uint(aUniforms.padding0))],aSS),aInput.tex0);
  }
  else {
    bool _tmp_z = (q < 2.0);
    if (_tmp_z) {
      texColor = texture(sampler2D(nil_builtin_GetTexture2D[nonuniformEXT(uint(aUniforms.padding1))],aSS),aInput.tex0);
    }
    else {
      {
        texColor = texture(sampler2D(nil_builtin_GetTexture2D[nonuniformEXT(uint(aUniforms.padding2))],aSS),aInput.tex0);
      }
    }
  }
  vec4 _tmp_S = (texColor*aInput.color);
  return nish_std_PixelOutput_new(_tmp_S);
}
// MODULE END TestGpuFuncs

// Pixel Shader main: TestGpuFuncs_texture_bindless_ps
layout(location = 0) in vec4 IN_0_aInput_position;
layout(location = 1) in vec4 IN_1_aInput_color;
layout(location = 2) in vec2 IN_2_aInput_tex0;
// type size: 48, underlying: float
layout(set = 0, binding = 0) uniform UBO_TestGpuFuncs_TestUniforms { TestGpuFuncs_TestUniforms v; } IN_1_aUniforms;
layout(set = 5, binding = 0) uniform sampler IN_1_aSS;
layout(location = 0) out vec4 OUT_0_rval_color;
void main(void) {
  nish_std_initialize();
  nish_std_VertexOutput aInput;
  TestGpuFuncs_TestUniforms aUniforms;
  aInput.position = IN_0_aInput_position;
  aInput.color = IN_1_aInput_color;
  aInput.tex0 = IN_2_aInput_tex0;
  aUniforms = IN_1_aUniforms.v;
  nish_std_PixelOutput _rval_ = TestGpuFuncs_texture_bindless_ps(aInput, aUniforms, IN_1_aSS);
  OUT_0_rval_color = _rval_.color;
}

