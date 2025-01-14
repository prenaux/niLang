#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_shader_image_load_formatted : enable
// DO IMPORTS BEGIN TestGpuFuncs
// MODULE BEGIN lib:shader
// MODULE PROLOGUE BEGIN lib:shader
mat4 nil_Mat4x3ToMat4x4(mat4x3 m) { return mat4(vec4(m[0], 0), vec4(m[1], 0), vec4(m[2], 0), vec4(m[3], 1)); }
mat4 nil_Mat3x4ToMat4x4(mat3x4 m) { return mat4(vec4(m[0]), vec4(m[1]), vec4(m[2]), vec4(0,0,0,1)); }
// MODULE PROLOGUE END lib:shader
// Type: RayHitPayload
struct lib_shader_RayHitPayload {
  vec2 baryCoord;
};
hitAttributeEXT lib_shader_RayHitPayload _hitAttributeEXT_RayHitPayload;
// Type: RayObjectParams
struct lib_shader_RayObjectParams {
  vec3 objectRayOrigin;
  vec3 objectRayDirection;
};
// Type: RayParams
struct lib_shader_RayParams {
  float rayTMin;
  float rayTMax;
  uint incomingRayFlags;
};
// TypeStaticFwd: Vec2
vec2 vec2_Zero;
// TypeMethFwd: RayHitPayload
lib_shader_RayHitPayload lib_shader_RayHitPayload_new_default();
// FunctionFwd: lib:shader
void vec2_static_initialize() {
  // TypeStatic: Vec2
  vec2_Zero = vec2(0.0,0.0);
}
// TypeMeth: RayHitPayload
lib_shader_RayHitPayload lib_shader_RayHitPayload_new_default() {
  lib_shader_RayHitPayload t;
  t.baryCoord = vec2_Zero;
  return t;
}
// Function: lib:shader
// ModuleInitialize: lib_shader
void lib_shader_initialize() {
  vec2_static_initialize();
}
// MODULE END lib:shader
// DO IMPORTS END TestGpuFuncs

// MODULE BEGIN TestGpuFuncs

// FunctionFwd: TestGpuFuncs
float TestGpuFuncs_sphereIntersect(vec3 aSphereCenter, float aSphereRadius, vec3 aRayOrigin, vec3 aRayDirection);
lib_shader_RayHitPayload TestGpuFuncs_sphere_rint(lib_shader_RayObjectParams aObject, lib_shader_RayParams aRayParams);

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
lib_shader_RayHitPayload TestGpuFuncs_sphere_rint(lib_shader_RayObjectParams aObject, lib_shader_RayParams aRayParams) {
  float radius = 0.3;
  vec3 center = vec3(0.0,0.0,0.0);
  vec3 rorig = aObject.objectRayOrigin;
  vec3 rdir = aObject.objectRayDirection;
  float hit = TestGpuFuncs_sphereIntersect(center,radius,rorig,rdir);
  lib_shader_RayHitPayload payload = lib_shader_RayHitPayload_new_default() /*SKIPPED COPY VALUETYPE: newed*/;
  vec3 hitPos = (rorig+(rdir*hit));
  vec3 normal = normalize((hitPos-center));
  float PI = 3.14159;
  float longitude = ((atan(normal.z, normal.x) / (2.0 * PI)) + 0.5);
  float latitude = ((asin(normal.y) / PI) + 0.5);
  bool _tmp_V = (hit > 0.0);
  if (_tmp_V) {
    reportIntersectionEXT(hit,0);
    payload.baryCoord = vec2(longitude,latitude);
  }
  return payload;
}
// MODULE END TestGpuFuncs

// Ray Shader main: TestGpuFuncs_sphere_rint
void main(void) {
  lib_shader_initialize();
  lib_shader_RayObjectParams aObject;
  lib_shader_RayParams aRayParams;
  aObject.objectRayOrigin = gl_ObjectRayOriginEXT;
  aObject.objectRayDirection = gl_ObjectRayDirectionEXT;
  aRayParams.rayTMin = gl_RayTminEXT;
  aRayParams.rayTMax = gl_RayTmaxEXT;
  aRayParams.incomingRayFlags = gl_IncomingRayFlagsEXT;
  lib_shader_RayHitPayload _rval_ = TestGpuFuncs_sphere_rint(aObject, aRayParams);
  _hitAttributeEXT_RayHitPayload = _rval_;
}

