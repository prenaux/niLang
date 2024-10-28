// glslangValidator -S vert -V triangle_vs.glsl -o triangle_vs.spv && bin2h triangle_vs.spv triangle_vs.spv.h triangle_vs_spv
#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in uint inColor;

layout(location = 0) out vec4 outColor;

void main() {
  gl_Position = vec4(inPosition, 1.0);
  outColor = vec4(
    float((inColor >> 16) & 0xFF) / 255.0,  // B
    float((inColor >> 8) & 0xFF) / 255.0,   // G
    float(inColor & 0xFF) / 255.0,          // R
    float((inColor >> 24) & 0xFF) / 255.0   // A
  );
}
