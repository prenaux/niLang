#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_shader_image_load_formatted : enable
// DO IMPORTS BEGIN TestGpuFuncs
// DO IMPORTS BEGIN nish:ray
// MODULE BEGIN nish:std
// Type: RayHitPayload
struct nish_std_RayHitPayload {
  vec2 baryCoord;
};
hitAttributeEXT nish_std_RayHitPayload _hitAttributeEXT_RayHitPayload;
// Type: RayParams
struct nish_std_RayParams {
  float rayTMin;
  float rayTMax;
  uint incomingRayFlags;
};
// Type: RayObjectParams
struct nish_std_RayObjectParams {
  vec3 objectRayOrigin;
  vec3 objectRayDirection;
};
// TypeStaticFwd: Vec2
vec2 vec2_Zero;
// TypeMethFwd: RayHitPayload
nish_std_RayHitPayload nish_std_RayHitPayload_new_default();
// FunctionFwd: nish:std
void vec2_static_initialize() {
  // TypeStatic: Vec2
  vec2_Zero = vec2(0.0,0.0);
}
// TypeMeth: RayHitPayload
nish_std_RayHitPayload nish_std_RayHitPayload_new_default() {
  nish_std_RayHitPayload t;
  t.baryCoord = vec2_Zero;
  return t;
}
// Function: nish:std
// ModuleInitialize: nish_std
void nish_std_initialize() {
  vec2_static_initialize();
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

// FunctionFwd: TestGpuFuncs
nish_std_RayHitPayload TestGpuFuncs_sphere_rint(nish_std_RayObjectParams aObject, nish_std_RayParams aRayParams);

// Function: TestGpuFuncs
nish_std_RayHitPayload TestGpuFuncs_sphere_rint(nish_std_RayObjectParams aObject, nish_std_RayParams aRayParams) {
  float radius = 0.3;
  vec3 center = vec3(0.0,0.0,0.0);
  vec3 rorig = aObject.objectRayOrigin;
  vec3 rdir = aObject.objectRayDirection;
  float hit = nish_ray_SphereIntersect(center,radius,rorig,rdir);
  /* mut */ nish_std_RayHitPayload payload = nish_std_RayHitPayload_new_default() /*SKIPPED COPY VALUETYPE: newed*/;
  bool _tmp_9 = (hit > 0.0);
  if (_tmp_9) {
    reportIntersectionEXT(hit,0);
    vec3 hitPos = (rorig+(rdir*hit));
    vec3 normal = normalize((hitPos-center));
    float PI = 3.14159;
    float longitude = ((atan(normal.z,normal.x) / (2.0 * PI)) + 0.5);
    float latitude = ((asin(normal.y) / PI) + 0.5);
    payload.baryCoord = vec2(longitude,latitude);
  }
  return payload;
}
// MODULE END TestGpuFuncs

// Ray Shader main: TestGpuFuncs_sphere_rint
void main(void) {
  nish_std_initialize();
  nish_std_RayObjectParams aObject;
  nish_std_RayParams aRayParams;
  aObject.objectRayOrigin = gl_ObjectRayOriginEXT;
  aObject.objectRayDirection = gl_ObjectRayDirectionEXT;
  aRayParams.rayTMin = gl_RayTminEXT;
  aRayParams.rayTMax = gl_RayTmaxEXT;
  aRayParams.incomingRayFlags = gl_IncomingRayFlagsEXT;
  nish_std_RayHitPayload _rval_ = TestGpuFuncs_sphere_rint(aObject, aRayParams);
  _hitAttributeEXT_RayHitPayload = _rval_;
}

