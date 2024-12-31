#version 460
#extension GL_EXT_ray_tracing : require
// DO IMPORTS BEGIN TestGpuFuncs
// MODULE BEGIN lib:shader
// MODULE PROLOGUE BEGIN lib:shader
mat4 nil_Mat4x3ToMat4x4(mat4x3 m) { return mat4(vec4(m[0], 0), vec4(m[1], 0), vec4(m[2], 0), vec4(m[3], 1)); }
mat4 nil_Mat3x4ToMat4x4(mat3x4 m) { return mat4(vec4(m[0]), vec4(m[1]), vec4(m[2]), vec4(0,0,0,1)); }
// MODULE PROLOGUE END lib:shader
// Type: RayDesc
struct lib_shader_RayDesc {
  vec3 origin;
  vec3 direction;
  float tMin;
  float tMax;
};
// Type: RayPayload
struct lib_shader_RayPayload {
  vec4 color;
  float hitT;
};
layout(location = 0) rayPayloadEXT lib_shader_RayPayload nil_builtin_RAY_PAYLOAD_0;
// Type: RayWorkDimensions
struct lib_shader_RayWorkDimensions {
  uvec3 launchId;
  uvec3 launchSize;
};
// TypeStaticFwd: RayFlags
uint uint_None;
// TypeStaticFwd: Vec4
vec4 vec4_Zero;
vec4 vec4_Black;
// TypeMethFwd: RayDesc
lib_shader_RayDesc lib_shader_RayDesc_new(vec3 a_origin, vec3 a_direction, float a_tMin, float a_tMax);
// TypeMethFwd: RayPayload
lib_shader_RayPayload lib_shader_RayPayload_new_default();
// FunctionFwd: lib:shader
lib_shader_RayPayload lib_shader_traceSimpleRay(accelerationStructureEXT aAS, lib_shader_RayDesc aRay, lib_shader_RayPayload aPayload);
void uint_static_initialize() {
  // TypeStatic: RayFlags
  uint_None = 0;
}
void vec4_static_initialize() {
  // TypeStatic: Vec4
  vec4_Zero = vec4(0.0,0.0,0.0,0.0);
  vec4_Black = vec4_Zero;
}
// TypeMeth: RayDesc
lib_shader_RayDesc lib_shader_RayDesc_new(vec3 a_origin, vec3 a_direction, float a_tMin, float a_tMax) {
  lib_shader_RayDesc t;
  t.origin = a_origin;
  t.direction = a_direction;
  t.tMin = a_tMin;
  t.tMax = a_tMax;
  return t;
}
// TypeMeth: RayPayload
lib_shader_RayPayload lib_shader_RayPayload_new_default() {
  lib_shader_RayPayload t;
  t.color = vec4_Black;
  t.hitT = 0.0;
  return t;
}
// Function: lib:shader
lib_shader_RayPayload lib_shader_traceSimpleRay(accelerationStructureEXT aAS, lib_shader_RayDesc aRay, lib_shader_RayPayload aPayload) {
  return (traceRayEXT(aAS, uint_None, 255, 0, 0, 0, aRay.origin, aRay.tMin, aRay.direction, aRay.tMax, 0),nil_builtin_RAY_PAYLOAD_0);
}
// ModuleInitialize: lib_shader
void lib_shader_initialize() {
  uint_static_initialize();
  vec4_static_initialize();
}
// MODULE END lib:shader
// DO IMPORTS END TestGpuFuncs

// MODULE BEGIN TestGpuFuncs

// FunctionFwd: TestGpuFuncs
void TestGpuFuncs_triangle_rgen(accelerationStructureEXT aAS, writeonly image2D aOutputImage, lib_shader_RayWorkDimensions aLaunch);

// Function: TestGpuFuncs
void TestGpuFuncs_triangle_rgen(accelerationStructureEXT aAS, writeonly image2D aOutputImage, lib_shader_RayWorkDimensions aLaunch) {
  uvec2 coords = (aLaunch.launchId.xy);
  uvec2 dims = (aLaunch.launchSize.xy);
  vec2 pixelCenter = (vec2(coords)+0.5);
  vec2 uv = (pixelCenter/vec2(dims));
  vec3 origin = vec3(0.0,0.0,-1.0);
  vec3 target = vec3(((uv.x * 2.0) - 1.0),(((1.0 - uv.y) * 2.0) - 1.0),1.0);
  vec3 dir = normalize((target-origin));
  vec3 _tmp_v = origin;
  vec3 _tmp_w = dir;
  float _tmp_x = 0.001;
  float _tmp_y = 1000.0;
  lib_shader_RayDesc ray = lib_shader_RayDesc_new(_tmp_v, _tmp_w, _tmp_x, _tmp_y) /*SKIPPED COPY VALUETYPE: newed*/;
  lib_shader_RayPayload payload = lib_shader_traceSimpleRay(aAS,ray,lib_shader_RayPayload_new_default());
  vec4 outColor = payload.color;
  imageStore(aOutputImage,ivec2(coords),outColor);
}
// MODULE END TestGpuFuncs

// Ray Shader main: TestGpuFuncs_triangle_rgen
layout(set = 7, binding = 0) uniform accelerationStructureEXT IN_1_aAS;
layout(set = 8, binding = 0,rgba8) uniform writeonly image2D IN_1_aOutputImage;
void main(void) {
  lib_shader_initialize();
  lib_shader_RayWorkDimensions aLaunch;
  aLaunch.launchId = gl_LaunchIDEXT;
  aLaunch.launchSize = gl_LaunchSizeEXT;
  TestGpuFuncs_triangle_rgen(IN_1_aAS, IN_1_aOutputImage, aLaunch);
}

