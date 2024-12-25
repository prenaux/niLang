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
layout(location = 0) rayPayloadEXT lib_shader_RayPayload nil_builtin_RAY_PAYLOAD_0;
// TypeStaticFwd: Vec4
vec4 vec4_Zero;
vec4 vec4_Black;
// TypeMethFwd: RayPayload
lib_shader_RayPayload lib_shader_RayPayload_new(vec4 a_color, float a_hitT);
// FunctionFwd: lib:shader
void vec4_static_initialize() {
  // TypeStatic: Vec4
  vec4_Zero = vec4(0.0,0.0,0.0,0.0);
  vec4_Black = vec4_Zero;
}
// TypeMeth: RayPayload
lib_shader_RayPayload lib_shader_RayPayload_new(vec4 a_color, float a_hitT) {
  lib_shader_RayPayload t;
  t.color = a_color;
  t.hitT = a_hitT;
  return t;
}
// Function: lib:shader
// ModuleInitialize: lib_shader
void lib_shader_initialize() {
  vec4_static_initialize();
}
// MODULE END lib:shader
// DO IMPORTS END TestGpuFuncs

// MODULE BEGIN TestGpuFuncs

// FunctionFwd: TestGpuFuncs
lib_shader_RayPayload TestGpuFuncs_triangle_rmiss();

// Function: TestGpuFuncs
lib_shader_RayPayload TestGpuFuncs_triangle_rmiss() {
  vec4 _tmp_1 = vec4_Black;
  float _tmp_2 = 0.0;
  return lib_shader_RayPayload_new(_tmp_1, _tmp_2);
}
// MODULE END TestGpuFuncs

// Ray Shader main: TestGpuFuncs_triangle_rmiss
void main(void) {
  lib_shader_initialize();
  lib_shader_RayPayload _rval_ = TestGpuFuncs_triangle_rmiss();
  nil_builtin_RAY_PAYLOAD_0 = _rval_;
}

