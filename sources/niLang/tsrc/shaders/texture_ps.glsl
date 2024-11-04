// glslangValidator -S frag -V "$WORK/niLang/sources/niLang/tsrc/shaders/texture_ps.glsl" -o "$WORK/niLang/sources/niLang/tsrc/shaders/texture_ps.spv" && bin2h "$WORK/niLang/sources/niLang/tsrc/shaders/texture_ps.spv" "$WORK/niLang/sources/niLang/tsrc/shaders/texture_ps.spv.h" texture_ps_spv
#version 450
layout(binding = 0) uniform texture2D texture0;
layout(binding = 1) uniform sampler sampler0;

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inTex1;
layout(location = 0) out vec4 outColor;

void main() {
  vec4 texColor = texture(sampler2D(texture0,sampler0), inTex1);

  outColor = texColor * inColor;
}
