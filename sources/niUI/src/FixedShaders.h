#pragma once
#ifndef __FIXEDSHADERS_H_33A3D115_04E2_4DB9_A299_00E46D74DA09__
#define __FIXEDSHADERS_H_33A3D115_04E2_4DB9_A299_00E46D74DA09__

#include "API/niUI_ModuleDef.h"

#define SHADER_MATERIAL_HAS_FLAG(FLAG) (matFlags & eMaterialFlags_##FLAG)
#define SHADER_MATERIAL_HAS_CHANNEL(CHANNEL) (matDesc.mChannels[eMaterialChannel_##CHANNEL].mTexture.IsOK())

#include "shaders/fixed_ps.h"

namespace ni {


niExportFunc(tBool) FixedShaders_UpdateConstants(iGraphicsContext* apContext, iShaderConstants* apBuffer, iDrawOperation* apDrawOp);

enum eFixedVertexShader {
  eFixedVertexShader_P = 0,
  eFixedVertexShader_PA = 1,
  eFixedVertexShader_PN = 2,
  eFixedVertexShader_PNA = 3,
  eFixedVertexShader_PNT1 = 4,
  eFixedVertexShader_PNAT1 = 5,
  eFixedVertexShader_PNT2 = 6,
  eFixedVertexShader_PNAT2 = 7,
  eFixedVertexShader_PB4INT1 = 8,
  eFixedVertexShader_PB4INAT1 = 9,
  eFixedVertexShader_Last = 10,
  eFixedVertexShader_ForceDWORD = 0xFFFFFFFF,
};

struct sFixedShaders : ImplRC<ni::iUnknown> {
  tBool _initialized;
  Ptr<iShader> _vertexShaders[eFixedVertexShader_Last];
  Ptr<iShader> _pixelShaders[e_fixed_ps_last];

  sFixedShaders() : _initialized(eFalse) {
  }

  static const char* GetFixedVertexShaderName(eFixedVertexShader vs) {
    switch (vs) {
    case eFixedVertexShader_P: return "fixed_vs_p";
    case eFixedVertexShader_PA: return "fixed_vs_pa";
    case eFixedVertexShader_PN: return "fixed_vs_pn";
    case eFixedVertexShader_PNA: return "fixed_vs_pna";
    case eFixedVertexShader_PNT1: return "fixed_vs_pnt1";
    case eFixedVertexShader_PNAT1: return "fixed_vs_pnat1";
    case eFixedVertexShader_PNT2: return "fixed_vs_pnt2";
    case eFixedVertexShader_PNAT2: return "fixed_vs_pnat2";
    case eFixedVertexShader_PB4INT1: return "fixed_vs_pb4int1";
    case eFixedVertexShader_PB4INAT1: return "fixed_vs_pb4inat1";
    }
    return NULL;
  }

  static const char* GetFixedPixelShaderName(tU32 ps) {
    if (ps >= e_fixed_ps_last)
      return NULL;
    return fn_fixed_ps[ps];
  }

  static Ptr<iHString> GetFixedShaderPath(const char* aShaderName) {
    return _H(niFmt("niUI://shaders/%s.cgo", aShaderName));
  }

  static eFixedVertexShader GetFixedVertexShader(const tFVF aFVF, const sMaterialDesc& aMatDesc) {
    const tU32 matFlags = aMatDesc.mFlags;

#define CASE_FVF(VF) \
    case eVertexFormat_##VF: return eFixedVertexShader_##VF;
#define CASE_CFVF(CVF,VF) \
    case eVertexFormat_##CVF: return (matFlags&eMaterialFlags_Vertex) ? eFixedVertexShader_##CVF:eFixedVertexShader_##VF;

    switch (aFVF) {
      CASE_FVF(P);
      CASE_CFVF(PA,P);
      CASE_FVF(PN);
      CASE_CFVF(PNA,PN);
      CASE_FVF(PNT1);
      CASE_CFVF(PNAT1,PNT1);
      CASE_FVF(PNT2);
      CASE_CFVF(PNAT2,PNT2);
      CASE_FVF(PB4INT1);
      CASE_CFVF(PB4INAT1,PB4INT1);
      // Below are FVFs that ended up in a bunch of vpk because the geometry
      // packer was or'ing eFVF_Tex1|eFVF_Tex2, which is incorrect, they end
      // up as an addition when ored, only of of them can be set on a FVF.
      // Fortunately eFVF_Tex3 has never been used in any real life models so
      // we can just return another FVF as a workaround for now.
    case eFVF_Position|eFVF_Normal|eFVF_ColorA|eFVF_Tex3:
      return (matFlags&eMaterialFlags_Vertex) ? eFixedVertexShader_PNAT2 : eFixedVertexShader_PNT2;
    case eFVF_PositionB4|eFVF_Indices|eFVF_Normal|eFVF_ColorA|eFVF_Tex3:
      return (matFlags&eMaterialFlags_Vertex) ? eFixedVertexShader_PB4INAT1 : eFixedVertexShader_PB4INT1;
    }

    return eFixedVertexShader_Last;
  }

  static tU32 GetFixedPixelShader(const sMaterialDesc& aMatDesc) {
    return select_shader_fixed_ps(aMatDesc);
  }

  static Ptr<iShader> LoadShader(ni::iGraphics* g, iHString* ahspPath) {
    Ptr<iShader> shader = g->CreateShaderFromRes(ahspPath);
    niCheck(shader.IsOK(), NULL);
    // niLog(Info, niFmt("Driver '%s' loaded fixed shader '%s'.", g->GetName(), aPath));
    return shader;
  }

  tBool Initialize(ni::iGraphics* g) {
    if (_initialized)
      return ni::eTrue;

    niCheckIsOK(g, ni::eFalse);

    niLoop(i,niCountOf(_vertexShaders)) {
      _vertexShaders[i] = LoadShader(g,GetFixedShaderPath(GetFixedVertexShaderName((eFixedVertexShader)i)));
      if (!_vertexShaders[i].IsOK()) {
        niError(niFmt("Can't load fixed vertex shader '%s'.", GetFixedVertexShaderName((eFixedVertexShader)i)));
        return ni::eFalse;
      }
    }
    niLoop(i,niCountOf(_pixelShaders)) {
      _pixelShaders[i] = LoadShader(g,GetFixedShaderPath(GetFixedPixelShaderName((e_fixed_ps)i)));
      if (!_pixelShaders[i].IsOK()) {
        niError(niFmt("Can't load fixed pixel shader '%s'.", GetFixedPixelShaderName((e_fixed_ps)i)));
        return ni::eFalse;
      }
    }

    _initialized = ni::eTrue;
    return ni::eTrue;
  }

  iShader* GetVertexShader(const tFVF aFVF, const sMaterialDesc& aMatDesc) const {
    eFixedVertexShader vs = GetFixedVertexShader(aFVF,aMatDesc);
    if (vs >= eFixedVertexShader_Last) {
      niWarning(niFmt("Can't get vertex shader for FVF '%s', mat: %s, matFlag: %x",
                      FVFToString(aFVF), aMatDesc.mhspName, niFlagsToChars(eMaterialFlags,aMatDesc.mFlags)));
      return NULL;
    }
    return _vertexShaders[vs];
  }

  iShader* GetPixelShader(const sMaterialDesc& aMatDesc) const {
    return _pixelShaders[GetFixedPixelShader(aMatDesc)];
  }

};

}
#endif // __FIXEDSHADERS_H_33A3D115_04E2_4DB9_A299_00E46D74DA09__
