// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Graphics.h"

//--------------------------------------------------------------------------------------------
//
//  Generic Vertex Array
//
//--------------------------------------------------------------------------------------------

//! System memory vertex array.
class cGenericVertexArray : public ImplRC<iVertexArray>
{
  niBeginClass(cGenericVertexArray);

 public:
  ///////////////////////////////////////////////
  cGenericVertexArray(tU32 ulNumVert, tU32 ulFVF) {
    ZeroMembers();
    mbLocked = eFalse;
    mFVF.Setup(ulFVF);
    mulNumVertices = ulNumVert;
    mulSize = mulNumVertices*mFVF.GetStride();
    mpMem = (tPtr)niMalloc(mulSize);
  }

  ///////////////////////////////////////////////
  ~cGenericVertexArray() {
    niSafeFree(mpMem);
  }

  ///////////////////////////////////////////////
  void ZeroMembers() {
    mpMem = NULL;
  }

  ///////////////////////////////////////////////
  tBool __stdcall IsOK() const {
    niClassIsOK(cGenericVertexArray);
    return mpMem != NULL;
  }

  ///////////////////////////////////////////////
  iHString* __stdcall GetDeviceResourceName() const {
    return NULL;
  }
  tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) { return eFalse; }
  tBool __stdcall ResetDeviceResource() { return eFalse; }

  ///////////////////////////////////////////////
  tFVF __stdcall GetFVF() const {
    return mFVF.GetFVF();
  }

  ///////////////////////////////////////////////
  tU32 __stdcall GetNumVertices() const {
    return mulNumVertices;
  }

  ///////////////////////////////////////////////
  eArrayUsage __stdcall GetUsage() const {
    return eArrayUsage_SystemMemory;
  }

  ///////////////////////////////////////////////
  tPtr __stdcall Lock(tU32 ulFirst, tU32 ulNum, eLock aLock) {
    if (mbLocked) {
      return NULL;
    }

    if (ulNum == 0)
      ulNum = mulNumVertices-ulFirst;

    const tU32 nFVFStride = mFVF.GetStride();
    if (ulFirst*nFVFStride+ulNum*nFVFStride > mulSize)
      return NULL;
    mbLocked = eTrue;
    return mpMem+(ulFirst*nFVFStride);
  }

  ///////////////////////////////////////////////
  tBool __stdcall Unlock() {
    if (!mbLocked) return eFalse;
    mbLocked = eFalse;
    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool __stdcall GetIsLocked() const {
    return mbLocked;
  }

  ///////////////////////////////////////////////
  virtual iDeviceResource* __stdcall Bind(iUnknown*) {
    return this;
  }

 public:
  cFVFDescription mFVF;
  tU32    mulNumVertices;
  tU32    mulSize;
  tBool   mbLocked;
  tPtr    mpMem;

  niEndClass(cGenericVertexArray);
};

iVertexArray* _CreateGenericVertexArray(tU32 anNumVertices, tFVF anFVF) {
  return niNew cGenericVertexArray(anNumVertices,anFVF);
}
const cFVFDescription& _GetGenericVertexArrayFVFDesc(iVertexArray* apVA) {
  niAssert(apVA->GetUsage() == eArrayUsage_SystemMemory);
  return ((cGenericVertexArray*)apVA)->mFVF;
}
tPtr _GetGenericVertexArrayMemPtr(iVertexArray* apVA) {
  niAssert(apVA->GetUsage() == eArrayUsage_SystemMemory);
  return ((cGenericVertexArray*)apVA)->mpMem;
}

//--------------------------------------------------------------------------------------------
//
//  cGraphics
//
//--------------------------------------------------------------------------------------------

///////////////////////////////////////////////
iVertexArray* __stdcall cGraphics::CreateVertexArray(tU32 anNumVertices, tFVF anFVF, eArrayUsage aUsage) {
  CHECKDRIVER(NULL);
  if (niFlagIs(mptrDrv->GetGraphicsDriverImplFlags(),eGraphicsDriverImplFlags_VertexArrayObject)) {
    return mptrDrv->CreateVertexArray(anNumVertices,anFVF,aUsage);
  }
  else {
    return _CreateGenericVertexArray(anNumVertices,anFVF);
  }
}
