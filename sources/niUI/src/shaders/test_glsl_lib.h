#ifdef __INPUT_HLSL__
// Standard vertex output
struct StdVertexOutput
{
  float4 ClipPosition : POSITION;
  float4  ColorA  : COLOR0;
  float4  Tex0 : TEXCOORD1;
};
#endif

float foolib(float a, float b) {
  return a + b;
}
