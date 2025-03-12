#version 460
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_ray_query : require
// DO IMPORTS BEGIN TestGpuFuncs
// MODULE BEGIN nish:std
// Type: PixelInput
struct nish_std_PixelInput {
  vec4 fragCoord;
  bool frontFacing;
};
// Type: PixelOutput
struct nish_std_PixelOutput {
  vec4 color;
};
// TypeStaticFwd: RayFlags
uint nish_std_RayFlags_CullBackFacingTriangles;
// TypeStaticFwd: RayQueryIntersectionType
uint nish_std_RayQueryIntersectionType_CommittedTriangle;
uint nish_std_RayQueryIntersectionType_CommittedBoundingVolume;
// TypeMethFwd: PixelOutput
nish_std_PixelOutput nish_std_PixelOutput_new(vec4 a_color);
// FunctionFwd: nish:std
vec3 nish_std_Vec3TransformCoord(vec3 v, mat4 m);
void nish_std_RayFlags_static_initialize() {
  // TypeStatic: RayFlags
  nish_std_RayFlags_CullBackFacingTriangles = 16;
}
void nish_std_RayQueryIntersectionType_static_initialize() {
  // TypeStatic: RayQueryIntersectionType
  nish_std_RayQueryIntersectionType_CommittedTriangle = 1;
  nish_std_RayQueryIntersectionType_CommittedBoundingVolume = 2;
}
// TypeMeth: PixelOutput
nish_std_PixelOutput nish_std_PixelOutput_new(vec4 a_color) {
  nish_std_PixelOutput t;
  t.color = a_color;
  return t;
}
// Function: nish:std
vec3 nish_std_Vec3TransformCoord(vec3 v, mat4 m) {
  vec3 _tmp_3 = v;
  vec4 r = (m * vec4(_tmp_3.x,_tmp_3.y,_tmp_3.z,1.0));
  return ((r/r.w).xyz);
}
// ModuleInitialize: nish_std
void nish_std_initialize() {
  nish_std_RayFlags_static_initialize();
  nish_std_RayQueryIntersectionType_static_initialize();
}
// MODULE END nish:std
// DO IMPORTS END TestGpuFuncs

// MODULE BEGIN TestGpuFuncs

// Type: RayUniforms
struct TestGpuFuncs_RayUniforms {
  float rtWidth;
  float rtHeight;
  float cameraFarClipPlane;
  float padding;
  mat4 cameraInvView;
  mat4 cameraInvViewProj;
};

// Type: RayInstanceData
struct TestGpuFuncs_RayInstanceData {
  uint ibIndex;
  uint vbIndex;
  uint texIndex;
  uint firstIndex;
  uint baseVertexIndex;
};

// Type: VertexRay
struct TestGpuFuncs_VertexRay {
  vec3 pos;
  vec3 normal;
  uint color;
  vec2 tex0;
};

// FunctionFwd: TestGpuFuncs
layout(scalar, set = 9, binding = 0) readonly buffer SBO_TestGpuFuncs_RayInstanceData { TestGpuFuncs_RayInstanceData v; } nil_builtin_GetRayInstanceData[];
layout(scalar, set = 9, binding = 0) readonly buffer SBO_GetIndex { uint v[]; } nil_builtin_GetIndex[];
layout(scalar, set = 9, binding = 0) readonly buffer SBO_GetVertexRay { TestGpuFuncs_VertexRay v[]; } nil_builtin_GetVertexRay[];
uvec3 TestGpuFuncs_GetTriangleIndices(uint aIBIndex, uint aPrimIndex, uint aFirstIndex);
vec3 TestGpuFuncs_NormalToColor(vec3 n);
vec3 TestGpuFuncs_BaryToVec3(vec2 aBary);
vec3 TestGpuFuncs_Lerp_4_Vec3_Vec3_Vec3_Vec3(vec3 aX, vec3 aY, vec3 aZ, vec3 aBary);
void TestGpuFuncs_InitRayQuery(/* mut */ rayQueryEXT aRayQuery, nish_std_PixelInput aInput, TestGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS);
nish_std_PixelOutput TestGpuFuncs_raytracer_normals_obj_ps(nish_std_PixelInput aInput, TestGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS);

// Function: TestGpuFuncs
uvec3 TestGpuFuncs_GetTriangleIndices(uint aIBIndex, uint aPrimIndex, uint aFirstIndex) {
  uint ibase = (aFirstIndex + (3 * aPrimIndex));
  return uvec3(nil_builtin_GetIndex[nonuniformEXT(aIBIndex)].v[nonuniformEXT((ibase + 0))],nil_builtin_GetIndex[nonuniformEXT(aIBIndex)].v[nonuniformEXT((ibase + 1))],nil_builtin_GetIndex[nonuniformEXT(aIBIndex)].v[nonuniformEXT((ibase + 2))]);
}
vec3 TestGpuFuncs_NormalToColor(vec3 n) {
  return ((n*0.5)+0.5);
}
vec3 TestGpuFuncs_BaryToVec3(vec2 aBary) {
  return vec3(((1.0 - aBary.x) - aBary.y),aBary.x,aBary.y);
}
vec3 TestGpuFuncs_Lerp_4_Vec3_Vec3_Vec3_Vec3(vec3 aX, vec3 aY, vec3 aZ, vec3 aBary) {
  return (((aX*aBary.x)+(aY*aBary.y))+(aZ*aBary.z));
}
void TestGpuFuncs_InitRayQuery(/* mut */ rayQueryEXT aRayQuery, nish_std_PixelInput aInput, TestGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS) {
  vec3 ndc = vec3((((aInput.fragCoord.x / aUniforms.rtWidth) * 2.0) - 1.0),(1.0 - ((aInput.fragCoord.y / aUniforms.rtHeight) * 2.0)),1.0);
  mat4 _tmp_c1 = aUniforms.cameraInvView;
  vec3 origin = vec3(_tmp_c1[3][0],_tmp_c1[3][1],_tmp_c1[3][2]);
  vec3 target = nish_std_Vec3TransformCoord(ndc,aUniforms.cameraInvViewProj);
  vec3 dir = normalize(((target-(origin.xyz)).xyz));
  rayQueryInitializeEXT(aRayQuery,aAS,nish_std_RayFlags_CullBackFacingTriangles,255,(origin.xyz),0.001,(dir.xyz),aUniforms.cameraFarClipPlane);
}
nish_std_PixelOutput TestGpuFuncs_raytracer_normals_obj_ps(nish_std_PixelInput aInput, TestGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS) {
  /* mut */ rayQueryEXT rayQuery/*__noinit__*/;
  TestGpuFuncs_InitRayQuery(rayQuery,aInput,aUniforms,aAS);
  bool done = rayQueryProceedEXT(rayQuery);
  uint intersectionType = rayQueryGetIntersectionTypeEXT(rayQuery,true);
  uint userInstIndex = uint(rayQueryGetIntersectionInstanceCustomIndexEXT(rayQuery,true));
  TestGpuFuncs_RayInstanceData instData = nil_builtin_GetRayInstanceData[nonuniformEXT(userInstIndex)].v;
  vec4 color;
  bool _tmp_F1 = (intersectionType == nish_std_RayQueryIntersectionType_CommittedTriangle);
  if (_tmp_F1) {
    bool _tmp_I1 = (instData.vbIndex == 0);
    if (_tmp_I1) {
      color = vec4(1.0,0.0,1.0,1.0);
    }
    else {
      {
        uint primIndex = uint(rayQueryGetIntersectionPrimitiveIndexEXT(rayQuery,true));
        uvec3 inds = TestGpuFuncs_GetTriangleIndices(instData.ibIndex,primIndex,instData.firstIndex);
        TestGpuFuncs_VertexRay v0 = nil_builtin_GetVertexRay[nonuniformEXT(instData.vbIndex)].v[nonuniformEXT((instData.baseVertexIndex + inds.x))];
        TestGpuFuncs_VertexRay v1 = nil_builtin_GetVertexRay[nonuniformEXT(instData.vbIndex)].v[nonuniformEXT((instData.baseVertexIndex + inds.y))];
        TestGpuFuncs_VertexRay v2 = nil_builtin_GetVertexRay[nonuniformEXT(instData.vbIndex)].v[nonuniformEXT((instData.baseVertexIndex + inds.z))];
        vec3 bary = TestGpuFuncs_BaryToVec3(rayQueryGetIntersectionBarycentricsEXT(rayQuery,true));
        vec3 n = TestGpuFuncs_Lerp_4_Vec3_Vec3_Vec3_Vec3(v0.normal,v1.normal,v2.normal,bary);
        vec3 _tmp_y2 = TestGpuFuncs_NormalToColor(n);
        color = vec4(_tmp_y2.x,_tmp_y2.y,_tmp_y2.z,1.0);
      }
    }
  }
  else {
    bool _tmp_A2 = (intersectionType == nish_std_RayQueryIntersectionType_CommittedBoundingVolume);
    if (_tmp_A2) {
      color = vec4(1.0,0.0,0.0,1.0);
    }
    else {
      {
        color = vec4(0.0,0.5,0.8,0.0);
      }
    }
  }
  vec4 _tmp_N2 = color;
  return nish_std_PixelOutput_new(_tmp_N2);
}
// MODULE END TestGpuFuncs

// Pixel Shader main: TestGpuFuncs_raytracer_normals_obj_ps
// type size: 48, underlying: float
layout(set = 0, binding = 0) uniform UBO_TestGpuFuncs_RayUniforms { TestGpuFuncs_RayUniforms v; } IN_1_aUniforms;
layout(set = 7, binding = 0) uniform accelerationStructureEXT IN_1_aAS;
layout(location = 0) out vec4 OUT_0_rval_color;
void main(void) {
  nish_std_initialize();
  nish_std_PixelInput aInput;
  TestGpuFuncs_RayUniforms aUniforms;
  aInput.fragCoord = gl_FragCoord;
  aInput.frontFacing = gl_FrontFacing;
  aUniforms = IN_1_aUniforms.v;
  nish_std_PixelOutput _rval_ = TestGpuFuncs_raytracer_normals_obj_ps(aInput, aUniforms, IN_1_aAS);
  OUT_0_rval_color = _rval_.color;
}

