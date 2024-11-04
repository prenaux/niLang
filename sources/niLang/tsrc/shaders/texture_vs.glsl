// glslangValidator -S vert -V "$WORK/niLang/sources/niLang/tsrc/shaders/texture_vs.glsl" -o "$WORK/niLang/sources/niLang/tsrc/shaders/texture_vs.spv" && bin2h "$WORK/niLang/sources/niLang/tsrc/shaders/texture_vs.spv" "$WORK/niLang/sources/niLang/tsrc/shaders/texture_vs.spv.h" texture_vs_spv
#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in uint inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 outTex1;

void main() {
  gl_Position = vec4(inPosition, 1.0);
  outColor = vec4(
    float((inColor >> 16) & 0xFF) / 255.0,  // B
    float((inColor >> 8) & 0xFF) / 255.0,   // G
    float(inColor & 0xFF) / 255.0,          // R
    float((inColor >> 24) & 0xFF) / 255.0   // A
  );
  outTex1 = inTexCoord;
}
