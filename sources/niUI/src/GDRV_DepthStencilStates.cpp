// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Graphics.h"
#include <niLang/Utils/DTS.h>
#include "API/niUI_ModuleDef.h"

//--------------------------------------------------------------------------------------------
//
// DepthStencilStates
//
//--------------------------------------------------------------------------------------------

template <tBool IS_RO>
class cDepthStencilStates :
    public cIUnknownImpl<iDepthStencilStates>,
    public sDepthStencilStatesDesc
{
  niBeginClass(cDepthStencilStates);

 private:
  cDepthStencilStates(const cDepthStencilStates&);
  cDepthStencilStates& operator = (const cDepthStencilStates&);

 public:
  cDepthStencilStates() {
    mbDepthTest = eFalse;
    mbDepthTestWrite = eTrue;
    mDepthTestCompare = eGraphicsCompare_LessEqual;
    mStencilMode = eStencilMode_None;
    mnStencilRef = 0;
    mnStencilMask = 0xFFFFFFFF;
    mStencilFrontCompare = eGraphicsCompare_Never;
    mStencilFrontFail = eStencilOp_Keep;
    mStencilFrontPassDepthFail = eStencilOp_Keep;
    mStencilFrontPassDepthPass = eStencilOp_Keep;
    mStencilBackCompare = eGraphicsCompare_Never;
    mStencilBackFail = eStencilOp_Keep;
    mStencilBackPassDepthFail = eStencilOp_Keep;
    mStencilBackPassDepthPass = eStencilOp_Keep;
  }
  cDepthStencilStates(const sDepthStencilStatesDesc& aRight) {
    *(sDepthStencilStatesDesc*)this = aRight;
  }
  ~cDepthStencilStates() {
  }

  tBool __stdcall IsOK() const {
    niClassIsOK(cDepthStencilStates);
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tPtr __stdcall GetDescStructPtr() const {
    return (tPtr)niStaticCast(const sDepthStencilStatesDesc*,this);
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall Copy(const iDepthStencilStates* apStates) {
    if (IS_RO)
      return eFalse;
    niCheckSilent(niIsOK(apStates),eFalse);
    SetDepthTest(apStates->GetDepthTest());
    SetDepthTestWrite(apStates->GetDepthTestWrite());
    SetDepthTestCompare(apStates->GetDepthTestCompare());
    SetStencilMode(apStates->GetStencilMode());
    SetStencilRef(apStates->GetStencilRef());
    SetStencilMask(apStates->GetStencilMask());
    SetStencilFrontCompare(apStates->GetStencilFrontCompare());
    SetStencilFrontFail(apStates->GetStencilFrontFail());
    SetStencilFrontPassDepthFail(apStates->GetStencilFrontPassDepthFail());
    SetStencilFrontPassDepthPass(apStates->GetStencilFrontPassDepthPass());
    SetStencilBackCompare(apStates->GetStencilBackCompare());
    SetStencilBackFail(apStates->GetStencilBackFail());
    SetStencilBackPassDepthFail(apStates->GetStencilBackPassDepthFail());
    SetStencilBackPassDepthPass(apStates->GetStencilBackPassDepthPass());
    return eTrue;
  }
  ///////////////////////////////////////////////
  virtual iDepthStencilStates* __stdcall Clone() const {
    iDepthStencilStates* pNew = niNew cDepthStencilStates<eFalse>();
    pNew->Copy(this);
    return pNew;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall GetIsCompiled() const {
    return eFalse;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SetDepthTest(tBool aVal) {
    if (IS_RO) return eFalse;
    mbDepthTest = aVal;
    return eTrue;
  }
  virtual tBool __stdcall GetDepthTest() const {
    return mbDepthTest;
  }
  ///////////////////////////////////////////////
  virtual tBool __stdcall SetDepthTestWrite(tBool aVal) {
    if (IS_RO) return eFalse;
    mbDepthTestWrite = aVal;
    return eTrue;
  }
  virtual tBool __stdcall GetDepthTestWrite() const {
    return mbDepthTestWrite;
  }
  ///////////////////////////////////////////////
  virtual tBool __stdcall SetDepthTestCompare(eGraphicsCompare aVal) {
    if (IS_RO) return eFalse;
    mDepthTestCompare = aVal;
    return eTrue;
  }
  virtual eGraphicsCompare __stdcall GetDepthTestCompare() const {
    return mDepthTestCompare;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SetStencilMode(eStencilMode aVal) {
    if (IS_RO) return eFalse;
    mStencilMode = aVal;
    return eTrue;
  }
  virtual eStencilMode __stdcall GetStencilMode() const {
    return mStencilMode;
  }
  ///////////////////////////////////////////////
  virtual tBool __stdcall SetStencilRef(tI32 aVal) {
    if (IS_RO) return eFalse;
    mnStencilRef = aVal;
    return eTrue;
  }
  virtual tI32 __stdcall GetStencilRef() const {
    return mnStencilRef;
  }
  ///////////////////////////////////////////////
  virtual tBool __stdcall SetStencilMask(tU32 aVal) {
    if (IS_RO) return eFalse;
    mnStencilMask = aVal;
    return eTrue;
  }
  virtual tU32 __stdcall GetStencilMask() const {
    return mnStencilMask;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SetStencilFrontCompare(eGraphicsCompare aVal) {
    if (IS_RO) return eFalse;
    mStencilFrontCompare = aVal;
    return eTrue;
  }
  virtual eGraphicsCompare __stdcall GetStencilFrontCompare() const {
    return mStencilFrontCompare;
  }
  ///////////////////////////////////////////////
  virtual tBool __stdcall SetStencilFrontFail(eStencilOp aVal) {
    if (IS_RO) return eFalse;
    mStencilFrontFail = aVal;
    return eTrue;
  }
  virtual eStencilOp __stdcall GetStencilFrontFail() const {
    return mStencilFrontFail;
  }
  ///////////////////////////////////////////////
  virtual tBool __stdcall SetStencilFrontPassDepthFail(eStencilOp aVal) {
    if (IS_RO) return eFalse;
    mStencilFrontPassDepthFail = aVal;
    return eTrue;
  }
  virtual eStencilOp __stdcall GetStencilFrontPassDepthFail() const {
    return mStencilFrontPassDepthFail;
  }
  ///////////////////////////////////////////////
  virtual tBool __stdcall SetStencilFrontPassDepthPass(eStencilOp aVal) {
    if (IS_RO) return eFalse;
    mStencilFrontPassDepthPass = aVal;
    return eTrue;
  }
  virtual eStencilOp __stdcall GetStencilFrontPassDepthPass() const {
    return mStencilFrontPassDepthPass;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SetStencilBackCompare(eGraphicsCompare aVal) {
    if (IS_RO) return eFalse;
    mStencilBackCompare = aVal;
    return eTrue;
  }
  virtual eGraphicsCompare __stdcall GetStencilBackCompare() const {
    return mStencilBackCompare;
  }
  ///////////////////////////////////////////////
  virtual tBool __stdcall SetStencilBackFail(eStencilOp aVal) {
    if (IS_RO) return eFalse;
    mStencilBackFail = aVal;
    return eTrue;
  }
  virtual eStencilOp __stdcall GetStencilBackFail() const {
    return mStencilBackFail;
  }
  ///////////////////////////////////////////////
  virtual tBool __stdcall SetStencilBackPassDepthFail(eStencilOp aVal) {
    if (IS_RO) return eFalse;
    mStencilBackPassDepthFail = aVal;
    return eTrue;
  }
  virtual eStencilOp __stdcall GetStencilBackPassDepthFail() const {
    return mStencilBackPassDepthFail;
  }
  ///////////////////////////////////////////////
  virtual tBool __stdcall SetStencilBackPassDepthPass(eStencilOp aVal) {
    if (IS_RO) return eFalse;
    mStencilBackPassDepthPass = aVal;
    return eTrue;
  }
  virtual eStencilOp __stdcall GetStencilBackPassDepthPass() const {
    return mStencilBackPassDepthPass;
  }

  ///////////////////////////////////////////////
  virtual ni::tBool __stdcall SerializeDataTable(ni::iDataTable* apDT, tSerializeFlags aFlags) {
    _DTS_INIT(apDT,aFlags,IS_RO);
    _DTS_B("depth_test",mbDepthTest);
    _DTS_B("depth_test_write",mbDepthTestWrite);
    _DTS_E("depth_test_compare",mDepthTestCompare,eGraphicsCompare);
    _DTS_E("stencil_mode",mStencilMode,eStencilMode);
    _DTS_I("stencil_ref",mnStencilRef);
    _DTS_I("stencil_mask",mnStencilMask);
    _DTS_E("stencil_front_compare",mStencilFrontCompare,eGraphicsCompare);
    _DTS_E("stencil_front_fail",mStencilFrontFail,eStencilOp);
    _DTS_E("stencil_front_pass_depth_fail",mStencilFrontPassDepthFail,eStencilOp);
    _DTS_E("stencil_front_pass_depth_pass",mStencilFrontPassDepthPass,eStencilOp);
    _DTS_E("stencil_back_compare",mStencilBackCompare,eGraphicsCompare);
    _DTS_E("stencil_back_fail",mStencilBackFail,eStencilOp);
    _DTS_E("stencil_back_pass_depth_fail",mStencilBackPassDepthFail,eStencilOp);
    _DTS_E("stencil_back_pass_depth_pass",mStencilBackPassDepthPass,eStencilOp);
    return ni::eTrue;
  }

  niEndClass(cDepthStencilStates);
};

//--------------------------------------------------------------------------------------------
//
//  cGraphics
//
//--------------------------------------------------------------------------------------------

tBool cGraphics::_CompileDefaultDepthStencilStates() {
#define BEGIN(STATE)                                            \
  {                                                             \
      const eCompiledStates cs = eCompiledStates_DS_##STATE;    \
      Ptr<cDepthStencilStates<eFalse> > s = niNew cDepthStencilStates<eFalse>();

#define END()                                               \
  astl::upsert(mmapDepthStencilStates,                 \
                    (tIntPtr)cs,                            \
                    niNew cDepthStencilStates<eTrue>(*s));   \
}

  BEGIN(NoDepthTest);
  s->SetDepthTest(eFalse);
  END();

  BEGIN(DepthTestAndWrite);
  s->SetDepthTest(eTrue);
  END();

  BEGIN(DepthTestOnly);
  s->SetDepthTest(eTrue);
  s->SetDepthTestWrite(eFalse);
  END();

  return eTrue;
}

///////////////////////////////////////////////
iDepthStencilStates* __stdcall cGraphics::CreateDepthStencilStates() {
  CHECKDRIVER(NULL);
  return niNew cDepthStencilStates<eFalse>();
}

///////////////////////////////////////////////
tIntPtr __stdcall cGraphics::CompileDepthStencilStates(iDepthStencilStates* apStates) {
  CHECKDRIVER(NULL);
  niCheckIsOK(apStates,NULL);

  const sDepthStencilStatesDesc& desc = *(sDepthStencilStatesDesc*)apStates->GetDescStructPtr();
  niLoopit(tDSMap::iterator,it,mmapDepthStencilStates) {
    if (*((sDepthStencilStatesDesc*)(it->second->GetDescStructPtr())) == desc)
      return it->first;
  }

  tIntPtr handle = NULL;
  if (niFlagIs(mptrDrv->GetGraphicsDriverImplFlags(),eGraphicsDriverImplFlags_CompileDepthStencilStates)) {
    handle = mptrDrv->CompileDepthStencilStates(apStates);
    if (!handle) {
      niError(_A("Driver can't compile the specified states."));
      return 0;
    }
    niAssert(handle >= eCompiledStates_Driver);
  }
  else {
    handle = mStatesHandleGen++;
  }

  Ptr<iDepthStencilStates> ds = niNew cDepthStencilStates<eTrue>(desc);
  astl::upsert(mmapDepthStencilStates,handle,ds);
  return handle;
}

///////////////////////////////////////////////
iDepthStencilStates* __stdcall cGraphics::GetCompiledDepthStencilStates(tIntPtr aHandle) const {
  if (!aHandle) return NULL;
  tDSMap::const_iterator it = mmapDepthStencilStates.find(aHandle);
  if (it == mmapDepthStencilStates.end()) return NULL;
  return it->second.ptr();
}
