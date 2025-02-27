#version 460
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_ray_query : require
// DO IMPORTS BEGIN TestGpuFuncs
// MODULE BEGIN lib:shader
// Type: PixelInput
struct lib_shader_PixelInput {
  vec4 fragCoord;
  bool frontFacing;
};
// TypeStaticFwd: RayFlags
uint lib_shader_RayFlags_None;
uint lib_shader_RayFlags_Opaque;
uint lib_shader_RayFlags_SkipClosestHitShader;
// TypeStaticFwd: RayQueryIntersectionType
uint lib_shader_RayQueryIntersectionType_CommittedTriangle;
uint lib_shader_RayQueryIntersectionType_CommittedBoundingVolume;
// TypeStaticFwd: Vec3
vec3 vec3_Zero;
// FunctionFwd: lib:shader
layout(set = 10, binding = 0) uniform texture2D nil_builtin_GetTexture2D[];
uint lib_shader_HashJenkins(uint aStartX);
vec3 lib_shader_UIntToHashColor(uint i);
vec3 lib_shader_Vec3Transform_2_Vec3_Matrix4x3(vec3 v, mat4x3 m);
vec3 lib_shader_Vec3TransformCoord(vec3 v, mat4 m);
vec3 lib_shader_Vec3TransformNormal_2_Vec3_Matrix4x3(vec3 v, mat4x3 m);
void lib_shader_RayFlags_static_initialize() {
  // TypeStatic: RayFlags
  lib_shader_RayFlags_None = 0;
  lib_shader_RayFlags_Opaque = 1;
  lib_shader_RayFlags_SkipClosestHitShader = 8;
}
void lib_shader_RayQueryIntersectionType_static_initialize() {
  // TypeStatic: RayQueryIntersectionType
  lib_shader_RayQueryIntersectionType_CommittedTriangle = 1;
  lib_shader_RayQueryIntersectionType_CommittedBoundingVolume = 2;
}
void vec3_static_initialize() {
  // TypeStatic: Vec3
  vec3_Zero = vec3(0.0,0.0,0.0);
}
// Function: lib:shader
uint lib_shader_HashJenkins(uint aStartX) {
  uint x = aStartX;
  x = (x + (x << 10));
  x = (x ^ (x >> 6));
  x = (x + (x << 3));
  x = (x ^ (x >> 11));
  x = (x + (x << 15));
  return x;
}
vec3 lib_shader_UIntToHashColor(uint i) {
  uint hash = lib_shader_HashJenkins(i);
  return vec3((float(((hash >> 0) & 255)) / 255.0),(float(((hash >> 8) & 255)) / 255.0),(float(((hash >> 16) & 255)) / 255.0));
}
vec3 lib_shader_Vec3Transform_2_Vec3_Matrix4x3(vec3 v, mat4x3 m) {
  vec3 _tmp_S = v;
  return (m * vec4(_tmp_S.x,_tmp_S.y,_tmp_S.z,1.0));
}
vec3 lib_shader_Vec3TransformCoord(vec3 v, mat4 m) {
  vec3 _tmp_W = v;
  vec4 r = (m * vec4(_tmp_W.x,_tmp_W.y,_tmp_W.z,1.0));
  return ((r/r.w).xyz);
}
vec3 lib_shader_Vec3TransformNormal_2_Vec3_Matrix4x3(vec3 v, mat4x3 m) {
  vec3 _tmp_51 = v;
  vec3 r = (m * vec4(_tmp_51.x,_tmp_51.y,_tmp_51.z,0.0));
  return (r.xyz);
}
// ModuleInitialize: lib_shader
void lib_shader_initialize() {
  lib_shader_RayFlags_static_initialize();
  lib_shader_RayQueryIntersectionType_static_initialize();
  vec3_static_initialize();
}
// MODULE END lib:shader
// DO IMPORTS END TestGpuFuncs

// MODULE BEGIN TestGpuFuncs

// Type: PixelOutput
struct TestGpuFuncs_PixelOutput {
  vec4 color;
};

// Type: RayInstanceData
struct TestGpuFuncs_RayInstanceData {
  uint ibIndex;
  uint vbIndex;
  uint texIndex;
};

// Type: RayUniforms
struct TestGpuFuncs_RayUniforms {
  float rtWidth;
  float rtHeight;
  float cameraFarClipPlane;
  float padding;
  mat4 cameraInvView;
  mat4 cameraInvViewProj;
};

// Type: VertexRay
struct TestGpuFuncs_VertexRay {
  vec3 pos;
  vec3 normal;
  vec2 tex0;
};

// TypeMethFwd: PixelOutput
TestGpuFuncs_PixelOutput TestGpuFuncs_PixelOutput_new(vec4 a_color);

// FunctionFwd: TestGpuFuncs
vec3 TestGpuFuncs_BaryToVec3(vec2 aBary);
float TestGpuFuncs_CosineBiasSat(float v, float b);
vec3 TestGpuFuncs_DirLight(vec3 worldPos, vec3 worldNormal, vec3 worldLightDir, vec3 lightColor, float cosBias, vec3 shadowColor, accelerationStructureEXT aAS);
float TestGpuFuncs_DirShadowRay(vec3 aPos, vec3 aDir, accelerationStructureEXT aAS);
layout(scalar, set = 9, binding = 0) readonly buffer SBO_GetIndex { uint v[]; } nil_builtin_GetIndex[];
layout(scalar, set = 9, binding = 0) readonly buffer SBO_TestGpuFuncs_RayInstanceData { TestGpuFuncs_RayInstanceData v; } nil_builtin_GetRayInstanceData[];
uvec3 TestGpuFuncs_GetTriangleIndices(uint aIBIndex, uint aPrimIndex);
layout(scalar, set = 9, binding = 0) readonly buffer SBO_GetVertexRay { TestGpuFuncs_VertexRay v[]; } nil_builtin_GetVertexRay[];
void TestGpuFuncs_InitRayQuery(rayQueryEXT aRayQuery, lib_shader_PixelInput aInput, TestGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS);
vec2 TestGpuFuncs_Lerp_4_Vec2_Vec2_Vec2_Vec3(vec2 aX, vec2 aY, vec2 aZ, vec3 aBary);
vec3 TestGpuFuncs_Lerp_4_Vec3_Vec3_Vec3_Vec3(vec3 aX, vec3 aY, vec3 aZ, vec3 aBary);
TestGpuFuncs_PixelOutput TestGpuFuncs_raytracer_lit_textured_cube_ps(lib_shader_PixelInput aInput, TestGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS, sampler aSS);

// TypeMeth: PixelOutput
TestGpuFuncs_PixelOutput TestGpuFuncs_PixelOutput_new(vec4 a_color) {
  TestGpuFuncs_PixelOutput t;
  t.color = a_color;
  return t;
}

// Function: TestGpuFuncs
vec3 TestGpuFuncs_BaryToVec3(vec2 aBary) {
  return vec3(((1.0 - aBary.x) - aBary.y),aBary.x,aBary.y);
}
float TestGpuFuncs_CosineBiasSat(float v, float b) {
  return clamp(max(0.0,((v * (1.0 - b)) + b)),0.0,1.0);
}
vec3 TestGpuFuncs_DirLight(vec3 worldPos, vec3 worldNormal, vec3 worldLightDir, vec3 lightColor, float cosBias, vec3 shadowColor, accelerationStructureEXT aAS) {
  float att = TestGpuFuncs_DirShadowRay(worldPos,-worldLightDir,aAS);
  vec3 D = lightColor;
  vec3 S = vec3_Zero;
  float NdotL = TestGpuFuncs_CosineBiasSat(dot(worldNormal,-worldLightDir),cosBias);
  vec3 C = (D+S);
  return mix((shadowColor*C),C,(att * NdotL));
}
float TestGpuFuncs_DirShadowRay(vec3 aPos, vec3 aDir, accelerationStructureEXT aAS) {
  rayQueryEXT rayQuery/*__noinit__*/;
  rayQueryInitializeEXT(rayQuery,aAS,(lib_shader_RayFlags_SkipClosestHitShader | lib_shader_RayFlags_Opaque),255,aPos,0.001,aDir,10000.0);
  rayQueryProceedEXT(rayQuery);
  bool hasHit = (rayQueryGetIntersectionTypeEXT(rayQuery,true) == lib_shader_RayQueryIntersectionType_CommittedTriangle);
  float _tmp_V;
  bool _tmp_W = hasHit;
  if (_tmp_W) {
    _tmp_V = 0.0;
  }
  else {
    {
      _tmp_V = 1.0;
    }
  }
  return _tmp_V;
}
uvec3 TestGpuFuncs_GetTriangleIndices(uint aIBIndex, uint aPrimIndex) {
  uint ibase = (3 * aPrimIndex);
  return uvec3(nil_builtin_GetIndex[nonuniformEXT(aIBIndex)].v[nonuniformEXT((ibase + 0))],nil_builtin_GetIndex[nonuniformEXT(aIBIndex)].v[nonuniformEXT((ibase + 1))],nil_builtin_GetIndex[nonuniformEXT(aIBIndex)].v[nonuniformEXT((ibase + 2))]);
}
void TestGpuFuncs_InitRayQuery(rayQueryEXT aRayQuery, lib_shader_PixelInput aInput, TestGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS) {
  vec3 ndc = vec3((((aInput.fragCoord.x / aUniforms.rtWidth) * 2.0) - 1.0),(1.0 - ((aInput.fragCoord.y / aUniforms.rtHeight) * 2.0)),1.0);
  mat4 _tmp_A1 = aUniforms.cameraInvView;
  vec3 origin = vec3(_tmp_A1[3][0],_tmp_A1[3][1],_tmp_A1[3][2]);
  vec3 target = lib_shader_Vec3TransformCoord(ndc,aUniforms.cameraInvViewProj);
  vec3 dir = normalize(((target-(origin.xyz)).xyz));
  rayQueryInitializeEXT(aRayQuery,aAS,lib_shader_RayFlags_None,255,(origin.xyz),0.001,(dir.xyz),aUniforms.cameraFarClipPlane);
}
vec2 TestGpuFuncs_Lerp_4_Vec2_Vec2_Vec2_Vec3(vec2 aX, vec2 aY, vec2 aZ, vec3 aBary) {
  return (((aX*aBary.x)+(aY*aBary.y))+(aZ*aBary.z));
}
vec3 TestGpuFuncs_Lerp_4_Vec3_Vec3_Vec3_Vec3(vec3 aX, vec3 aY, vec3 aZ, vec3 aBary) {
  return (((aX*aBary.x)+(aY*aBary.y))+(aZ*aBary.z));
}
TestGpuFuncs_PixelOutput TestGpuFuncs_raytracer_lit_textured_cube_ps(lib_shader_PixelInput aInput, TestGpuFuncs_RayUniforms aUniforms, accelerationStructureEXT aAS, sampler aSS) {
  rayQueryEXT rayQuery/*__noinit__*/;
  TestGpuFuncs_InitRayQuery(rayQuery,aInput,aUniforms,aAS);
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
  TestGpuFuncs_RayInstanceData instData = nil_builtin_GetRayInstanceData[nonuniformEXT(userInstIndex)].v;
  vec4 color;
  bool _tmp_S2 = (intersectionType == lib_shader_RayQueryIntersectionType_CommittedTriangle);
  if (_tmp_S2) {
    bool _tmp_V2 = (instData.vbIndex == 0);
    if (_tmp_V2) {
      color = vec4(1.0,0.0,1.0,1.0);
    }
    else {
      {
        uint primIndex = uint(rayQueryGetIntersectionPrimitiveIndexEXT(rayQuery,true));
        uvec3 inds = TestGpuFuncs_GetTriangleIndices(instData.ibIndex,primIndex);
        TestGpuFuncs_VertexRay v0 = nil_builtin_GetVertexRay[nonuniformEXT(instData.vbIndex)].v[nonuniformEXT(inds.x)];
        TestGpuFuncs_VertexRay v1 = nil_builtin_GetVertexRay[nonuniformEXT(instData.vbIndex)].v[nonuniformEXT(inds.y)];
        TestGpuFuncs_VertexRay v2 = nil_builtin_GetVertexRay[nonuniformEXT(instData.vbIndex)].v[nonuniformEXT(inds.z)];
        vec3 bary = TestGpuFuncs_BaryToVec3(rayQueryGetIntersectionBarycentricsEXT(rayQuery,true));
        vec3 n = normalize(TestGpuFuncs_Lerp_4_Vec3_Vec3_Vec3_Vec3(v0.normal,v1.normal,v2.normal,bary));
        vec3 worldNormal = normalize(lib_shader_Vec3TransformNormal_2_Vec3_Matrix4x3(n,rayQueryGetIntersectionObjectToWorldEXT(rayQuery,true)));
        vec3 p = TestGpuFuncs_Lerp_4_Vec3_Vec3_Vec3_Vec3(v0.pos,v1.pos,v2.pos,bary);
        vec3 worldPos = lib_shader_Vec3Transform_2_Vec3_Matrix4x3(p,rayQueryGetIntersectionObjectToWorldEXT(rayQuery,true));
        vec2 uv = TestGpuFuncs_Lerp_4_Vec2_Vec2_Vec2_Vec3(v0.tex0,v1.tex0,v2.tex0,bary);
        vec3 texColor;
        bool _tmp_04 = (instData.texIndex > 0);
        if (_tmp_04) {
          texColor = (texture(sampler2D(nil_builtin_GetTexture2D[nonuniformEXT(instData.texIndex)],aSS),uv).rgb);
        }
        else {
          {
            texColor = lib_shader_UIntToHashColor(uint(rayQueryGetIntersectionInstanceCustomIndexEXT(rayQuery,true)));
          }
        }
        vec3 C = ((TestGpuFuncs_DirLight(worldPos,worldNormal,worldLightDir0,(lightColor0*texColor),cosBias,shadowColor,aAS)+TestGpuFuncs_DirLight(worldPos,worldNormal,worldLightDir1,(lightColor1*texColor),cosBias,shadowColor,aAS))+TestGpuFuncs_DirLight(worldPos,worldNormal,worldLightDir2,(lightColor2*texColor),cosBias,shadowColor,aAS));
        vec3 _tmp_J4 = C;
        color = vec4(_tmp_J4.x,_tmp_J4.y,_tmp_J4.z,1.0);
      }
    }
  }
  else {
    bool _tmp_K4 = (intersectionType == lib_shader_RayQueryIntersectionType_CommittedBoundingVolume);
    if (_tmp_K4) {
      color = vec4(1.0,0.0,0.0,1.0);
    }
    else {
      {
        color = vec4(0.0,0.5,0.8,0.0);
      }
    }
  }
  vec4 _tmp_X4 = color;
  return TestGpuFuncs_PixelOutput_new(_tmp_X4);
}
// MODULE END TestGpuFuncs

// Pixel Shader main: TestGpuFuncs_raytracer_lit_textured_cube_ps
// type size: 48, underlying: float
layout(set = 0, binding = 0) uniform UBO_TestGpuFuncs_RayUniforms { TestGpuFuncs_RayUniforms v; } IN_1_aUniforms;
layout(set = 7, binding = 0) uniform accelerationStructureEXT IN_1_aAS;
layout(set = 5, binding = 0) uniform sampler IN_1_aSS;
layout(location = 0) out vec4 OUT_0_rval_color;
void main(void) {
  lib_shader_initialize();
  lib_shader_PixelInput aInput;
  TestGpuFuncs_RayUniforms aUniforms;
  aInput.fragCoord = gl_FragCoord;
  aInput.frontFacing = gl_FrontFacing;
  aUniforms = IN_1_aUniforms.v;
  TestGpuFuncs_PixelOutput _rval_ = TestGpuFuncs_raytracer_lit_textured_cube_ps(aInput, aUniforms, IN_1_aAS, IN_1_aSS);
  OUT_0_rval_color = _rval_.color;
}

