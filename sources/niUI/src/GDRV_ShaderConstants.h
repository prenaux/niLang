#pragma once
#ifndef __GDRV_SHADERCONSTANTS_H_8E91DB95_A3C3_4E76_8987_A8B395792197__
#define __GDRV_SHADERCONSTANTS_H_8E91DB95_A3C3_4E76_8987_A8B395792197__

#include "API/niUI/IShader.h"

namespace ni {

//--------------------------------------------------------------------------------------------
//
//  Generic Shader Constants Buffer
//
//  Implemented inline so that we can use it in the shader compiler without
//  having a dependency on the module
//
//--------------------------------------------------------------------------------------------
class cShaderConstants :
    public cIUnknownImpl<iShaderConstants,eIUnknownImplFlags_DontInherit1,iSerializable>,
    public sShaderConstantsDesc
{
  niBeginClass(cShaderConstants);

 public:
  cShaderConstants(tU32 anMaxRegisters) {
    ZeroMembers();
    mnMaxRegisters = anMaxRegisters;
  }
  ~cShaderConstants() {
    Invalidate();
  }

  tBool __stdcall IsOK() const {
    niClassIsOK(cShaderConstants);
    return eTrue;
  }

  void __stdcall ZeroMembers() {
    mnMaxRegisters = 0;
  }

  void __stdcall Invalidate() {
    mvFloatRegisters.clear();
    mvIntRegisters.clear();
    mvConstants.clear();
  }

  ///////////////////////////////////////////////
  virtual iShaderConstants* __stdcall Clone() const {
    cShaderConstants* pNew = niNew cShaderConstants(mnMaxRegisters);
    pNew->mnMaxRegisters = mnMaxRegisters;
    pNew->mvFloatRegisters = mvFloatRegisters;
    pNew->mvIntRegisters = mvIntRegisters;
    pNew->mvConstants = mvConstants;
    pNew->mmapConstants = mmapConstants;
    return pNew;
  }

  ///////////////////////////////////////////////
  virtual tU32 __stdcall GetMaxNumRegisters() const {
    return mnMaxRegisters;
  }

  ///////////////////////////////////////////////
  virtual tU32 __stdcall AddConstant(iHString* ahspName, eShaderRegisterType aType, tU32 anSize) {
    niCheckSilent(
        aType == eShaderRegisterType_Sampler ||
        aType == eShaderRegisterType_ConstBool ||
        aType == eShaderRegisterType_ConstInt ||
        aType == eShaderRegisterType_ConstFloat,
        eInvalidHandle);

    tU32 regIndex = 0;
    if (aType == eShaderRegisterType_Sampler) {
      // nothing more to do for sampler registers... only the HwIndex should be set for it
    }
    else {
      niCheckSilent(mvFloatRegisters.size()+mvIntRegisters.size()+anSize <= mnMaxRegisters,
                    eInvalidHandle);
      if (aType == eShaderRegisterType_ConstInt ||
          aType == eShaderRegisterType_ConstBool) {
        regIndex = (tU32)mvIntRegisters.size();
        mvIntRegisters.resize(mvIntRegisters.size()+anSize);
      }
      else {
        regIndex = (tU32)mvFloatRegisters.size();
        mvFloatRegisters.resize(mvFloatRegisters.size()+anSize);
      }
    }
    mvConstants.push_back(sConstant(ahspName,aType,anSize,regIndex));
    if (!HStringIsEmpty(ahspName)) {
      astl::upsert(mmapConstants,ahspName,(tU32)(mvConstants.size()-1));
    }
    return (tU32)(mvConstants.size()-1);
  }

  ///////////////////////////////////////////////
  virtual tU32 __stdcall GetNumConstants() const {
    return (tU32)mvConstants.size();
  }

  inline tU32 _GetConstantIndex(iHString* ahspName) const {
    tConstMap::const_iterator it = mmapConstants.find(ahspName);
    return (it == mmapConstants.end()) ? eInvalidHandle : it->second;
  }

  ///////////////////////////////////////////////
  virtual tU32 __stdcall GetConstantIndex(iHString* ahspName) const {
    return _GetConstantIndex(ahspName);
  }
  virtual iHString* __stdcall GetConstantName(tU32 anConstIndex) const {
    niCheckSilent(anConstIndex < mvConstants.size(),0);
    return mvConstants[anConstIndex].hspName;
  }
  virtual tU32 __stdcall GetConstantSize(tU32 anConstIndex) const {
    niCheckSilent(anConstIndex < mvConstants.size(),0);
    return mvConstants[anConstIndex].nSize;
  }
  virtual eShaderRegisterType __stdcall GetConstantType(tU32 anConstIndex) const {
    niCheckSilent(anConstIndex < mvConstants.size(),eShaderRegisterType_Last);
    return (eShaderRegisterType)mvConstants[anConstIndex].Type;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SetHwIndex(tU32 anConstIndex, tU32 anRegisterIndex) {
    niCheckSilent(anConstIndex < mvConstants.size(),eFalse);
    mvConstants[anConstIndex].nHwIndex = anRegisterIndex;
    return eTrue;
  }
  virtual tU32 __stdcall GetHwIndex(tU32 anConstIndex) const {
    niCheckSilent(anConstIndex < mvConstants.size(),eInvalidHandle);
    return mvConstants[anConstIndex].nHwIndex;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SetFloatArray(tU32 anConstIndex, const tVec4fCVec* apV, tU32 anSize = 1) {
    niCheckSilent(tVec4fCVec::IsSameType(apV),eFalse);
    return SetFloatPointer(anConstIndex,apV->_Data(),anSize);
  }
  ///////////////////////////////////////////////
  virtual tBool __stdcall SetFloatPointer(tU32 anConstIndex, const sVec4f* apV, tU32 anSize = 1) {
    niCheckSilent(anConstIndex < mvConstants.size(),eFalse);
    sConstant& c = mvConstants[anConstIndex];
    niCheckSilent(c.Type == eShaderRegisterType_ConstFloat,eFalse);
    sVec4f* pDest = (sVec4f*)&mvFloatRegisters[c.nDataIndex];
    for (tU32 i = 0; i < c.nSize; ++i) {
      pDest[i] = apV[i];
    }
    return eTrue;
  }
  ///////////////////////////////////////////////
  virtual tBool __stdcall SetFloat(tU32 anConstIndex, const sVec4f& aV) {
    niCheckSilent(anConstIndex < mvConstants.size(),eFalse);
    sConstant& c = mvConstants[anConstIndex];
    niCheckSilent(c.Type == eShaderRegisterType_ConstFloat,eFalse);
    sVec4f* pDest = (sVec4f*)&mvFloatRegisters[c.nDataIndex];
    *pDest = aV;
    return eTrue;
  }
  ///////////////////////////////////////////////
  virtual tBool __stdcall SetFloatMatrixArray(tU32 anConstIndex, const tMatrixfCVec* apV, tU32 anSize = 1) {
    niCheckSilent(tMatrixfCVec::IsSameType(apV),eFalse);
    niCheckSilent(anConstIndex < mvConstants.size(),eFalse);
    sConstant& c = mvConstants[anConstIndex];
    niCheckSilent(c.Type == eShaderRegisterType_ConstFloat,eFalse);
    const sVec4f* pSrc = (const sVec4f*)apV->_Data();
    sVec4f* pDest = (sVec4f*)&mvFloatRegisters[c.nDataIndex];
    niLoop(i,ni::Min(anSize,c.nSize)) {
      *pDest++ = *pSrc++;
    }
    return eTrue;
  }
  ///////////////////////////////////////////////
  virtual tBool __stdcall SetFloatMatrix(tU32 anConstIndex, const sMatrixf& aV) {
    niCheckSilent(anConstIndex < mvConstants.size(),eFalse);
    sConstant& c = mvConstants[anConstIndex];
    niCheckSilent(c.Type == eShaderRegisterType_ConstFloat,eFalse);
    const sVec4f* pSrc = (sVec4f*)aV.ptr();
    sVec4f* pDest = &mvFloatRegisters[c.nDataIndex];
    niAssert(c.nSize <= 4);
    niLoop(i,ni::Min(4,c.nSize)) {
      *pDest++ = *pSrc++;
    }
    return eTrue;
  }
  ///////////////////////////////////////////////
  virtual tBool __stdcall SetIntArray(tU32 anConstIndex, const tVec4iCVec* apV, tU32 anSize = 1) {
    niCheckSilent(tVec4iCVec::IsSameType(apV),eFalse);
    return SetIntPointer(anConstIndex,apV->_Data(),anSize);
  }
  ///////////////////////////////////////////////
  virtual tBool __stdcall SetIntPointer(tU32 anConstIndex, const sVec4i* apV, tU32 anSize = 1) {
    niCheckSilent(anConstIndex < mvConstants.size(),eFalse);
    sConstant& c = mvConstants[anConstIndex];
    niCheckSilent(c.Type != eShaderRegisterType_ConstFloat,eFalse);
    niCheckSilent(anSize <= c.nSize,eFalse);
    sVec4i* pDest = (sVec4i*)&mvIntRegisters[c.nDataIndex];
    for (tU32 i = 0; i < c.nSize; ++i) {
      pDest[i] = apV[i];
    }
    return eTrue;
  }
  ///////////////////////////////////////////////
  virtual tBool __stdcall SetInt(tU32 anConstIndex, const sVec4i& aV) {
    niCheckSilent(anConstIndex < mvConstants.size(),eFalse);
    sConstant& c = mvConstants[anConstIndex];
    niCheckSilent(c.Type != eShaderRegisterType_ConstFloat,eFalse);
    sVec4i* pDest = (sVec4i*)&mvIntRegisters[c.nDataIndex];
    *pDest = aV;
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual sVec4f __stdcall GetFloat(tU32 anConstIndex, tU32 anOffset) const {
    niCheckSilent(anConstIndex < mvConstants.size(),sVec4f::Zero());
    const sConstant& c = mvConstants[anConstIndex];
    niCheckSilent(c.Type == eShaderRegisterType_ConstFloat,sVec4f::Zero());
    niCheckSilent(anOffset < c.nSize,sVec4f::Zero());
    sVec4f* pSrc = (sVec4f*)&mvFloatRegisters[c.nDataIndex];
    return pSrc[anOffset];
  }
  ///////////////////////////////////////////////
  virtual sMatrixf __stdcall GetFloatMatrix(tU32 anConstIndex) const {
    niCheckSilent(anConstIndex < mvConstants.size(),sMatrixf::Identity());
    const sConstant& c = mvConstants[anConstIndex];
    niCheckSilent(c.Type == eShaderRegisterType_ConstFloat,sMatrixf::Identity());
    niCheckSilent(4 <= c.nSize,sMatrixf::Zero());
    sVec4f* pSrc = (sVec4f*)&mvFloatRegisters[c.nDataIndex];
    return *((sMatrixf*)pSrc);
  }
  ///////////////////////////////////////////////
  virtual sVec4f* __stdcall GetFloatPointer(tU32 anConstIndex) const {
    niCheckSilent(anConstIndex < mvConstants.size(),NULL);
    const sConstant& c = mvConstants[anConstIndex];
    niCheckSilent(c.Type == eShaderRegisterType_ConstFloat,NULL);
    sVec4f* pSrc = (sVec4f*)&mvFloatRegisters[c.nDataIndex];
    return pSrc;
  }
  ///////////////////////////////////////////////
  virtual sVec4i __stdcall GetInt(tU32 anConstIndex, tU32 anOffset) const {
    niCheckSilent(anConstIndex < mvConstants.size(),sVec4i::Zero());
    const sConstant& c = mvConstants[anConstIndex];
    niCheckSilent(c.Type != eShaderRegisterType_ConstFloat,sVec4i::Zero());
    niCheckSilent(anOffset < c.nSize,sVec4i::Zero());
    sVec4i* pSrc = (sVec4i*)&mvIntRegisters[c.nDataIndex];
    return pSrc[anOffset];
  }
  ///////////////////////////////////////////////
  virtual sVec4i* __stdcall GetIntPointer(tU32 anConstIndex) const {
    niCheckSilent(anConstIndex < mvConstants.size(),NULL);
    const sConstant& c = mvConstants[anConstIndex];
    niCheckSilent(c.Type != eShaderRegisterType_ConstFloat,NULL);
    sVec4i* pSrc = (sVec4i*)&mvIntRegisters[c.nDataIndex];
    return pSrc;
  }

  ///////////////////////////////////////////////
  tPtr __stdcall GetDescStructPtr() const {
    return (tPtr)niStaticCast(const sShaderConstantsDesc*,this);
  }

  ///////////////////////////////////////////////
  void __stdcall SetConstantMetadata(tU32 anConstIndex, iHString* ahspMetadata) {
    niCheckSilent(anConstIndex < mvConstants.size(),);
    mvConstants[anConstIndex].hspMetadata = ahspMetadata;
  }
  iHString* __stdcall GetConstantMetadata(tU32 anConstIndex) const {
    niCheckSilent(anConstIndex < mvConstants.size(),0);
    return mvConstants[anConstIndex].hspMetadata;
  }

  ///////////////////////////////////////////////
  const ni::achar* __stdcall GetSerializeObjectTypeID() const {
    return NULL;
  }
  tSize __stdcall Serialize(iFile* apFile, eSerializeMode aMode)
  {
    switch (aMode) {
      case eSerializeMode_Read:
      case eSerializeMode_ReadRaw:
        {
          tI64 nPos = apFile->Tell();
          tU32 nVer = apFile->ReadLE32();
          if (nVer != niMakeVersion(1,0,1)) {
            niError(_A("Invalid version."));
            return eInvalidHandle;
          }
          mnMaxRegisters = apFile->ReadLE32();
          {
            const tU32 numFloatRegs = apFile->ReadLE32();
            mvFloatRegisters.resize(numFloatRegs);
            if (!mvFloatRegisters.empty()) {
              apFile->ReadF32Array(mvFloatRegisters[0].ptr(),numFloatRegs*4);
            }
          }
          {
            const tU32 numIntRegs = apFile->ReadLE32();
            mvIntRegisters.resize(numIntRegs);
            if (!mvIntRegisters.empty()) {
              apFile->ReadLE32Array((tU32*)mvIntRegisters[0].ptr(),numIntRegs*4);
            }
          }
          {
            const tU32 numConsts = apFile->ReadLE32();
            mmapConstants.clear();
            //                 mvConstants.clear();
            mvConstants.resize(numConsts);
            niLoop(i,numConsts) {
              sConstant& c = mvConstants[i];
              c.hspName = _H(apFile->ReadBitsString());
              c.Type = (eShaderRegisterType)apFile->Read8();
              c.nSize = apFile->ReadLE32();
              c.nDataIndex = apFile->ReadLE32();
              c.nHwIndex = apFile->ReadLE32();
              if (!ni::HStringIsEmpty(c.hspName)) {
                astl::upsert(mmapConstants,c.hspName,i);
              }
            }
          }
          return (tSize)(apFile->Tell()-nPos);
        }
      case eSerializeMode_Write:
      case eSerializeMode_WriteRaw:
        {
          tU32 nSize = 0;
          nSize += apFile->WriteLE32(niMakeVersion(1,0,1));
          nSize += apFile->WriteLE32(mnMaxRegisters);
          nSize += apFile->WriteLE32((tU32)mvFloatRegisters.size());
          if (!mvFloatRegisters.empty()) {
            nSize += apFile->WriteF32Array(mvFloatRegisters[0].ptr(),(tU32)mvFloatRegisters.size()*4);
          }
          nSize += apFile->WriteLE32((tU32)mvIntRegisters.size());
          if (!mvIntRegisters.empty()) {
            nSize += apFile->WriteLE32Array((tU32*)mvIntRegisters[0].ptr(),(tU32)mvIntRegisters.size()*4);
          }
          nSize += apFile->WriteLE32((tU32)mvConstants.size());
          niLoop(i,mvConstants.size()) {
            const sConstant& c = mvConstants[i];
            nSize += apFile->WriteBitsString(niHStr(c.hspName));
            niAssert(c.Type <= 0xFF);
            nSize += apFile->Write8((tU8)c.Type);
            nSize += apFile->WriteLE32(c.nSize);
            nSize += apFile->WriteLE32(c.nDataIndex);
            nSize += apFile->WriteLE32(c.nHwIndex);
          }
          return nSize;
        }
    }
    niAssertMsg(0,_A("Unreachable code."));
    return eInvalidHandle;
  }
  niEndClass(cShaderConstants);
};

}
#endif // __GDRV_SHADERCONSTANTS_H_8E91DB95_A3C3_4E76_8987_A8B395792197__
