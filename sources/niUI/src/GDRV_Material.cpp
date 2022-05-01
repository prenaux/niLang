// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Graphics.h"
#include "API/niUI_ModuleDef.h"
#include <niLang/Utils/Userdata.h>

#define DEFAULT_SS eCompiledStates_SS_SmoothRepeat

_HDecl(Base);

static const sMaterialDesc& _GetDefaultMaterial() {
  static sMaterialDesc* _pDefaultMaterial = NULL;
  if (!_pDefaultMaterial) {
    _pDefaultMaterial = new sMaterialDesc();
    memset(_pDefaultMaterial,0,sizeof(sMaterialDesc));
    niLoop(i,eMaterialChannel_Last) {
      sMaterialChannel& ch = _pDefaultMaterial->mChannels[i];
      ch.mhSS = DEFAULT_SS;
    }
    _pDefaultMaterial->mChannels[eMaterialChannel_Base].mColor = sColor4f::One();
    _pDefaultMaterial->mChannels[eMaterialChannel_Emissive].mColor = sColor4f::One();
    _pDefaultMaterial->mChannels[eMaterialChannel_Ambient].mColor = sColor4f::One();
    _pDefaultMaterial->mChannels[eMaterialChannel_Bump].mColor = Vec4f(0.5f,1,1,0.5f);
    _pDefaultMaterial->mChannels[eMaterialChannel_Specular].mColor = Vec4f(0.5f,0.5f,0.5f,0.5f);
    _pDefaultMaterial->mChannels[eMaterialChannel_Refraction].mColor = Vec4f(0.5f,1,1,0.5f);
    _pDefaultMaterial->mChannels[eMaterialChannel_Environment].mColor = Vec4f(1,1,1,0.3f);
  }
  return *_pDefaultMaterial;
}

//--------------------------------------------------------------------------------------------
//
// cMaterial implementation.
//
//--------------------------------------------------------------------------------------------

class cMaterial :
    public ni::cUserdata<ni::cIUnknownImpl<ni::iMaterial> >,
    public sMaterialDesc
{
  niBeginClass(cMaterial);

 public:
  ///////////////////////////////////////////////
  cMaterial(ni::iGraphics* apGraphics)
  {
    mptrGraphics = apGraphics;
    _MaterialDesc() = _GetDefaultMaterial();
  }

  ///////////////////////////////////////////////
  ~cMaterial() {
    Invalidate();
  }

  ///////////////////////////////////////////////
  ni::tBool __stdcall IsOK() const
  {
    niClassIsOK(cMaterial);
    return ni::eTrue;
  }

  ///////////////////////////////////////////////
  void __stdcall Invalidate() {
    if (!mptrGraphics.IsOK()) return;
    mptrGraphics = NULL;
  }

  ///////////////////////////////////////////////
  sMaterialDesc& _MaterialDesc() {
    return *niStaticCast(sMaterialDesc*,this);
  }
  const sMaterialDesc& _MaterialDesc() const {
    return *niStaticCast(const sMaterialDesc*,this);
  }
  tPtr __stdcall GetDescStructPtr() const {
    return (tPtr)&_MaterialDesc();
  }

  ///////////////////////////////////////////////
  iGraphics* __stdcall GetGraphics() const {
    return mptrGraphics;
  }

  ///////////////////////////////////////////////
  void __stdcall SetName(iHString* ahspName) {
    mhspName = ahspName;
  }
  iHString* __stdcall GetName() const {
    return mhspName;
  }

  ///////////////////////////////////////////////
  void __stdcall SetClass(iHString* ahspClass) {
    mhspClass = ahspClass;
  }
  iHString* __stdcall GetClass() const {
    return mhspClass;
  }

  ///////////////////////////////////////////////
  tU32 __stdcall GetWidth() const {
    return mChannels[eMaterialChannel_Base].mTexture.IsOK() ? mChannels[eMaterialChannel_Base].mTexture->GetWidth():16;
  }
  tU32 __stdcall GetHeight() const {
    return mChannels[eMaterialChannel_Base].mTexture.IsOK() ? mChannels[eMaterialChannel_Base].mTexture->GetHeight():16;
  }
  tU32 __stdcall GetDepth() const {
    return mChannels[eMaterialChannel_Base].mTexture.IsOK() ? mChannels[eMaterialChannel_Base].mTexture->GetDepth():16;
  }

  ///////////////////////////////////////////////
  void __stdcall SetFlags(tMaterialFlags aFlags) {
    mFlags = aFlags;
  }
  tMaterialFlags __stdcall GetFlags() const {
    return mFlags;
  }

  ///////////////////////////////////////////////
  tBool __stdcall SetBlendMode(eBlendMode aBlendMode) {
    niAssert(aBlendMode <= 8);
    mBlendMode = aBlendMode;
    return eTrue;
  }
  eBlendMode __stdcall GetBlendMode() const {
    return mBlendMode;
  }

  tBool __stdcall HasChannelTexture(ni::eMaterialChannel aChannel) const;
  tBool __stdcall SetChannelTexture(ni::eMaterialChannel aChannel, iTexture *apTexture);
  iTexture * __stdcall GetChannelTexture(ni::eMaterialChannel aChannel) const;
  tBool __stdcall SetChannelColor(ni::eMaterialChannel aChannel, const sColor4f &aColor);
  const sColor4f & __stdcall GetChannelColor(ni::eMaterialChannel aChannel) const;
  tBool __stdcall SetChannelSamplerStates(eMaterialChannel aChannel, tIntPtr aHandle);
  tIntPtr __stdcall GetChannelSamplerStates(eMaterialChannel aChannel) const;

  tBool __stdcall CopyChannel(eMaterialChannel aDestChannel, const iMaterial* apSource, eMaterialChannel aSrcChannel) {
    niCheckIsOK(apSource,eFalse);
    niCheck(aDestChannel < eMaterialChannel_Last,eFalse);
    niCheck(aSrcChannel < eMaterialChannel_Last,eFalse);

    const sMaterialDesc* pSrcMat = (const sMaterialDesc*)apSource->GetDescStructPtr();
    this->mChannels[aDestChannel] = pSrcMat->mChannels[aSrcChannel];
    return eTrue;
  }

  tBool __stdcall Copy(const iMaterial* apMat);
  iMaterial* __stdcall Clone() const;

  tBool __stdcall SetRasterizerStates(tIntPtr aHandle) {
    mhRS = aHandle;
    return eTrue;
  }
  tIntPtr __stdcall GetRasterizerStates() const {
    return mhRS;
  }
  tBool __stdcall SetDepthStencilStates(tIntPtr aHandle) {
    mhDS = aHandle;
    return eTrue;
  }
  tIntPtr __stdcall GetDepthStencilStates() const {
    return mhDS;
  }

  void __stdcall SetPolygonOffset(const ni::sVec2f& avOffset) {
    mvPolygonOffset = avOffset;
  }

  sVec2f __stdcall GetPolygonOffset() const {
    return mvPolygonOffset;
  }

  tBool __stdcall GetHasShader() const;
  tBool __stdcall SetShader(eShaderUnit aUnit, iShader* apProgram);
  iShader* __stdcall GetShader(eShaderUnit aUnit) const;
  tBool __stdcall SetShaderConstants(iShaderConstants* apBuffer);
  iShaderConstants* __stdcall GetShaderConstants() const;

  tBool SerializeWrite(iDataTableWriteStack* apDT, iHString* ahspBasePath);
  tBool SerializeRead(iDataTableReadStack* apDT, iHString* ahspBasePath);
  tBool __stdcall Serialize(eSerializeMode aMode, iDataTable* apDT, iHString* ahspBasePath);

  virtual void __stdcall SetExpressionObject(eMaterialExpression aExpr, iHString* ahspExpr, iExpression* apExpr) {
    niCheckSilent(aExpr < eMaterialExpression_Last,;);
    mhspExpressions[aExpr] = ahspExpr;
    mptrExpressions[aExpr] = niGetIfOK(apExpr);
  }

  virtual iExpression* __stdcall GetExpressionObject(eMaterialExpression aExpr) {
    niCheckSilent(aExpr < eMaterialExpression_Last,NULL);
    return mptrExpressions[aExpr];
  }

  virtual void __stdcall SetExpression(eMaterialExpression aExpr, iHString* ahspExpr) {
    niCheckSilent(aExpr < eMaterialExpression_Last,;);
    if (ahspExpr == mhspExpressions[aExpr])
      return; // same expression, just move on...
    mhspExpressions[aExpr] = ahspExpr;
    if (HStringIsNotEmpty(ahspExpr)) {
      mptrExpressions[aExpr] = ni::GetLang()->GetExpressionContext()->CreateExpression(niHStr(ahspExpr));
    }
    else {
      mptrExpressions[aExpr] = NULL;
    }
  }

  virtual iHString* __stdcall GetExpression(eMaterialExpression aExpr) const {
    niCheckSilent(aExpr < eMaterialExpression_Last,NULL);
    return mhspExpressions[aExpr];
  }

  virtual sMatrixf __stdcall GetExpressionValueMatrix(eMaterialExpression aExpr) const {
    niCheckSilent(aExpr < eMaterialExpression_Last,sMatrixf::Identity());
    if (mptrExpressions[aExpr].IsOK()) {
      iExpressionVariable* pVar = mptrExpressions[aExpr]->Eval();
      if (pVar) {
        return pVar->GetMatrix();
      }
    }
    return sMatrixf::Identity();
  }

  virtual sVec4f __stdcall GetExpressionValueVector(eMaterialExpression aExpr) const {
    niCheckSilent(aExpr < eMaterialExpression_Last,sVec4f::Zero());
    if (mptrExpressions[aExpr].IsOK()) {
      iExpressionVariable* pVar = mptrExpressions[aExpr]->Eval();
      if (pVar) {
        return pVar->GetVec4();
      }
    }
    return sVec4f::Zero();
  }
  //// ni::iMaterial ///////////////////////////

 private:
  Ptr<iGraphics>  mptrGraphics;

  Ptr<iExpressionContext> mptrExpressionContext;

  typedef astl::hstring_hash_map<Ptr<iUnknown> > tUserDataMap;
  typedef tUserDataMap::iterator          tUserDataMapIt;
  typedef tUserDataMap::const_iterator    tUserDataMapCIt;
  tUserDataMap                            mmapUserData;

  typedef astl::vector<Ptr<iMaterial> >   tMaterialPtrVec;
  tMaterialPtrVec                         mvSubMaterials;

  tHStringPtr mhspExpressions[eMaterialExpression_Last];

  niEndClass(cMaterial);
};



///////////////////////////////////////////////
tBool __stdcall cMaterial::HasChannelTexture(eMaterialChannel aChannel) const
{
  if (aChannel >= eMaterialChannel_Last) return eFalse;
  return niIsOK(mChannels[aChannel].mTexture);
}

///////////////////////////////////////////////
tBool __stdcall cMaterial::SetChannelTexture(eMaterialChannel aChannel, iTexture* apTexture)
{
  if (aChannel >= eMaterialChannel_Last) return eFalse;
  if (mChannels[aChannel].mTexture == apTexture) return eTrue;
  mChannels[aChannel].mTexture = niGetIfOK(apTexture);
  return niIsOK(mChannels[aChannel].mTexture);
}

///////////////////////////////////////////////
iTexture* __stdcall cMaterial::GetChannelTexture(eMaterialChannel aChannel) const
{
  if (aChannel >= eMaterialChannel_Last) return NULL;
  return mChannels[aChannel].mTexture;
}

///////////////////////////////////////////////
tBool __stdcall cMaterial::SetChannelColor(eMaterialChannel aChannel, const sColor4f& aColor)
{
  if (aChannel >= eMaterialChannel_Last) return eFalse;
  mChannels[aChannel].mColor = aColor;
  return eTrue;
}

///////////////////////////////////////////////
const sColor4f&  __stdcall cMaterial::GetChannelColor(eMaterialChannel aChannel) const
{
  if (aChannel >= eMaterialChannel_Last) return sColor4f::Zero();
  return mChannels[aChannel].mColor;
}

///////////////////////////////////////////////
tBool __stdcall cMaterial::SetChannelSamplerStates(eMaterialChannel aChannel, tIntPtr aHandle)
{
  if (aChannel >= eMaterialChannel_Last) return eFalse;
  mChannels[aChannel].mhSS = aHandle;
  return eTrue;
}

///////////////////////////////////////////////
tIntPtr __stdcall cMaterial::GetChannelSamplerStates(eMaterialChannel aChannel) const
{
  if (aChannel >= eMaterialChannel_Last) return NULL;
  return mChannels[aChannel].mhSS;
}

///////////////////////////////////////////////
tBool __stdcall cMaterial::Copy(const iMaterial* apMat)
{
  if (apMat == this) {
    niWarning(_A("Trying to copy a material on itself."));
    return eTrue;
  }

  *((sMaterialDesc*)this->GetDescStructPtr()) =
      *((sMaterialDesc*)apMat->GetDescStructPtr());

  mmapUserData.clear();
  niLoop(i,apMat->GetNumUserdata()) {
    iHString* hspName = apMat->GetUserdataName(i);
    SetUserdata(hspName,apMat->GetUserdata(hspName));
  }

  return eTrue;
}

///////////////////////////////////////////////
iMaterial* __stdcall cMaterial::Clone() const
{
  if (!mptrGraphics.IsOK()) return NULL;
  cMaterial* pNew = niNew cMaterial(mptrGraphics);
  pNew->Copy(this);
  return pNew;
}

///////////////////////////////////////////////
tBool __stdcall cMaterial::GetHasShader() const
{
  for (tU32 i = 0; i < eShaderUnit_Last; ++i) {
    if (mShaders[i].IsOK())
      return eTrue;
  }
  return eFalse;
}

///////////////////////////////////////////////
tBool __stdcall cMaterial::SetShader(eShaderUnit aUnit, iShader* apProgram)
{
  niCheckSilent(aUnit < eShaderUnit_Last,eFalse);
  mShaders[aUnit] = niGetIfOK(apProgram);
  return mShaders[aUnit].IsOK();
}

///////////////////////////////////////////////
iShader* __stdcall cMaterial::GetShader(eShaderUnit aUnit) const
{
  niCheckSilent(aUnit < eShaderUnit_Last,NULL);
  return mShaders[aUnit];
}

///////////////////////////////////////////////
tBool __stdcall cMaterial::SetShaderConstants(iShaderConstants* apBuffer) {
  mptrConstants = niGetIfOK(apBuffer);
  return eFalse;
}

///////////////////////////////////////////////
iShaderConstants* __stdcall cMaterial::GetShaderConstants() const {
  return mptrConstants;
}

///////////////////////////////////////////////
tBool cMaterial::SerializeWrite(iDataTableWriteStack* apDT, iHString* ahspBasePath) {
  if (!mptrGraphics.IsOK()) return eFalse;

  apDT->SetHString(_A("name"),GetName());
  apDT->SetHString(_A("class"),GetClass());
  apDT->SetEnum(_A("flags"),niFlagsExpr(eMaterialFlags),GetFlags());
  apDT->SetEnum(_A("blend_mode"),niEnumExpr(eBlendMode),GetBlendMode());
  apDT->SetVec2(_A("polygon_offset"), GetPolygonOffset());

  // rasterizer states
  iRasterizerStates* pRS = mptrGraphics->GetCompiledRasterizerStates(GetRasterizerStates());
  if (pRS) {
    apDT->Push(_A("RasterizerStates"));
    pRS->SerializeDataTable(
        apDT->GetTop(),
        eSerializeFlags_Write|eSerializeFlags_CategoryAll);
    apDT->Pop();
  }

  // depth stencil states
  iDepthStencilStates* pDS = mptrGraphics->GetCompiledDepthStencilStates(GetDepthStencilStates());
  if (pDS) {
    apDT->Push(_A("DepthStencilStates"));
    pDS->SerializeDataTable(
        apDT->GetTop(),
        eSerializeFlags_Write|eSerializeFlags_CategoryAll);
    apDT->Pop();
  }

  const sMaterialDesc& defaultMaterial = _GetDefaultMaterial();

  // channels
  apDT->Push(_A("Channels"));
  niLoop(i,eMaterialChannel_Last) {
    eMaterialChannel c = (eMaterialChannel)i;
    if (memcmp(&mChannels[c],&defaultMaterial.mChannels[c],sizeof(sMaterialChannel)) == 0)
      continue;
    apDT->PushNew(_A("Channel"));
    // base parameters
    apDT->SetEnum(_A("type"),niEnumExpr(eMaterialChannel),c);
    apDT->SetVec4(_A("color"),GetChannelColor(c));

    // texture
    iTexture* pTex = GetChannelTexture(c);
    tHStringPtr hspTexName = niIsOK(pTex) ? pTex->GetDeviceResourceName():NULL;
    if (pTex) {
      apDT->SetHString(_A("texture"),hspTexName);
      apDT->SetEnum(_A("texture_flags"),niFlagsExpr(eTextureFlags),pTex->GetFlags());
    }
    else {
      apDT->SetString(_A("texture"),_A(""));
      apDT->SetString(_A("texture_flags"),_A("eTextureFlags.Default"));
    }

    if (GetChannelSamplerStates(c) != DEFAULT_SS) {
      // sampler states
      iSamplerStates* pSS = mptrGraphics->GetCompiledSamplerStates(GetChannelSamplerStates(c));
      if (pSS) {
        apDT->Push(_A("SamplerStates"));
        pSS->SerializeDataTable(
            apDT->GetTop(),
            eSerializeFlags_Write|eSerializeFlags_CategoryAll);
        apDT->Pop();
      }
    }
    apDT->Pop();
  }
  apDT->Pop();

  // shaders
  if (GetHasShader()) {
    apDT->Push(_A("Shaders"));
    niLoop(i, eShaderUnit_Last) {
      Ptr<iShader> ptrShader = mShaders[i];
      if (!ptrShader.IsOK())
        continue;

      tHStringPtr hspShaderName = ptrShader->GetDeviceResourceName();
      if (HStringIsEmpty(hspShaderName))
        continue;

      apDT->PushNew("Shader");
      apDT->SetEnum("unit",niEnumExpr(eShaderUnit),i);
      apDT->SetHString("path",ptrShader->GetDeviceResourceName());
      apDT->Pop();
    }
    apDT->Pop();
  }

  return eTrue;
}

///////////////////////////////////////////////
tBool cMaterial::SerializeRead(iDataTableReadStack* apDT, iHString* ahspBasePath) {
  if (!mptrGraphics.IsOK()) return eFalse;
  niProfileBlock(cMaterial_SerializeRead);

  SetName(apDT->GetHStringDefault(_A("name"),GetName()));
  SetClass(apDT->GetHStringDefault(_A("class"),GetClass()));
  SetFlags(apDT->GetEnumDefault(_A("flags"),niFlagsExpr(eMaterialFlags),GetFlags()));
  SetBlendMode((eBlendMode)apDT->GetEnumDefault(_A("blend_mode"),niEnumExpr(eBlendMode),GetBlendMode()));
  SetPolygonOffset(apDT->GetVec2Default(_A("polygon_offset"), GetPolygonOffset()));

  // rasterizer states
  if (apDT->PushFail(_A("RasterizerStates"))) {
    niProfileBlock(cMaterial_SerializeRead_RasStates);
    Ptr<iRasterizerStates> ptrRS = mptrGraphics->CreateRasterizerStates();
    ptrRS->SerializeDataTable(
        apDT->GetTop(),
        eSerializeFlags_Read|eSerializeFlags_CategoryAll);
    SetRasterizerStates(mptrGraphics->CompileRasterizerStates(ptrRS));
    apDT->Pop();
  }

  // depth stencil states
  if (apDT->PushFail(_A("DepthStencilStates"))) {
    niProfileBlock(cMaterial_SerializeRead_DSStates);
    Ptr<iDepthStencilStates> ptrDS = mptrGraphics->CreateDepthStencilStates();
    ptrDS->SerializeDataTable(
        apDT->GetTop(),
        eSerializeFlags_Read|eSerializeFlags_CategoryAll);
    SetDepthStencilStates(mptrGraphics->CompileDepthStencilStates(ptrDS));
    apDT->Pop();
  }

  // channels
  if (apDT->PushFail(_A("Channels"))) {
    Ptr<iSamplerStates> ptrSS;
    niProfileBlock(cMaterial_SerializeRead_Channels);

    tBool loadedChannel[eMaterialChannel_Last] = {0};

    niLoop(i,apDT->GetNumChildren()) {
      apDT->PushChild(i);

      eMaterialChannel c = eMaterialChannel_Last;
      {
        niProfileBlock(cMaterial_SerializeRead_Channel_Type);
        tHStringPtr hspType = apDT->GetHStringDefault("type",NULL);
        if (!HStringIsEmpty(hspType)) {
          c = (eMaterialChannel)ni::GetLang()->StringToEnum(
            niHStr(hspType), niEnumExpr(eMaterialChannel));
          if ((int)c == 0 && hspType != _HC(Base)) {
            c = eMaterialChannel_Last;
          }
        }
      }

      // niDebugFmt(("... CHANNEL %s: %d: %s", apDT->GetString("type"), c, apDT->GetString("texture")));
      if (c < eMaterialChannel_Last) {
        loadedChannel[c] = eTrue;

        // base properties
        {
          niProfileBlock(cMaterial_SerializeRead_Channel_BaseProps);
          SetChannelColor(c,apDT->GetVec4Default(_A("color"),GetChannelColor(c)));
          // for backward compatibility
          if (c == eMaterialChannel_Base && apDT->HasProperty("blend_mode")) {
            SetBlendMode((eBlendMode)apDT->GetEnumDefault(
              _A("blend_mode"),niEnumExpr(eBlendMode),GetBlendMode()));
          }
        }

        // texture
        {
          niProfileBlock(cMaterial_SerializeRead_Channel_Texture);
          Ptr<iTexture> ptrTex = GetChannelTexture(c);
          tHStringPtr hspTex = apDT->GetHStringDefault("texture",ptrTex.IsOK()?ptrTex->GetDeviceResourceName():_H("").ptr());
          if (!HStringIsEmpty(hspTex)) {
            ptrTex = mptrGraphics->CreateTextureFromRes(hspTex, ahspBasePath, eTextureFlags_Default);
            SetChannelTexture(c,ptrTex);
          }
          else {
            // Clear the texture channel if its actually declared as empty
            SetChannelTexture(c,NULL);
          }
        }

        // sampler states
        if (apDT->PushFail(_A("SamplerStates"))) {
          niProfileBlock(cMaterial_SerializeRead_Channel_SamplerStates);
          if (!ptrSS.IsOK()) {
            ptrSS = mptrGraphics->CreateSamplerStates();
          }
          ptrSS->SerializeDataTable(
              apDT->GetTop(),
              eSerializeFlags_Read|eSerializeFlags_CategoryAll);
          apDT->Pop();

          SetChannelSamplerStates(c,mptrGraphics->CompileSamplerStates(ptrSS));
        }
        else {
          SetChannelSamplerStates(c,DEFAULT_SS);
        }
      }
      apDT->Pop();
    }

    {
      // All the channels are serialized, any non-specified channel uses the
      // value of the default material.
      const sMaterialDesc& defaultMaterial = _GetDefaultMaterial();
      niLoop(i,eMaterialChannel_Last) {
        if (!loadedChannel[i]) {
          mChannels[i] = defaultMaterial.mChannels[i];
        }
      }
    }

    apDT->Pop();
  }

  // shaders
  if (apDT->PushFail("Shaders")) {
    niLoop(i,apDT->GetNumChildren()) {
      apDT->PushChild(i);
      eShaderUnit u = (eShaderUnit)ni::GetLang()->StringToEnum(apDT->GetStringDefault("unit",NULL).Chars(), niEnumExpr(eShaderUnit));
      if (u < eShaderUnit_Last) {
        Ptr<iShader> ptrShader = mShaders[i];
        tHStringPtr hspPath = apDT->GetHStringDefault("path",ptrShader.IsOK()?ptrShader->GetDeviceResourceName():_H("").ptr());
        if (!HStringIsEmpty(hspPath)) {
          ptrShader = mptrGraphics->CreateShaderFromRes(hspPath);
          SetShader(u,ptrShader);
        }
        else {
          // Clear the shader if its actually declared as empty
          SetShader(u,NULL);
        }
      }
      apDT->Pop();
    }
    apDT->Pop();
  }

  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cMaterial::Serialize(eSerializeMode aMode, iDataTable* apDT, iHString* ahspBasePath)
{
  niCheckIsOK(apDT,eFalse);

  switch (aMode) {
    case eSerializeMode_Write:
    case eSerializeMode_WriteRaw:
      {
        Ptr<iDataTableWriteStack> dtW = ni::CreateDataTableWriteStack(apDT);
        return SerializeWrite(dtW,ahspBasePath);
        break;
      }
    case eSerializeMode_Read:
    case eSerializeMode_ReadRaw:
      {
        Ptr<iDataTableReadStack> dtR = ni::CreateDataTableReadStack(apDT);
        return SerializeRead(dtR,ahspBasePath);
        break;
      }
  }
  return eTrue;
}

//--------------------------------------------------------------------------------------------
//
//  cGraphics
//
//--------------------------------------------------------------------------------------------

///////////////////////////////////////////////
iMaterial* __stdcall cGraphics::CreateMaterial() {
  return niNew cMaterial(this);
}
