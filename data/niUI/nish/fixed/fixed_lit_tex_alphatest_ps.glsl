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
// TypeStaticFwd: Vec3
vec3 vec3_Zero;
// TypeMethFwd: PixelOutput
nish_std_PixelOutput nish_std_PixelOutput_new(vec4 a_color);
// FunctionFwd: nish:std
void nish_std_AlphaTest(float aAlphaValue, float aAlphaRef);
void vec3_static_initialize() {
  // TypeStatic: Vec3
  vec3_Zero = vec3(0.0,0.0,0.0);
}
// TypeMeth: PixelOutput
nish_std_PixelOutput nish_std_PixelOutput_new(vec4 a_color) {
  nish_std_PixelOutput t;
  t.color = a_color;
  return t;
}
// Function: nish:std
void nish_std_AlphaTest(float aAlphaValue, float aAlphaRef) {
  bool _tmp_3 = (aAlphaValue < aAlphaRef);
  if (_tmp_3) {
    discard;
  }
}
// ModuleInitialize: nish_std
void nish_std_initialize() {
  vec3_static_initialize();
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
float niUIGpuFuncs_CosineBiasSat(float v, float b);
vec3 niUIGpuFuncs_DirLight(vec3 worldPos, vec3 worldNormal, vec3 worldLightDir, vec3 lightColor, float cosBias, vec3 shadowColor);
vec3 niUIGpuFuncs_ComputeFixedLighting(vec3 aWorldPos, vec3 aWorldNormal);
nish_std_PixelOutput niUIGpuFuncs_fixed_lit_tex_alphatest_ps(nish_std_VertexOutput aInput, niUIGpuFuncs_FixedUniforms aUniforms, texture2D aTexture, sampler aSS);

// Function: niUIGpuFuncs
float niUIGpuFuncs_CosineBiasSat(float v, float b) {
  return clamp(max(0.0,((v * (1.0 - b)) + b)),0.0,1.0);
}
vec3 niUIGpuFuncs_DirLight(vec3 worldPos, vec3 worldNormal, vec3 worldLightDir, vec3 lightColor, float cosBias, vec3 shadowColor) {
  float att = 1.0;
  vec3 D = lightColor;
  vec3 S = vec3_Zero;
  float NdotL = niUIGpuFuncs_CosineBiasSat(dot(worldNormal,-worldLightDir),cosBias);
  vec3 C = (D+S);
  return mix((shadowColor*C),C,(att * NdotL));
}
vec3 niUIGpuFuncs_ComputeFixedLighting(vec3 aWorldPos, vec3 aWorldNormal) {
  float cosBias = 0.5;
  vec3 shadowColor = vec3(0.3,0.3,0.3);
  vec3 lightColor0 = vec3(0.8,0.8,0.8);
  vec3 worldLightDir0 = normalize(vec3(-0.5,-0.7,0.9));
  vec3 lightColor1 = vec3(0.4,0.2,0.0);
  vec3 worldLightDir1 = normalize(vec3(0.707107,0.0,0.707107));
  vec3 lightColor2 = vec3(0.0,0.2,0.4);
  vec3 worldLightDir2 = vec3(0.0,-1.0,0.0);
  vec3 worldPos = aWorldPos;
  vec3 worldNormal = normalize(aWorldNormal);
  vec3 C = ((niUIGpuFuncs_DirLight(worldPos,worldNormal,worldLightDir0,lightColor0,cosBias,shadowColor)+niUIGpuFuncs_DirLight(worldPos,worldNormal,worldLightDir1,lightColor1,cosBias,shadowColor))+niUIGpuFuncs_DirLight(worldPos,worldNormal,worldLightDir2,lightColor2,cosBias,shadowColor));
  return C;
}
nish_std_PixelOutput niUIGpuFuncs_fixed_lit_tex_alphatest_ps(nish_std_VertexOutput aInput, niUIGpuFuncs_FixedUniforms aUniforms, texture2D aTexture, sampler aSS) {
  vec4 texColor = texture(sampler2D(aTexture,aSS),aInput.tex0);
  nish_std_AlphaTest(texColor.w,aUniforms.alphaRef);
  vec3 diffuse = niUIGpuFuncs_ComputeFixedLighting((aInput.position.xyz),aInput.normal);
  vec3 _tmp_u1 = diffuse;
  vec4 _tmp_r1 = ((texColor*aInput.color)*vec4(_tmp_u1.x,_tmp_u1.y,_tmp_u1.z,1.0));
  return nish_std_PixelOutput_new(_tmp_r1);
}
// MODULE END niUIGpuFuncs

// Pixel Shader main: niUIGpuFuncs_fixed_lit_tex_alphatest_ps
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
  nish_std_initialize();
  nish_std_VertexOutput aInput;
  niUIGpuFuncs_FixedUniforms aUniforms;
  aInput.position = IN_0_aInput_position;
  aInput.normal = IN_3_aInput_normal;
  aInput.color = IN_1_aInput_color;
  aInput.tex0 = IN_2_aInput_tex0;
  aUniforms = IN_1_aUniforms.v;
  nish_std_PixelOutput _rval_ = niUIGpuFuncs_fixed_lit_tex_alphatest_ps(aInput, aUniforms, IN_1_aTexture, IN_1_aSS);
  OUT_0_rval_color = _rval_.color;
}

