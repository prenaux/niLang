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
// TypeMethFwd: PixelOutput
nish_std_PixelOutput nish_std_PixelOutput_new(vec4 a_color);
// FunctionFwd: nish:std
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
// TypeMeth: PixelOutput
nish_std_PixelOutput nish_std_PixelOutput_new(vec4 a_color) {
  nish_std_PixelOutput t;
  t.color = a_color;
  return t;
}
// Function: nish:std
vec3 nish_std_Vec3TransformCoord(vec3 v, mat4 m) {
  vec3 _tmp_3 = v;
  vec4 r = (m * vec4(_tmp_3.x,_tmp_3.y,_tmp_3.z,1.0));
  return ((r/r.w).xyz);
}
// ModuleInitialize: nish_std
void nish_std_initialize() {
  nish_std_RayFlags_static_initialize();
  nish_std_RayQueryIntersectionType_static_initialize();
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
vec2 niUIGpuFuncs_Lerp_4_Vec2_Vec2_Vec2_Vec3(vec2 aX, vec2 aY, vec2 aZ, vec3 aBary);
void niUIGpuFuncs_InitRayQuery_4_RayQuery_PixelInput_RayUniforms_RayInstances(/* mut */ rayQueryEXT aRayQuery, nish_std_PixelInput aInput, niUIGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS);
void niUIGpuFuncs_InitRayQuery_5_RayQuery_uint_PixelInput_RayUniforms_RayInstances(/* mut */ rayQueryEXT aRayQuery, uint aRayFlags, nish_std_PixelInput aInput, niUIGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS);
nish_std_PixelOutput niUIGpuFuncs_raytracer_tex0_ps(nish_std_PixelInput aInput, niUIGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS);

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
void niUIGpuFuncs_InitRayQuery_4_RayQuery_PixelInput_RayUniforms_RayInstances(/* mut */ rayQueryEXT aRayQuery, nish_std_PixelInput aInput, niUIGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS) {
  niUIGpuFuncs_InitRayQuery_5_RayQuery_uint_PixelInput_RayUniforms_RayInstances(aRayQuery,nish_std_RayFlags_None,aInput,aUniforms,aAS);
}
void niUIGpuFuncs_InitRayQuery_5_RayQuery_uint_PixelInput_RayUniforms_RayInstances(/* mut */ rayQueryEXT aRayQuery, uint aRayFlags, nish_std_PixelInput aInput, niUIGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS) {
  vec3 ndc = vec3((((aInput.fragCoord.x / aUniforms.rtWidth) * 2.0) - 1.0),(1.0 - ((aInput.fragCoord.y / aUniforms.rtHeight) * 2.0)),1.0);
  mat4 _tmp_b1 = aUniforms.cameraInvView;
  vec3 origin = vec3(_tmp_b1[3][0],_tmp_b1[3][1],_tmp_b1[3][2]);
  vec3 target = nish_std_Vec3TransformCoord(ndc,aUniforms.cameraInvViewProj);
  vec3 dir = normalize(((target-(origin.xyz)).xyz));
  rayQueryInitializeEXT(aRayQuery,aAS,aRayFlags,255,(origin.xyz),0.001,(dir.xyz),aUniforms.cameraFarClipPlane);
}
nish_std_PixelOutput niUIGpuFuncs_raytracer_tex0_ps(nish_std_PixelInput aInput, niUIGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS) {
  /* mut */ rayQueryEXT rayQuery/*__noinit__*/;
  niUIGpuFuncs_InitRayQuery_4_RayQuery_PixelInput_RayUniforms_RayInstances(rayQuery,aInput,aUniforms,aAS);
  bool done = rayQueryProceedEXT(rayQuery);
  uint intersectionType = rayQueryGetIntersectionTypeEXT(rayQuery,true);
  uint userInstIndex = uint(rayQueryGetIntersectionInstanceCustomIndexEXT(rayQuery,true));
  niUIGpuFuncs_RayInstanceData instData = nil_builtin_GetRayInstanceData[nonuniformEXT(userInstIndex)].v;
  vec4 color;
  bool _tmp_E1 = (intersectionType == nish_std_RayQueryIntersectionType_CommittedTriangle);
  if (_tmp_E1) {
    bool _tmp_H1 = (instData.vbIndex == 0);
    if (_tmp_H1) {
      color = vec4(1.0,0.0,1.0,1.0);
    }
    else {
      {
        uint primIndex = uint(rayQueryGetIntersectionPrimitiveIndexEXT(rayQuery,true));
        uvec3 inds = niUIGpuFuncs_GetTriangleIndices(instData.ibIndex,primIndex,instData.firstIndex);
        niUIGpuFuncs_VertexRay v0 = nil_builtin_GetVertexRay[nonuniformEXT(instData.vbIndex)].v[nonuniformEXT((instData.baseVertexIndex + inds.x))];
        niUIGpuFuncs_VertexRay v1 = nil_builtin_GetVertexRay[nonuniformEXT(instData.vbIndex)].v[nonuniformEXT((instData.baseVertexIndex + inds.y))];
        niUIGpuFuncs_VertexRay v2 = nil_builtin_GetVertexRay[nonuniformEXT(instData.vbIndex)].v[nonuniformEXT((instData.baseVertexIndex + inds.z))];
        vec3 bary = niUIGpuFuncs_BaryToVec3(rayQueryGetIntersectionBarycentricsEXT(rayQuery,true));
        vec2 uvi = niUIGpuFuncs_Lerp_4_Vec2_Vec2_Vec2_Vec3(v0.tex0,v1.tex0,v2.tex0,bary);
        vec2 uv;
        bool _tmp_x2 = (bary.x > 0.85);
        if (_tmp_x2) {
          uv = v0.tex0;
        }
        else {
          bool _tmp_C2 = (bary.y > 0.85);
          if (_tmp_C2) {
            uv = v1.tex0;
          }
          else {
            bool _tmp_H2 = (bary.z > 0.85);
            if (_tmp_H2) {
              uv = v2.tex0;
            }
            else {
              {
                uv = uvi;
              }
            }
          }
        }
        vec4 instanceColor = vec4(uv.x,uv.y,0.0,1.0);
        color = instanceColor;
      }
    }
  }
  else {
    bool _tmp_T2 = (intersectionType == nish_std_RayQueryIntersectionType_CommittedBoundingVolume);
    if (_tmp_T2) {
      color = vec4(1.0,0.0,0.0,1.0);
    }
    else {
      {
        color = vec4(0.0,0.5,0.8,0.0);
      }
    }
  }
  vec4 _tmp_63 = color;
  return nish_std_PixelOutput_new(_tmp_63);
}
// MODULE END niUIGpuFuncs

// Pixel Shader main: niUIGpuFuncs_raytracer_tex0_ps
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
  nish_std_PixelOutput _rval_ = niUIGpuFuncs_raytracer_tex0_ps(aInput, aUniforms, IN_1_aAS);
  OUT_0_rval_color = _rval_.color;
}

