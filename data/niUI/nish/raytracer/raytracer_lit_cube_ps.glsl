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
uint nish_std_RayFlags_Opaque;
uint nish_std_RayFlags_SkipClosestHitShader;
uint nish_std_RayFlags_CullBackFacingTriangles;
// TypeStaticFwd: RayQueryIntersectionType
uint nish_std_RayQueryIntersectionType_CommittedTriangle;
uint nish_std_RayQueryIntersectionType_CommittedBoundingVolume;
// TypeStaticFwd: Vec3
vec3 vec3_Zero;
// TypeMethFwd: PixelOutput
nish_std_PixelOutput nish_std_PixelOutput_new(vec4 a_color);
// FunctionFwd: nish:std
vec3 nish_std_Vec3Transform_2_Vec3_Matrix4x3(vec3 v, mat4x3 m);
vec3 nish_std_Vec3TransformNormal_2_Vec3_Matrix4x3(vec3 v, mat4x3 m);
vec3 nish_std_Vec3TransformCoord(vec3 v, mat4 m);
void nish_std_RayFlags_static_initialize() {
  // TypeStatic: RayFlags
  nish_std_RayFlags_Opaque = 1;
  nish_std_RayFlags_SkipClosestHitShader = 8;
  nish_std_RayFlags_CullBackFacingTriangles = 16;
}
void nish_std_RayQueryIntersectionType_static_initialize() {
  // TypeStatic: RayQueryIntersectionType
  nish_std_RayQueryIntersectionType_CommittedTriangle = 1;
  nish_std_RayQueryIntersectionType_CommittedBoundingVolume = 2;
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
  vec3 _tmp_7 = v;
  return (m * vec4(_tmp_7.x,_tmp_7.y,_tmp_7.z,1.0));
}
vec3 nish_std_Vec3TransformNormal_2_Vec3_Matrix4x3(vec3 v, mat4x3 m) {
  vec3 _tmp_b = v;
  vec3 r = (m * vec4(_tmp_b.x,_tmp_b.y,_tmp_b.z,0.0));
  return (r.xyz);
}
vec3 nish_std_Vec3TransformCoord(vec3 v, mat4 m) {
  vec3 _tmp_h = v;
  vec4 r = (m * vec4(_tmp_h.x,_tmp_h.y,_tmp_h.z,1.0));
  return ((r/r.w).xyz);
}
// ModuleInitialize: nish_std
void nish_std_initialize() {
  nish_std_RayFlags_static_initialize();
  nish_std_RayQueryIntersectionType_static_initialize();
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

// FunctionFwd: niUIGpuFuncs
layout(scalar, set = 9, binding = 0) readonly buffer SBO_niUIGpuFuncs_RayInstanceData { niUIGpuFuncs_RayInstanceData v; } nil_builtin_GetRayInstanceData[];
layout(scalar, set = 9, binding = 0) readonly buffer SBO_GetIndex { uint v[]; } nil_builtin_GetIndex[];
layout(scalar, set = 9, binding = 0) readonly buffer SBO_GetVertexRay { niUIGpuFuncs_VertexRay v[]; } nil_builtin_GetVertexRay[];
uvec3 niUIGpuFuncs_GetTriangleIndices(uint aIBIndex, uint aPrimIndex, uint aFirstIndex);
vec3 niUIGpuFuncs_BaryToVec3(vec2 aBary);
vec3 niUIGpuFuncs_Lerp_4_Vec3_Vec3_Vec3_Vec3(vec3 aX, vec3 aY, vec3 aZ, vec3 aBary);
void niUIGpuFuncs_InitRayQuery(/* mut */ rayQueryEXT aRayQuery, nish_std_PixelInput aInput, niUIGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS);
float niUIGpuFuncs_CosineBiasSat(float v, float b);
float niUIGpuFuncs_DirShadowRay(vec3 aPos, vec3 aDir, accelerationStructureEXT aAS);
vec3 niUIGpuFuncs_DirLight(vec3 worldPos, vec3 worldNormal, vec3 worldLightDir, vec3 lightColor, float cosBias, vec3 shadowColor, accelerationStructureEXT aAS);
nish_std_PixelOutput niUIGpuFuncs_raytracer_lit_cube_ps(nish_std_PixelInput aInput, niUIGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS);

// Function: niUIGpuFuncs
uvec3 niUIGpuFuncs_GetTriangleIndices(uint aIBIndex, uint aPrimIndex, uint aFirstIndex) {
  uint ibase = (aFirstIndex + (3 * aPrimIndex));
  return uvec3(nil_builtin_GetIndex[nonuniformEXT(aIBIndex)].v[nonuniformEXT((ibase + 0))],nil_builtin_GetIndex[nonuniformEXT(aIBIndex)].v[nonuniformEXT((ibase + 1))],nil_builtin_GetIndex[nonuniformEXT(aIBIndex)].v[nonuniformEXT((ibase + 2))]);
}
vec3 niUIGpuFuncs_BaryToVec3(vec2 aBary) {
  return vec3(((1.0 - aBary.x) - aBary.y),aBary.x,aBary.y);
}
vec3 niUIGpuFuncs_Lerp_4_Vec3_Vec3_Vec3_Vec3(vec3 aX, vec3 aY, vec3 aZ, vec3 aBary) {
  return (((aX*aBary.x)+(aY*aBary.y))+(aZ*aBary.z));
}
void niUIGpuFuncs_InitRayQuery(/* mut */ rayQueryEXT aRayQuery, nish_std_PixelInput aInput, niUIGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS) {
  vec3 ndc = vec3((((aInput.fragCoord.x / aUniforms.rtWidth) * 2.0) - 1.0),(1.0 - ((aInput.fragCoord.y / aUniforms.rtHeight) * 2.0)),1.0);
  mat4 _tmp_61 = aUniforms.cameraInvView;
  vec3 origin = vec3(_tmp_61[3][0],_tmp_61[3][1],_tmp_61[3][2]);
  vec3 target = nish_std_Vec3TransformCoord(ndc,aUniforms.cameraInvViewProj);
  vec3 dir = normalize(((target-(origin.xyz)).xyz));
  rayQueryInitializeEXT(aRayQuery,aAS,nish_std_RayFlags_CullBackFacingTriangles,255,(origin.xyz),0.001,(dir.xyz),aUniforms.cameraFarClipPlane);
}
float niUIGpuFuncs_CosineBiasSat(float v, float b) {
  return clamp(max(0.0,((v * (1.0 - b)) + b)),0.0,1.0);
}
float niUIGpuFuncs_DirShadowRay(vec3 aPos, vec3 aDir, accelerationStructureEXT aAS) {
  /* mut */ rayQueryEXT rayQuery/*__noinit__*/;
  rayQueryInitializeEXT(rayQuery,aAS,(nish_std_RayFlags_SkipClosestHitShader | nish_std_RayFlags_Opaque),255,aPos,0.001,aDir,10000.0);
  rayQueryProceedEXT(rayQuery);
  bool hasHit = (rayQueryGetIntersectionTypeEXT(rayQuery,true) == nish_std_RayQueryIntersectionType_CommittedTriangle);
  float _tmp_Q1;
  bool _tmp_R1 = hasHit;
  if (_tmp_R1) {
    _tmp_Q1 = 0.0;
  }
  else {
    {
      _tmp_Q1 = 1.0;
    }
  }
  return _tmp_Q1;
}
vec3 niUIGpuFuncs_DirLight(vec3 worldPos, vec3 worldNormal, vec3 worldLightDir, vec3 lightColor, float cosBias, vec3 shadowColor, accelerationStructureEXT aAS) {
  float att = niUIGpuFuncs_DirShadowRay(worldPos,-worldLightDir,aAS);
  vec3 D = lightColor;
  vec3 S = vec3_Zero;
  float NdotL = niUIGpuFuncs_CosineBiasSat(dot(worldNormal,-worldLightDir),cosBias);
  vec3 C = (D+S);
  return mix((shadowColor*C),C,(att * NdotL));
}
nish_std_PixelOutput niUIGpuFuncs_raytracer_lit_cube_ps(nish_std_PixelInput aInput, niUIGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS) {
  /* mut */ rayQueryEXT rayQuery/*__noinit__*/;
  niUIGpuFuncs_InitRayQuery(rayQuery,aInput,aUniforms,aAS);
  float cosBias = 0.5;
  vec3 shadowColor = vec3(0.3,0.3,0.3);
  vec3 lightColor0 = vec3(0.8,0.8,0.8);
  vec3 worldLightDir0 = normalize(vec3(-0.5,-0.7,0.9));
  vec3 lightColor1 = vec3(0.4,0.2,0.0);
  vec3 worldLightDir1 = normalize(vec3(0.707107,0.0,0.707107));
  vec3 lightColor2 = vec3(0.0,0.2,0.4);
  vec3 worldLightDir2 = vec3(0.0,-1.0,0.0);
  bool done = rayQueryProceedEXT(rayQuery);
  uint intersectionType = rayQueryGetIntersectionTypeEXT(rayQuery,true);
  uint userInstIndex = uint(rayQueryGetIntersectionInstanceCustomIndexEXT(rayQuery,true));
  niUIGpuFuncs_RayInstanceData instData = nil_builtin_GetRayInstanceData[nonuniformEXT(userInstIndex)].v;
  vec4 color;
  bool _tmp_G2 = (intersectionType == nish_std_RayQueryIntersectionType_CommittedTriangle);
  if (_tmp_G2) {
    bool _tmp_J2 = (instData.vbIndex == 0);
    if (_tmp_J2) {
      color = vec4(1.0,0.0,1.0,1.0);
    }
    else {
      {
        uint primIndex = uint(rayQueryGetIntersectionPrimitiveIndexEXT(rayQuery,true));
        uint firstIndex = instData.firstIndex;
        uvec3 inds = niUIGpuFuncs_GetTriangleIndices(instData.ibIndex,primIndex,instData.firstIndex);
        niUIGpuFuncs_VertexRay v0 = nil_builtin_GetVertexRay[nonuniformEXT(instData.vbIndex)].v[nonuniformEXT((instData.baseVertexIndex + inds.x))];
        niUIGpuFuncs_VertexRay v1 = nil_builtin_GetVertexRay[nonuniformEXT(instData.vbIndex)].v[nonuniformEXT((instData.baseVertexIndex + inds.y))];
        niUIGpuFuncs_VertexRay v2 = nil_builtin_GetVertexRay[nonuniformEXT(instData.vbIndex)].v[nonuniformEXT((instData.baseVertexIndex + inds.z))];
        vec3 bary = niUIGpuFuncs_BaryToVec3(rayQueryGetIntersectionBarycentricsEXT(rayQuery,true));
        vec3 n = normalize(niUIGpuFuncs_Lerp_4_Vec3_Vec3_Vec3_Vec3(v0.normal,v1.normal,v2.normal,bary));
        vec3 worldNormal = normalize(nish_std_Vec3TransformNormal_2_Vec3_Matrix4x3(n,rayQueryGetIntersectionObjectToWorldEXT(rayQuery,true)));
        vec3 p = niUIGpuFuncs_Lerp_4_Vec3_Vec3_Vec3_Vec3(v0.pos,v1.pos,v2.pos,bary);
        vec3 worldPos = nish_std_Vec3Transform_2_Vec3_Matrix4x3(p,rayQueryGetIntersectionObjectToWorldEXT(rayQuery,true));
        vec3 C = ((niUIGpuFuncs_DirLight(worldPos,worldNormal,worldLightDir0,lightColor0,cosBias,shadowColor,aAS)+niUIGpuFuncs_DirLight(worldPos,worldNormal,worldLightDir1,lightColor1,cosBias,shadowColor,aAS))+niUIGpuFuncs_DirLight(worldPos,worldNormal,worldLightDir2,lightColor2,cosBias,shadowColor,aAS));
        vec3 _tmp_j4 = C;
        color = vec4(_tmp_j4.x,_tmp_j4.y,_tmp_j4.z,1.0);
      }
    }
  }
  else {
    bool _tmp_k4 = (intersectionType == nish_std_RayQueryIntersectionType_CommittedBoundingVolume);
    if (_tmp_k4) {
      color = vec4(1.0,0.0,0.0,1.0);
    }
    else {
      {
        color = vec4(0.0,0.5,0.8,0.0);
      }
    }
  }
  vec4 _tmp_x4 = color;
  return nish_std_PixelOutput_new(_tmp_x4);
}
// MODULE END niUIGpuFuncs

// Pixel Shader main: niUIGpuFuncs_raytracer_lit_cube_ps
// type size: 48, underlying: float
layout(set = 0, binding = 0) uniform UBO_niUIGpuFuncs_RayUniforms { niUIGpuFuncs_RayUniforms v; } IN_1_aUniforms;
layout(set = 7, binding = 0) uniform accelerationStructureEXT IN_1_aAS;
layout(location = 0) out vec4 OUT_0_rval_color;
void main(void) {
  nish_std_initialize();
  nish_std_PixelInput aInput;
  niUIGpuFuncs_RayUniforms aUniforms;
  aInput.fragCoord = gl_FragCoord;
  aInput.frontFacing = gl_FrontFacing;
  aUniforms = IN_1_aUniforms.v;
  nish_std_PixelOutput _rval_ = niUIGpuFuncs_raytracer_lit_cube_ps(aInput, aUniforms, IN_1_aAS);
  OUT_0_rval_color = _rval_.color;
}

