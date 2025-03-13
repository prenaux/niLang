#version 460
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_nonuniform_qualifier : require
// DO IMPORTS BEGIN TestGpuFuncs
// MODULE BEGIN nish:std
// Type: VertexPAT1
struct nish_std_VertexPAT1 {
  vec3 position;
  vec4 color;
  vec2 tex0;
};
// Type: VertexFuncIds
struct nish_std_VertexFuncIds {
  uint vertexIndex;
  uint firstVertexIndex;
  uint instanceIndex;
  uint firstInstanceIndex;
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
// DO IMPORTS END TestGpuFuncs

// MODULE BEGIN TestGpuFuncs

// Type: TestInstanceData
struct TestGpuFuncs_TestInstanceData {
  mat4 mtxWorld;
  uint texIndex0;
  uint texIndex1;
  uint texIndex2;
  uint texIndex3;
};

// FunctionFwd: TestGpuFuncs
layout(scalar, set = 9, binding = 0) readonly buffer SBO_TestGpuFuncs_TestInstanceData { TestGpuFuncs_TestInstanceData v; } nil_builtin_GetTestInstanceData[];
nish_std_VertexOutput TestGpuFuncs_texture_bindless_vs(nish_std_VertexPAT1 aInput, nish_std_VertexFuncIds aVertexInfo);

// Function: TestGpuFuncs
nish_std_VertexOutput TestGpuFuncs_texture_bindless_vs(nish_std_VertexPAT1 aInput, nish_std_VertexFuncIds aVertexInfo) {
  TestGpuFuncs_TestInstanceData instData = nil_builtin_GetTestInstanceData[nonuniformEXT(aVertexInfo.instanceIndex)].v;
  vec3 _tmp_3 = aInput.position;
  vec4 outPos = vec4(_tmp_3.x,_tmp_3.y,_tmp_3.z,1.0);
  vec4 outColor = aInput.color;
  vec4 _tmp_8 = (instData.mtxWorld * outPos);
  vec4 _tmp_c = outColor;
  vec2 _tmp_d = aInput.tex0;
  return nish_std_VertexOutput_new(_tmp_8, _tmp_c, _tmp_d);
}
// MODULE END TestGpuFuncs

// Vertex Shader main: TestGpuFuncs_texture_bindless_vs
layout(location = 0) in vec3 IN_0_aInput_position;
layout(location = 4) in vec4 IN_4_aInput_color;
layout(location = 5) in vec2 IN_5_aInput_tex0;
// layout() in vec4 gl_Position;
layout(location = 1) out vec4 OUT_1_rval_color;
layout(location = 2) out vec2 OUT_2_rval_tex0;
void main(void) {
  nish_std_VertexPAT1 aInput;
  nish_std_VertexFuncIds aVertexInfo;
  aInput.position = IN_0_aInput_position;
  aInput.color = IN_4_aInput_color;
  aInput.tex0 = IN_5_aInput_tex0;
  aVertexInfo.vertexIndex = uint(gl_VertexIndex);
  aVertexInfo.firstVertexIndex = uint(gl_BaseVertex);
  aVertexInfo.instanceIndex = uint(gl_InstanceIndex);
  aVertexInfo.firstInstanceIndex = uint(gl_BaseInstance);
  nish_std_VertexOutput _rval_ = TestGpuFuncs_texture_bindless_vs(aInput, aVertexInfo);
  gl_Position = _rval_.position;
  OUT_1_rval_color = _rval_.color;
  OUT_2_rval_tex0 = _rval_.tex0;
}

