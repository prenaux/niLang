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
  virtual ni::tBool __stdcall SerializeDataTable(ni::iDataTable* apDT, tSerializeFlags aFlags) {
    _DTS_INIT(apDT,aFlags,IS_RO);
    _DTS_B("wireframe", mbWireframe);
    _DTS_E("culling_mode", mCullingMode, eCullingMode);
    _DTS_L("color_write_mask", mColorWriteMask, eColorWriteMask);
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
  BEGIN(Wireframe);
  s->SetCullingMode(eCullingMode_CCW);
  s->SetWireframe(eTrue);
  END();

  BEGIN(NoCullingColorWriteNone);
  s->SetCullingMode(eCullingMode_None);
  s->SetColorWriteMask(eColorWriteMask_None);
  END();
  BEGIN(NoCullingFilled);
  s->SetCullingMode(eCullingMode_None);
  END();
  BEGIN(NoCullingWireframe);
  s->SetCullingMode(eCullingMode_None);
  s->SetWireframe(eTrue);
  END();

  BEGIN(CWCullingColorWriteNone);
  s->SetCullingMode(eCullingMode_CW);
  s->SetColorWriteMask(eColorWriteMask_None);
  END();
  BEGIN(CWCullingFilled);
  s->SetCullingMode(eCullingMode_CW);
  END();
  BEGIN(CWCullingWireframe);
  s->SetCullingMode(eCullingMode_CW);
  s->SetWireframe(eTrue);
  END();

  return eTrue;
}

///////////////////////////////////////////////
iRasterizerStates* __stdcall cGraphics::CreateRasterizerStates() {
  CHECKDRIVER(NULL);
  return niNew cRasterizerStates<eFalse>();
}

///////////////////////////////////////////////
iRasterizerStates* __stdcall cGraphics::GetCompiledRasterizerStates(tIntPtr aHandle) const {
  if (!aHandle) return NULL;
  tRSMap::const_iterator it = mmapRasterizerStates.find(aHandle);
  if (it == mmapRasterizerStates.end()) return NULL;
  return it->second.ptr();
}
