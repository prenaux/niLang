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
// TypeStaticFwd: RayQueryIntersectionType
uint nish_std_RayQueryIntersectionType_CommittedTriangle;
uint nish_std_RayQueryIntersectionType_CommittedBoundingVolume;
// TypeStaticFwd: RayQueryCandidateIntersectionType
uint nish_std_RayQueryCandidateIntersectionType_CandidateTriangle;
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
}
void nish_std_RayQueryIntersectionType_static_initialize() {
  // TypeStatic: RayQueryIntersectionType
  nish_std_RayQueryIntersectionType_CommittedTriangle = 1;
  nish_std_RayQueryIntersectionType_CommittedBoundingVolume = 2;
}
void nish_std_RayQueryCandidateIntersectionType_static_initialize() {
  // TypeStatic: RayQueryCandidateIntersectionType
  nish_std_RayQueryCandidateIntersectionType_CandidateTriangle = 0;
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
  nish_std_RayQueryCandidateIntersectionType_static_initialize();
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
  vec3 objPos;
  vec3 worldPos;
  vec3 objNormal;
  vec3 worldNormal;
  vec2 uv;
  bool isFrontFacing;
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
bool niUIGpuFuncs__op__eq_2__IntersectionTypeValue__IntersectionTypeValue(niUIGpuFuncs__IntersectionTypeValue aLeft, niUIGpuFuncs__IntersectionTypeValue aRight);

// TypeMethFwd: IntersectionResult
niUIGpuFuncs_IntersectionResult niUIGpuFuncs_IntersectionResult_new_default();
bool niUIGpuFuncs_IntersectionResult_SetFromCommittedIntersection(inout /* mut */ niUIGpuFuncs_IntersectionResult r, rayQueryEXT aRayQuery);
void niUIGpuFuncs_IntersectionResult_SetFromTriangleData(inout /* mut */ niUIGpuFuncs_IntersectionResult r, uint aVBIndex, uint aIBIndex, uint aPrimIndex, uint aFirstIndex, uint aBaseVertexIndex, vec2 aIntersectionBaryCentrics, mat4x3 aObjectToWorld, bool aIsFrontFacing);

// FunctionFwd: niUIGpuFuncs
layout(scalar, set = 9, binding = 0) readonly buffer SBO_niUIGpuFuncs_RayInstanceData { niUIGpuFuncs_RayInstanceData v; } nil_builtin_GetRayInstanceData[];
layout(scalar, set = 9, binding = 0) readonly buffer SBO_GetIndex { uint v[]; } nil_builtin_GetIndex[];
layout(scalar, set = 9, binding = 0) readonly buffer SBO_GetVertexRay { niUIGpuFuncs_VertexRay v[]; } nil_builtin_GetVertexRay[];
uvec3 niUIGpuFuncs_GetTriangleIndices(uint aIBIndex, uint aPrimIndex, uint aFirstIndex);
vec3 niUIGpuFuncs_BaryToVec3(vec2 aBary);
vec2 niUIGpuFuncs_Lerp_4_Vec2_Vec2_Vec2_Vec3(vec2 aX, vec2 aY, vec2 aZ, vec3 aBary);
vec3 niUIGpuFuncs_Lerp_4_Vec3_Vec3_Vec3_Vec3(vec3 aX, vec3 aY, vec3 aZ, vec3 aBary);
niUIGpuFuncs_IntersectionResult niUIGpuFuncs_TraceRay_6_uint_Vec3_Vec3_float_float_RayInstances(uint aRayFlags, vec3 aOrigin, vec3 aDir, float aMinT, float aMaxT, accelerationStructureEXT aAS);
niUIGpuFuncs_IntersectionResult niUIGpuFuncs_TraceRay_5_Vec3_Vec3_float_float_RayInstances(vec3 aOrigin, vec3 aDir, float aMinT, float aMaxT, accelerationStructureEXT aAS);
float niUIGpuFuncs_GetReflectionFactorFromMaterialColor(vec4 aMatColor);
nish_std_PixelOutput niUIGpuFuncs_rayTraceShowMatDiffuse(uint aRGBAChannel, nish_std_PixelInput aInput, niUIGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS, sampler aSS);
nish_std_PixelOutput niUIGpuFuncs_raytracer_mat_diffuse_r_ps(nish_std_PixelInput aInput, niUIGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS, sampler aSS);
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
  t.objPos = vec3_Zero;
  t.worldPos = vec3_Zero;
  t.objNormal = vec3_Zero;
  t.worldNormal = vec3_Zero;
  t.uv = vec2_Zero;
  t.isFrontFacing = true;
  return t;
}
bool niUIGpuFuncs_IntersectionResult_SetFromCommittedIntersection(inout /* mut */ niUIGpuFuncs_IntersectionResult r, rayQueryEXT aRayQuery) {
  uint committedType = rayQueryGetIntersectionTypeEXT(aRayQuery,true);
  r.userInstIndex = uint(rayQueryGetIntersectionInstanceCustomIndexEXT(aRayQuery,true));
  r.instData = nil_builtin_GetRayInstanceData[nonuniformEXT(r.userInstIndex)].v /*SKIPPED COPY VALUETYPE: funcret*/;
  bool _tmp_t = (committedType == nish_std_RayQueryIntersectionType_CommittedTriangle);
  if (_tmp_t) {
    r.primIndex = uint(rayQueryGetIntersectionPrimitiveIndexEXT(aRayQuery,true));
    niUIGpuFuncs_IntersectionResult_SetFromTriangleData(r,r.instData.vbIndex,r.instData.ibIndex,r.primIndex,r.instData.firstIndex,r.instData.baseVertexIndex,rayQueryGetIntersectionBarycentricsEXT(aRayQuery,true),rayQueryGetIntersectionObjectToWorldEXT(aRayQuery,true),rayQueryGetIntersectionFrontFaceEXT(aRayQuery,true));
  }
  else {
    bool _tmp_W = (committedType == nish_std_RayQueryIntersectionType_CommittedBoundingVolume);
    if (_tmp_W) {
      r.type = niUIGpuFuncs_IntersectionType_BoundingVolume;
    }
  }
  return false;
}
void niUIGpuFuncs_IntersectionResult_SetFromTriangleData(inout /* mut */ niUIGpuFuncs_IntersectionResult r, uint aVBIndex, uint aIBIndex, uint aPrimIndex, uint aFirstIndex, uint aBaseVertexIndex, vec2 aIntersectionBaryCentrics, mat4x3 aObjectToWorld, bool aIsFrontFacing) {
  bool _tmp_31 = (aVBIndex == 0);
  if (_tmp_31) {
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
      r.objNormal = normalize(niUIGpuFuncs_Lerp_4_Vec3_Vec3_Vec3_Vec3(v0.normal,v1.normal,v2.normal,r.bary));
      bool _tmp_M1 = (aIsFrontFacing == false);
      if (_tmp_M1) {
        r.objNormal = -r.objNormal;
      }
      r.worldNormal = normalize(nish_std_Vec3TransformNormal_2_Vec3_Matrix4x3(r.objNormal,aObjectToWorld));
      r.objPos = niUIGpuFuncs_Lerp_4_Vec3_Vec3_Vec3_Vec3(v0.pos,v1.pos,v2.pos,r.bary);
      r.worldPos = nish_std_Vec3Transform_2_Vec3_Matrix4x3(r.objPos,aObjectToWorld);
      r.uv = niUIGpuFuncs_Lerp_4_Vec2_Vec2_Vec2_Vec3(v0.tex0,v1.tex0,v2.tex0,r.bary);
      r.isFrontFacing = aIsFrontFacing;
    }
  }
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
niUIGpuFuncs_IntersectionResult niUIGpuFuncs_TraceRay_6_uint_Vec3_Vec3_float_float_RayInstances(uint aRayFlags, vec3 aOrigin, vec3 aDir, float aMinT, float aMaxT, accelerationStructureEXT aAS) {
  /* mut */ rayQueryEXT rayQuery/*__noinit__*/;
  rayQueryInitializeEXT(rayQuery,aAS,aRayFlags,255,aOrigin,aMinT,aDir,aMaxT);
  while(true) {
    bool _tmp_v3 = rayQueryProceedEXT(rayQuery);
    if (!(_tmp_v3)) {
      break;
    }
    uint candidateType = rayQueryGetIntersectionTypeEXT(rayQuery,false);
    bool _tmp_A3 = (candidateType == nish_std_RayQueryCandidateIntersectionType_CandidateTriangle);
    if (_tmp_A3) {
      rayQueryConfirmIntersectionEXT(rayQuery);
      rayQueryTerminateEXT(rayQuery);
    }
    else {
      {
        rayQueryTerminateEXT(rayQuery);
      }
    }
  }
  /* mut */ niUIGpuFuncs_IntersectionResult intersection = niUIGpuFuncs_IntersectionResult_new_default() /*SKIPPED COPY VALUETYPE: newed*/;
  uint intersectionType = rayQueryGetIntersectionTypeEXT(rayQuery,true);
  bool _tmp_I3 = (intersectionType == nish_std_RayQueryIntersectionType_CommittedTriangle);
  if (_tmp_I3) {
    niUIGpuFuncs_IntersectionResult_SetFromCommittedIntersection(intersection,rayQuery);
  }
  else {
    {
      intersection.type = niUIGpuFuncs_IntersectionType_None;
    }
  }
  return intersection;
}
niUIGpuFuncs_IntersectionResult niUIGpuFuncs_TraceRay_5_Vec3_Vec3_float_float_RayInstances(vec3 aOrigin, vec3 aDir, float aMinT, float aMaxT, accelerationStructureEXT aAS) {
  return niUIGpuFuncs_TraceRay_6_uint_Vec3_Vec3_float_float_RayInstances(nish_std_RayFlags_None,aOrigin,aDir,aMinT,aMaxT,aAS);
}
float niUIGpuFuncs_GetReflectionFactorFromMaterialColor(vec4 aMatColor) {
  float factor = (aMatColor.z * (1.0 - (1.1 * max(aMatColor.x,aMatColor.y))));
  float _tmp_a4;
  bool _tmp_b4 = (factor > 0.1);
  if (_tmp_b4) {
    _tmp_a4 = min(factor,1.0);
  }
  else {
    {
      _tmp_a4 = 0.0;
    }
  }
  return _tmp_a4;
}
nish_std_PixelOutput niUIGpuFuncs_rayTraceShowMatDiffuse(uint aRGBAChannel, nish_std_PixelInput aInput, niUIGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS, sampler aSS) {
  vec3 ndc = vec3((((aInput.fragCoord.x / aUniforms.rtWidth) * 2.0) - 1.0),(1.0 - ((aInput.fragCoord.y / aUniforms.rtHeight) * 2.0)),1.0);
  mat4 _tmp_B4 = aUniforms.cameraInvView;
  vec3 origin = vec3(_tmp_B4[3][0],_tmp_B4[3][1],_tmp_B4[3][2]);
  vec3 target = nish_std_Vec3TransformCoord(ndc,aUniforms.cameraInvViewProj);
  vec3 dir = normalize(((target-(origin.xyz)).xyz));
  niUIGpuFuncs_IntersectionResult primaryHit = niUIGpuFuncs_TraceRay_5_Vec3_Vec3_float_float_RayInstances(origin,dir,0.001,aUniforms.cameraFarClipPlane,aAS);
  vec4 finalColor;
  bool _tmp_R4 = niUIGpuFuncs__op__eq_2__IntersectionTypeValue__IntersectionTypeValue(primaryHit.type,niUIGpuFuncs_IntersectionType_Triangle);
  if (_tmp_R4) {
    vec4 matColor = niUIGpuFuncs_RayInstanceData_SampleMaterialDiffuseColor(primaryHit.instData,aSS,primaryHit.uv);
    bool _tmp_15 = (aRGBAChannel == 1);
    if (_tmp_15) {
      finalColor = vec4(matColor.x,0.0,0.0,1.0);
    }
    else {
      bool _tmp_95 = (aRGBAChannel == 2);
      if (_tmp_95) {
        finalColor = vec4(0.0,matColor.y,0.0,1.0);
      }
      else {
        bool _tmp_h5 = (aRGBAChannel == 3);
        if (_tmp_h5) {
          finalColor = vec4(0.0,0.0,matColor.z,1.0);
        }
        else {
          bool _tmp_p5 = (aRGBAChannel == 4);
          if (_tmp_p5) {
            finalColor = vec4(matColor.w,matColor.w,matColor.w,1.0);
          }
          else {
            bool _tmp_z5 = (aRGBAChannel == 5);
            if (_tmp_z5) {
              float reflectionFactor = niUIGpuFuncs_GetReflectionFactorFromMaterialColor(matColor);
              finalColor = vec4(reflectionFactor,reflectionFactor,reflectionFactor,1.0);
            }
            else {
              {
                finalColor = vec4(matColor.x,matColor.y,matColor.z,matColor.w);
              }
            }
          }
        }
      }
    }
  }
  else {
    bool _tmp_Q5 = niUIGpuFuncs__op__eq_2__IntersectionTypeValue__IntersectionTypeValue(primaryHit.type,niUIGpuFuncs_IntersectionType_BoundingVolume);
    if (_tmp_Q5) {
      finalColor = vec4(0.5,0.0,0.5,1.0);
    }
    else {
      bool _tmp_Y5 = niUIGpuFuncs__op__eq_2__IntersectionTypeValue__IntersectionTypeValue(primaryHit.type,niUIGpuFuncs_IntersectionType_InvalidTriangle);
      if (_tmp_Y5) {
        finalColor = vec4(1.0,0.0,1.0,1.0);
      }
      else {
        {
          finalColor = vec4(0.0,0.0,0.0,0.0);
        }
      }
    }
  }
  vec4 _tmp_c6 = finalColor;
  return nish_std_PixelOutput_new(_tmp_c6);
}
nish_std_PixelOutput niUIGpuFuncs_raytracer_mat_diffuse_r_ps(nish_std_PixelInput aInput, niUIGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS, sampler aSS) {
  return niUIGpuFuncs_rayTraceShowMatDiffuse(1,aInput,aUniforms,aAS,aSS);
}

// ModuleInitialize: niUIGpuFuncs
void niUIGpuFuncs_initialize() {
  niUIGpuFuncs_IntersectionType_static_initialize();
}
// MODULE END niUIGpuFuncs

// Pixel Shader main: niUIGpuFuncs_raytracer_mat_diffuse_r_ps
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
  nish_std_PixelOutput _rval_ = niUIGpuFuncs_raytracer_mat_diffuse_r_ps(aInput, aUniforms, IN_1_aAS, IN_1_aSS);
  OUT_0_rval_color = _rval_.color;
}

