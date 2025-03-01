#version 460
#extension GL_EXT_nonuniform_qualifier : require
// DO IMPORTS BEGIN TestGpuFuncs
// MODULE BEGIN nish:std
// Type: VertexOutputPN
struct nish_std_VertexOutputPN {
  vec4 position;
  vec3 normal;
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
// DO IMPORTS END TestGpuFuncs

// MODULE BEGIN TestGpuFuncs

// FunctionFwd: TestGpuFuncs
nish_std_PixelOutput TestGpuFuncs_skybox_ps(nish_std_VertexOutputPN aInput, textureCube aTexture, sampler aSS);

// Function: TestGpuFuncs
nish_std_PixelOutput TestGpuFuncs_skybox_ps(nish_std_VertexOutputPN aInput, textureCube aTexture, sampler aSS) {
  vec4 texColor = texture(samplerCube(aTexture,aSS),aInput.normal);
  vec4 _tmp_6 = texColor;
  return nish_std_PixelOutput_new(_tmp_6);
}
// MODULE END TestGpuFuncs

// Pixel Shader main: TestGpuFuncs_skybox_ps
layout(location = 0) in vec4 IN_0_aInput_position;
layout(location = 3) in vec3 IN_3_aInput_normal;
layout(set = 2, binding = 0) uniform textureCube IN_1_aTexture;
layout(set = 5, binding = 0) uniform sampler IN_1_aSS;
layout(location = 0) out vec4 OUT_0_rval_color;
void main(void) {
  nish_std_VertexOutputPN aInput;
  aInput.position = IN_0_aInput_position;
  aInput.normal = IN_3_aInput_normal;
  nish_std_PixelOutput _rval_ = TestGpuFuncs_skybox_ps(aInput, IN_1_aTexture, IN_1_aSS);
  OUT_0_rval_color = _rval_.color;
}

