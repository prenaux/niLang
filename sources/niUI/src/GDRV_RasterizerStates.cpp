// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Graphics.h"
#include <niLang/Utils/DTS.h>
#include "API/niUI_ModuleDef.h"

//--------------------------------------------------------------------------------------------
//
//  RasterizerStates
//
//--------------------------------------------------------------------------------------------

template <tBool IS_RO>
class cRasterizerStates :
    public ImplRC<iRasterizerStates>,
    public sRasterizerStatesDesc
{
  niBeginClass(cRasterizerStates);

 private:
  cRasterizerStates(cRasterizerStates&);
  cRasterizerStates& operator = (const cRasterizerStates&);

 public:
  cRasterizerStates() {
    mbWireframe = eFalse;
    mCullingMode = eCullingMode_CCW;
    mColorWriteMask = eColorWriteMask_All;
    mbScissorTest = eFalse;
    mfDepthBiasFactor = 0.0f;
    mfDepthBiasUnitScale = 0.0f;
  }
  cRasterizerStates(const sRasterizerStatesDesc& aDesc) {
    *(sRasterizerStatesDesc*)this = aDesc;
  }
  ~cRasterizerStates() {
  }

  tBool __stdcall IsOK() const {
    niClassIsOK(cRasterizerStates);
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tPtr __stdcall GetDescStructPtr() const {
    return (tPtr)niStaticCast(const sRasterizerStatesDesc*,this);
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall Copy(const iRasterizerStates* apStates) {
    if (IS_RO)
      return eFalse;
    niCheckSilent(niIsOK(apStates),eFalse);
    SetWireframe(apStates->GetWireframe());
    SetCullingMode(apStates->GetCullingMode());
    SetColorWriteMask(apStates->GetColorWriteMask());
    SetScissorTest(apStates->GetScissorTest());
    SetDepthBiasFactor(apStates->GetDepthBiasFactor());
    SetDepthBiasUnitScale(apStates->GetDepthBiasUnitScale());
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall GetIsCompiled() const {
    return eFalse;
  }

  ///////////////////////////////////////////////
  virtual iRasterizerStates* __stdcall Clone() const {
    iRasterizerStates* pNew = niNew cRasterizerStates<eFalse>();
    pNew->Copy(this);
    return pNew;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SetWireframe(tBool abWireframe) {
    if (IS_RO) return eFalse;
    mbWireframe = abWireframe;
    return eTrue;
  }
  virtual tBool __stdcall GetWireframe() const {
    return mbWireframe;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SetCullingMode(eCullingMode aMode) {
    if (IS_RO) return eFalse;
    mCullingMode = aMode;
    return eTrue;
  }
  virtual eCullingMode __stdcall GetCullingMode() const {
    return mCullingMode;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SetColorWriteMask(eColorWriteMask aMask) {
    if (IS_RO) return eFalse;
    mColorWriteMask = aMask;
    return eTrue;
  }
  virtual eColorWriteMask __stdcall GetColorWriteMask() const {
    return mColorWriteMask;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SetScissorTest(tBool abTest) {
    if (IS_RO) return eFalse;
    mbScissorTest = abTest;
    return eTrue;
  }
  virtual tBool __stdcall GetScissorTest() const {
    return mbScissorTest;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SetDepthBiasFactor(tF32 aVal) {
    if (IS_RO) return eFalse;
    mfDepthBiasFactor = aVal;
    return eTrue;
  }
  virtual tF32 __stdcall GetDepthBiasFactor() const {
    return mfDepthBiasFactor;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SetDepthBiasUnitScale(tF32 aVal) {
    if (IS_RO) return eFalse;
    mfDepthBiasUnitScale = aVal;
    return eTrue;
  }
  virtual tF32 __stdcall GetDepthBiasUnitScale() const {
    return mfDepthBiasUnitScale;
  }

  ///////////////////////////////////////////////
  virtual ni::tBool __stdcall SerializeDataTable(ni::iDataTable* apDT, tSerializeFlags aFlags) {
    _DTS_INIT(apDT,aFlags,IS_RO);
    _DTS_B("wireframe", mbWireframe);
    _DTS_E("culling_mode", mCullingMode, eCullingMode);
    _DTS_L("color_write_mask", mColorWriteMask, eColorWriteMask);
    _DTS_B("scissor_test", mbScissorTest);
    _DTS_F("depth_bias_factor", mfDepthBiasFactor);
    _DTS_F("depth_bias_unit_scale", mfDepthBiasUnitScale);
    return ni::eTrue;
  }

  niEndClass(cRasterizerStates);
};

//--------------------------------------------------------------------------------------------
//
//  cGraphics
//
//--------------------------------------------------------------------------------------------

tBool cGraphics::_CompileDefaultRasterizerStates() {
#define BEGIN(STATE)                                          \
  {                                                           \
      const eCompiledStates cs = eCompiledStates_RS_##STATE;  \
      Ptr<cRasterizerStates<eFalse> > s = niNew cRasterizerStates<eFalse>();

#define END()                                             \
  astl::upsert(mmapRasterizerStates,                 \
                    (tIntPtr)cs,                          \
                    niNew cRasterizerStates<eTrue>(*s));   \
}

  BEGIN(ColorWriteNone);
  s->SetCullingMode(eCullingMode_CCW);
  s->SetColorWriteMask(eColorWriteMask_None);
  END();
  BEGIN(Filled);
  s->SetCullingMode(eCullingMode_CCW);
  END();
  BEGIN(FilledScissor);
  s->SetCullingMode(eCullingMode_CCW);
  s->SetScissorTest(eTrue);
  END();
  BEGIN(Wireframe);
  s->SetCullingMode(eCullingMode_CCW);
  s->SetWireframe(eTrue);
  END();
  BEGIN(WireframeScissor);
  s->SetCullingMode(eCullingMode_CCW);
  s->SetWireframe(eTrue);
  s->SetScissorTest(eTrue);
  END();

  BEGIN(NoCullingColorWriteNone);
  s->SetCullingMode(eCullingMode_None);
  s->SetColorWriteMask(eColorWriteMask_None);
  END();
  BEGIN(NoCullingFilled);
  s->SetCullingMode(eCullingMode_None);
  END();
  BEGIN(NoCullingFilledScissor);
  s->SetCullingMode(eCullingMode_None);
  s->SetScissorTest(eTrue);
  END();
  BEGIN(NoCullingWireframe);
  s->SetCullingMode(eCullingMode_None);
  s->SetWireframe(eTrue);
  END();
  BEGIN(NoCullingWireframeScissor);
  s->SetCullingMode(eCullingMode_None);
  s->SetWireframe(eTrue);
  s->SetScissorTest(eTrue);
  END();

  BEGIN(CWCullingColorWriteNone);
  s->SetCullingMode(eCullingMode_CW);
  s->SetColorWriteMask(eColorWriteMask_None);
  END();
  BEGIN(CWCullingFilled);
  s->SetCullingMode(eCullingMode_CW);
  END();
  BEGIN(CWCullingFilledScissor);
  s->SetCullingMode(eCullingMode_CW);
  s->SetScissorTest(eTrue);
  END();
  BEGIN(CWCullingWireframe);
  s->SetCullingMode(eCullingMode_CW);
  s->SetWireframe(eTrue);
  END();
  BEGIN(CWCullingWireframeScissor);
  s->SetCullingMode(eCullingMode_CW);
  s->SetWireframe(eTrue);
  s->SetScissorTest(eTrue);
  END();

  return eTrue;
}

///////////////////////////////////////////////
iRasterizerStates* __stdcall cGraphics::CreateRasterizerStates() {
  CHECKDRIVER(NULL);
  return niNew cRasterizerStates<eFalse>();
}

///////////////////////////////////////////////
tIntPtr __stdcall cGraphics::CompileRasterizerStates(iRasterizerStates* apStates) {
  CHECKDRIVER(NULL);
  niCheckIsOK(apStates,NULL);

  const sRasterizerStatesDesc& desc = *(sRasterizerStatesDesc*)apStates->GetDescStructPtr();
  niLoopit(tRSMap::iterator,it,mmapRasterizerStates) {
    if (*((sRasterizerStatesDesc*)(it->second->GetDescStructPtr())) == desc)
      return it->first;
  }

  tIntPtr handle = NULL;
  if (niFlagIs(mptrDrv->GetGraphicsDriverImplFlags(),eGraphicsDriverImplFlags_CompileRasterizerStates)) {
    handle = mptrDrv->CompileRasterizerStates(apStates);
    if (!handle) {
      niError(_A("Driver can't compile the specified states."));
      return 0;
    }
    niAssert(handle >= eCompiledStates_Driver);
  }
  else {
    handle = mStatesHandleGen++;
  }

  Ptr<iRasterizerStates> ds = niNew cRasterizerStates<eTrue>(desc);
  astl::upsert(mmapRasterizerStates,handle,ds);
  return handle;
}

///////////////////////////////////////////////
iRasterizerStates* __stdcall cGraphics::GetCompiledRasterizerStates(tIntPtr aHandle) const {
  if (!aHandle) return NULL;
  tRSMap::const_iterator it = mmapRasterizerStates.find(aHandle);
  if (it == mmapRasterizerStates.end()) return NULL;
  return it->second.ptr();
}
