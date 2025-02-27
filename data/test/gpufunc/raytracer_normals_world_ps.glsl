#version 460
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_ray_query : require
// DO IMPORTS BEGIN TestGpuFuncs
// MODULE BEGIN lib:shader
// Type: PixelInput
struct lib_shader_PixelInput {
  vec4 fragCoord;
  bool frontFacing;
};
// TypeStaticFwd: RayFlags
uint lib_shader_RayFlags_None;
// TypeStaticFwd: RayQueryIntersectionType
uint lib_shader_RayQueryIntersectionType_CommittedTriangle;
uint lib_shader_RayQueryIntersectionType_CommittedBoundingVolume;
// FunctionFwd: lib:shader
vec3 lib_shader_Vec3TransformCoord(vec3 v, mat4 m);
vec3 lib_shader_Vec3TransformNormal_2_Vec3_Matrix4x3(vec3 v, mat4x3 m);
void lib_shader_RayFlags_static_initialize() {
  // TypeStatic: RayFlags
  lib_shader_RayFlags_None = 0;
}
void lib_shader_RayQueryIntersectionType_static_initialize() {
  // TypeStatic: RayQueryIntersectionType
  lib_shader_RayQueryIntersectionType_CommittedTriangle = 1;
  lib_shader_RayQueryIntersectionType_CommittedBoundingVolume = 2;
}
// Function: lib:shader
vec3 lib_shader_Vec3TransformCoord(vec3 v, mat4 m) {
  vec3 _tmp_3 = v;
  vec4 r = (m * vec4(_tmp_3.x,_tmp_3.y,_tmp_3.z,1.0));
  return ((r/r.w).xyz);
}
vec3 lib_shader_Vec3TransformNormal_2_Vec3_Matrix4x3(vec3 v, mat4x3 m) {
  vec3 _tmp_c = v;
  vec3 r = (m * vec4(_tmp_c.x,_tmp_c.y,_tmp_c.z,0.0));
  return (r.xyz);
}
// ModuleInitialize: lib_shader
void lib_shader_initialize() {
  lib_shader_RayFlags_static_initialize();
  lib_shader_RayQueryIntersectionType_static_initialize();
}
// MODULE END lib:shader
// DO IMPORTS END TestGpuFuncs

// MODULE BEGIN TestGpuFuncs

// Type: PixelOutput
struct TestGpuFuncs_PixelOutput {
  vec4 color;
};

// Type: RayInstanceData
struct TestGpuFuncs_RayInstanceData {
  uint ibIndex;
  uint vbIndex;
  uint texIndex;
};

// Type: RayUniforms
struct TestGpuFuncs_RayUniforms {
  float rtWidth;
  float rtHeight;
  float cameraFarClipPlane;
  float padding;
  mat4 cameraInvView;
  mat4 cameraInvViewProj;
};

// Type: VertexRay
struct TestGpuFuncs_VertexRay {
  vec3 pos;
  vec3 normal;
  vec2 tex0;
};

// TypeMethFwd: PixelOutput
TestGpuFuncs_PixelOutput TestGpuFuncs_PixelOutput_new(vec4 a_color);

// FunctionFwd: TestGpuFuncs
vec3 TestGpuFuncs_BaryToVec3(vec2 aBary);
layout(scalar, set = 9, binding = 0) readonly buffer SBO_GetIndex { uint v[]; } nil_builtin_GetIndex[];
layout(scalar, set = 9, binding = 0) readonly buffer SBO_TestGpuFuncs_RayInstanceData { TestGpuFuncs_RayInstanceData v; } nil_builtin_GetRayInstanceData[];
uvec3 TestGpuFuncs_GetTriangleIndices(uint aIBIndex, uint aPrimIndex);
layout(scalar, set = 9, binding = 0) readonly buffer SBO_GetVertexRay { TestGpuFuncs_VertexRay v[]; } nil_builtin_GetVertexRay[];
void TestGpuFuncs_InitRayQuery(rayQueryEXT aRayQuery, lib_shader_PixelInput aInput, TestGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS);
vec3 TestGpuFuncs_Lerp_4_Vec3_Vec3_Vec3_Vec3(vec3 aX, vec3 aY, vec3 aZ, vec3 aBary);
vec3 TestGpuFuncs_NormalToColor(vec3 n);
TestGpuFuncs_PixelOutput TestGpuFuncs_raytracer_normals_world_ps(lib_shader_PixelInput aInput, TestGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS);

// TypeMeth: PixelOutput
TestGpuFuncs_PixelOutput TestGpuFuncs_PixelOutput_new(vec4 a_color) {
  TestGpuFuncs_PixelOutput t;
  t.color = a_color;
  return t;
}

// Function: TestGpuFuncs
vec3 TestGpuFuncs_BaryToVec3(vec2 aBary) {
  return vec3(((1.0 - aBary.x) - aBary.y),aBary.x,aBary.y);
}
uvec3 TestGpuFuncs_GetTriangleIndices(uint aIBIndex, uint aPrimIndex) {
  uint ibase = (3 * aPrimIndex);
  return uvec3(nil_builtin_GetIndex[nonuniformEXT(aIBIndex)].v[nonuniformEXT((ibase + 0))],nil_builtin_GetIndex[nonuniformEXT(aIBIndex)].v[nonuniformEXT((ibase + 1))],nil_builtin_GetIndex[nonuniformEXT(aIBIndex)].v[nonuniformEXT((ibase + 2))]);
}
void TestGpuFuncs_InitRayQuery(rayQueryEXT aRayQuery, lib_shader_PixelInput aInput, TestGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS) {
  vec3 ndc = vec3((((aInput.fragCoord.x / aUniforms.rtWidth) * 2.0) - 1.0),(1.0 - ((aInput.fragCoord.y / aUniforms.rtHeight) * 2.0)),1.0);
  mat4 _tmp_Q = aUniforms.cameraInvView;
  vec3 origin = vec3(_tmp_Q[3][0],_tmp_Q[3][1],_tmp_Q[3][2]);
  vec3 target = lib_shader_Vec3TransformCoord(ndc,aUniforms.cameraInvViewProj);
  vec3 dir = normalize(((target-(origin.xyz)).xyz));
  rayQueryInitializeEXT(aRayQuery,aAS,lib_shader_RayFlags_None,255,(origin.xyz),0.001,(dir.xyz),aUniforms.cameraFarClipPlane);
}
vec3 TestGpuFuncs_Lerp_4_Vec3_Vec3_Vec3_Vec3(vec3 aX, vec3 aY, vec3 aZ, vec3 aBary) {
  return (((aX*aBary.x)+(aY*aBary.y))+(aZ*aBary.z));
}
vec3 TestGpuFuncs_NormalToColor(vec3 n) {
  return ((n*0.5)+0.5);
}
TestGpuFuncs_PixelOutput TestGpuFuncs_raytracer_normals_world_ps(lib_shader_PixelInput aInput, TestGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS) {
  rayQueryEXT rayQuery/*__noinit__*/;
  TestGpuFuncs_InitRayQuery(rayQuery,aInput,aUniforms,aAS);
  bool done = rayQueryProceedEXT(rayQuery);
  uint intersectionType = rayQueryGetIntersectionTypeEXT(rayQuery,true);
  uint userInstIndex = uint(rayQueryGetIntersectionInstanceCustomIndexEXT(rayQuery,true));
  TestGpuFuncs_RayInstanceData instData = nil_builtin_GetRayInstanceData[nonuniformEXT(userInstIndex)].v;
  vec4 color;
  bool _tmp_D1 = (intersectionType == lib_shader_RayQueryIntersectionType_CommittedTriangle);
  if (_tmp_D1) {
    bool _tmp_G1 = (instData.vbIndex == 0);
    if (_tmp_G1) {
      color = vec4(1.0,0.0,1.0,1.0);
    }
    else {
      {
        uint primIndex = uint(rayQueryGetIntersectionPrimitiveIndexEXT(rayQuery,true));
        uvec3 inds = TestGpuFuncs_GetTriangleIndices(instData.ibIndex,primIndex);
        TestGpuFuncs_VertexRay v0 = nil_builtin_GetVertexRay[nonuniformEXT(instData.vbIndex)].v[nonuniformEXT(inds.x)];
        TestGpuFuncs_VertexRay v1 = nil_builtin_GetVertexRay[nonuniformEXT(instData.vbIndex)].v[nonuniformEXT(inds.y)];
        TestGpuFuncs_VertexRay v2 = nil_builtin_GetVertexRay[nonuniformEXT(instData.vbIndex)].v[nonuniformEXT(inds.z)];
        vec3 bary = TestGpuFuncs_BaryToVec3(rayQueryGetIntersectionBarycentricsEXT(rayQuery,true));
        vec3 n = normalize(TestGpuFuncs_Lerp_4_Vec3_Vec3_Vec3_Vec3(v0.normal,v1.normal,v2.normal,bary));
        vec3 worldNormal = lib_shader_Vec3TransformNormal_2_Vec3_Matrix4x3(n,rayQueryGetIntersectionObjectToWorldEXT(rayQuery,true));
        vec3 _tmp_q2 = TestGpuFuncs_NormalToColor(worldNormal);
        color = vec4(_tmp_q2.x,_tmp_q2.y,_tmp_q2.z,1.0);
      }
    }
  }
  else {
    bool _tmp_s2 = (intersectionType == lib_shader_RayQueryIntersectionType_CommittedBoundingVolume);
    if (_tmp_s2) {
      color = vec4(1.0,0.0,0.0,1.0);
    }
    else {
      {
        color = vec4(0.0,0.5,0.8,0.0);
      }
    }
  }
  vec4 _tmp_F2 = color;
  return TestGpuFuncs_PixelOutput_new(_tmp_F2);
}
// MODULE END TestGpuFuncs

// Pixel Shader main: TestGpuFuncs_raytracer_normals_world_ps
// type size: 48, underlying: float
layout(set = 0, binding = 0) uniform UBO_TestGpuFuncs_RayUniforms { TestGpuFuncs_RayUniforms v; } IN_1_aUniforms;
layout(set = 7, binding = 0) uniform accelerationStructureEXT IN_1_aAS;
layout(location = 0) out vec4 OUT_0_rval_color;
void main(void) {
  lib_shader_initialize();
  lib_shader_PixelInput aInput;
  TestGpuFuncs_RayUniforms aUniforms;
  aInput.fragCoord = gl_FragCoord;
  aInput.frontFacing = gl_FrontFacing;
  aUniforms = IN_1_aUniforms.v;
  TestGpuFuncs_PixelOutput _rval_ = TestGpuFuncs_raytracer_normals_world_ps(aInput, aUniforms, IN_1_aAS);
  OUT_0_rval_color = _rval_.color;
}

