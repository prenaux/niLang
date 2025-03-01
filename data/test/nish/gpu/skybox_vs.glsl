#version 460
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_nonuniform_qualifier : require
// DO IMPORTS BEGIN TestGpuFuncs
// MODULE BEGIN nish:std
// Type: VertexP
struct nish_std_VertexP {
  vec3 position;
};
// Type: VertexOutputPN
struct nish_std_VertexOutputPN {
  vec4 position;
  vec3 normal;
};
// TypeMethFwd: VertexOutputPN
nish_std_VertexOutputPN nish_std_VertexOutputPN_new(vec4 a_position, vec3 a_normal);
// FunctionFwd: nish:std
// TypeMeth: VertexOutputPN
nish_std_VertexOutputPN nish_std_VertexOutputPN_new(vec4 a_position, vec3 a_normal) {
  nish_std_VertexOutputPN t;
  t.position = a_position;
  t.normal = a_normal;
  return t;
}
// Function: nish:std
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
nish_std_VertexOutputPN TestGpuFuncs_skybox_vs(nish_std_VertexP aInput, TestGpuFuncs_TestUniforms aUniforms);

// Function: TestGpuFuncs
nish_std_VertexOutputPN TestGpuFuncs_skybox_vs(nish_std_VertexP aInput, TestGpuFuncs_TestUniforms aUniforms) {
  vec3 _tmp_1 = aInput.position;
  vec4 outPos = vec4(_tmp_1.x,_tmp_1.y,_tmp_1.z,1.0);
  vec4 _tmp_5 = (aUniforms.mtxWVP * outPos);
  vec3 _tmp_9 = normalize((outPos.xyz));
  return nish_std_VertexOutputPN_new(_tmp_5, _tmp_9);
}
// MODULE END TestGpuFuncs

// Vertex Shader main: TestGpuFuncs_skybox_vs
layout(location = 0) in vec3 IN_0_aInput_position;
// type size: 48, underlying: float
layout(set = 0, binding = 0) uniform UBO_TestGpuFuncs_TestUniforms { TestGpuFuncs_TestUniforms v; } IN_1_aUniforms;
// layout() in vec4 gl_Position;
layout(location = 3) out vec3 OUT_3_rval_normal;
void main(void) {
  nish_std_VertexP aInput;
  TestGpuFuncs_TestUniforms aUniforms;
  aInput.position = IN_0_aInput_position;
  aUniforms = IN_1_aUniforms.v;
  nish_std_VertexOutputPN _rval_ = TestGpuFuncs_skybox_vs(aInput, aUniforms);
  gl_Position = _rval_.position;
  OUT_3_rval_normal = _rval_.normal;
}

