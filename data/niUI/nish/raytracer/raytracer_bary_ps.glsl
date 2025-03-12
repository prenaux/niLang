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

// FunctionFwd: TestGpuFuncs
vec3 TestGpuFuncs_BaryToVec3(vec2 aBary);
void TestGpuFuncs_InitRayQuery(/* mut */ rayQueryEXT aRayQuery, nish_std_PixelInput aInput, TestGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS);
nish_std_PixelOutput TestGpuFuncs_raytracer_bary_ps(nish_std_PixelInput aInput, TestGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS);

// Function: TestGpuFuncs
vec3 TestGpuFuncs_BaryToVec3(vec2 aBary) {
  return vec3(((1.0 - aBary.x) - aBary.y),aBary.x,aBary.y);
}
void TestGpuFuncs_InitRayQuery(/* mut */ rayQueryEXT aRayQuery, nish_std_PixelInput aInput, TestGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS) {
  vec3 ndc = vec3((((aInput.fragCoord.x / aUniforms.rtWidth) * 2.0) - 1.0),(1.0 - ((aInput.fragCoord.y / aUniforms.rtHeight) * 2.0)),1.0);
  mat4 _tmp_y = aUniforms.cameraInvView;
  vec3 origin = vec3(_tmp_y[3][0],_tmp_y[3][1],_tmp_y[3][2]);
  vec3 target = nish_std_Vec3TransformCoord(ndc,aUniforms.cameraInvViewProj);
  vec3 dir = normalize(((target-(origin.xyz)).xyz));
  rayQueryInitializeEXT(aRayQuery,aAS,nish_std_RayFlags_CullBackFacingTriangles,255,(origin.xyz),0.001,(dir.xyz),aUniforms.cameraFarClipPlane);
}
nish_std_PixelOutput TestGpuFuncs_raytracer_bary_ps(nish_std_PixelInput aInput, TestGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS) {
  /* mut */ rayQueryEXT rayQuery/*__noinit__*/;
  TestGpuFuncs_InitRayQuery(rayQuery,aInput,aUniforms,aAS);
  bool done = rayQueryProceedEXT(rayQuery);
  uint intersectionType = rayQueryGetIntersectionTypeEXT(rayQuery,true);
  vec4 color;
  bool _tmp_Z = (intersectionType == nish_std_RayQueryIntersectionType_CommittedTriangle);
  if (_tmp_Z) {
    float isFrontFacing = float(rayQueryGetIntersectionFrontFaceEXT(rayQuery,false));
    vec2 tribary = rayQueryGetIntersectionBarycentricsEXT(rayQuery,true);
    vec3 _tmp_81 = (TestGpuFuncs_BaryToVec3(tribary)*isFrontFacing);
    color = vec4(_tmp_81.x,_tmp_81.y,_tmp_81.z,1.0);
  }
  else {
    bool _tmp_c1 = (intersectionType == nish_std_RayQueryIntersectionType_CommittedBoundingVolume);
    if (_tmp_c1) {
      color = vec4(1.0,0.0,0.0,1.0);
    }
    else {
      {
        color = vec4(0.0,0.5,0.8,0.0);
      }
    }
  }
  vec4 _tmp_p1 = color;
  return nish_std_PixelOutput_new(_tmp_p1);
}
// MODULE END TestGpuFuncs

// Pixel Shader main: TestGpuFuncs_raytracer_bary_ps
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
  nish_std_PixelOutput _rval_ = TestGpuFuncs_raytracer_bary_ps(aInput, aUniforms, IN_1_aAS);
  OUT_0_rval_color = _rval_.color;
}

