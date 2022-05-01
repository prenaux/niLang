using namespace metal;

struct InVertex {
  metal::packed_float3 pos;
  // metal::packed_uchar4 causes the compiler to crash on AMD GPUs when creating a new render pipeline
  metal::uchar4 colora;
};

struct OutVertex {
  float4 position [[position]];
  float4 color;
};

vertex OutVertex vertex_main(
  device InVertex *vertices [[buffer(0)]],
  uint vid [[vertex_id]])
{
  const InVertex inV = vertices[vid];
  OutVertex outV;
  outV.position = float4(inV.pos,1);
  outV.color = float4(uchar4(inV.colora).bgra) / 255;
  return outV;
}

fragment float4 fragment_main(OutVertex inVertex [[stage_in]])
{
  return inVertex.color;
}
