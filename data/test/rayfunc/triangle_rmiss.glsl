#version 460
#extension GL_EXT_ray_tracing : require
// DO IMPORTS BEGIN TestGpuFuncs
// MODULE BEGIN lib:shader
// MODULE PROLOGUE BEGIN lib:shader
mat4 nil_Mat4x3ToMat4x4(mat4x3 m) { return mat4(vec4(m[0], 0), vec4(m[1], 0), vec4(m[2], 0), vec4(m[3], 1)); }
mat4 nil_Mat3x4ToMat4x4(mat3x4 m) { return mat4(vec4(m[0]), vec4(m[1]), vec4(m[2]), vec4(0,0,0,1)); }
// MODULE PROLOGUE END lib:shader
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
lib_shader_RayPayload TestGpuFuncs_triangle_rmiss();

// Function: TestGpuFuncs
lib_shader_RayPayload TestGpuFuncs_triangle_rmiss() {
  vec4 _tmp_1 = vec4(0.0,1.0,1.0,0.0);
  float _tmp_6 = 0.0;
  return lib_shader_RayPayload_new(_tmp_1, _tmp_6);
}
// MODULE END TestGpuFuncs

// Ray Shader main: TestGpuFuncs_triangle_rmiss
void main(void) {
  lib_shader_RayPayload _rval_ = TestGpuFuncs_triangle_rmiss();
  nil_builtin_RAY_PAYLOAD_0 = _rval_;
}

