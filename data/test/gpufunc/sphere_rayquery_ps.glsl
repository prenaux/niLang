#version 460
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_ray_query : require
// DO IMPORTS BEGIN TestGpuFuncs
// MODULE BEGIN lib:shader
// MODULE PROLOGUE BEGIN lib:shader
mat4 nil_Mat4x3ToMat4x4(mat4x3 m) { return mat4(vec4(m[0], 0), vec4(m[1], 0), vec4(m[2], 0), vec4(m[3], 1)); }
mat4 nil_Mat3x4ToMat4x4(mat3x4 m) { return mat4(vec4(m[0]), vec4(m[1]), vec4(m[2]), vec4(0,0,0,1)); }
// MODULE PROLOGUE END lib:shader
// Type: PixelInput
struct lib_shader_PixelInput {
  vec4 fragCoord;
  bool frontFacing;
};
// TypeStaticFwd: RayFlags
uint lib_shader_RayFlags_None;
// TypeStaticFwd: RayQueryCandidateIntersectionType
uint lib_shader_RayQueryCandidateIntersectionType_CandidateTriangle;
// TypeStaticFwd: RayQueryIntersectionType
uint lib_shader_RayQueryIntersectionType_CommittedTriangle;
uint lib_shader_RayQueryIntersectionType_CommittedBoundingVolume;
// TypeStaticFwd: Vec3
vec3 vec3_Zero;
// FunctionFwd: lib:shader
uint lib_shader_HashJenkins(uint aStartX);
vec3 lib_shader_UIntToHashColor(uint i);
vec3 lib_shader_Vec3TransformCoord(vec3 v, mat4 m);
void lib_shader_RayFlags_static_initialize() {
  // TypeStatic: RayFlags
  lib_shader_RayFlags_None = 0;
}
void lib_shader_RayQueryCandidateIntersectionType_static_initialize() {
  // TypeStatic: RayQueryCandidateIntersectionType
  lib_shader_RayQueryCandidateIntersectionType_CandidateTriangle = 0;
}
void lib_shader_RayQueryIntersectionType_static_initialize() {
  // TypeStatic: RayQueryIntersectionType
  lib_shader_RayQueryIntersectionType_CommittedTriangle = 1;
  lib_shader_RayQueryIntersectionType_CommittedBoundingVolume = 2;
}
void vec3_static_initialize() {
  // TypeStatic: Vec3
  vec3_Zero = vec3(0.0,0.0,0.0);
}
// Function: lib:shader
uint lib_shader_HashJenkins(uint aStartX) {
  uint x = aStartX;
  x = (x + (x << 10));
  x = (x ^ (x >> 6));
  x = (x + (x << 3));
  x = (x ^ (x >> 11));
  x = (x + (x << 15));
  return x;
}
vec3 lib_shader_UIntToHashColor(uint i) {
  uint hash = lib_shader_HashJenkins(i);
  return vec3((float(((hash >> 0) & 255)) / 255.0),(float(((hash >> 8) & 255)) / 255.0),(float(((hash >> 16) & 255)) / 255.0));
}
vec3 lib_shader_Vec3TransformCoord(vec3 v, mat4 m) {
  vec3 _tmp_R = v;
  vec4 r = (m * vec4(_tmp_R.x,_tmp_R.y,_tmp_R.z,1.0));
  return ((r/r.w).xyz);
}
// ModuleInitialize: lib_shader
void lib_shader_initialize() {
  lib_shader_RayFlags_static_initialize();
  lib_shader_RayQueryCandidateIntersectionType_static_initialize();
  lib_shader_RayQueryIntersectionType_static_initialize();
  vec3_static_initialize();
}
// MODULE END lib:shader
// DO IMPORTS END TestGpuFuncs

// MODULE BEGIN TestGpuFuncs

// Type: PixelOutput
struct TestGpuFuncs_PixelOutput {
  vec4 color;
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

// TypeMethFwd: PixelOutput
TestGpuFuncs_PixelOutput TestGpuFuncs_PixelOutput_new(vec4 a_color);

// FunctionFwd: TestGpuFuncs
float TestGpuFuncs_sphereIntersect(vec3 aSphereCenter, float aSphereRadius, vec3 aRayOrigin, vec3 aRayDirection);
TestGpuFuncs_PixelOutput TestGpuFuncs_sphere_rayquery_ps(lib_shader_PixelInput aInput, TestGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS);

// TypeMeth: PixelOutput
TestGpuFuncs_PixelOutput TestGpuFuncs_PixelOutput_new(vec4 a_color) {
  TestGpuFuncs_PixelOutput t;
  t.color = a_color;
  return t;
}

// Function: TestGpuFuncs
float TestGpuFuncs_sphereIntersect(vec3 aSphereCenter, float aSphereRadius, vec3 aRayOrigin, vec3 aRayDirection) {
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
TestGpuFuncs_PixelOutput TestGpuFuncs_sphere_rayquery_ps(lib_shader_PixelInput aInput, TestGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS) {
  vec3 ndc = vec3((((aInput.fragCoord.x / aUniforms.rtWidth) * 2.0) - 1.0),(1.0 - ((aInput.fragCoord.y / aUniforms.rtHeight) * 2.0)),1.0);
  mat4 _tmp_K = aUniforms.cameraInvView;
  vec3 origin = vec3(_tmp_K[3][0],_tmp_K[3][1],_tmp_K[3][2]);
  vec3 target = lib_shader_Vec3TransformCoord(ndc,aUniforms.cameraInvViewProj);
  vec3 dir = normalize(((target-(origin.xyz)).xyz));
  rayQueryEXT rayQuery/*__noinit__*/;
  rayQueryInitializeEXT(rayQuery,aAS,lib_shader_RayFlags_None,255,(origin.xyz),0.001,(dir.xyz),aUniforms.cameraFarClipPlane);
  while(true) {
    bool _tmp_51 = rayQueryProceedEXT(rayQuery);
    if (!(_tmp_51)) {
      break;
    }
    uint candType = rayQueryGetIntersectionTypeEXT(rayQuery,false);
    bool _tmp_a1 = (candType == lib_shader_RayQueryCandidateIntersectionType_CandidateTriangle);
    if (_tmp_a1) {
      rayQueryConfirmIntersectionEXT(rayQuery);
      rayQueryTerminateEXT(rayQuery);
    }
    else {
      {
        vec3 rorig = rayQueryGetIntersectionObjectRayOriginEXT(rayQuery,false);
        vec3 rdir = rayQueryGetIntersectionObjectRayDirectionEXT(rayQuery,false);
        vec3 center = vec3_Zero;
        float radius = 0.3;
        float hit = TestGpuFuncs_sphereIntersect(center,radius,rorig,rdir);
        bool _tmp_r1 = (hit >= 0.0);
        if (_tmp_r1) {
          rayQueryGenerateIntersectionEXT(rayQuery,hit);
          rayQueryTerminateEXT(rayQuery);
        }
      }
    }
  }
  uint intersectionType = rayQueryGetIntersectionTypeEXT(rayQuery,true);
  vec3 _tmp_A1 = lib_shader_UIntToHashColor((uint(rayQueryGetIntersectionInstanceCustomIndexEXT(rayQuery,true)) + 1));
  vec4 instanceColor = vec4(_tmp_A1.x,_tmp_A1.y,_tmp_A1.z,1.0);
  vec4 _tmp_H1;
  bool _tmp_I1 = (intersectionType == lib_shader_RayQueryIntersectionType_CommittedTriangle);
  if (_tmp_I1) {
    _tmp_H1 = instanceColor;
  }
  else {
    bool _tmp_L1 = (intersectionType == lib_shader_RayQueryIntersectionType_CommittedBoundingVolume);
    if (_tmp_L1) {
      _tmp_H1 = instanceColor;
    }
    else {
      {
        _tmp_H1 = vec4(0.0,0.5,0.8,0.0);
      }
    }
  }
  return TestGpuFuncs_PixelOutput_new(_tmp_H1);
}
// MODULE END TestGpuFuncs

// Pixel Shader main: TestGpuFuncs_sphere_rayquery_ps
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
  TestGpuFuncs_PixelOutput _rval_ = TestGpuFuncs_sphere_rayquery_ps(aInput, aUniforms, IN_1_aAS);
  OUT_0_rval_color = _rval_.color;
}

