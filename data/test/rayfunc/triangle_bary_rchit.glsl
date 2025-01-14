#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_shader_image_load_formatted : enable
// DO IMPORTS BEGIN TestGpuFuncs
// MODULE BEGIN lib:shader
// MODULE PROLOGUE BEGIN lib:shader
mat4 nil_Mat4x3ToMat4x4(mat4x3 m) { return mat4(vec4(m[0], 0), vec4(m[1], 0), vec4(m[2], 0), vec4(m[3], 1)); }
mat4 nil_Mat3x4ToMat4x4(mat3x4 m) { return mat4(vec4(m[0]), vec4(m[1]), vec4(m[2]), vec4(0,0,0,1)); }
// MODULE PROLOGUE END lib:shader
// Type: RayGeometryIds
struct lib_shader_RayGeometryIds {
  uint primitiveIndex;
  uint instanceIndex;
  uint customInstanceId;
  uint geometryIndex;
};
// Type: RayHitInfo
struct lib_shader_RayHitInfo {
  float hitT;
  uint hitKind;
};
hitAttributeEXT vec2 _hitAttributeEXT_baryCoord;
// Type: RayHitPayload
struct lib_shader_RayHitPayload {
  vec2 baryCoord;
};
// Type: RayPayload
struct lib_shader_RayPayload {
  vec4 color;
  float hitT;
};
layout(location = 0) rayPayloadInEXT lib_shader_RayPayload nil_builtin_RAY_PAYLOAD_0;
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
lib_shader_RayPayload TestGpuFuncs_triangle_bary_rchit(lib_shader_RayHitInfo aHitInfo, lib_shader_RayGeometryIds aIds, lib_shader_RayHitPayload aHitPayload);

// Function: TestGpuFuncs
lib_shader_RayPayload TestGpuFuncs_triangle_bary_rchit(lib_shader_RayHitInfo aHitInfo, lib_shader_RayGeometryIds aIds, lib_shader_RayHitPayload aHitPayload) {
  vec4 color = vec4(aHitPayload.baryCoord.x,aHitPayload.baryCoord.y,0.0,1.0);
  vec4 _tmp_9 = color;
  float _tmp_a = aHitInfo.hitT;
  return lib_shader_RayPayload_new(_tmp_9, _tmp_a);
}
// MODULE END TestGpuFuncs

// Ray Shader main: TestGpuFuncs_triangle_bary_rchit
void main(void) {
  lib_shader_RayHitInfo aHitInfo;
  lib_shader_RayGeometryIds aIds;
  lib_shader_RayHitPayload aHitPayload;
  aHitInfo.hitT = gl_HitTEXT;
  aHitInfo.hitKind = gl_HitKindEXT;
  aIds.primitiveIndex = uint(gl_PrimitiveID);
  aIds.instanceIndex = uint(gl_InstanceID);
  aIds.customInstanceId = uint(gl_InstanceCustomIndexEXT);
  aIds.geometryIndex = uint(gl_GeometryIndexEXT);
  aHitPayload.baryCoord = _hitAttributeEXT_baryCoord;
  lib_shader_RayPayload _rval_ = TestGpuFuncs_triangle_bary_rchit(aHitInfo, aIds, aHitPayload);
  nil_builtin_RAY_PAYLOAD_0 = _rval_;
}

