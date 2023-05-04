// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Graphics.h"
#include "Bitmap2D.h"
#include "BitmapCube.h"
#include "Bitmap3D.h"
#include "Intersection.h"
#include "Frustum.h"
#include "Frustum.inl"
#include "API/niUI/Utils/Polygon.h"
#include "API/niUI/Utils/AABB.h"
#include "Transform.h"
#include "PixelFormatStd.h"
#include "PixelFormatDXTn.h"
#include <niLang/Utils/ConcurrentImpl.h>
#include "Font.h"
#include "FontTTF.h"
#include "BmpIO_ABM.h"
#include <niLang/Utils/URLFileHandler.h>

#define __MODULE_HSTRING_TABLE_IMPL__
#include "niUI_HString.h"

iMaterialLibrary* __stdcall New_MaterialLibrary(iGraphics* apGraphics);
iBitmapFormat* __stdcall New_BitmapFormat(iGraphics* apGraphics);

niExportFunc(iUnknown*) New_niUI_Graphics(const Var&, const Var&) {
  return niNew cGraphics();
}

niExportFunc(iUnknown*) New_BitmapLoader_abm(const Var&,const Var&);
niExportFunc(iUnknown*) New_BitmapSaver_abm(const Var&,const Var&);

static inline void _RegisterBitmapLoader(const achar* aName, iUnknown* apInstance) {
  ni::GetLang()->GetGlobalInstanceMap()->insert(
      astl::make_pair(niFmt("BitmapLoader.%s",aName),apInstance));
}
static inline void _RegisterBitmapSaver(const achar* aName, iUnknown* apInstance) {
  ni::GetLang()->GetGlobalInstanceMap()->insert(
      astl::make_pair(niFmt("BitmapSaver.%s",aName),apInstance));
}

eBlendMode _kFixedPipelineChannelBlendModes[eMaterialChannel_Last];

//////////////////////////////////////////////////////////////////////////////////////////////
// cGraphics implementation

///////////////////////////////////////////////
cGraphics::cGraphics()
{
  niGuardConstructor(cGraphics);
  RegisterModuleDataDirDefaultURLFileHandler("niLang","niUI");

  _kFixedPipelineChannelBlendModes[eMaterialChannel_Opacity] = eBlendMode_ReplaceAlpha;
  _kFixedPipelineChannelBlendModes[eMaterialChannel_Ambient] = eBlendMode_Modulate;
  _kFixedPipelineChannelBlendModes[eMaterialChannel_Emissive] = eBlendMode_Additive;
  _kFixedPipelineChannelBlendModes[eMaterialChannel_Environment] = eBlendMode_Modulate;

  mbRegisteredSystemFonts = eFalse;

  mStatesHandleGen = eCompiledStates_Driver;

  {
    _RegisterBitmapLoader("abm",New_BitmapLoader_abm(this,niVarNull));
    _RegisterBitmapSaver("abm",New_BitmapSaver_abm(this,niVarNull));
    _RegisterBitmapLoader("bmp",New_BitmapLoader_bmp(this,niVarNull));
    _RegisterBitmapSaver("bmp",New_BitmapSaver_bmp(this,niVarNull));
    {
      QPtr<iBitmapLoader> jpegLoader = New_BitmapLoader_jpeg(this,niVarNull);
      _RegisterBitmapLoader("jpe",jpegLoader);
      _RegisterBitmapLoader("jpg",jpegLoader);
      _RegisterBitmapLoader("jpeg",jpegLoader);
      _RegisterBitmapSaver("jpg",New_BitmapSaver_jpeg(this,niVarNull));
    }
    _RegisterBitmapLoader("png",New_BitmapLoader_png(this,niVarNull));
    _RegisterBitmapSaver("png",New_BitmapSaver_png(this,niVarNull));
    _RegisterBitmapLoader("tga",New_BitmapLoader_tga(this,niVarNull));
    _RegisterBitmapSaver("tga",New_BitmapSaver_tga(this,niVarNull));
    _RegisterBitmapLoader("dds",New_BitmapLoader_dds(this,niVarNull));
  }
}

///////////////////////////////////////////////
cGraphics::~cGraphics()
{
  if (mptrDrv.IsOK()) {
    mptrDrv->Invalidate();
    mptrDrv = NULL;
  }
  mptrFTLib = NULL;
}

///////////////////////////////////////////////
tBool cGraphics::IsOK() const
{
  return eTrue;
}

///////////////////////////////////////////////
iPixelFormat* cGraphics::CreatePixelFormat(const achar* aszFormat)
{
  if (!niStringIsOK(aszFormat)) {
    niError("Empty pixel format.");
    return NULL;
  }

  Ptr<iPixelFormat> pxf;
  if (StrStartsWithI(aszFormat,"dxt")) {
#if !defined PIXELFORMAT_NO_DXT
    pxf = niNew cPixelFormatDXTn(aszFormat);
#endif
  }
  else {
    pxf = niNew cPixelFormatStd(aszFormat);
  }
  if (!pxf.IsOK()) {
    niError(niFmt("Can't create pixel format '%s'.", aszFormat));
    return NULL;
  }

  return pxf.GetRawAndSetNull();
}

///////////////////////////////////////////////
iBitmapBase* __stdcall cGraphics::CreateBitmap(iHString* ahspName, eBitmapType aType, const achar* aaszFormat, tU32 anNumMipMaps, tU32 anWidth, tU32 anHeight, tU32 anDepth)
{
  Ptr<iPixelFormat> ptrPixFmt = CreatePixelFormat(aaszFormat);
  if (!niIsOK(ptrPixFmt)) {
    niError(niFmt(_A("Can't create pixel format '%s'."), aaszFormat));
    return NULL;
  }

  iBitmapBase* pRet = NULL;
  switch (aType) {
    case eBitmapType_2D:
      niCheck(anWidth > 0, NULL);
      niCheck(anHeight > 0, NULL);
      pRet = niNew cBitmap2D(this, ahspName, anWidth, anHeight, ptrPixFmt);
      break;
    case eBitmapType_Cube:
      niCheck(anWidth > 0, NULL);
      pRet = niNew cBitmapCube(this, ahspName, anWidth, ptrPixFmt);
      break;
    case eBitmapType_3D:
      niCheck(anWidth > 0, NULL);
      niCheck(anHeight > 0, NULL);
      niCheck(anDepth > 0, NULL);
      pRet = niNew cBitmap3D(this, ahspName, anWidth, anHeight, anDepth, ptrPixFmt, eTrue);
      break;
  }
  if (pRet && anNumMipMaps) {
    pRet->CreateMipMaps(anNumMipMaps,eFalse);
  }
  return pRet;
}

///////////////////////////////////////////////
iBitmapFormat* __stdcall cGraphics::CreateBitmapFormat(eBitmapType aType, const achar* aaszFormat, tU32 anNumMipMaps, tU32 anWidth, tU32 anHeight, tU32 anDepth)
{
  Ptr<iBitmapFormat> fmt = New_BitmapFormat(this);
  niCheckSilent(fmt.IsOK(),NULL);
  fmt->SetType(aType);
  fmt->SetWidth(anWidth);
  fmt->SetHeight(anHeight);
  fmt->SetDepth(anDepth);
  fmt->SetNumMipMaps(anNumMipMaps);
  fmt->SetPixelFormat(CreatePixelFormat(aaszFormat));
  return fmt.GetRawAndSetNull();
}

///////////////////////////////////////////////
iBitmapFormat* __stdcall cGraphics::CreateBitmapFormatEx(eBitmapType aType, iPixelFormat* pFmt, tU32 anNumMipMaps, tU32 anWidth, tU32 anHeight, tU32 anDepth)
{
  Ptr<iBitmapFormat> fmt = New_BitmapFormat(this);
  niCheckSilent(fmt.IsOK(),NULL);
  fmt->SetType(aType);
  fmt->SetWidth(anWidth);
  fmt->SetHeight(anHeight);
  fmt->SetDepth(anDepth);
  fmt->SetNumMipMaps(anNumMipMaps);
  fmt->SetPixelFormat(pFmt);
  return fmt.GetRawAndSetNull();
}

///////////////////////////////////////////////
iBitmapFormat* __stdcall cGraphics::CreateBitmapFormatEmpty()
{
  return New_BitmapFormat(this);
}

///////////////////////////////////////////////
iBitmapBase* __stdcall cGraphics::CreateBitmapEx(iHString* ahspName, eBitmapType aType, iPixelFormat* pFmt, tU32 anNumMipMaps, tU32 anWidth, tU32 anHeight, tU32 anDepth)
{
  Ptr<iBitmapBase> bmp = NULL;
  switch (aType) {
    case eBitmapType_2D:
      niCheck(anWidth > 0, NULL);
      niCheck(anHeight > 0, NULL);
      bmp = niNew cBitmap2D(this, ahspName, anWidth, anHeight, pFmt);
      break;
    case eBitmapType_Cube:
      niCheck(anWidth > 0, NULL);
      niCheck(anHeight > 0, NULL);
      bmp = niNew cBitmapCube(this, ahspName, anWidth, pFmt);
      break;
    case eBitmapType_3D:
      niCheck(anWidth > 0, NULL);
      niCheck(anHeight > 0, NULL);
      niCheck(anDepth > 0, NULL);
      bmp = niNew cBitmap3D(this, ahspName, anWidth, anHeight, anDepth, pFmt, eTrue);
      break;
  }
  if (!bmp.IsOK()) {
    niError(_A("Can't create bitmap."));
    return NULL;
  }
  if (anNumMipMaps) {
    if (!bmp->CreateMipMaps(anNumMipMaps,eFalse)) {
      niError(_A("Can't create the bitmap's mipmaps."));
      return NULL;
    }
  }
  return bmp.GetRawAndSetNull();
}

///////////////////////////////////////////////
iBitmap2D* cGraphics::CreateBitmap2DEx(tU32 nW, tU32 nH, iPixelFormat* pPixFmt)
{
  niAssert(nW > 0 && nH > 0);
  niCheckSilent(nW > 0 && nH > 0, NULL);
  return niNew cBitmap2D(this, NULL, nW, nH, pPixFmt);
}

///////////////////////////////////////////////
iBitmap2D* cGraphics::CreateBitmap2D(tU32 nW, tU32 nH, const achar* aszPixFmt)
{
  niAssert(nW > 0 && nH > 0);
  niCheckSilent(nW > 0 && nH > 0, NULL);

  Ptr<iPixelFormat> ptrPixFmt = CreatePixelFormat(aszPixFmt);
  if (!niIsOK(ptrPixFmt)) {
    niError(niFmt(_A("Can't create pixel format '%s'."), aszPixFmt));
    return NULL;
  }

  return niNew cBitmap2D(this, NULL, nW, nH, ptrPixFmt);
}

///////////////////////////////////////////////
iBitmap2D* __stdcall cGraphics::CreateBitmap2DMemoryEx(tU32 nW, tU32 nH, iPixelFormat* pFmt, tU32 anPitch, tPtr ptrAddr, tBool bFreeAddr)
{
  niAssert(nW > 0 && nH > 0);
  niCheckSilent(nW > 0 && nH > 0, NULL);
  return niNew cBitmap2D(this, NULL, nW, nH, pFmt, anPitch, ptrAddr, bFreeAddr);
}
iBitmap2D* __stdcall cGraphics::CreateBitmap2DMemory(tU32 nW, tU32 nH, const achar* aszPixFmt, tU32 anPitch, tPtr ptrAddr, tBool bFreeAddr)
{
  niAssert(nW > 0 && nH > 0);
  niCheckSilent(nW > 0 && nH > 0, NULL);
  Ptr<iPixelFormat> ptrPixFmt = CreatePixelFormat(aszPixFmt);
  if (!niIsOK(ptrPixFmt)) {
    niError(niFmt(_A("Can't create pixel format '%s'."), aszPixFmt));
    return NULL;
  }
  return niNew cBitmap2D(this, NULL, nW, nH, ptrPixFmt, anPitch, ptrAddr, bFreeAddr);
}

///////////////////////////////////////////////
iBitmapCube* cGraphics::CreateBitmapCubeEx(tU32 ulSize, iPixelFormat* pFmt)
{
  niAssert(ulSize > 0);
  niCheckSilent(ulSize > 0, NULL);
  return niNew cBitmapCube(this, NULL, ulSize, pFmt);
}
iBitmapCube* cGraphics::CreateBitmapCube(tU32 ulSize, const achar* aszPixFmt)
{
  niAssert(ulSize > 0);
  niCheckSilent(ulSize > 0, NULL);
  Ptr<iPixelFormat> ptrPixFmt = CreatePixelFormat(aszPixFmt);
  if (!ptrPixFmt.IsOK()) {
    niError(niFmt(_A("Can't create pixel format '%s'."), aszPixFmt));
    return NULL;
  }
  return niNew cBitmapCube(this, NULL, ulSize, ptrPixFmt);
}

///////////////////////////////////////////////
iBitmap3D* cGraphics::CreateBitmap3DEx(tU32 nW, tU32 nH, tU32 nD, iPixelFormat* pFmt)
{
  niAssert(nW > 0 && nH > 0 && nD > 0);
  niCheckSilent(nW > 0 && nH > 0 && nD > 0, NULL);
  return niNew cBitmap3D(this, NULL, nW, nH, nD, pFmt, eTrue);
}
iBitmap3D* cGraphics::CreateBitmap3D(tU32 nW, tU32 nH, tU32 nD, const achar* aszPixFmt)
{
  niAssert(nW > 0 && nH > 0 && nD > 0);
  niCheckSilent(nW > 0 && nH > 0 && nD > 0, NULL);
  Ptr<iPixelFormat> ptrPixFmt = CreatePixelFormat(aszPixFmt);
  if (!ptrPixFmt.IsOK()) {
    niError(niFmt(_A("Can't create pixel format '%s'."), aszPixFmt));
    return NULL;
  }
  return niNew cBitmap3D(this, NULL, nW, nH, nD, ptrPixFmt, eTrue);
}

///////////////////////////////////////////////
iBitmap3D* __stdcall cGraphics::CreateBitmap3DMemoryEx(tU32 nW, tU32 nH, tU32 nD, iPixelFormat* pFmt, tU32 anRowPitch, tU32 anSlicePitch, tPtr ptrAddr, tBool bFreeAddr)
{
  niAssert(nW > 0 && nH > 0 && nD > 0);
  niCheckSilent(nW > 0 && nH > 0 && nD > 0, NULL);
  Ptr<iBitmap3D> bmp = niNew cBitmap3D(this,NULL,nW,nH,nD,pFmt,eFalse);
  bmp->SetMemoryAddress(ptrAddr,bFreeAddr,anRowPitch,anSlicePitch);
  return bmp.GetRawAndSetNull();
}
iBitmap3D* __stdcall cGraphics::CreateBitmap3DMemory(tU32 nW, tU32 nH, tU32 nD, const achar* aszPixFmt, tU32 anRowPitch, tU32 anSlicePitch, tPtr ptrAddr, tBool bFreeAddr)
{
  niAssert(nW > 0 && nH > 0 && nD > 0);
  niCheckSilent(nW > 0 && nH > 0 && nD > 0, NULL);
  Ptr<iPixelFormat> ptrPixFmt = CreatePixelFormat(aszPixFmt);
  if (!ptrPixFmt.IsOK()) {
    niError(niFmt(_A("Can't create pixel format '%s'."), aszPixFmt));
    return NULL;
  }
  Ptr<iBitmap3D> bmp = niNew cBitmap3D(this,NULL,nW,nH,nD,ptrPixFmt,eFalse);
  bmp->SetMemoryAddress(ptrAddr,bFreeAddr,anRowPitch,anSlicePitch);
  return bmp.GetRawAndSetNull();
}

///////////////////////////////////////////////
iBitmapBase* cGraphics::LoadBitmap(iFile* apFile)
{
  niCheckIsOK(apFile,NULL);
  cString ext = _ASTR(apFile->GetSourcePath()).RAfter(".").ToLower();
  return LoadBitmapEx(ext.Chars(), apFile);
}

///////////////////////////////////////////////
iBitmapBase* cGraphics::LoadBitmapEx(const achar* aaszFormat, iFile* apFile)
{
  niCheckIsOK(apFile,NULL);
  QPtr<iBitmapLoader> ptrLoader;

  const tI64 pos = apFile->Tell();
  const tU32 fccHeader = apFile->ReadLE32();
  if (fccHeader == _kfccABM4 || fccHeader == _kfccABM5) {
    ptrLoader = ni::GetLang()->GetGlobalInstance("BitmapLoader.abm");
  }
  else {
    ptrLoader = ni::GetLang()->GetGlobalInstance(niFmt("BitmapLoader.%s",aaszFormat));
  }
  apFile->SeekSet(pos);

  if (!ptrLoader.IsOK()) {
    niError(niFmt("Can't find a bitmap loader for file type '%s'.",aaszFormat));
    return NULL;
  }

  return ptrLoader->LoadBitmap(this,apFile);
}

///////////////////////////////////////////////
iBitmapBase* __stdcall cGraphics::LoadBitmapFromRes(iHString* ahspRes, iHString* ahspBasePath) {
  Ptr<iFile> fp = this->OpenBitmapFile(niHStr(ahspRes),niHStr(ahspBasePath));
  if (!fp.IsOK()) {
    niError(niFmt("Can't open bitmap resource '%s'.",ahspRes));
    return NULL;
  }
  QPtr<iBitmapBase> bmp = this->LoadBitmap(fp);
  niCheck(bmp.IsOK(),NULL);
  return bmp.GetRawAndSetNull();
}

///////////////////////////////////////////////
tBool cGraphics::SaveBitmap(const achar* aszFilename, iBitmapBase* pBmp, tU32 ulCompression)
{
  niCheckIsOK(pBmp,NULL);

  const cString ext = _ASTR(aszFilename).RAfter(".").ToLower();

  QPtr<iBitmapSaver> ptrSaver = ni::GetLang()->GetGlobalInstance(niFmt("BitmapSaver.%s",ext));
  if (!ptrSaver.IsOK()) {
    niError(niFmt("Can't find a bitmap saver for file type '%s'.",ext));
    return NULL;
  }

  Ptr<iFile> ptrFile = GetRootFS()->FileOpen(aszFilename, eFileOpenMode_Write);
  if (!ptrFile.IsOK()) {
    niError(niFmt(_A("Can't open file %s."), aszFilename));
    return eFalse;
  }

  return ptrSaver->SaveBitmap(this,ptrFile,pBmp,ulCompression);
}

///////////////////////////////////////////////
tBool __stdcall cGraphics::SaveBitmapEx(const achar* aaszFormat, iFile* apFile, iBitmapBase* apBmp, tU32 anCompression)
{
  niCheckIsOK(apBmp,NULL);
  niCheckIsOK(apFile,NULL);

  QPtr<iBitmapSaver> ptrSaver = ni::GetLang()->GetGlobalInstance(niFmt("BitmapSaver.%s",aaszFormat));
  if (!ptrSaver.IsOK()) {
    niError(niFmt("Can't find a bitmap saver for file type '%s'.",aaszFormat));
    return NULL;
  }

  return ptrSaver->SaveBitmap(this,apFile,apBmp,anCompression);
}

///////////////////////////////////////////////
cString __stdcall cGraphics::URLFindBitmapFilePath(const achar* aszRes, const achar* aszBasePath)
{
  return ni::GetLang()->URLFindFilePath(aszRes, aszBasePath, "BitmapLoader.");
}

///////////////////////////////////////////////
iFile* __stdcall cGraphics::OpenBitmapFile(const achar* aszRes, const achar* aszBasePath)
{
  Ptr<iFile> ptrFile;
  {
    cString path = URLFindBitmapFilePath(aszRes,aszBasePath);
    if (!path.empty()) {
      ptrFile = ni::GetLang()->URLOpen(path.Chars());
    }
  }
  if (!ptrFile.IsOK()) {
    niError(niFmt(_A("Can't open bitmap file '%s' (%s)."), aszRes, aszBasePath));
    return NULL;
  }
  return ptrFile.GetRawAndSetNull();
}

///////////////////////////////////////////////
iGeometryModifier* __stdcall cGraphics::CreateGeometryModifier(const achar* aszName, iGeometry* apGeometry, iUnknown* apInitData)
{
  QPtr<iGeometryModifier> modifier = ni::GetLang()->CreateInstance(
      niFmt("GeometryModifier.%s",aszName), apGeometry, apInitData);
  if (!modifier.IsOK()) {
    niError(niFmt(_A("Can't create '%s' geometry modifier."), aszName));
    return NULL;
  }
  return modifier.GetRawAndSetNull();
}

///////////////////////////////////////////////
iMaterialLibrary* __stdcall cGraphics::CreateMaterialLibrary()
{
  return New_MaterialLibrary(this);
}

///////////////////////////////////////////////
tU32 __stdcall cGraphics::FVFGetTexCooDim(tFVF anFVF, tU32 anTexCooIndex) const {
  return eFVF_TexCooDim(anFVF,anTexCooIndex);
}

///////////////////////////////////////////////
tU32 __stdcall cGraphics::FVFGetNumTexCoos(tFVF anFVF) const {
  return eFVF_TexNumCoo(anFVF);
}

///////////////////////////////////////////////
cString __stdcall cGraphics::FVFToString(tFVF aFVF) const {
  return ni::FVFToString(aFVF);
}

///////////////////////////////////////////////
tFVF __stdcall cGraphics::FVFFromString(const achar* aaszString) const {
  return ni::FVFFromString(aaszString);
}

///////////////////////////////////////////////
tU32 __stdcall cGraphics::FVFGetComponentOffset(tFVF aFVF, eFVF C) const {
  return ni::FVFGetComponentOffset(aFVF,C);
}

///////////////////////////////////////////////
tU32 __stdcall cGraphics::FVFGetStride(tFVF aFVF) const {
  return ni::FVFGetStride(aFVF);
}

///////////////////////////////////////////////
iTransform* __stdcall cGraphics::_CreateTransform() {
  return niNew cTransform();
}

///////////////////////////////////////////////
iFrustum* __stdcall cGraphics::_CreateFrustum() {
  return niNew cFrustumf();
}

///////////////////////////////////////////////
iBoundingVolume* __stdcall cGraphics::_CreateAABB() {
  return niNew cBoundingVolumeAABB();
}

///////////////////////////////////////////////
iIntersection* __stdcall cGraphics::_CreateIntersection() {
  return niNew cIntersection();
}

///////////////////////////////////////////////
iCamera* __stdcall cGraphics::_CreateCamera() {
  return (iCamera*)New_niUI_Camera(niVarNull,niVarNull);
}
