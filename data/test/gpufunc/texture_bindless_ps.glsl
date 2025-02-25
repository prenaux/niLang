#version 460
#extension GL_EXT_nonuniform_qualifier : require
// DO IMPORTS BEGIN TestGpuFuncs
// MODULE BEGIN lib:shader
// TypeStaticFwd: FloatConsts
float lib_shader_FloatConsts_niPi2;
float lib_shader_FloatConsts_ni2Pi;
// FunctionFwd: lib:shader
layout(set = 10, binding = 0) uniform texture2D nil_builtin_GetTexture2D[];
void lib_shader_FloatConsts_static_initialize() {
  // TypeStatic: FloatConsts
  lib_shader_FloatConsts_niPi2 = 1.5707963267949;
  lib_shader_FloatConsts_ni2Pi = 6.2831853071796;
}
// Function: lib:shader
// ModuleInitialize: lib_shader
void lib_shader_initialize() {
  lib_shader_FloatConsts_static_initialize();
}
// MODULE END lib:shader
// DO IMPORTS END TestGpuFuncs

// MODULE BEGIN TestGpuFuncs

// Type: PixelOutput
struct TestGpuFuncs_PixelOutput {
  vec4 color;
};

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

// TypeMethFwd: PixelOutput
TestGpuFuncs_PixelOutput TestGpuFuncs_PixelOutput_new(vec4 a_color);

// TypeMethFwd: VertexOutput
TestGpuFuncs_PixelOutput TestGpuFuncs_VertexOutput_texture_bindless_ps(TestGpuFuncs_VertexOutput aInput, TestGpuFuncs_TestUniforms aUniforms, sampler aSS);

// FunctionFwd: TestGpuFuncs
float TestGpuFuncs_ComputePieQuadrant(vec2 pos, float numQuadrants);

// TypeMeth: PixelOutput
TestGpuFuncs_PixelOutput TestGpuFuncs_PixelOutput_new(vec4 a_color) {
  TestGpuFuncs_PixelOutput t;
  t.color = a_color;
  return t;
}

// TypeMeth: VertexOutput
TestGpuFuncs_PixelOutput TestGpuFuncs_VertexOutput_texture_bindless_ps(TestGpuFuncs_VertexOutput aInput, TestGpuFuncs_TestUniforms aUniforms, sampler aSS) {
  vec2 texCoo = aInput.tex0;
  float q = TestGpuFuncs_ComputePieQuadrant(texCoo,3.0);
  vec4 texColor;
  bool _tmp_3 = (q < 1.0);
  if (_tmp_3) {
    texColor = texture(sampler2D(nil_builtin_GetTexture2D[nonuniformEXT(uint(aUniforms.padding0))],aSS),aInput.tex0);
  }
  else {
    bool _tmp_d = (q < 2.0);
    if (_tmp_d) {
      texColor = texture(sampler2D(nil_builtin_GetTexture2D[nonuniformEXT(uint(aUniforms.padding1))],aSS),aInput.tex0);
    }
    else {
      {
        texColor = texture(sampler2D(nil_builtin_GetTexture2D[nonuniformEXT(uint(aUniforms.padding2))],aSS),aInput.tex0);
      }
    }
  }
  vec4 _tmp_w = (texColor*aInput.color);
  return TestGpuFuncs_PixelOutput_new(_tmp_w);
}

// Function: TestGpuFuncs
float TestGpuFuncs_ComputePieQuadrant(vec2 pos, float numQuadrants) {
  float centered_x = (pos.x - 0.5);
  float centered_y = (pos.y - 0.5);
  float angle = (atan(centered_y, centered_x) + lib_shader_FloatConsts_niPi2);
  float angle_normalized;
  bool _tmp_K = (angle < 0.0);
  if (_tmp_K) {
    angle_normalized = (angle + lib_shader_FloatConsts_ni2Pi);
  }
  else {
    {
      angle_normalized = angle;
    }
  }
  float quadrantF = ((angle_normalized / lib_shader_FloatConsts_ni2Pi) * numQuadrants);
  return floor(quadrantF);
}
// MODULE END TestGpuFuncs

// Pixel Shader main: TestGpuFuncs_VertexOutput_texture_bindless_ps
layout(location = 0) in vec4 IN_0_aInput_position;
layout(location = 1) in vec4 IN_1_aInput_color;
layout(location = 2) in vec2 IN_2_aInput_tex0;
// type size: 48, underlying: float
layout(set = 0, binding = 0) uniform UBO_TestGpuFuncs_TestUniforms { TestGpuFuncs_TestUniforms v; } IN_1_aUniforms;
layout(set = 5, binding = 0) uniform sampler IN_1_aSS;
layout(location = 0) out vec4 OUT_0_rval_color;
void main(void) {
  lib_shader_initialize();
  TestGpuFuncs_VertexOutput aInput;
  TestGpuFuncs_TestUniforms aUniforms;
  aInput.position = IN_0_aInput_position;
  aInput.color = IN_1_aInput_color;
  aInput.tex0 = IN_2_aInput_tex0;
  aUniforms = IN_1_aUniforms.v;
  TestGpuFuncs_PixelOutput _rval_ = TestGpuFuncs_VertexOutput_texture_bindless_ps(aInput, aUniforms, IN_1_aSS);
  OUT_0_rval_color = _rval_.color;
}

