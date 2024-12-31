#version 460
#extension GL_EXT_ray_tracing : require
// DO IMPORTS BEGIN TestGpuFuncs
// MODULE BEGIN lib:shader
// MODULE PROLOGUE BEGIN lib:shader
mat4 nil_Mat4x3ToMat4x4(mat4x3 m) { return mat4(vec4(m[0], 0), vec4(m[1], 0), vec4(m[2], 0), vec4(m[3], 1)); }
mat4 nil_Mat3x4ToMat4x4(mat3x4 m) { return mat4(vec4(m[0]), vec4(m[1]), vec4(m[2]), vec4(0,0,0,1)); }
// MODULE PROLOGUE END lib:shader
// Type: RayGeometryIds
struct lib_shader_RayGeometryIds {
  int primitiveId;
  int instanceId;
  int instanceCustomIndex;
  int geometryIndex;
};
// Type: RayHitInfo
struct lib_shader_RayHitInfo {
  float hitT;
  uint hitKind;
};
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
// Type: RayPayload
struct lib_shader_RayPayload {
  vec4 color;
  float hitT;
};
layout(location = 0) rayPayloadInEXT lib_shader_RayPayload nil_builtin_RAY_PAYLOAD_0;
// Type: RayTransforms
struct lib_shader_RayTransforms {
  mat4 objectToWorld;
  mat4 worldToObject;
};
// Type: RayWorldParams
struct lib_shader_RayWorldParams {
  vec3 worldRayOrigin;
  vec3 worldRayDirection;
};
// TypeMethFwd: RayPayload
lib_shader_RayPayload lib_shader_RayPayload_new(vec4 a_color, float a_hitT);
// FunctionFwd: lib:shader
// TypeMeth: RayPayload
lib_shader_RayPayload lib_shader_RayPayload_new(vec4 a_color, float a_hitT) {
  lib_shader_RayPayload t;
  t.color = a_color;
  t.hitT = a_hitT;
  return t;
}
// Function: lib:shader
// MODULE END lib:shader
// DO IMPORTS END TestGpuFuncs

// MODULE BEGIN TestGpuFuncs

// FunctionFwd: TestGpuFuncs
lib_shader_RayPayload TestGpuFuncs_triangle_rchit(lib_shader_RayHitInfo aHitInfo, lib_shader_RayGeometryIds aIds, lib_shader_RayObjectParams aObject, lib_shader_RayWorldParams aWorld, lib_shader_RayParams aRay, lib_shader_RayTransforms aTransforms);

// Function: TestGpuFuncs
lib_shader_RayPayload TestGpuFuncs_triangle_rchit(lib_shader_RayHitInfo aHitInfo, lib_shader_RayGeometryIds aIds, lib_shader_RayObjectParams aObject, lib_shader_RayWorldParams aWorld, lib_shader_RayParams aRay, lib_shader_RayTransforms aTransforms) {
  vec4 _tmp_1 = vec4(1.0,0.0,0.0,1.0);
  float _tmp_6 = aHitInfo.hitT;
  return lib_shader_RayPayload_new(_tmp_1, _tmp_6);
}
// MODULE END TestGpuFuncs

// Ray Shader main: TestGpuFuncs_triangle_rchit
void main(void) {
  lib_shader_RayHitInfo aHitInfo;
  lib_shader_RayGeometryIds aIds;
  lib_shader_RayObjectParams aObject;
  lib_shader_RayWorldParams aWorld;
  lib_shader_RayParams aRay;
  lib_shader_RayTransforms aTransforms;
  aHitInfo.hitT = gl_HitTEXT;
  aHitInfo.hitKind = gl_HitKindEXT;
  aIds.primitiveId = gl_PrimitiveID;
  aIds.instanceId = gl_InstanceID;
  aIds.instanceCustomIndex = gl_InstanceCustomIndexEXT;
  aIds.geometryIndex = gl_GeometryIndexEXT;
  aObject.objectRayOrigin = gl_ObjectRayOriginEXT;
  aObject.objectRayDirection = gl_ObjectRayDirectionEXT;
  aWorld.worldRayOrigin = gl_WorldRayOriginEXT;
  aWorld.worldRayDirection = gl_WorldRayDirectionEXT;
  aRay.rayTMin = gl_RayTminEXT;
  aRay.rayTMax = gl_RayTmaxEXT;
  aRay.incomingRayFlags = gl_IncomingRayFlagsEXT;
  aTransforms.objectToWorld = nil_Mat4x3ToMat4x4(gl_ObjectToWorldEXT);
  aTransforms.worldToObject = nil_Mat4x3ToMat4x4(gl_WorldToObjectEXT);
  lib_shader_RayPayload _rval_ = TestGpuFuncs_triangle_rchit(aHitInfo, aIds, aObject, aWorld, aRay, aTransforms);
  nil_builtin_RAY_PAYLOAD_0 = _rval_;
}

