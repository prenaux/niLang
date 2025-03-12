#version 460
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_ray_query : require
// DO IMPORTS BEGIN TestGpuFuncs
// MODULE BEGIN nish:std
// Type: PixelInput
struct nish_std_PixelInput {
  vec4 fragCoord;
  bool frontFacing;
};
// Type: PixelOutput
struct nish_std_PixelOutput {
  vec4 color;
};
// TypeStaticFwd: RayFlags
uint nish_std_RayFlags_None;
uint nish_std_RayFlags_Opaque;
uint nish_std_RayFlags_SkipClosestHitShader;
// TypeStaticFwd: RayQueryIntersectionType
uint nish_std_RayQueryIntersectionType_CommittedTriangle;
uint nish_std_RayQueryIntersectionType_CommittedBoundingVolume;
// TypeStaticFwd: Vec2
vec2 vec2_Zero;
// TypeStaticFwd: Vec3
vec3 vec3_Zero;
// TypeMethFwd: PixelOutput
nish_std_PixelOutput nish_std_PixelOutput_new(vec4 a_color);
// FunctionFwd: nish:std
vec3 nish_std_UIntToHashColor(uint i);
uint nish_std_HashJenkins(uint aStartX);
layout(set = 10, binding = 0) uniform texture2D nil_builtin_GetTexture2D[];
vec3 nish_std_Vec3Transform_2_Vec3_Matrix4x3(vec3 v, mat4x3 m);
vec3 nish_std_Vec3TransformNormal_2_Vec3_Matrix4x3(vec3 v, mat4x3 m);
vec3 nish_std_Vec3TransformCoord(vec3 v, mat4 m);
void nish_std_RayFlags_static_initialize() {
  // TypeStatic: RayFlags
  nish_std_RayFlags_None = 0;
  nish_std_RayFlags_Opaque = 1;
  nish_std_RayFlags_SkipClosestHitShader = 8;
}
void nish_std_RayQueryIntersectionType_static_initialize() {
  // TypeStatic: RayQueryIntersectionType
  nish_std_RayQueryIntersectionType_CommittedTriangle = 1;
  nish_std_RayQueryIntersectionType_CommittedBoundingVolume = 2;
}
void vec2_static_initialize() {
  // TypeStatic: Vec2
  vec2_Zero = vec2(0.0,0.0);
}
void vec3_static_initialize() {
  // TypeStatic: Vec3
  vec3_Zero = vec3(0.0,0.0,0.0);
}
// TypeMeth: PixelOutput
nish_std_PixelOutput nish_std_PixelOutput_new(vec4 a_color) {
  nish_std_PixelOutput t;
  t.color = a_color;
  return t;
}
// Function: nish:std
vec3 nish_std_UIntToHashColor(uint i) {
  uint hash = nish_std_HashJenkins(i);
  return vec3((float(((hash >> 0) & 255)) / 255.0),(float(((hash >> 8) & 255)) / 255.0),(float(((hash >> 16) & 255)) / 255.0));
}
uint nish_std_HashJenkins(uint aStartX) {
  /* mut */ uint x = aStartX;
  x = (x + (x << 10));
  x = (x ^ (x >> 6));
  x = (x + (x << 3));
  x = (x ^ (x >> 11));
  x = (x + (x << 15));
  return x;
}
vec3 nish_std_Vec3Transform_2_Vec3_Matrix4x3(vec3 v, mat4x3 m) {
  vec3 _tmp_U = v;
  return (m * vec4(_tmp_U.x,_tmp_U.y,_tmp_U.z,1.0));
}
vec3 nish_std_Vec3TransformNormal_2_Vec3_Matrix4x3(vec3 v, mat4x3 m) {
  vec3 _tmp_Y = v;
  vec3 r = (m * vec4(_tmp_Y.x,_tmp_Y.y,_tmp_Y.z,0.0));
  return (r.xyz);
}
vec3 nish_std_Vec3TransformCoord(vec3 v, mat4 m) {
  vec3 _tmp_41 = v;
  vec4 r = (m * vec4(_tmp_41.x,_tmp_41.y,_tmp_41.z,1.0));
  return ((r/r.w).xyz);
}
// ModuleInitialize: nish_std
void nish_std_initialize() {
  nish_std_RayFlags_static_initialize();
  nish_std_RayQueryIntersectionType_static_initialize();
  vec2_static_initialize();
  vec3_static_initialize();
}
// MODULE END nish:std
// DO IMPORTS END TestGpuFuncs

// MODULE BEGIN TestGpuFuncs

// Type: RayUniforms
struct TestGpuFuncs_RayUniforms {
  float rtWidth;
  float rtHeight;
  float cameraFarClipPlane;
  float padding;
  mat4 cameraInvView;
  mat4 cameraInvViewProj;
};

// Type: RayInstanceData
struct TestGpuFuncs_RayInstanceData {
  uint ibIndex;
  uint vbIndex;
  uint texIndex;
  uint firstIndex;
  uint baseVertexIndex;
};

// Type: VertexRay
struct TestGpuFuncs_VertexRay {
  vec3 pos;
  vec3 normal;
  uint color;
  vec2 tex0;
};

// Type: IntersectionResult
struct TestGpuFuncs_IntersectionResult {
  int type;
  uint userInstIndex;
  uint primIndex;
  TestGpuFuncs_RayInstanceData instData;
  vec3 bary;
  vec3 worldPos;
  vec3 worldNormal;
  vec2 uv;
};

// TypeStaticFwd: IntersectionType
int TestGpuFuncs_IntersectionType_InvalidTriangle;
int TestGpuFuncs_IntersectionType_None;
int TestGpuFuncs_IntersectionType_Triangle;
int TestGpuFuncs_IntersectionType_BoundingVolume;

// TypeMethFwd: RayInstanceData
TestGpuFuncs_RayInstanceData TestGpuFuncs_RayInstanceData_new_default();

// TypeMethFwd: IntersectionResult
TestGpuFuncs_IntersectionResult TestGpuFuncs_IntersectionResult_new_default();
bool TestGpuFuncs_IntersectionResult_SetFromCommittedIntersection(inout /* mut */ TestGpuFuncs_IntersectionResult r, rayQueryEXT aRayQuery);
void TestGpuFuncs_IntersectionResult_SetFromTriangleData(inout /* mut */ TestGpuFuncs_IntersectionResult r, uint aVBIndex, uint aIBIndex, uint aPrimIndex, uint aFirstIndex, vec2 aIntersectionBaryCentrics, mat4x3 aObjectToWorld);

// FunctionFwd: TestGpuFuncs
layout(scalar, set = 9, binding = 0) readonly buffer SBO_TestGpuFuncs_RayInstanceData { TestGpuFuncs_RayInstanceData v; } nil_builtin_GetRayInstanceData[];
layout(scalar, set = 9, binding = 0) readonly buffer SBO_GetIndex { uint v[]; } nil_builtin_GetIndex[];
layout(scalar, set = 9, binding = 0) readonly buffer SBO_GetVertexRay { TestGpuFuncs_VertexRay v[]; } nil_builtin_GetVertexRay[];
uvec3 TestGpuFuncs_GetTriangleIndices(uint aIBIndex, uint aPrimIndex, uint aFirstIndex);
vec3 TestGpuFuncs_BaryToVec3(vec2 aBary);
vec2 TestGpuFuncs_Lerp_4_Vec2_Vec2_Vec2_Vec3(vec2 aX, vec2 aY, vec2 aZ, vec3 aBary);
vec3 TestGpuFuncs_Lerp_4_Vec3_Vec3_Vec3_Vec3(vec3 aX, vec3 aY, vec3 aZ, vec3 aBary);
float TestGpuFuncs_SkyStep(float a, float b, float v);
vec3 TestGpuFuncs_ComputeSimpleSkyColor(vec3 aRayDir, vec3 aSunDir);
float TestGpuFuncs_CosineBiasSat(float v, float b);
float TestGpuFuncs_DirShadowRay(vec3 aPos, vec3 aDir, accelerationStructureEXT aAS);
vec3 TestGpuFuncs_DirLight(vec3 worldPos, vec3 worldNormal, vec3 worldLightDir, vec3 lightColor, float cosBias, vec3 shadowColor, accelerationStructureEXT aAS);
nish_std_PixelOutput TestGpuFuncs_raytracer_lit_textured_cube_ps(nish_std_PixelInput aInput, TestGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS, sampler aSS);
void TestGpuFuncs_IntersectionType_static_initialize() {

  // TypeStatic: IntersectionType
  TestGpuFuncs_IntersectionType_InvalidTriangle = -1;
  TestGpuFuncs_IntersectionType_None = 0;
  TestGpuFuncs_IntersectionType_Triangle = 1;
  TestGpuFuncs_IntersectionType_BoundingVolume = 2;
}

// TypeMeth: RayInstanceData
TestGpuFuncs_RayInstanceData TestGpuFuncs_RayInstanceData_new_default() {
  TestGpuFuncs_RayInstanceData t;
  t.ibIndex = 0;
  t.vbIndex = 0;
  t.texIndex = 0;
  t.firstIndex = 0;
  t.baseVertexIndex = 0;
  return t;
}

// TypeMeth: IntersectionResult
TestGpuFuncs_IntersectionResult TestGpuFuncs_IntersectionResult_new_default() {
  TestGpuFuncs_IntersectionResult t;
  t.type = TestGpuFuncs_IntersectionType_None;
  t.userInstIndex = 0;
  t.primIndex = 0;
  t.instData = TestGpuFuncs_RayInstanceData_new_default() /*SKIPPED COPY VALUETYPE: newed*/;
  t.bary = vec3_Zero;
  t.worldPos = vec3_Zero;
  t.worldNormal = vec3_Zero;
  t.uv = vec2_Zero;
  return t;
}
bool TestGpuFuncs_IntersectionResult_SetFromCommittedIntersection(inout /* mut */ TestGpuFuncs_IntersectionResult r, rayQueryEXT aRayQuery) {
  uint committedType = rayQueryGetIntersectionTypeEXT(aRayQuery,true);
  r.userInstIndex = uint(rayQueryGetIntersectionInstanceCustomIndexEXT(aRayQuery,true));
  r.instData = nil_builtin_GetRayInstanceData[nonuniformEXT(r.userInstIndex)].v /*SKIPPED COPY VALUETYPE: funcret*/;
  bool _tmp_6 = (committedType == nish_std_RayQueryIntersectionType_CommittedTriangle);
  if (_tmp_6) {
    r.primIndex = uint(rayQueryGetIntersectionPrimitiveIndexEXT(aRayQuery,true));
    TestGpuFuncs_IntersectionResult_SetFromTriangleData(r,r.instData.vbIndex,r.instData.ibIndex,r.primIndex,r.instData.firstIndex,rayQueryGetIntersectionBarycentricsEXT(aRayQuery,true),rayQueryGetIntersectionObjectToWorldEXT(aRayQuery,true));
  }
  else {
    bool _tmp_u = (committedType == nish_std_RayQueryIntersectionType_CommittedBoundingVolume);
    if (_tmp_u) {
      r.type = TestGpuFuncs_IntersectionType_BoundingVolume;
    }
  }
  return false;
}
void TestGpuFuncs_IntersectionResult_SetFromTriangleData(inout /* mut */ TestGpuFuncs_IntersectionResult r, uint aVBIndex, uint aIBIndex, uint aPrimIndex, uint aFirstIndex, vec2 aIntersectionBaryCentrics, mat4x3 aObjectToWorld) {
  bool _tmp_B = (aVBIndex == 0);
  if (_tmp_B) {
    r.type = TestGpuFuncs_IntersectionType_InvalidTriangle;
  }
  else {
    {
      r.type = TestGpuFuncs_IntersectionType_Triangle;
      uvec3 inds = TestGpuFuncs_GetTriangleIndices(aIBIndex,aPrimIndex,aFirstIndex);
      TestGpuFuncs_VertexRay v0 = nil_builtin_GetVertexRay[nonuniformEXT(aVBIndex)].v[nonuniformEXT(inds.x)];
      TestGpuFuncs_VertexRay v1 = nil_builtin_GetVertexRay[nonuniformEXT(aVBIndex)].v[nonuniformEXT(inds.y)];
      TestGpuFuncs_VertexRay v2 = nil_builtin_GetVertexRay[nonuniformEXT(aVBIndex)].v[nonuniformEXT(inds.z)];
      r.bary = TestGpuFuncs_BaryToVec3(aIntersectionBaryCentrics);
      vec3 n = normalize(TestGpuFuncs_Lerp_4_Vec3_Vec3_Vec3_Vec3(v0.normal,v1.normal,v2.normal,r.bary));
      r.worldNormal = normalize(nish_std_Vec3TransformNormal_2_Vec3_Matrix4x3(n,aObjectToWorld));
      vec3 p = TestGpuFuncs_Lerp_4_Vec3_Vec3_Vec3_Vec3(v0.pos,v1.pos,v2.pos,r.bary);
      r.worldPos = nish_std_Vec3Transform_2_Vec3_Matrix4x3(p,aObjectToWorld);
      r.uv = TestGpuFuncs_Lerp_4_Vec2_Vec2_Vec2_Vec3(v0.tex0,v1.tex0,v2.tex0,r.bary);
    }
  }
}

// Function: TestGpuFuncs
uvec3 TestGpuFuncs_GetTriangleIndices(uint aIBIndex, uint aPrimIndex, uint aFirstIndex) {
  uint ibase = (aFirstIndex + (3 * aPrimIndex));
  return uvec3(nil_builtin_GetIndex[nonuniformEXT(aIBIndex)].v[nonuniformEXT((ibase + 0))],nil_builtin_GetIndex[nonuniformEXT(aIBIndex)].v[nonuniformEXT((ibase + 1))],nil_builtin_GetIndex[nonuniformEXT(aIBIndex)].v[nonuniformEXT((ibase + 2))]);
}
vec3 TestGpuFuncs_BaryToVec3(vec2 aBary) {
  return vec3(((1.0 - aBary.x) - aBary.y),aBary.x,aBary.y);
}
vec2 TestGpuFuncs_Lerp_4_Vec2_Vec2_Vec2_Vec3(vec2 aX, vec2 aY, vec2 aZ, vec3 aBary) {
  return (((aX*aBary.x)+(aY*aBary.y))+(aZ*aBary.z));
}
vec3 TestGpuFuncs_Lerp_4_Vec3_Vec3_Vec3_Vec3(vec3 aX, vec3 aY, vec3 aZ, vec3 aBary) {
  return (((aX*aBary.x)+(aY*aBary.y))+(aZ*aBary.z));
}
float TestGpuFuncs_SkyStep(float a, float b, float v) {
  return smoothstep(a, b, v);
}
vec3 TestGpuFuncs_ComputeSimpleSkyColor(vec3 aRayDir, vec3 aSunDir) {
  vec3 groundColor = vec3(0.35,0.3,0.35);
  vec3 skyColorHorizon = vec3(1.0,1.0,1.0);
  vec3 skyColorZenith = vec3(0.08,0.37,0.73);
  vec3 sunColor = vec3(1.0,0.9,0.7);
  float sunSize = 0.0025;
  float sunIntensity = 10.0;
  float skyGradientT = pow(TestGpuFuncs_SkyStep(0.0001,0.4,aRayDir.y),0.35);
  vec3 skyGradient = mix(skyColorHorizon,skyColorZenith,skyGradientT);
  float groundToSkyT = TestGpuFuncs_SkyStep(-0.02,0.0,aRayDir.y);
  float cosAngle = dot(aRayDir,aSunDir);
  float sunDisk = (TestGpuFuncs_SkyStep((1.0 - sunSize),1.0,cosAngle) * sunIntensity);
  float sunVisibility = float((groundToSkyT >= 1.0));
  return (mix(groundColor,skyGradient,groundToSkyT)+((sunDisk*sunColor)*sunVisibility));
}
float TestGpuFuncs_CosineBiasSat(float v, float b) {
  return clamp(max(0.0,((v * (1.0 - b)) + b)),0.0,1.0);
}
float TestGpuFuncs_DirShadowRay(vec3 aPos, vec3 aDir, accelerationStructureEXT aAS) {
  /* mut */ rayQueryEXT rayQuery/*__noinit__*/;
  rayQueryInitializeEXT(rayQuery,aAS,(nish_std_RayFlags_SkipClosestHitShader | nish_std_RayFlags_Opaque),255,aPos,0.001,aDir,10000.0);
  rayQueryProceedEXT(rayQuery);
  bool hasHit = (rayQueryGetIntersectionTypeEXT(rayQuery,true) == nish_std_RayQueryIntersectionType_CommittedTriangle);
  float _tmp_R3;
  bool _tmp_S3 = hasHit;
  if (_tmp_S3) {
    _tmp_R3 = 0.0;
  }
  else {
    {
      _tmp_R3 = 1.0;
    }
  }
  return _tmp_R3;
}
vec3 TestGpuFuncs_DirLight(vec3 worldPos, vec3 worldNormal, vec3 worldLightDir, vec3 lightColor, float cosBias, vec3 shadowColor, accelerationStructureEXT aAS) {
  float att = TestGpuFuncs_DirShadowRay(worldPos,-worldLightDir,aAS);
  vec3 D = lightColor;
  vec3 S = vec3_Zero;
  float NdotL = TestGpuFuncs_CosineBiasSat(dot(worldNormal,-worldLightDir),cosBias);
  vec3 C = (D+S);
  return mix((shadowColor*C),C,(att * NdotL));
}
nish_std_PixelOutput TestGpuFuncs_raytracer_lit_textured_cube_ps(nish_std_PixelInput aInput, TestGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS, sampler aSS) {
  /* mut */ rayQueryEXT rayQuery/*__noinit__*/;
  vec3 ndc = vec3((((aInput.fragCoord.x / aUniforms.rtWidth) * 2.0) - 1.0),(1.0 - ((aInput.fragCoord.y / aUniforms.rtHeight) * 2.0)),1.0);
  mat4 _tmp_x4 = aUniforms.cameraInvView;
  vec3 origin = vec3(_tmp_x4[3][0],_tmp_x4[3][1],_tmp_x4[3][2]);
  vec3 target = nish_std_Vec3TransformCoord(ndc,aUniforms.cameraInvViewProj);
  vec3 dir = normalize(((target-(origin.xyz)).xyz));
  rayQueryInitializeEXT(rayQuery,aAS,nish_std_RayFlags_None,255,(origin.xyz),0.001,(dir.xyz),aUniforms.cameraFarClipPlane);
  vec3 sunDir = normalize(vec3(0.1,0.1,1.0));
  float cosBias = 0.5;
  vec3 shadowColor = vec3(0.3,0.3,0.3);
  vec3 lightColor0 = vec3(0.8,0.8,0.8);
  vec3 worldLightDir0 = normalize(vec3(-0.5,-0.7,0.9));
  vec3 lightColor1 = vec3(0.4,0.2,0.0);
  vec3 worldLightDir1 = normalize(vec3(0.707107,0.0,0.707107));
  vec3 lightColor2 = vec3(0.0,0.2,0.4);
  vec3 worldLightDir2 = vec3(0.0,-1.0,0.0);
  bool done = rayQueryProceedEXT(rayQuery);
  /* mut */ TestGpuFuncs_IntersectionResult intersection = TestGpuFuncs_IntersectionResult_new_default() /*SKIPPED COPY VALUETYPE: newed*/;
  TestGpuFuncs_IntersectionResult_SetFromCommittedIntersection(intersection,rayQuery);
  /* mut */ TestGpuFuncs_RayInstanceData instData = intersection.instData /*COPY VALUETYPE: membervar*/;
  vec4 color;
  bool _tmp_n5 = (intersection.type == TestGpuFuncs_IntersectionType_Triangle);
  if (_tmp_n5) {
    vec3 worldPos = intersection.worldPos;
    vec3 worldNormal = intersection.worldNormal;
    vec2 uv = intersection.uv;
    vec3 texColor;
    bool _tmp_y5 = (instData.texIndex > 0);
    if (_tmp_y5) {
      texColor = (texture(sampler2D(nil_builtin_GetTexture2D[nonuniformEXT(instData.texIndex)],aSS),uv).rgb);
    }
    else {
      {
        texColor = nish_std_UIntToHashColor(intersection.userInstIndex);
      }
    }
    vec3 C = ((TestGpuFuncs_DirLight(worldPos,worldNormal,worldLightDir0,(lightColor0*texColor),cosBias,shadowColor,aAS)+TestGpuFuncs_DirLight(worldPos,worldNormal,worldLightDir1,(lightColor1*texColor),cosBias,shadowColor,aAS))+TestGpuFuncs_DirLight(worldPos,worldNormal,worldLightDir2,(lightColor2*texColor),cosBias,shadowColor,aAS));
    vec3 _tmp_h6 = C;
    color = vec4(_tmp_h6.x,_tmp_h6.y,_tmp_h6.z,1.0);
  }
  else {
    bool _tmp_i6 = (intersection.type == TestGpuFuncs_IntersectionType_BoundingVolume);
    if (_tmp_i6) {
      color = vec4(1.0,0.0,0.0,1.0);
    }
    else {
      bool _tmp_q6 = (intersection.type < TestGpuFuncs_IntersectionType_None);
      if (_tmp_q6) {
        color = vec4(1.0,0.0,1.0,1.0);
      }
      else {
        {
          vec3 _tmp_z6 = TestGpuFuncs_ComputeSimpleSkyColor(dir,sunDir);
          color = vec4(_tmp_z6.x,_tmp_z6.y,_tmp_z6.z,0.0);
        }
      }
    }
  }
  vec4 _tmp_E6 = color;
  return nish_std_PixelOutput_new(_tmp_E6);
}

// ModuleInitialize: TestGpuFuncs
void TestGpuFuncs_initialize() {
  TestGpuFuncs_IntersectionType_static_initialize();
}
// MODULE END TestGpuFuncs

// Pixel Shader main: TestGpuFuncs_raytracer_lit_textured_cube_ps
// type size: 48, underlying: float
layout(set = 0, binding = 0) uniform UBO_TestGpuFuncs_RayUniforms { TestGpuFuncs_RayUniforms v; } IN_1_aUniforms;
layout(set = 7, binding = 0) uniform accelerationStructureEXT IN_1_aAS;
layout(set = 5, binding = 0) uniform sampler IN_1_aSS;
layout(location = 0) out vec4 OUT_0_rval_color;
void main(void) {
  nish_std_initialize();
  TestGpuFuncs_initialize();
  nish_std_PixelInput aInput;
  TestGpuFuncs_RayUniforms aUniforms;
  aInput.fragCoord = gl_FragCoord;
  aInput.frontFacing = gl_FrontFacing;
  aUniforms = IN_1_aUniforms.v;
  nish_std_PixelOutput _rval_ = TestGpuFuncs_raytracer_lit_textured_cube_ps(aInput, aUniforms, IN_1_aAS, IN_1_aSS);
  OUT_0_rval_color = _rval_.color;
}

