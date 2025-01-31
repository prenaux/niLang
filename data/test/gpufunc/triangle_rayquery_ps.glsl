#version 460
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
uint lib_shader_RayQueryIntersectionType_CommittedGenerated;
// FunctionFwd: lib:shader
void lib_shader_RayFlags_static_initialize() {
  // TypeStatic: RayFlags
  lib_shader_RayFlags_None = 0;
}
void lib_shader_RayQueryIntersectionType_static_initialize() {
  // TypeStatic: RayQueryIntersectionType
  lib_shader_RayQueryIntersectionType_CommittedTriangle = 1;
  lib_shader_RayQueryIntersectionType_CommittedGenerated = 2;
}
// Function: lib:shader
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

// TypeMethFwd: PixelOutput
TestGpuFuncs_PixelOutput TestGpuFuncs_PixelOutput_new(vec4 a_color);

// FunctionFwd: TestGpuFuncs
TestGpuFuncs_PixelOutput TestGpuFuncs_triangle_rayquery_ps(lib_shader_PixelInput aInput, accelerationStructureEXT aAS);

// TypeMeth: PixelOutput
TestGpuFuncs_PixelOutput TestGpuFuncs_PixelOutput_new(vec4 a_color) {
  TestGpuFuncs_PixelOutput t;
  t.color = a_color;
  return t;
}

// Function: TestGpuFuncs
TestGpuFuncs_PixelOutput TestGpuFuncs_triangle_rayquery_ps(lib_shader_PixelInput aInput, accelerationStructureEXT aAS) {
  vec2 coords = (aInput.fragCoord.xy);
  vec2 dims = vec2(512.0,512.0);
  vec2 uv = (coords/dims);
  vec3 origin = vec3(0.0,0.0,-1.0);
  vec3 target = vec3(((uv.x * 2.0) - 1.0),(((1.0 - uv.y) * 2.0) - 1.0),1.0);
  vec3 dir = normalize((target-origin));
  rayQueryEXT rayQuery/*__noinit__*/;
  rayQueryInitializeEXT(rayQuery,aAS,lib_shader_RayFlags_None,255,origin,0.001,dir,1000.0);
  bool done = rayQueryProceedEXT(rayQuery);
  uint intersectionType = rayQueryGetIntersectionTypeEXT(rayQuery,true);
  vec4 _tmp_D;
  bool _tmp_E = (intersectionType == lib_shader_RayQueryIntersectionType_CommittedTriangle);
  if (_tmp_E) {
    _tmp_D = vec4(0.0,1.0,0.0,1.0);
  }
  else {
    bool _tmp_L = (intersectionType == lib_shader_RayQueryIntersectionType_CommittedGenerated);
    if (_tmp_L) {
      _tmp_D = vec4(1.0,0.0,0.0,1.0);
    }
    else {
      {
        _tmp_D = vec4(0.0,0.5,0.8,0.0);
      }
    }
  }
  return TestGpuFuncs_PixelOutput_new(_tmp_D);
}
// MODULE END TestGpuFuncs

// Pixel Shader main: TestGpuFuncs_triangle_rayquery_ps
layout(set = 7, binding = 0) uniform accelerationStructureEXT IN_1_aAS;
layout(location = 0) out vec4 OUT_0_rval_color;
void main(void) {
  lib_shader_initialize();
  lib_shader_PixelInput aInput;
  aInput.fragCoord = gl_FragCoord;
  aInput.frontFacing = gl_FrontFacing;
  TestGpuFuncs_PixelOutput _rval_ = TestGpuFuncs_triangle_rayquery_ps(aInput, IN_1_aAS);
  OUT_0_rval_color = _rval_.color;
}

