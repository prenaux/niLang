#version 450
// Module: lib:shader
// FunctionFwd: lib:shader
// Function: lib:shader

// Module: TestGpuFuncs

// Type: TestUniforms
struct TestGpuFuncs_TestUniforms {
  mat4 mtxWVP;
  float alphaRef;
  float padding0;
  float padding1;
  float padding2;
  vec4 materialColor;
};

// Type: VertexOutputPN
struct TestGpuFuncs_VertexOutputPN {
  vec4 position;
  vec3 normal;
};

// Type: VertexP
struct TestGpuFuncs_VertexP {
  vec3 position;
};

// TypeMethFwd: VertexOutputPN
TestGpuFuncs_VertexOutputPN TestGpuFuncs_VertexOutputPN_new(vec4 a_position, vec3 a_normal);

// TypeMethFwd: VertexP
TestGpuFuncs_VertexOutputPN TestGpuFuncs_VertexP_skybox_vs(TestGpuFuncs_VertexP aInput, TestGpuFuncs_TestUniforms aUniforms);

// FunctionFwd: TestGpuFuncs

// TypeMeth: VertexOutputPN
TestGpuFuncs_VertexOutputPN TestGpuFuncs_VertexOutputPN_new(vec4 a_position, vec3 a_normal) {
  TestGpuFuncs_VertexOutputPN t;
  t.position = a_position;
  t.normal = a_normal;
  return t;
}

// TypeMeth: VertexP
TestGpuFuncs_VertexOutputPN TestGpuFuncs_VertexP_skybox_vs(TestGpuFuncs_VertexP aInput, TestGpuFuncs_TestUniforms aUniforms) {
  vec3 _tmp_1 = aInput.position;
  vec4 outPos = vec4(_tmp_1.x,_tmp_1.y,_tmp_1.z,1.0);
  vec4 _tmp_5 = (aUniforms.mtxWVP * outPos);
  vec3 _tmp_9 = normalize((outPos.xyz));
  return TestGpuFuncs_VertexOutputPN_new(_tmp_5, _tmp_9);
}

// Function: TestGpuFuncs

// Vertex Shader main: TestGpuFuncs_VertexP_skybox_vs
layout(location = 0) in vec3 IN_0_aInput_position;
// type size: 48
layout(set = 0, binding = 0) uniform UBO_TestGpuFuncs_TestUniforms { TestGpuFuncs_TestUniforms v; } IN_1_aUniforms;
// layout() in vec4 gl_Position;
layout(location = 3) out vec3 OUT_3_rval_normal;
void main(void) {
  TestGpuFuncs_VertexP aInput;
  TestGpuFuncs_TestUniforms aUniforms;
  aInput.position = IN_0_aInput_position;
  aUniforms = IN_1_aUniforms.v;
  TestGpuFuncs_VertexOutputPN _rval_ = TestGpuFuncs_VertexP_skybox_vs(aInput, aUniforms);
  gl_Position = _rval_.position;
  OUT_3_rval_normal = _rval_.normal;
}

