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
// TypeStaticFwd: RayQueryIntersectionType
uint lib_shader_RayQueryIntersectionType_CommittedTriangle;
uint lib_shader_RayQueryIntersectionType_CommittedBoundingVolume;
// FunctionFwd: lib:shader
vec3 lib_shader_Vec3TransformCoord(vec3 v, mat4 m);
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
void TestGpuFuncs_InitRayQuery(rayQueryEXT aRayQuery, lib_shader_PixelInput aInput, TestGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS);
TestGpuFuncs_PixelOutput TestGpuFuncs_raytracer_bary_ps(lib_shader_PixelInput aInput, TestGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS);

// TypeMeth: PixelOutput
TestGpuFuncs_PixelOutput TestGpuFuncs_PixelOutput_new(vec4 a_color) {
  TestGpuFuncs_PixelOutput t;
  t.color = a_color;
  return t;
}

// Function: TestGpuFuncs
void TestGpuFuncs_InitRayQuery(rayQueryEXT aRayQuery, lib_shader_PixelInput aInput, TestGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS) {
  vec3 ndc = vec3((((aInput.fragCoord.x / aUniforms.rtWidth) * 2.0) - 1.0),(1.0 - ((aInput.fragCoord.y / aUniforms.rtHeight) * 2.0)),1.0);
  mat4 _tmp_l = aUniforms.cameraInvView;
  vec3 origin = vec3(_tmp_l[3][0],_tmp_l[3][1],_tmp_l[3][2]);
  vec3 target = lib_shader_Vec3TransformCoord(ndc,aUniforms.cameraInvViewProj);
  vec3 dir = normalize(((target-(origin.xyz)).xyz));
  rayQueryInitializeEXT(aRayQuery,aAS,lib_shader_RayFlags_None,255,(origin.xyz),0.001,(dir.xyz),aUniforms.cameraFarClipPlane);
}
TestGpuFuncs_PixelOutput TestGpuFuncs_raytracer_bary_ps(lib_shader_PixelInput aInput, TestGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS) {
  rayQueryEXT rayQuery/*__noinit__*/;
  TestGpuFuncs_InitRayQuery(rayQuery,aInput,aUniforms,aAS);
  bool done = rayQueryProceedEXT(rayQuery);
  uint intersectionType = rayQueryGetIntersectionTypeEXT(rayQuery,true);
  vec4 _tmp_O;
  bool _tmp_P = (intersectionType == lib_shader_RayQueryIntersectionType_CommittedTriangle);
  if (_tmp_P) {
    vec2 tribary = rayQueryGetIntersectionBarycentricsEXT(rayQuery,true);
    _tmp_O = vec4(tribary.x,tribary.y,0.0,1.0);
  }
  else {
    bool _tmp_01 = (intersectionType == lib_shader_RayQueryIntersectionType_CommittedBoundingVolume);
    if (_tmp_01) {
      _tmp_O = vec4(1.0,0.0,0.0,1.0);
    }
    else {
      {
        _tmp_O = vec4(0.0,0.5,0.8,0.0);
      }
    }
  }
  return TestGpuFuncs_PixelOutput_new(_tmp_O);
}
// MODULE END TestGpuFuncs

// Pixel Shader main: TestGpuFuncs_raytracer_bary_ps
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
  TestGpuFuncs_PixelOutput _rval_ = TestGpuFuncs_raytracer_bary_ps(aInput, aUniforms, IN_1_aAS);
  OUT_0_rval_color = _rval_.color;
}

