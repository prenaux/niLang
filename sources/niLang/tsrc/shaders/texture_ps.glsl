// glslangValidator -S frag -V texture_ps.glsl -o texture_ps.spv && bin2h texture_ps.spv texture_ps.spv.h texture_ps_spv
#version 450
layout(binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec4 inColor;
layout(location = 1) in vec2 inTex1;
layout(location = 0) out vec4 outColor;

void main() {
  vec4 texColor = texture(texSampler, inTex1);

  outColor = texColor * inColor;
}
