// glslangValidator -S frag -V triangle_ps.glsl -o triangle_ps.spv && bin2h triangle_ps.spv triangle_ps.spv.h triangle_ps
#version 450

layout(location = 0) in vec4 inColor;
layout(location = 0) out vec4 outColor;

void main() {
  outColor = inColor;
}
