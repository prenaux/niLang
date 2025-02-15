// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Graphics.h"
#include <niLang/Utils/DTS.h>
#include "API/niUI_ModuleDef.h"

//--------------------------------------------------------------------------------------------
//
//| SamplerStates
//
//--------------------------------------------------------------------------------------------

template <tBool IS_RO>
class cSamplerStates :
    public ImplRC<iSamplerStates>,
    public sSamplerStatesDesc
{
  niBeginClass(cSamplerStates);


 private:
  cSamplerStates(cSamplerStates&);
  cSamplerStates& operator = (const cSamplerStates&);

 public:

  cSamplerStates() {
    mFilter = eSamplerFilter_Point;
    mWrapS = eSamplerWrap_Clamp;
    mWrapT = eSamplerWrap_Clamp;
    mWrapR = eSamplerWrap_Clamp;
    mcolBorder = sColor4f::Zero();
  }
  cSamplerStates(const sSamplerStatesDesc& aDesc) {
    *(sSamplerStatesDesc*)this = aDesc;
  }
  ~cSamplerStates() {
  }

  tBool __stdcall IsOK() const {
    niClassIsOK(cSamplerStates);
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tPtr __stdcall GetDescStructPtr() const {
    return (tPtr)niStaticCast(const sSamplerStatesDesc*,this);
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall Copy(const iSamplerStates* apStates) {
    if (IS_RO) return eFalse;
    niCheckSilent(niIsOK(apStates),eFalse);
    SetFilter(apStates->GetFilter());
    SetWrapS(apStates->GetWrapS());
    SetWrapT(apStates->GetWrapT());
    SetWrapR(apStates->GetWrapR());
    SetBorderColor(apStates->GetBorderColor());
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual iSamplerStates* __stdcall Clone() const {
    iSamplerStates* pNew = niNew cSamplerStates<eFalse>();
    pNew->Copy(this);
    return pNew;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall GetIsCompiled() const {
    return eFalse;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SetFilter(eSamplerFilter aFilter) {
    if (IS_RO) return eFalse;
    mFilter = aFilter;
    return eTrue;
  }
  virtual eSamplerFilter __stdcall GetFilter() const {
    return mFilter;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SetWrapS(eSamplerWrap aWrap) {
    if (IS_RO) return eFalse;
    mWrapS = aWrap;
    return eTrue;
  }
  virtual eSamplerWrap __stdcall GetWrapS() const {
    return mWrapS;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SetWrapT(eSamplerWrap aWrap) {
    if (IS_RO) return eFalse;
    mWrapT = aWrap;
    return eTrue;
  }
  virtual eSamplerWrap __stdcall GetWrapT() const {
    return mWrapT;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SetWrapR(eSamplerWrap aWrap) {
    if (IS_RO) return eFalse;
    mWrapR = aWrap;
    return eTrue;
  }
  virtual eSamplerWrap __stdcall GetWrapR() const {
    return mWrapR;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SetBorderColor(const sColor4f& aColor) {
    if (IS_RO) return eFalse;
    mcolBorder = aColor;
    return eTrue;
  }
  virtual const sColor4f& __stdcall GetBorderColor() const {
    return mcolBorder;
  }

  ///////////////////////////////////////////////
  virtual ni::tBool __stdcall SerializeDataTable(ni::iDataTable* apDT, tSerializeFlags aFlags) {
    _DTS_INIT(apDT,aFlags,IS_RO);
    _DTS_E("filter", mFilter, eSamplerFilter);
    _DTS_E("wrap_s", mWrapS, eSamplerWrap);
    _DTS_E("wrap_t", mWrapT, eSamplerWrap);
    _DTS_E("wrap_r", mWrapR, eSamplerWrap);
    _DTS_RGBA("border", mcolBorder);
    return ni::eTrue;
  }

  niEndClass(cSamplerStates);
};

//--------------------------------------------------------------------------------------------
//
//  cGraphics
//
//--------------------------------------------------------------------------------------------

tBool cGraphics::_CompileDefaultSamplerStates() {
#define BEGIN(STATE)                                          \
  {                                                           \
      const eCompiledStates cs = eCompiledStates_SS_##STATE;  \
      Ptr<cSamplerStates<eFalse> > s = niNew cSamplerStates<eFalse>();

#define END()                                         \
  astl::upsert(mmapSamplerStates,                \
                    (tIntPtr)cs,                      \
                    niNew cSamplerStates<eTrue>(*s));  \
}

#define SS(STATE,FILTER,WRAP)                   \
  BEGIN(STATE);                                 \
  s->SetFilter(eSamplerFilter_##FILTER);        \
  s->SetWrapS(eSamplerWrap_##WRAP);             \
  s->SetWrapT(eSamplerWrap_##WRAP);             \
  s->SetWrapR(eSamplerWrap_##WRAP);             \
  s->SetBorderColor(sColor4f::White());         \
  END();

  SS(PointRepeat,Point,Repeat);
  SS(PointClamp,Point,Clamp);
  SS(PointMirror,Point,Mirror);
  SS(PointWhiteBorder,Point,Border);
  SS(SmoothRepeat,Smooth,Repeat);
  SS(SmoothClamp,Smooth,Clamp);
  SS(SmoothMirror,Smooth,Mirror);
  SS(SmoothWhiteBorder,Smooth,Border);

  return eTrue;
}

///////////////////////////////////////////////
iSamplerStates* __stdcall cGraphics::CreateSamplerStates() {
  CHECKDRIVER(NULL);
  return niNew cSamplerStates<eFalse>();
}

///////////////////////////////////////////////
iSamplerStates* __stdcall cGraphics::GetCompiledSamplerStates(tIntPtr aHandle) const {
  if (!aHandle) return NULL;
  tSSMap::const_iterator it = mmapSamplerStates.find(aHandle);
  if (it == mmapSamplerStates.end()) return NULL;
  return it->second.ptr();
}
