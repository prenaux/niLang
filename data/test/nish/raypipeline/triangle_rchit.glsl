#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_shader_image_load_formatted : enable
// DO IMPORTS BEGIN TestGpuFuncs
// DO IMPORTS BEGIN nish:ray
// MODULE BEGIN nish:std
// Type: RayPayload
struct nish_std_RayPayload {
  vec4 color;
  float hitT;
};
layout(location = 0) rayPayloadInEXT nish_std_RayPayload nil_builtin_RAY_PAYLOAD_0;
// Type: RayWorldParams
struct nish_std_RayWorldParams {
  vec3 worldRayOrigin;
  vec3 worldRayDirection;
};
// Type: RayParams
struct nish_std_RayParams {
  float rayTMin;
  float rayTMax;
  uint incomingRayFlags;
};
// Type: RayGeometryIds
struct nish_std_RayGeometryIds {
  uint primitiveIndex;
  uint instanceImplIndex;
  uint instanceUserIndex;
  uint geometryIndex;
};
// Type: RayObjectParams
struct nish_std_RayObjectParams {
  vec3 objectRayOrigin;
  vec3 objectRayDirection;
};
// Type: RayHitInfo
struct nish_std_RayHitInfo {
  float hitT;
  uint hitKind;
};
// Type: RayTransforms
struct nish_std_RayTransforms {
  mat4x3 objectToWorld;
  mat4x3 worldToObject;
};
// TypeMethFwd: RayPayload
nish_std_RayPayload nish_std_RayPayload_new(vec4 a_color, float a_hitT);
// FunctionFwd: nish:std
// TypeMeth: RayPayload
nish_std_RayPayload nish_std_RayPayload_new(vec4 a_color, float a_hitT) {
  nish_std_RayPayload t;
  t.color = a_color;
  t.hitT = a_hitT;
  return t;
}
// Function: nish:std
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
nish_std_RayPayload TestGpuFuncs_triangle_rchit(nish_std_RayHitInfo aHitInfo, nish_std_RayGeometryIds aIds, nish_std_RayObjectParams aObject, nish_std_RayWorldParams aWorld, nish_std_RayParams aRay, nish_std_RayTransforms aTransforms);

// Function: TestGpuFuncs
nish_std_RayPayload TestGpuFuncs_triangle_rchit(nish_std_RayHitInfo aHitInfo, nish_std_RayGeometryIds aIds, nish_std_RayObjectParams aObject, nish_std_RayWorldParams aWorld, nish_std_RayParams aRay, nish_std_RayTransforms aTransforms) {
  vec4 _tmp_1 = vec4(0.0,1.0,0.0,1.0);
  float _tmp_6 = aHitInfo.hitT;
  return nish_std_RayPayload_new(_tmp_1, _tmp_6);
}
// MODULE END TestGpuFuncs

// Ray Shader main: TestGpuFuncs_triangle_rchit
void main(void) {
  nish_std_RayHitInfo aHitInfo;
  nish_std_RayGeometryIds aIds;
  nish_std_RayObjectParams aObject;
  nish_std_RayWorldParams aWorld;
  nish_std_RayParams aRay;
  nish_std_RayTransforms aTransforms;
  aHitInfo.hitT = gl_HitTEXT;
  aHitInfo.hitKind = gl_HitKindEXT;
  aIds.primitiveIndex = uint(gl_PrimitiveID);
  aIds.instanceImplIndex = uint(gl_InstanceID);
  aIds.instanceUserIndex = uint(gl_InstanceCustomIndexEXT);
  aIds.geometryIndex = uint(gl_GeometryIndexEXT);
  aObject.objectRayOrigin = gl_ObjectRayOriginEXT;
  aObject.objectRayDirection = gl_ObjectRayDirectionEXT;
  aWorld.worldRayOrigin = gl_WorldRayOriginEXT;
  aWorld.worldRayDirection = gl_WorldRayDirectionEXT;
  aRay.rayTMin = gl_RayTminEXT;
  aRay.rayTMax = gl_RayTmaxEXT;
  aRay.incomingRayFlags = gl_IncomingRayFlagsEXT;
  aTransforms.objectToWorld = gl_ObjectToWorldEXT;
  aTransforms.worldToObject = gl_WorldToObjectEXT;
  nish_std_RayPayload _rval_ = TestGpuFuncs_triangle_rchit(aHitInfo, aIds, aObject, aWorld, aRay, aTransforms);
  nil_builtin_RAY_PAYLOAD_0 = _rval_;
}

