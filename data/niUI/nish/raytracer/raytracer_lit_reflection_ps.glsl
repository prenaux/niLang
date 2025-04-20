#version 460
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_ray_query : require
// DO IMPORTS BEGIN niUIGpuFuncs
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
uint nish_std_RayFlags_TerminateOnFirstHit;
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
layout(set = 10, binding = 0) uniform texture2D nil_builtin_GetTexture2D[];
vec3 nish_std_Vec3Transform_2_Vec3_Matrix4x3(vec3 v, mat4x3 m);
vec3 nish_std_Vec3TransformNormal_2_Vec3_Matrix4x3(vec3 v, mat4x3 m);
vec3 nish_std_Vec3TransformCoord(vec3 v, mat4 m);
void nish_std_RayFlags_static_initialize() {
  // TypeStatic: RayFlags
  nish_std_RayFlags_None = 0;
  nish_std_RayFlags_Opaque = 1;
  nish_std_RayFlags_TerminateOnFirstHit = 4;
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
vec3 nish_std_Vec3Transform_2_Vec3_Matrix4x3(vec3 v, mat4x3 m) {
  vec3 _tmp_9 = v;
  return (m * vec4(_tmp_9.x,_tmp_9.y,_tmp_9.z,1.0));
}
vec3 nish_std_Vec3TransformNormal_2_Vec3_Matrix4x3(vec3 v, mat4x3 m) {
  vec3 _tmp_d = v;
  vec3 r = (m * vec4(_tmp_d.x,_tmp_d.y,_tmp_d.z,0.0));
  return (r.xyz);
}
vec3 nish_std_Vec3TransformCoord(vec3 v, mat4 m) {
  vec3 _tmp_j = v;
  vec4 r = (m * vec4(_tmp_j.x,_tmp_j.y,_tmp_j.z,1.0));
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
// DO IMPORTS END niUIGpuFuncs

// MODULE BEGIN niUIGpuFuncs

// Type: RayInstanceData
struct niUIGpuFuncs_RayInstanceData {
  uint ibIndex;
  uint vbIndex;
  uint firstIndex;
  uint baseVertexIndex;
  uint texIndex;
  uint materialColor;
};

// Type: RayUniforms
struct niUIGpuFuncs_RayUniforms {
  float rtWidth;
  float rtHeight;
  float cameraFarClipPlane;
  float padding;
  mat4 cameraInvView;
  mat4 cameraInvViewProj;
};

// Type: VertexRay
struct niUIGpuFuncs_VertexRay {
  vec3 pos;
  vec3 normal;
  uint color;
  vec2 tex0;
};

// Type: _IntersectionTypeValue
struct niUIGpuFuncs__IntersectionTypeValue {
  int _v;
};

// Type: IntersectionResult
struct niUIGpuFuncs_IntersectionResult {
  niUIGpuFuncs__IntersectionTypeValue type;
  uint userInstIndex;
  uint primIndex;
  niUIGpuFuncs_RayInstanceData instData;
  vec3 bary;
  vec3 worldPos;
  vec3 worldNormal;
  vec2 uv;
};

// TypeStaticFwd: IntersectionType
niUIGpuFuncs__IntersectionTypeValue niUIGpuFuncs_IntersectionType_InvalidTriangle;
niUIGpuFuncs__IntersectionTypeValue niUIGpuFuncs_IntersectionType_None;
niUIGpuFuncs__IntersectionTypeValue niUIGpuFuncs_IntersectionType_Triangle;
niUIGpuFuncs__IntersectionTypeValue niUIGpuFuncs_IntersectionType_BoundingVolume;

// TypeMethFwd: RayInstanceData
niUIGpuFuncs_RayInstanceData niUIGpuFuncs_RayInstanceData_new_default();
vec4 niUIGpuFuncs_RayInstanceData_SampleMaterialDiffuseColor(niUIGpuFuncs_RayInstanceData aInstData, sampler aSS, vec2 aUV);

// TypeMethFwd: _IntersectionTypeValue
niUIGpuFuncs__IntersectionTypeValue niUIGpuFuncs__IntersectionTypeValue_new(int a__v);
bool niUIGpuFuncs__op__ne_2__IntersectionTypeValue__IntersectionTypeValue(niUIGpuFuncs__IntersectionTypeValue aLeft, niUIGpuFuncs__IntersectionTypeValue aRight);
bool niUIGpuFuncs__op__eq_2__IntersectionTypeValue__IntersectionTypeValue(niUIGpuFuncs__IntersectionTypeValue aLeft, niUIGpuFuncs__IntersectionTypeValue aRight);

// TypeMethFwd: IntersectionResult
niUIGpuFuncs_IntersectionResult niUIGpuFuncs_IntersectionResult_new_default();
bool niUIGpuFuncs_IntersectionResult_SetFromCommittedIntersection(inout /* mut */ niUIGpuFuncs_IntersectionResult r, rayQueryEXT aRayQuery);
void niUIGpuFuncs_IntersectionResult_SetFromTriangleData(inout /* mut */ niUIGpuFuncs_IntersectionResult r, uint aVBIndex, uint aIBIndex, uint aPrimIndex, uint aFirstIndex, uint aBaseVertexIndex, vec2 aIntersectionBaryCentrics, mat4x3 aObjectToWorld);
vec4 niUIGpuFuncs_IntersectionResult_ShadeIntersection(niUIGpuFuncs_IntersectionResult aIntersection, vec3 aViewDir, niUIGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS, sampler aSS);

// FunctionFwd: niUIGpuFuncs
layout(scalar, set = 9, binding = 0) readonly buffer SBO_niUIGpuFuncs_RayInstanceData { niUIGpuFuncs_RayInstanceData v; } nil_builtin_GetRayInstanceData[];
layout(scalar, set = 9, binding = 0) readonly buffer SBO_GetIndex { uint v[]; } nil_builtin_GetIndex[];
layout(scalar, set = 9, binding = 0) readonly buffer SBO_GetVertexRay { niUIGpuFuncs_VertexRay v[]; } nil_builtin_GetVertexRay[];
uvec3 niUIGpuFuncs_GetTriangleIndices(uint aIBIndex, uint aPrimIndex, uint aFirstIndex);
vec3 niUIGpuFuncs_BaryToVec3(vec2 aBary);
vec2 niUIGpuFuncs_Lerp_4_Vec2_Vec2_Vec2_Vec3(vec2 aX, vec2 aY, vec2 aZ, vec3 aBary);
vec3 niUIGpuFuncs_Lerp_4_Vec3_Vec3_Vec3_Vec3(vec3 aX, vec3 aY, vec3 aZ, vec3 aBary);
float niUIGpuFuncs_SkyStep(float a, float b, float v);
vec3 niUIGpuFuncs_ComputeSimpleSkyColor(vec3 aRayDir, vec3 aSunDir);
float niUIGpuFuncs_CosineBiasSat(float v, float b);
float niUIGpuFuncs_DirShadowRay(vec3 aPos, vec3 aDir, accelerationStructureEXT aAS);
vec3 niUIGpuFuncs_DirLight(vec3 worldPos, vec3 worldNormal, vec3 worldLightDir, vec3 lightColor, float cosBias, vec3 shadowColor, accelerationStructureEXT aAS);
niUIGpuFuncs_IntersectionResult niUIGpuFuncs_TraceRay(vec3 aOrigin, vec3 aDir, float aMinT, float aMaxT, accelerationStructureEXT aAS);
vec4 niUIGpuFuncs_CalculateReflectionColor(uint aMaxRecursionDepth, float aSurfaceBias, float aMinReflectionFactorForBounce, niUIGpuFuncs_IntersectionResult aSurfaceHit, vec3 aIncomingDir, niUIGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS, sampler aSS);
float niUIGpuFuncs_GetReflectionFactorFromMaterialColor(vec4 aMatColor);
nish_std_PixelOutput niUIGpuFuncs_raytracer_lit_reflection_ps(nish_std_PixelInput aInput, niUIGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS, sampler aSS);
void niUIGpuFuncs_IntersectionType_static_initialize() {

  // TypeStatic: IntersectionType
  int _tmp_0 = -1;
  niUIGpuFuncs_IntersectionType_InvalidTriangle = niUIGpuFuncs__IntersectionTypeValue_new(_tmp_0) /*SKIPPED COPY VALUETYPE: newed*/;
  int _tmp_1 = 0;
  niUIGpuFuncs_IntersectionType_None = niUIGpuFuncs__IntersectionTypeValue_new(_tmp_1) /*SKIPPED COPY VALUETYPE: newed*/;
  int _tmp_2 = 1;
  niUIGpuFuncs_IntersectionType_Triangle = niUIGpuFuncs__IntersectionTypeValue_new(_tmp_2) /*SKIPPED COPY VALUETYPE: newed*/;
  int _tmp_3 = 2;
  niUIGpuFuncs_IntersectionType_BoundingVolume = niUIGpuFuncs__IntersectionTypeValue_new(_tmp_3) /*SKIPPED COPY VALUETYPE: newed*/;
}

// TypeMeth: RayInstanceData
niUIGpuFuncs_RayInstanceData niUIGpuFuncs_RayInstanceData_new_default() {
  niUIGpuFuncs_RayInstanceData t;
  t.ibIndex = 0;
  t.vbIndex = 0;
  t.firstIndex = 0;
  t.baseVertexIndex = 0;
  t.texIndex = 0;
  t.materialColor = 4294967295;
  return t;
}
vec4 niUIGpuFuncs_RayInstanceData_SampleMaterialDiffuseColor(niUIGpuFuncs_RayInstanceData aInstData, sampler aSS, vec2 aUV) {
  vec4 materialColor = unpackUnorm4x8(aInstData.materialColor).zyxw;
  vec4 _tmp_6;
  bool _tmp_7 = (aInstData.texIndex > 0);
  if (_tmp_7) {
    _tmp_6 = (texture(sampler2D(nil_builtin_GetTexture2D[nonuniformEXT(aInstData.texIndex)],aSS),aUV)*materialColor);
  }
  else {
    {
      _tmp_6 = materialColor;
    }
  }
  return _tmp_6;
}

// TypeMeth: _IntersectionTypeValue
niUIGpuFuncs__IntersectionTypeValue niUIGpuFuncs__IntersectionTypeValue_new(int a__v) {
  niUIGpuFuncs__IntersectionTypeValue t;
  t._v = a__v;
  return t;
}
bool niUIGpuFuncs__op__ne_2__IntersectionTypeValue__IntersectionTypeValue(niUIGpuFuncs__IntersectionTypeValue aLeft, niUIGpuFuncs__IntersectionTypeValue aRight) {
  return (aLeft._v != aRight._v);
}
bool niUIGpuFuncs__op__eq_2__IntersectionTypeValue__IntersectionTypeValue(niUIGpuFuncs__IntersectionTypeValue aLeft, niUIGpuFuncs__IntersectionTypeValue aRight) {
  return (aLeft._v == aRight._v);
}

// TypeMeth: IntersectionResult
niUIGpuFuncs_IntersectionResult niUIGpuFuncs_IntersectionResult_new_default() {
  niUIGpuFuncs_IntersectionResult t;
  t.type = niUIGpuFuncs_IntersectionType_None;
  t.userInstIndex = 0;
  t.primIndex = 0;
  t.instData = niUIGpuFuncs_RayInstanceData_new_default() /*SKIPPED COPY VALUETYPE: newed*/;
  t.bary = vec3_Zero;
  t.worldPos = vec3_Zero;
  t.worldNormal = vec3_Zero;
  t.uv = vec2_Zero;
  return t;
}
bool niUIGpuFuncs_IntersectionResult_SetFromCommittedIntersection(inout /* mut */ niUIGpuFuncs_IntersectionResult r, rayQueryEXT aRayQuery) {
  uint committedType = rayQueryGetIntersectionTypeEXT(aRayQuery,true);
  r.userInstIndex = uint(rayQueryGetIntersectionInstanceCustomIndexEXT(aRayQuery,true));
  r.instData = nil_builtin_GetRayInstanceData[nonuniformEXT(r.userInstIndex)].v /*SKIPPED COPY VALUETYPE: funcret*/;
  bool _tmp_y = (committedType == nish_std_RayQueryIntersectionType_CommittedTriangle);
  if (_tmp_y) {
    r.primIndex = uint(rayQueryGetIntersectionPrimitiveIndexEXT(aRayQuery,true));
    niUIGpuFuncs_IntersectionResult_SetFromTriangleData(r,r.instData.vbIndex,r.instData.ibIndex,r.primIndex,r.instData.firstIndex,r.instData.baseVertexIndex,rayQueryGetIntersectionBarycentricsEXT(aRayQuery,true),rayQueryGetIntersectionObjectToWorldEXT(aRayQuery,true));
  }
  else {
    bool _tmp_Z = (committedType == nish_std_RayQueryIntersectionType_CommittedBoundingVolume);
    if (_tmp_Z) {
      r.type = niUIGpuFuncs_IntersectionType_BoundingVolume;
    }
  }
  return false;
}
void niUIGpuFuncs_IntersectionResult_SetFromTriangleData(inout /* mut */ niUIGpuFuncs_IntersectionResult r, uint aVBIndex, uint aIBIndex, uint aPrimIndex, uint aFirstIndex, uint aBaseVertexIndex, vec2 aIntersectionBaryCentrics, mat4x3 aObjectToWorld) {
  bool _tmp_61 = (aVBIndex == 0);
  if (_tmp_61) {
    r.type = niUIGpuFuncs_IntersectionType_InvalidTriangle;
  }
  else {
    {
      r.type = niUIGpuFuncs_IntersectionType_Triangle;
      uvec3 inds = niUIGpuFuncs_GetTriangleIndices(aIBIndex,aPrimIndex,aFirstIndex);
      niUIGpuFuncs_VertexRay v0 = nil_builtin_GetVertexRay[nonuniformEXT(aVBIndex)].v[nonuniformEXT((aBaseVertexIndex + inds.x))];
      niUIGpuFuncs_VertexRay v1 = nil_builtin_GetVertexRay[nonuniformEXT(aVBIndex)].v[nonuniformEXT((aBaseVertexIndex + inds.y))];
      niUIGpuFuncs_VertexRay v2 = nil_builtin_GetVertexRay[nonuniformEXT(aVBIndex)].v[nonuniformEXT((aBaseVertexIndex + inds.z))];
      r.bary = niUIGpuFuncs_BaryToVec3(aIntersectionBaryCentrics);
      vec3 n = normalize(niUIGpuFuncs_Lerp_4_Vec3_Vec3_Vec3_Vec3(v0.normal,v1.normal,v2.normal,r.bary));
      r.worldNormal = normalize(nish_std_Vec3TransformNormal_2_Vec3_Matrix4x3(n,aObjectToWorld));
      vec3 p = niUIGpuFuncs_Lerp_4_Vec3_Vec3_Vec3_Vec3(v0.pos,v1.pos,v2.pos,r.bary);
      r.worldPos = nish_std_Vec3Transform_2_Vec3_Matrix4x3(p,aObjectToWorld);
      r.uv = niUIGpuFuncs_Lerp_4_Vec2_Vec2_Vec2_Vec3(v0.tex0,v1.tex0,v2.tex0,r.bary);
    }
  }
}
vec4 niUIGpuFuncs_IntersectionResult_ShadeIntersection(niUIGpuFuncs_IntersectionResult aIntersection, vec3 aViewDir, niUIGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS, sampler aSS) {
  vec3 sunDir = normalize(vec3(0.1,0.1,1.0));
  vec4 _tmp_i2;
  bool _tmp_j2 = niUIGpuFuncs__op__eq_2__IntersectionTypeValue__IntersectionTypeValue(aIntersection.type,niUIGpuFuncs_IntersectionType_Triangle);
  if (_tmp_j2) {
    /* mut */ niUIGpuFuncs_RayInstanceData instData = aIntersection.instData /*COPY VALUETYPE: membervar*/;
    vec3 worldPos = aIntersection.worldPos;
    vec3 worldNormal = aIntersection.worldNormal;
    vec2 uv = aIntersection.uv;
    vec3 matColor = (niUIGpuFuncs_RayInstanceData_SampleMaterialDiffuseColor(instData,aSS,uv).rgb);
    float cosBias = 0.5;
    vec3 shadowColor = vec3(0.3,0.3,0.3);
    vec3 lightColor0 = vec3(0.8,0.8,0.8);
    vec3 worldLightDir0 = normalize(vec3(-0.5,-0.7,0.9));
    vec3 lightColor1 = vec3(0.4,0.2,0.0);
    vec3 worldLightDir1 = normalize(vec3(0.707107,0.0,0.707107));
    vec3 lightColor2 = vec3(0.0,0.2,0.4);
    vec3 worldLightDir2 = vec3(0.0,-1.0,0.0);
    vec3 C = ((niUIGpuFuncs_DirLight(worldPos,worldNormal,worldLightDir0,(lightColor0*matColor),cosBias,shadowColor,aAS)+niUIGpuFuncs_DirLight(worldPos,worldNormal,worldLightDir1,(lightColor1*matColor),cosBias,shadowColor,aAS))+niUIGpuFuncs_DirLight(worldPos,worldNormal,worldLightDir2,(lightColor2*matColor),cosBias,shadowColor,aAS));
    vec3 _tmp_C3 = C;
    _tmp_i2 = vec4(_tmp_C3.x,_tmp_C3.y,_tmp_C3.z,1.0);
  }
  else {
    bool _tmp_D3 = niUIGpuFuncs__op__eq_2__IntersectionTypeValue__IntersectionTypeValue(aIntersection.type,niUIGpuFuncs_IntersectionType_BoundingVolume);
    if (_tmp_D3) {
      _tmp_i2 = vec4(1.0,0.0,0.0,1.0);
    }
    else {
      bool _tmp_L3 = niUIGpuFuncs__op__eq_2__IntersectionTypeValue__IntersectionTypeValue(aIntersection.type,niUIGpuFuncs_IntersectionType_InvalidTriangle);
      if (_tmp_L3) {
        _tmp_i2 = vec4(1.0,0.0,1.0,1.0);
      }
      else {
        {
          vec3 _tmp_U3 = niUIGpuFuncs_ComputeSimpleSkyColor(-aViewDir,sunDir);
          _tmp_i2 = vec4(_tmp_U3.x,_tmp_U3.y,_tmp_U3.z,0.0);
        }
      }
    }
  }
  return _tmp_i2;
}

// Function: niUIGpuFuncs
uvec3 niUIGpuFuncs_GetTriangleIndices(uint aIBIndex, uint aPrimIndex, uint aFirstIndex) {
  uint ibase = (aFirstIndex + (3 * aPrimIndex));
  return uvec3(nil_builtin_GetIndex[nonuniformEXT(aIBIndex)].v[nonuniformEXT((ibase + 0))],nil_builtin_GetIndex[nonuniformEXT(aIBIndex)].v[nonuniformEXT((ibase + 1))],nil_builtin_GetIndex[nonuniformEXT(aIBIndex)].v[nonuniformEXT((ibase + 2))]);
}
vec3 niUIGpuFuncs_BaryToVec3(vec2 aBary) {
  return vec3(((1.0 - aBary.x) - aBary.y),aBary.x,aBary.y);
}
vec2 niUIGpuFuncs_Lerp_4_Vec2_Vec2_Vec2_Vec3(vec2 aX, vec2 aY, vec2 aZ, vec3 aBary) {
  return (((aX*aBary.x)+(aY*aBary.y))+(aZ*aBary.z));
}
vec3 niUIGpuFuncs_Lerp_4_Vec3_Vec3_Vec3_Vec3(vec3 aX, vec3 aY, vec3 aZ, vec3 aBary) {
  return (((aX*aBary.x)+(aY*aBary.y))+(aZ*aBary.z));
}
float niUIGpuFuncs_SkyStep(float a, float b, float v) {
  return smoothstep(a, b, v);
}
vec3 niUIGpuFuncs_ComputeSimpleSkyColor(vec3 aRayDir, vec3 aSunDir) {
  vec3 groundColor = vec3(0.35,0.3,0.35);
  vec3 skyColorHorizon = vec3(1.0,1.0,1.0);
  vec3 skyColorZenith = vec3(0.08,0.37,0.73);
  vec3 sunColor = vec3(1.0,0.9,0.7);
  float sunSize = 0.0025;
  float sunIntensity = 10.0;
  float skyGradientT = pow(niUIGpuFuncs_SkyStep(0.0001,0.4,aRayDir.y),0.35);
  vec3 skyGradient = mix(skyColorHorizon,skyColorZenith,skyGradientT);
  float groundToSkyT = niUIGpuFuncs_SkyStep(-0.02,0.0,aRayDir.y);
  float cosAngle = dot(aRayDir,aSunDir);
  float sunDisk = (niUIGpuFuncs_SkyStep((1.0 - sunSize),1.0,cosAngle) * sunIntensity);
  float sunVisibility = float((groundToSkyT >= 1.0));
  return (mix(groundColor,skyGradient,groundToSkyT)+((sunDisk*sunColor)*sunVisibility));
}
float niUIGpuFuncs_CosineBiasSat(float v, float b) {
  return clamp(max(0.0,((v * (1.0 - b)) + b)),0.0,1.0);
}
float niUIGpuFuncs_DirShadowRay(vec3 aPos, vec3 aDir, accelerationStructureEXT aAS) {
  /* mut */ rayQueryEXT rayQuery/*__noinit__*/;
  rayQueryInitializeEXT(rayQuery,aAS,((nish_std_RayFlags_SkipClosestHitShader | nish_std_RayFlags_Opaque) | nish_std_RayFlags_TerminateOnFirstHit),255,aPos,0.001,aDir,10000.0);
  rayQueryProceedEXT(rayQuery);
  bool hasHit = (rayQueryGetIntersectionTypeEXT(rayQuery,true) == nish_std_RayQueryIntersectionType_CommittedTriangle);
  float _tmp_e6;
  bool _tmp_f6 = hasHit;
  if (_tmp_f6) {
    _tmp_e6 = 0.0;
  }
  else {
    {
      _tmp_e6 = 1.0;
    }
  }
  return _tmp_e6;
}
vec3 niUIGpuFuncs_DirLight(vec3 worldPos, vec3 worldNormal, vec3 worldLightDir, vec3 lightColor, float cosBias, vec3 shadowColor, accelerationStructureEXT aAS) {
  float att = niUIGpuFuncs_DirShadowRay(worldPos,-worldLightDir,aAS);
  vec3 D = lightColor;
  vec3 S = vec3_Zero;
  float NdotL = niUIGpuFuncs_CosineBiasSat(dot(worldNormal,-worldLightDir),cosBias);
  vec3 C = (D+S);
  return mix((shadowColor*C),C,(att * NdotL));
}
niUIGpuFuncs_IntersectionResult niUIGpuFuncs_TraceRay(vec3 aOrigin, vec3 aDir, float aMinT, float aMaxT, accelerationStructureEXT aAS) {
  /* mut */ rayQueryEXT rayQuery/*__noinit__*/;
  rayQueryInitializeEXT(rayQuery,aAS,nish_std_RayFlags_None,255,aOrigin,aMinT,aDir,aMaxT);
  rayQueryProceedEXT(rayQuery);
  /* mut */ niUIGpuFuncs_IntersectionResult intersection = niUIGpuFuncs_IntersectionResult_new_default() /*SKIPPED COPY VALUETYPE: newed*/;
  niUIGpuFuncs_IntersectionResult_SetFromCommittedIntersection(intersection,rayQuery);
  return intersection;
}
vec4 niUIGpuFuncs_CalculateReflectionColor(uint aMaxRecursionDepth, float aSurfaceBias, float aMinReflectionFactorForBounce, niUIGpuFuncs_IntersectionResult aSurfaceHit, vec3 aIncomingDir, niUIGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS, sampler aSS) {
  bool _tmp_M6 = niUIGpuFuncs__op__ne_2__IntersectionTypeValue__IntersectionTypeValue(aSurfaceHit.type,niUIGpuFuncs_IntersectionType_Triangle);
  if (_tmp_M6) {
    return vec4(0.0,0.0,0.0,1.0);
  }
  float throughput = 1.0;
  float minThroughput = 0.01;
  vec3 currentOrigin = (aSurfaceHit.worldPos+(aSurfaceHit.worldNormal*aSurfaceBias));
  vec3 currentDir = reflect(aIncomingDir,aSurfaceHit.worldNormal);
  uint bounceIndex = 1;
  while(true) {
    niUIGpuFuncs_IntersectionResult hit = niUIGpuFuncs_TraceRay(currentOrigin,currentDir,aSurfaceBias,aUniforms.cameraFarClipPlane,aAS);
    bool _tmp_e7 = niUIGpuFuncs__op__eq_2__IntersectionTypeValue__IntersectionTypeValue(hit.type,niUIGpuFuncs_IntersectionType_Triangle);
    if (_tmp_e7) {
      vec4 hitMatColor = niUIGpuFuncs_RayInstanceData_SampleMaterialDiffuseColor(hit.instData,aSS,hit.uv);
      float hitReflectionFactor = niUIGpuFuncs_GetReflectionFactorFromMaterialColor(hitMatColor);
      bool terminate = (((bounceIndex >= aMaxRecursionDepth) || (hitReflectionFactor < aMinReflectionFactorForBounce)) || (throughput < minThroughput));
      bool _tmp_B7 = terminate;
      if (_tmp_B7) {
        vec4 shadedColor = niUIGpuFuncs_IntersectionResult_ShadeIntersection(hit,-currentDir,aUniforms,aAS,aSS);
        vec3 _tmp_M7 = ((shadedColor*throughput).rgb);
        return vec4(_tmp_M7.x,_tmp_M7.y,_tmp_M7.z,shadedColor.a);
      }
      else {
        {
          currentOrigin = (hit.worldPos+(hit.worldNormal*aSurfaceBias));
          currentDir = reflect(currentDir,hit.worldNormal);
          throughput = (throughput * hitReflectionFactor);
        }
      }
    }
    else {
      {
        vec4 shadedColor = niUIGpuFuncs_IntersectionResult_ShadeIntersection(hit,-currentDir,aUniforms,aAS,aSS);
        vec3 _tmp_d8 = ((shadedColor*throughput).rgb);
        return vec4(_tmp_d8.x,_tmp_d8.y,_tmp_d8.z,shadedColor.a);
      }
    }
    bounceIndex = (bounceIndex + 1);
    bool _tmp_l8 = (bounceIndex > aMaxRecursionDepth);
    if (_tmp_l8) {
      break;
    }
  }
  return vec4(0.0,0.0,0.0,1.0);
}
float niUIGpuFuncs_GetReflectionFactorFromMaterialColor(vec4 aMatColor) {
  float factor = (aMatColor.z * (1.0 - (1.1 * max(aMatColor.x,aMatColor.y))));
  float _tmp_F8;
  bool _tmp_G8 = (factor > 0.01);
  if (_tmp_G8) {
    _tmp_F8 = min(factor,1.0);
  }
  else {
    {
      _tmp_F8 = 0.0;
    }
  }
  return _tmp_F8;
}
nish_std_PixelOutput niUIGpuFuncs_raytracer_lit_reflection_ps(nish_std_PixelInput aInput, niUIGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS, sampler aSS) {
  vec3 ndc = vec3((((aInput.fragCoord.x / aUniforms.rtWidth) * 2.0) - 1.0),(1.0 - ((aInput.fragCoord.y / aUniforms.rtHeight) * 2.0)),1.0);
  mat4 _tmp_69 = aUniforms.cameraInvView;
  vec3 origin = vec3(_tmp_69[3][0],_tmp_69[3][1],_tmp_69[3][2]);
  vec3 target = nish_std_Vec3TransformCoord(ndc,aUniforms.cameraInvViewProj);
  vec3 dir = normalize(((target-(origin.xyz)).xyz));
  niUIGpuFuncs_IntersectionResult primaryHit = niUIGpuFuncs_TraceRay(origin,dir,0.001,aUniforms.cameraFarClipPlane,aAS);
  vec4 finalColor;
  bool _tmp_m9 = niUIGpuFuncs__op__eq_2__IntersectionTypeValue__IntersectionTypeValue(primaryHit.type,niUIGpuFuncs_IntersectionType_Triangle);
  if (_tmp_m9) {
    vec4 litColor = niUIGpuFuncs_IntersectionResult_ShadeIntersection(primaryHit,-dir,aUniforms,aAS,aSS);
    vec4 matColor = niUIGpuFuncs_RayInstanceData_SampleMaterialDiffuseColor(primaryHit.instData,aSS,primaryHit.uv);
    float reflectionFactor = niUIGpuFuncs_GetReflectionFactorFromMaterialColor(matColor);
    bool _tmp_F9 = (reflectionFactor < 0.1);
    if (_tmp_F9) {
      finalColor = litColor;
    }
    else {
      {
        uint reflectionMaxBounces = 1;
        float reflectionSurfaceBias = 0.001;
        float minFactorForBounce = 0.1;
        vec4 reflectionColor = niUIGpuFuncs_CalculateReflectionColor(reflectionMaxBounces,reflectionSurfaceBias,minFactorForBounce,primaryHit,dir,aUniforms,aAS,aSS);
        finalColor = mix(litColor,reflectionColor,reflectionFactor);
      }
    }
  }
  else {
    bool _tmp_X9 = niUIGpuFuncs__op__eq_2__IntersectionTypeValue__IntersectionTypeValue(primaryHit.type,niUIGpuFuncs_IntersectionType_BoundingVolume);
    if (_tmp_X9) {
      finalColor = vec4(0.5,0.0,0.5,1.0);
    }
    else {
      bool _tmp_5a = niUIGpuFuncs__op__eq_2__IntersectionTypeValue__IntersectionTypeValue(primaryHit.type,niUIGpuFuncs_IntersectionType_InvalidTriangle);
      if (_tmp_5a) {
        finalColor = vec4(1.0,0.0,1.0,1.0);
      }
      else {
        {
          vec3 sunDir = normalize(vec3(0.1,0.1,1.0));
          vec3 _tmp_ja = niUIGpuFuncs_ComputeSimpleSkyColor(dir,sunDir);
          finalColor = vec4(_tmp_ja.x,_tmp_ja.y,_tmp_ja.z,0.0);
        }
      }
    }
  }
  vec4 _tmp_oa = finalColor;
  return nish_std_PixelOutput_new(_tmp_oa);
}

// ModuleInitialize: niUIGpuFuncs
void niUIGpuFuncs_initialize() {
  niUIGpuFuncs_IntersectionType_static_initialize();
}
// MODULE END niUIGpuFuncs

// Pixel Shader main: niUIGpuFuncs_raytracer_lit_reflection_ps
// type size: 48, underlying: float
layout(set = 0, binding = 0) uniform UBO_niUIGpuFuncs_RayUniforms { niUIGpuFuncs_RayUniforms v; } IN_1_aUniforms;
layout(set = 7, binding = 0) uniform accelerationStructureEXT IN_1_aAS;
layout(set = 5, binding = 0) uniform sampler IN_1_aSS;
layout(location = 0) out vec4 OUT_0_rval_color;
void main(void) {
  nish_std_initialize();
  niUIGpuFuncs_initialize();
  nish_std_PixelInput aInput;
  niUIGpuFuncs_RayUniforms aUniforms;
  aInput.fragCoord = gl_FragCoord;
  aInput.frontFacing = gl_FrontFacing;
  aUniforms = IN_1_aUniforms.v;
  nish_std_PixelOutput _rval_ = niUIGpuFuncs_raytracer_lit_reflection_ps(aInput, aUniforms, IN_1_aAS, IN_1_aSS);
  OUT_0_rval_color = _rval_.color;
}

