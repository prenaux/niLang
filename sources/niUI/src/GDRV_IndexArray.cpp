// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Graphics.h"

//--------------------------------------------------------------------------------------------
//
//  Generic Index Array
//
//--------------------------------------------------------------------------------------------
class cGenericIndexArray : public ImplRC<iIndexArray>
{
  niBeginClass(cGenericIndexArray);

 public:
  ///////////////////////////////////////////////
  cGenericIndexArray(eGraphicsPrimitiveType aPrimType, tU32 ulNumIndex, tU32 ulMaxVertexIndex) {
    ZeroMembers();
    mPrimType = aPrimType;
    mbLocked = eFalse;
    mulNumIndices = ulNumIndex;
    mulMaxVertexIndex = ulMaxVertexIndex;
    mpMem = (tIndex*)niMalloc(sizeof(tIndex)*mulNumIndices);
  }

  ///////////////////////////////////////////////
  ~cGenericIndexArray() {
    niSafeFree(mpMem);
  }

  ///////////////////////////////////////////////
  void ZeroMembers() {
    mpMem = NULL;
  }

  ///////////////////////////////////////////////
  tBool __stdcall IsOK() const {
    niClassIsOK(cGenericIndexArray);
    return (mpMem != NULL);
  }

  ///////////////////////////////////////////////
  iHString* __stdcall GetDeviceResourceName() const {
    return NULL;
  }
  tBool __stdcall HasDeviceResourceBeenReset(tBool abClearFlag) { return eFalse; }
  tBool __stdcall ResetDeviceResource() { return eFalse; }

  ///////////////////////////////////////////////
  eGraphicsPrimitiveType __stdcall GetPrimitiveType() const {
    return mPrimType;
  }

  ///////////////////////////////////////////////
  tU32 __stdcall GetNumIndices() const {
    return mulNumIndices;
  }

  ///////////////////////////////////////////////
  tU32 __stdcall GetMaxVertexIndex() const {
    return mulMaxVertexIndex;
  }

  ///////////////////////////////////////////////
  eArrayUsage __stdcall GetUsage() const {
    return eArrayUsage_SystemMemory;
  }

  ///////////////////////////////////////////////
  tPtr __stdcall Lock(tU32 ulFirst, tU32 ulNum, eLock aLock) {
    niAssert(mbLocked != eTrue);

    if (ulNum == 0) ulNum = mulNumIndices-ulFirst;

    mbLocked = eTrue;
    return tPtr(mpMem+ulFirst);
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
  iDeviceResource* __stdcall Bind(iUnknown*) {
    return this;
  }

 public:
  eGraphicsPrimitiveType  mPrimType;
  tU32    mulNumIndices;
  tU32    mulMaxVertexIndex;
  tBool   mbLocked;
  tIndex* mpMem;
  niEndClass(cGenericIndexArray);
};

iIndexArray* _CreateGenericIndexArray(eGraphicsPrimitiveType aPrimitiveType, tU32 anNumIndex, tU32 anMaxVertexIndex) {
  return niNew cGenericIndexArray(aPrimitiveType,anNumIndex,anMaxVertexIndex);
}
tPtr _GetGenericIndexArrayMemPtr(iIndexArray* apIA) {
  niAssert(apIA->GetUsage() == eArrayUsage_SystemMemory);
  return (tPtr)((cGenericIndexArray*)apIA)->mpMem;
}

//--------------------------------------------------------------------------------------------
//
//  cGraphics
//
//--------------------------------------------------------------------------------------------

///////////////////////////////////////////////
iIndexArray* __stdcall cGraphics::CreateIndexArray(eGraphicsPrimitiveType aPrimitiveType, tU32 anNumIndex, tU32 anMaxVertexIndex, eArrayUsage aUsage) {
  CHECKDRIVER(NULL);
  if (niFlagIs(mptrDrv->GetGraphicsDriverImplFlags(),eGraphicsDriverImplFlags_IndexArrayObject)) {
    return mptrDrv->CreateIndexArray(aPrimitiveType,anNumIndex,anMaxVertexIndex,aUsage);
  }
  else {
    return niNew cGenericIndexArray(aPrimitiveType,anNumIndex,anMaxVertexIndex);
  }
}
