#version 450
#extension GL_EXT_nonuniform_qualifier : require
// DO IMPORTS BEGIN TestGpuFuncs
// MODULE BEGIN lib:shader
// MODULE PROLOGUE BEGIN lib:shader
mat4 nil_Mat4x3ToMat4x4(mat4x3 m) { return mat4(vec4(m[0], 0), vec4(m[1], 0), vec4(m[2], 0), vec4(m[3], 1)); }
mat4 nil_Mat3x4ToMat4x4(mat3x4 m) { return mat4(vec4(m[0]), vec4(m[1]), vec4(m[2]), vec4(0,0,0,1)); }
// MODULE PROLOGUE END lib:shader
// FunctionFwd: lib:shader
// Function: lib:shader
layout(set = 10, binding = 0) uniform texture2D nil_builtin_AllTextures2D[];
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

// TypeMeth: PixelOutput
TestGpuFuncs_PixelOutput TestGpuFuncs_PixelOutput_new(vec4 a_color) {
  TestGpuFuncs_PixelOutput t;
  t.color = a_color;
  return t;
}

// TypeMeth: VertexOutput
TestGpuFuncs_PixelOutput TestGpuFuncs_VertexOutput_texture_bindless_ps(TestGpuFuncs_VertexOutput aInput, TestGpuFuncs_TestUniforms aUniforms, sampler aSS) {
  vec2 texCoo = aInput.tex0;
  vec4 texColor;
  bool _tmp_1 = (texCoo.x < 0.3);
  if (_tmp_1) {
    texColor = texture(sampler2D(nil_builtin_AllTextures2D[uint(aUniforms.padding0)],aSS),aInput.tex0);
  }
  else {
    bool _tmp_c = (texCoo.x < 0.7);
    if (_tmp_c) {
      texColor = texture(sampler2D(nil_builtin_AllTextures2D[uint(aUniforms.padding1)],aSS),aInput.tex0);
    }
    else {
      {
        texColor = texture(sampler2D(nil_builtin_AllTextures2D[uint(aUniforms.padding2)],aSS),aInput.tex0);
      }
    }
  }
  vec4 _tmp_w = (texColor*aInput.color);
  return TestGpuFuncs_PixelOutput_new(_tmp_w);
}

// Function: TestGpuFuncs
// MODULE END TestGpuFuncs

// Pixel Shader main: TestGpuFuncs_VertexOutput_texture_bindless_ps
layout(location = 0) in vec4 IN_0_aInput_position;
layout(location = 1) in vec4 IN_1_aInput_color;
layout(location = 2) in vec2 IN_2_aInput_tex0;
// type size: 48
layout(set = 0, binding = 0) uniform UBO_TestGpuFuncs_TestUniforms { TestGpuFuncs_TestUniforms v; } IN_1_aUniforms;
layout(set = 5, binding = 0) uniform sampler IN_1_aSS;
layout(location = 0) out vec4 OUT_0_rval_color;
void main(void) {
  TestGpuFuncs_VertexOutput aInput;
  TestGpuFuncs_TestUniforms aUniforms;
  aInput.position = IN_0_aInput_position;
  aInput.color = IN_1_aInput_color;
  aInput.tex0 = IN_2_aInput_tex0;
  aUniforms = IN_1_aUniforms.v;
  TestGpuFuncs_PixelOutput _rval_ = TestGpuFuncs_VertexOutput_texture_bindless_ps(aInput, aUniforms, IN_1_aSS);
  OUT_0_rval_color = _rval_.color;
}

