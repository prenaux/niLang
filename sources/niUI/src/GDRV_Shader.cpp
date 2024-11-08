// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Graphics.h"
#include "GDRV_ShaderConstants.h"


///////////////////////////////////////////////
tU32 __stdcall cGraphics::GetNumShaderProfile(eShaderUnit aUnit) const {
  CHECKDRIVER(0);
  return mptrDrv->GetNumShaderProfile(aUnit);
}

///////////////////////////////////////////////
iHString* __stdcall cGraphics::GetShaderProfile(eShaderUnit aUnit, tU32 anIndex) const {
  CHECKDRIVER(NULL);
  return mptrDrv->GetShaderProfile(aUnit,anIndex);
}

///////////////////////////////////////////////
iShaderConstants* __stdcall cGraphics::CreateShaderConstants(tU32 anMaxRegisters) const {
  return niNew cShaderConstants(anMaxRegisters);
}

///////////////////////////////////////////////
tBool __stdcall cGraphics::SerializeShaderConstants(iShaderConstants* apConsts, iDataTable* apDT, tSerializeFlags aFlags)
{
  niCheckIsOK(apConsts,eFalse);
  niCheckIsOK(apDT,eFalse);
  if (aFlags&eSerializeFlags_Read) {
    iDataTable* pVS = apDT->GetChild("VS");
    if (pVS) {
      SerializeShaderConstants(apConsts,pVS,aFlags);
      iDataTable* pPS = apDT->GetChild("PS");
      SerializeShaderConstants(apConsts,pPS,aFlags);
      return eTrue;
    }
    if (aFlags&eSerializeFlags_TypeInfoMetadata) {
      niLoop(i,apDT->GetNumProperties()) {
        cString strName = apDT->GetPropertyName(i);
        if (strName.empty())
          continue;

        tU32 nConstIndex = apConsts->GetConstantIndex(_H(strName));
        if (nConstIndex == eInvalidHandle)
          continue;

        switch (apConsts->GetConstantType(nConstIndex)) {
          case eShaderRegisterType_ConstFloat: {
            if (apConsts->GetConstantSize(nConstIndex) == 4) {
              apConsts->SetFloatMatrix(
                  nConstIndex,apDT->GetMatrixFromIndex(i));
            }
            else {
              apConsts->SetFloat(
                  nConstIndex,apDT->GetVec4FromIndex(i));
            }
            apConsts->SetConstantMetadata(
                nConstIndex,apDT->GetMetadataFromIndex(i));
            break;
          }
          default:
            return eFalse;
        }
      }
    }
    else {
      niLoop(i,apDT->GetNumChildren()) {
        Ptr<iDataTable> cdt = apDT->GetChildFromIndex(i);
        if (StrCmp(cdt->GetName(),"Uniform") != 0)
          continue;

        cString strName = apDT->GetString("name");
        if (strName.empty())
          continue;

        tU32 valueProperty = apDT->GetPropertyIndex("value");
        if (valueProperty == eInvalidHandle)
          return eFalse;

        tHStringPtr hspName = _H(strName);
        tU32 nConstIndex = apConsts->GetConstantIndex(hspName);
        if (nConstIndex == eInvalidHandle) {
          cString strType = apDT->GetString("type");
          if (strType.empty())
            return eFalse;
          if (strType.IEq("mat4")) {
            apConsts->AddConstant(hspName,eShaderRegisterType_ConstFloat,4);
          }
          else if (strType.IEq("vec4")) {
            apConsts->AddConstant(hspName,eShaderRegisterType_ConstFloat,1);
          }
          else if (strType.IEq("vec3")) {
            apConsts->AddConstant(hspName,eShaderRegisterType_ConstFloat,1);
          }
          else if (strType.IEq("vec2")) {
            apConsts->AddConstant(hspName,eShaderRegisterType_ConstFloat,1);
          }
          else if (strType.IEq("float")) {
            apConsts->AddConstant(hspName,eShaderRegisterType_ConstFloat,1);
          }
        }

        switch (apConsts->GetConstantType(nConstIndex)) {
          case eShaderRegisterType_ConstFloat: {
            if (apConsts->GetConstantSize(nConstIndex) == 4) {
              apConsts->SetFloatMatrix(
                  nConstIndex,apDT->GetMatrixFromIndex(valueProperty));
            }
            else if (apConsts->GetConstantSize(nConstIndex) == 1) {
              apConsts->SetFloat(
                  nConstIndex,apDT->GetVec4FromIndex(valueProperty));
            }
            break;
          }
          default:
            return eFalse;
        }
      }
    }
  }
  else if (aFlags&eSerializeFlags_Write) {
    niLoop(i,apConsts->GetNumConstants()) {
      tHStringPtr hspName = apConsts->GetConstantName(i);
      switch (apConsts->GetConstantType(i)) {
        case eShaderRegisterType_ConstFloat: {
          if (aFlags&eSerializeFlags_TypeInfoMetadata) {
            if (apConsts->GetConstantSize(i) == 4) {
              apDT->SetMatrix(
                  niHStr(hspName),
                  apConsts->GetFloatMatrix(i));
            }
            else if (apConsts->GetConstantSize(i) == 1) {
              apDT->SetVec4(
                  niHStr(hspName),
                  apConsts->GetFloat(i,0));
            }
            apDT->SetMetadata(
                niHStr(hspName),apConsts->GetConstantMetadata(i));
          }
          else {
            Ptr<iDataTable> udt = ni::CreateDataTable("Uniform");
            udt->SetString("name",niHStr(hspName));
            if (apConsts->GetConstantSize(i) == 4) {
              udt->SetString("type","mat4");
              udt->SetMatrix("value",apConsts->GetFloatMatrix(i));
            }
            else if (apConsts->GetConstantSize(i) == 1) {
              udt->SetString("type","vec4");
              udt->SetVec4("value",apConsts->GetFloat(i,0));
            }
          }
          break;
        }
        default:
          return eFalse;
      }
    }
  }
  return eFalse;
}

///////////////////////////////////////////////
iDeviceResourceManager* __stdcall cGraphics::GetShaderDeviceResourceManager() const {
  return mptrDRMShaders;
}

///////////////////////////////////////////////
tU32 __stdcall cGraphics::GetNumShaders() const {
  CHECKDRIVER(0);
  return mptrDRMShaders->GetSize();
}

///////////////////////////////////////////////
iShader* __stdcall cGraphics::GetShaderFromName(iHString* ahspName) const {
  CHECKDRIVER(NULL);
  return ni::QueryInterface<iShader>(mptrDRMShaders->GetFromName(ahspName));
}

///////////////////////////////////////////////
iShader* __stdcall cGraphics::GetShaderFromIndex(tU32 anIndex) const {
  CHECKDRIVER(NULL);
  return ni::QueryInterface<iShader>(mptrDRMShaders->GetFromIndex(anIndex));
}

///////////////////////////////////////////////
iShader* __stdcall cGraphics::CreateShader(iHString* ahspName, iFile* apFile) {
  CHECKDRIVER(NULL);
  return mptrDrv->CreateShader(ahspName,apFile);
}

///////////////////////////////////////////////
iShader* __stdcall cGraphics::CreateShaderFromRes(iHString* ahspRes) {
  Ptr<iShader> ptrShader = GetShaderFromName(ahspRes);
  if (ptrShader.IsOK()) {
#ifdef CAN_RECREATE_SHADER_WHEN_CHANGED
    if (mbRecreateShaderWhenChanged) {
      tResourceTimeMap::iterator it = mShaderTimeMap.find(ahspRes);
      if (it != mShaderTimeMap.end()) {
        Ptr<iTime> ptrPrevTime = it->second;
        Ptr<iFile> ptrFP = ni::GetLang()->URLOpen(niHStr(ahspRes));
        if (ptrFP.IsOK()) {
          Ptr<iTime> ptrFileTime = ni::GetLang()->GetCurrentTime()->Clone();
          if (ptrFP->GetTime(eFileTime_LastWrite,ptrFileTime)) {
            if (ptrPrevTime->Compare(ptrFileTime) != 0) {
              niLog(Info,niFmt("Already loaded shader '%s' but it has changed, it will be reloaded.", ahspRes));
              Ptr<iDeviceResourceManager> texResMan = this->GetShaderDeviceResourceManager();
              texResMan->Unregister(ptrShader);
              ptrShader = NULL;
            }
          }
        }
      }
    }
    if (ptrShader.IsOK())
#endif
    {
      return ptrShader;
    }
  }

  Ptr<iFile> ptrFP = ni::GetLang()->URLOpen(niHStr(ahspRes));
  if (!ptrFP.IsOK()) {
    niError(niFmt("Can't open shader file '%s'.", ahspRes));
    return NULL;
  }

  ptrShader = this->CreateShader(ahspRes, ptrFP);
#ifdef CAN_RECREATE_SHADER_WHEN_CHANGED
  if (mbRecreateShaderWhenChanged) {
    Ptr<iTime> ptrFileTime = ni::GetLang()->GetCurrentTime()->Clone();
    if (ptrFP->GetTime(eFileTime_LastWrite,ptrFileTime)) {
      astl::upsert(mShaderTimeMap,ahspRes,ptrFileTime);
    }
  }
#endif
  return ptrShader.GetRawAndSetNull();
}

///////////////////////////////////////////////
void __stdcall cGraphics::SetRecreateShaderWhenChanged(tBool abEnabled) {
#ifdef CAN_RECREATE_SHADER_WHEN_CHANGED
  mbRecreateShaderWhenChanged = abEnabled;
#else
  niUnused(abEnabled);
#endif
}
tBool __stdcall cGraphics::GetRecreateShaderWhenChanged() const {
#ifdef CAN_RECREATE_SHADER_WHEN_CHANGED
  return mbRecreateShaderWhenChanged;
#else
  return eFalse;
#endif
}
