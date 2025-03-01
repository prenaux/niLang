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
// Type: RayHitPayload
struct nish_std_RayHitPayload {
  vec2 baryCoord;
};
hitAttributeEXT nish_std_RayHitPayload _hitAttributeEXT_RayHitPayload;
// Type: RayGeometryIds
struct nish_std_RayGeometryIds {
  uint primitiveIndex;
  uint instanceImplIndex;
  uint instanceUserIndex;
  uint geometryIndex;
};
// Type: RayHitInfo
struct nish_std_RayHitInfo {
  float hitT;
  uint hitKind;
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
nish_std_RayPayload TestGpuFuncs_triangle_bary_rchit(nish_std_RayHitInfo aHitInfo, nish_std_RayGeometryIds aIds, nish_std_RayHitPayload aHitPayload);

// Function: TestGpuFuncs
nish_std_RayPayload TestGpuFuncs_triangle_bary_rchit(nish_std_RayHitInfo aHitInfo, nish_std_RayGeometryIds aIds, nish_std_RayHitPayload aHitPayload) {
  vec4 color = vec4(aHitPayload.baryCoord.x,aHitPayload.baryCoord.y,0.0,1.0);
  vec4 _tmp_9 = color;
  float _tmp_a = aHitInfo.hitT;
  return nish_std_RayPayload_new(_tmp_9, _tmp_a);
}
// MODULE END TestGpuFuncs

// Ray Shader main: TestGpuFuncs_triangle_bary_rchit
void main(void) {
  nish_std_RayHitInfo aHitInfo;
  nish_std_RayGeometryIds aIds;
  nish_std_RayHitPayload aHitPayload;
  aHitInfo.hitT = gl_HitTEXT;
  aHitInfo.hitKind = gl_HitKindEXT;
  aIds.primitiveIndex = uint(gl_PrimitiveID);
  aIds.instanceImplIndex = uint(gl_InstanceID);
  aIds.instanceUserIndex = uint(gl_InstanceCustomIndexEXT);
  aIds.geometryIndex = uint(gl_GeometryIndexEXT);
  aHitPayload.baryCoord = _hitAttributeEXT_RayHitPayload.baryCoord;
  nish_std_RayPayload _rval_ = TestGpuFuncs_triangle_bary_rchit(aHitInfo, aIds, aHitPayload);
  nil_builtin_RAY_PAYLOAD_0 = _rval_;
}

