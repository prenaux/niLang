#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_shader_image_load_formatted : enable
// DO IMPORTS BEGIN TestGpuFuncs
// DO IMPORTS BEGIN nish:ray
// MODULE BEGIN nish:std
// Type: RayDesc
struct nish_std_RayDesc {
  vec3 origin;
  vec3 direction;
  float tMin;
  float tMax;
};
// Type: RayPayload
struct nish_std_RayPayload {
  vec4 color;
  float hitT;
};
layout(location = 0) rayPayloadEXT nish_std_RayPayload nil_builtin_RAY_PAYLOAD_0;
// Type: RayWorkDimensions
struct nish_std_RayWorkDimensions {
  uvec3 launchIndex;
  uvec3 launchSize;
};
// TypeStaticFwd: RayFlags
uint nish_std_RayFlags_None;
// TypeStaticFwd: Vec4
vec4 vec4_Zero;
vec4 vec4_Black;
// TypeMethFwd: RayDesc
nish_std_RayDesc nish_std_RayDesc_new(vec3 a_origin, vec3 a_direction, float a_tMin, float a_tMax);
// TypeMethFwd: RayPayload
nish_std_RayPayload nish_std_RayPayload_new_default();
// FunctionFwd: nish:std
nish_std_RayPayload nish_std_TraceSimpleRay(accelerationStructureEXT aAS, nish_std_RayDesc aRay, nish_std_RayPayload aPayload);
void nish_std_RayFlags_static_initialize() {
  // TypeStatic: RayFlags
  nish_std_RayFlags_None = 0;
}
void vec4_static_initialize() {
  // TypeStatic: Vec4
  vec4_Zero = vec4(0.0,0.0,0.0,0.0);
  vec4_Black = vec4_Zero;
}
// TypeMeth: RayDesc
nish_std_RayDesc nish_std_RayDesc_new(vec3 a_origin, vec3 a_direction, float a_tMin, float a_tMax) {
  nish_std_RayDesc t;
  t.origin = a_origin;
  t.direction = a_direction;
  t.tMin = a_tMin;
  t.tMax = a_tMax;
  return t;
}
// TypeMeth: RayPayload
nish_std_RayPayload nish_std_RayPayload_new_default() {
  nish_std_RayPayload t;
  t.color = vec4_Black;
  t.hitT = 0.0;
  return t;
}
// Function: nish:std
nish_std_RayPayload nish_std_TraceSimpleRay(accelerationStructureEXT aAS, nish_std_RayDesc aRay, nish_std_RayPayload aPayload) {
  return (traceRayEXT(aAS, nish_std_RayFlags_None, 255, 0, 0, 0, aRay.origin, aRay.tMin, aRay.direction, aRay.tMax, 0),nil_builtin_RAY_PAYLOAD_0);
}
// ModuleInitialize: nish_std
void nish_std_initialize() {
  nish_std_RayFlags_static_initialize();
  vec4_static_initialize();
}
// MODULE END nish:std
// DO IMPORTS END nish:ray
// MODULE BEGIN nish:ray
// FunctionFwd: nish:ray
// Function: nish:ray
// MODULE END nish:ray
// MODULE SKIPPED, ALREADY PROCESSED: nish:std
// DO IMPORTS END TestGpuFuncs

// MODULE BEGIN TestGpuFuncs

// FunctionFwd: TestGpuFuncs
void TestGpuFuncs_triangle_rgen(accelerationStructureEXT aAS, writeonly image2D aOutputImage, nish_std_RayWorkDimensions aLaunch);

// Function: TestGpuFuncs
void TestGpuFuncs_triangle_rgen(accelerationStructureEXT aAS, writeonly image2D aOutputImage, nish_std_RayWorkDimensions aLaunch) {
  uvec2 coords = (aLaunch.launchIndex.xy);
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
  /* mut */ nish_std_RayDesc ray = nish_std_RayDesc_new(_tmp_v, _tmp_w, _tmp_x, _tmp_y) /*SKIPPED COPY VALUETYPE: newed*/;
  /* mut */ nish_std_RayPayload payload = nish_std_TraceSimpleRay(aAS,ray,nish_std_RayPayload_new_default()) /*SKIPPED COPY VALUETYPE: funcret*/;
  imageStore(aOutputImage,ivec2(coords),payload.color);
}
// MODULE END TestGpuFuncs

// Ray Shader main: TestGpuFuncs_triangle_rgen
layout(set = 7, binding = 0) uniform accelerationStructureEXT IN_1_aAS;
layout(set = 8, binding = 0) uniform writeonly image2D IN_1_aOutputImage;
void main(void) {
  nish_std_initialize();
  nish_std_RayWorkDimensions aLaunch;
  aLaunch.launchIndex = gl_LaunchIDEXT;
  aLaunch.launchSize = gl_LaunchSizeEXT;
  TestGpuFuncs_triangle_rgen(IN_1_aAS, IN_1_aOutputImage, aLaunch);
}

