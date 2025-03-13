#version 460
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_ray_query : require
// DO IMPORTS BEGIN TestGpuFuncs
// DO IMPORTS BEGIN nish:ray
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
uint nish_std_RayFlags_None;
// TypeStaticFwd: RayQueryIntersectionType
uint nish_std_RayQueryIntersectionType_CommittedTriangle;
uint nish_std_RayQueryIntersectionType_CommittedBoundingVolume;
// TypeStaticFwd: RayQueryCandidateIntersectionType
uint nish_std_RayQueryCandidateIntersectionType_CandidateTriangle;
// TypeStaticFwd: Vec3
vec3 vec3_Zero;
// TypeMethFwd: PixelOutput
nish_std_PixelOutput nish_std_PixelOutput_new(vec4 a_color);
// FunctionFwd: nish:std
vec3 nish_std_UIntToHashColor(uint i);
uint nish_std_HashJenkins(uint aStartX);
vec3 nish_std_Vec3TransformCoord(vec3 v, mat4 m);
void nish_std_RayFlags_static_initialize() {
  // TypeStatic: RayFlags
  nish_std_RayFlags_None = 0;
}
void nish_std_RayQueryIntersectionType_static_initialize() {
  // TypeStatic: RayQueryIntersectionType
  nish_std_RayQueryIntersectionType_CommittedTriangle = 1;
  nish_std_RayQueryIntersectionType_CommittedBoundingVolume = 2;
}
void nish_std_RayQueryCandidateIntersectionType_static_initialize() {
  // TypeStatic: RayQueryCandidateIntersectionType
  nish_std_RayQueryCandidateIntersectionType_CandidateTriangle = 0;
}
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
vec3 nish_std_UIntToHashColor(uint i) {
  uint hash = nish_std_HashJenkins(i);
  return vec3((float(((hash >> 0) & 255)) / 255.0),(float(((hash >> 8) & 255)) / 255.0),(float(((hash >> 16) & 255)) / 255.0));
}
uint nish_std_HashJenkins(uint aStartX) {
  /* mut */ uint x = aStartX;
  x = (x + (x << 10));
  x = (x ^ (x >> 6));
  x = (x + (x << 3));
  x = (x ^ (x >> 11));
  x = (x + (x << 15));
  return x;
}
vec3 nish_std_Vec3TransformCoord(vec3 v, mat4 m) {
  vec3 _tmp_R = v;
  vec4 r = (m * vec4(_tmp_R.x,_tmp_R.y,_tmp_R.z,1.0));
  return ((r/r.w).xyz);
}
// ModuleInitialize: nish_std
void nish_std_initialize() {
  nish_std_RayFlags_static_initialize();
  nish_std_RayQueryIntersectionType_static_initialize();
  nish_std_RayQueryCandidateIntersectionType_static_initialize();
  vec3_static_initialize();
}
// MODULE END nish:std
// DO IMPORTS END nish:ray
// MODULE BEGIN nish:ray
// FunctionFwd: nish:ray
float nish_ray_SphereIntersect(vec3 aSphereCenter, float aSphereRadius, vec3 aRayOrigin, vec3 aRayDirection);
// Function: nish:ray
float nish_ray_SphereIntersect(vec3 aSphereCenter, float aSphereRadius, vec3 aRayOrigin, vec3 aRayDirection) {
  vec3 oc = (aRayOrigin-aSphereCenter);
  float b = dot(oc,aRayDirection);
  float c = (dot(oc,oc) - (aSphereRadius * aSphereRadius));
  float h = ((b * b) - c);
  bool _tmp_e = (h < 0.0);
  if (_tmp_e) {
    return -1.0;
  }
  return (-b - sqrt(h));
}
// MODULE END nish:ray
// MODULE SKIPPED, ALREADY PROCESSED: nish:std
// DO IMPORTS END TestGpuFuncs

// MODULE BEGIN TestGpuFuncs

// Type: TestRayUniforms
struct TestGpuFuncs_TestRayUniforms {
  float rtWidth;
  float rtHeight;
  float cameraFarClipPlane;
  float padding;
  mat4 cameraInvView;
  mat4 cameraInvViewProj;
};

// FunctionFwd: TestGpuFuncs
nish_std_PixelOutput TestGpuFuncs_sphere_rayquery_ps(nish_std_PixelInput aInput, TestGpuFuncs_TestRayUniforms aUniforms, accelerationStructureEXT aAS);

// Function: TestGpuFuncs
nish_std_PixelOutput TestGpuFuncs_sphere_rayquery_ps(nish_std_PixelInput aInput, TestGpuFuncs_TestRayUniforms aUniforms, accelerationStructureEXT aAS) {
  vec3 ndc = vec3((((aInput.fragCoord.x / aUniforms.rtWidth) * 2.0) - 1.0),(1.0 - ((aInput.fragCoord.y / aUniforms.rtHeight) * 2.0)),1.0);
  mat4 _tmp_l = aUniforms.cameraInvView;
  vec3 origin = vec3(_tmp_l[3][0],_tmp_l[3][1],_tmp_l[3][2]);
  vec3 target = nish_std_Vec3TransformCoord(ndc,aUniforms.cameraInvViewProj);
  vec3 dir = normalize(((target-(origin.xyz)).xyz));
  /* mut */ rayQueryEXT rayQuery/*__noinit__*/;
  rayQueryInitializeEXT(rayQuery,aAS,nish_std_RayFlags_None,255,(origin.xyz),0.001,(dir.xyz),aUniforms.cameraFarClipPlane);
  while(true) {
    bool _tmp_G = rayQueryProceedEXT(rayQuery);
    if (!(_tmp_G)) {
      break;
    }
    uint candType = rayQueryGetIntersectionTypeEXT(rayQuery,false);
    bool _tmp_L = (candType == nish_std_RayQueryCandidateIntersectionType_CandidateTriangle);
    if (_tmp_L) {
      rayQueryConfirmIntersectionEXT(rayQuery);
      rayQueryTerminateEXT(rayQuery);
    }
    else {
      {
        vec3 rorig = rayQueryGetIntersectionObjectRayOriginEXT(rayQuery,false);
        vec3 rdir = rayQueryGetIntersectionObjectRayDirectionEXT(rayQuery,false);
        vec3 center = vec3_Zero;
        float radius = 0.3;
        float hit = nish_ray_SphereIntersect(center,radius,rorig,rdir);
        bool _tmp_21 = (hit >= 0.0);
        if (_tmp_21) {
          rayQueryGenerateIntersectionEXT(rayQuery,hit);
          rayQueryTerminateEXT(rayQuery);
        }
      }
    }
  }
  uint intersectionType = rayQueryGetIntersectionTypeEXT(rayQuery,true);
  vec3 _tmp_b1 = nish_std_UIntToHashColor((uint(rayQueryGetIntersectionInstanceCustomIndexEXT(rayQuery,true)) + 1));
  vec4 instanceColor = vec4(_tmp_b1.x,_tmp_b1.y,_tmp_b1.z,1.0);
  vec4 _tmp_i1;
  bool _tmp_j1 = (intersectionType == nish_std_RayQueryIntersectionType_CommittedTriangle);
  if (_tmp_j1) {
    _tmp_i1 = instanceColor;
  }
  else {
    bool _tmp_m1 = (intersectionType == nish_std_RayQueryIntersectionType_CommittedBoundingVolume);
    if (_tmp_m1) {
      _tmp_i1 = instanceColor;
    }
    else {
      {
        _tmp_i1 = vec4(0.0,0.5,0.8,0.0);
      }
    }
  }
  return nish_std_PixelOutput_new(_tmp_i1);
}
// MODULE END TestGpuFuncs

// Pixel Shader main: TestGpuFuncs_sphere_rayquery_ps
// type size: 48, underlying: float
layout(set = 0, binding = 0) uniform UBO_TestGpuFuncs_TestRayUniforms { TestGpuFuncs_TestRayUniforms v; } IN_1_aUniforms;
layout(set = 7, binding = 0) uniform accelerationStructureEXT IN_1_aAS;
layout(location = 0) out vec4 OUT_0_rval_color;
void main(void) {
  nish_std_initialize();
  nish_std_PixelInput aInput;
  TestGpuFuncs_TestRayUniforms aUniforms;
  aInput.fragCoord = gl_FragCoord;
  aInput.frontFacing = gl_FrontFacing;
  aUniforms = IN_1_aUniforms.v;
  nish_std_PixelOutput _rval_ = TestGpuFuncs_sphere_rayquery_ps(aInput, aUniforms, IN_1_aAS);
  OUT_0_rval_color = _rval_.color;
}

