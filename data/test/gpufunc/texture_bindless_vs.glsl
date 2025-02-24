#version 460
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_nonuniform_qualifier : require
// DO IMPORTS BEGIN TestGpuFuncs
// MODULE BEGIN lib:shader
// MODULE PROLOGUE BEGIN lib:shader
mat4 nil_Mat4x3ToMat4x4(mat4x3 m) { return mat4(vec4(m[0], 0), vec4(m[1], 0), vec4(m[2], 0), vec4(m[3], 1)); }
mat4 nil_Mat3x4ToMat4x4(mat3x4 m) { return mat4(vec4(m[0]), vec4(m[1]), vec4(m[2]), vec4(0,0,0,1)); }
// MODULE PROLOGUE END lib:shader
// Type: VertexFuncIds
struct lib_shader_VertexFuncIds {
  uint vertexIndex;
  uint firstVertexIndex;
  uint instanceIndex;
  uint firstInstanceIndex;
};
// FunctionFwd: lib:shader
// Function: lib:shader
// MODULE END lib:shader
// DO IMPORTS END TestGpuFuncs

// MODULE BEGIN TestGpuFuncs

// Type: InstanceData
struct TestGpuFuncs_InstanceData {
  mat4 mtxWorld;
  uint texIndex0;
  uint texIndex1;
  uint texIndex2;
  uint texIndex3;
};

// Type: VertexOutput
struct TestGpuFuncs_VertexOutput {
  vec4 position;
  vec4 color;
  vec2 tex0;
};

// Type: VertexPAT1
struct TestGpuFuncs_VertexPAT1 {
  vec3 position;
  vec4 color;
  vec2 tex0;
};

// TypeMethFwd: VertexOutput
TestGpuFuncs_VertexOutput TestGpuFuncs_VertexOutput_new(vec4 a_position, vec4 a_color, vec2 a_tex0);

// TypeMethFwd: VertexPAT1
TestGpuFuncs_VertexOutput TestGpuFuncs_VertexPAT1_texture_bindless_vs(TestGpuFuncs_VertexPAT1 aInput, lib_shader_VertexFuncIds aVertexInfo);

// FunctionFwd: TestGpuFuncs
layout(scalar, set = 9, binding = 0) readonly buffer SBO_TestGpuFuncs_InstanceData { TestGpuFuncs_InstanceData v; } nil_builtin_GetInstanceData[];

// TypeMeth: VertexOutput
TestGpuFuncs_VertexOutput TestGpuFuncs_VertexOutput_new(vec4 a_position, vec4 a_color, vec2 a_tex0) {
  TestGpuFuncs_VertexOutput t;
  t.position = a_position;
  t.color = a_color;
  t.tex0 = a_tex0;
  return t;
}

// TypeMeth: VertexPAT1
TestGpuFuncs_VertexOutput TestGpuFuncs_VertexPAT1_texture_bindless_vs(TestGpuFuncs_VertexPAT1 aInput, lib_shader_VertexFuncIds aVertexInfo) {
  TestGpuFuncs_InstanceData instData = nil_builtin_GetInstanceData[nonuniformEXT(aVertexInfo.instanceIndex)].v;
  vec3 _tmp_3 = aInput.position;
  vec4 outPos = vec4(_tmp_3.x,_tmp_3.y,_tmp_3.z,1.0);
  vec4 outColor = aInput.color;
  vec4 _tmp_8 = (instData.mtxWorld * outPos);
  vec4 _tmp_c = outColor;
  vec2 _tmp_d = aInput.tex0;
  return TestGpuFuncs_VertexOutput_new(_tmp_8, _tmp_c, _tmp_d);
}

// Function: TestGpuFuncs
// MODULE END TestGpuFuncs

// Vertex Shader main: TestGpuFuncs_VertexPAT1_texture_bindless_vs
layout(location = 0) in vec3 IN_0_aInput_position;
layout(location = 4) in vec4 IN_4_aInput_color;
layout(location = 5) in vec2 IN_5_aInput_tex0;
// layout() in vec4 gl_Position;
layout(location = 1) out vec4 OUT_1_rval_color;
layout(location = 2) out vec2 OUT_2_rval_tex0;
void main(void) {
  TestGpuFuncs_VertexPAT1 aInput;
  lib_shader_VertexFuncIds aVertexInfo;
  aInput.position = IN_0_aInput_position;
  aInput.color = IN_4_aInput_color;
  aInput.tex0 = IN_5_aInput_tex0;
  aVertexInfo.vertexIndex = uint(gl_VertexIndex);
  aVertexInfo.firstVertexIndex = uint(gl_BaseVertex);
  aVertexInfo.instanceIndex = uint(gl_InstanceIndex);
  aVertexInfo.firstInstanceIndex = uint(gl_BaseInstance);
  TestGpuFuncs_VertexOutput _rval_ = TestGpuFuncs_VertexPAT1_texture_bindless_vs(aInput, aVertexInfo);
  gl_Position = _rval_.position;
  OUT_1_rval_color = _rval_.color;
  OUT_2_rval_tex0 = _rval_.tex0;
}

