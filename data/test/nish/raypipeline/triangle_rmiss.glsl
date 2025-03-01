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
nish_std_RayPayload TestGpuFuncs_triangle_rmiss();

// Function: TestGpuFuncs
nish_std_RayPayload TestGpuFuncs_triangle_rmiss() {
  vec4 _tmp_1 = vec4(0.0,0.5,0.8,0.0);
  float _tmp_6 = 0.0;
  return nish_std_RayPayload_new(_tmp_1, _tmp_6);
}
// MODULE END TestGpuFuncs

// Ray Shader main: TestGpuFuncs_triangle_rmiss
void main(void) {
  nish_std_RayPayload _rval_ = TestGpuFuncs_triangle_rmiss();
  nil_builtin_RAY_PAYLOAD_0 = _rval_;
}

