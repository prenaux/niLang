// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Graphics.h"
#include "BlitClip.h"
#include "VideoUtils.h"

// #define CREATE_AUTOGEN_MIPMAPS_ALWAYS
// #define CREATE_AUTOGEN_MIPMAPS_NOT_OVERLAY

sRecti _NextMip(const sRecti& aV) {
  sRecti r = aV / 2;
  r.SetWidth(ni::Max(1,r.GetWidth()));
  r.SetHeight(ni::Max(1,r.GetHeight()));
  return r;
}

sVec3i _NextMip(const sVec3i& aV) {
  sVec3i r = aV / 2;
  VecMaximize(r,r,sVec3i::One());
  return r;
}

//--------------------------------------------------------------------------------------------
//
// cGraphics
//
//--------------------------------------------------------------------------------------------

///////////////////////////////////////////////
tBool __stdcall cGraphics::CheckTextureFormat(iBitmapFormat* apFormat, tTextureFlags aFlags) {
  CHECKDRIVER(eFalse);
  niCheckIsOK(apFormat,eFalse);
  return mptrDrv->CheckTextureFormat(apFormat, aFlags);
}

///////////////////////////////////////////////
iDeviceResourceManager* __stdcall cGraphics::GetTextureDeviceResourceManager() const {
  //  CHECKDRIVER(NULL);  // removed because initialize context might need it
  return mptrDRMTextures;
}

///////////////////////////////////////////////
tU32 __stdcall cGraphics::GetNumTextures() const {
  CHECKDRIVER(0);
  return mptrDRMTextures->GetSize();
}

///////////////////////////////////////////////
iTexture* __stdcall cGraphics::GetTextureFromName(iHString* ahspName) const {
  CHECKDRIVER(NULL);
  return ni::QueryInterface<iTexture>(mptrDRMTextures->GetFromName(ahspName));
}

///////////////////////////////////////////////
iTexture* __stdcall cGraphics::GetTextureFromIndex(tU32 anIndex) const {
  CHECKDRIVER(NULL);
  return ni::QueryInterface<iTexture>(mptrDRMTextures->GetFromIndex(anIndex));
}

///////////////////////////////////////////////
iTexture* __stdcall cGraphics::CreateTexture(iHString* ahspName, eBitmapType aType, const achar* aaszFormat, tU32 anNumMipMaps, tU32 anWidth, tU32 anHeight, tU32 anDepth, tTextureFlags aFlags) {
  CHECKDRIVER(NULL);
  return mptrDrv->CreateTexture(ahspName,aType,aaszFormat,anNumMipMaps,anWidth,anHeight,anDepth,aFlags);
}

///////////////////////////////////////////////
iTexture* __stdcall cGraphics::CreateTextureFromBitmap(iHString* ahspName, iBitmapBase* apBitmap, tTextureFlags aFlags) {
  CHECKDRIVER(NULL);
  niCheck(niIsOK(apBitmap),NULL);

  Ptr<iBitmapBase> ptrBmp = apBitmap;

  {
    Ptr<iTexture> ptrTex = GetTextureFromName(ahspName);
    if (ptrTex.IsOK()) {
      if (ptrTex->GetType() != ptrBmp->GetType()) {
        niError(niFmt(_A("Texture '%s' already created, but not of the same type."),ahspName));
        return NULL;
      }
      else {
        niWarning(niFmt(_A("Texture '%s' already created, not reloaded."),ahspName));
        return ptrTex.GetRawAndSetNull();
      }
    }
  }

#if defined CREATE_AUTOGEN_MIPMAPS_ALWAYS
  if (!apBitmap->GetNumMipMaps()) {
    apBitmap->CreateMipMaps(0,ni::eTrue);
    niWarning(niFmt(
        "Auto-Generated %d mipmaps for texture '%s' fmt: %s res: %dx%dx%d.\n",
        apBitmap->GetNumMipMaps(),
        ahspName,
        apBitmap->GetPixelFormat()->GetFormat(),
        apBitmap->GetWidth(),apBitmap->GetHeight(),apBitmap->GetDepth()));
  }
#elif defined CREATE_AUTOGEN_MIPMAPS_NOT_OVERLAY
  if (!niFlagIs(aFlags,eTextureFlags_Overlay) && !apBitmap->GetNumMipMaps()) {
    niWarning(niFmt("Automatically computing mipmaps for '%s' (%dx%dx%d %s).",
                    ahspName,
                    ptrBmp->GetWidth(),
                    ptrBmp->GetHeight(),
                    ptrBmp->GetDepth(),
                    ptrBmp->GetPixelFormat()->GetFormat()));
    apBitmap->CreateMipMaps(0,eTrue);
  }
#else
  if (niFlagIs(aFlags,eTextureFlags_MipMaps) && !apBitmap->GetNumMipMaps()) {
    apBitmap->CreateMipMaps(0,ni::eTrue);
    niWarning(niFmt(
        "Auto-Generated %d mipmaps for texture '%s' fmt: %s res: %dx%dx%d.\n",
        apBitmap->GetNumMipMaps(),
        ahspName,
        apBitmap->GetPixelFormat()->GetFormat(),
        apBitmap->GetWidth(),apBitmap->GetHeight(),apBitmap->GetDepth()));
  }
#endif

  Ptr<iTexture> ptrTex = CreateTexture(
    ahspName,
    ptrBmp->GetType(),
    ptrBmp->GetPixelFormat()->GetFormat(),
    ptrBmp->GetNumMipMaps(),
    ptrBmp->GetWidth(),
    ptrBmp->GetHeight(),
    ptrBmp->GetDepth(),
    aFlags);
  niCheck(ptrTex.IsOK(),NULL);

  switch (ptrBmp->GetType()) {
    case eBitmapType_2D:
      {
        QPtr<iBitmap2D> ptrBmp2D = ptrBmp.ptr();
        niCheck(ptrBmp2D.IsOK(),NULL);
        if (!BlitBitmapToTexture(ptrBmp2D,ptrTex,eInvalidHandle)) {
          niError(_A("Can't blit the bitmap to the texture."));
          return NULL;
        }
        break;
      }
    case eBitmapType_Cube:
      {
        QPtr<iBitmapCube> ptrBmpCube = ptrBmp.ptr();
        niCheck(ptrBmpCube.IsOK(),NULL);
        for (tU32 i = 0; i < 6; ++i) {
          Ptr<iTexture> ptrFaceTex = ptrTex->GetSubTexture(i);
          if (!ptrFaceTex.IsOK()) {
            niError(niFmt(_A("Can't get the texture's face '%d'."),i));
            return NULL;
          }
          if (!BlitBitmapToTexture(ptrBmpCube->GetFace((eBitmapCubeFace)i),ptrFaceTex,eInvalidHandle)) {
            niError(niFmt(_A("Can't blit the bitmap to the texture face '%d' !."),i));
            return NULL;
          }
        }
        break;
      }
    case eBitmapType_3D:
      {
        QPtr<iBitmap3D> ptrBmp3D = ptrBmp.ptr();
        niCheck(ptrBmp3D.IsOK(),NULL);
        if (!BlitBitmap3DToTexture(ptrBmp3D,ptrTex,eInvalidHandle)) {
          niError(_A("Can't blit the bitmap3d to the texture."));
          return NULL;
        }
        break;
      }
  }

  return ptrTex.GetRawAndSetNull();
}

///////////////////////////////////////////////
iTexture* __stdcall cGraphics::CreateTextureFromRes(iHString* ahspRes, iHString* ahspBasePath, tTextureFlags aFlags) {
  if (HStringIsEmpty(ahspRes)) return NULL;

  Ptr<iFile> ptrFP;

  Ptr<iTexture> ptrTex = GetTextureFromName(ahspRes);
  if (ptrTex.IsOK()) {
#ifdef CAN_RECREATE_TEXTURE_WHEN_CHANGED
    if (mbRecreateTextureWhenChanged &&
        // don't recreate videos...
        !Video_FileNameIsVideo(niHStr(ahspRes)))
    {
      tResourceTimeMap::iterator it = mTextureTimeMap.find(ahspRes);
      if (it != mTextureTimeMap.end()) {
        Ptr<iTime> ptrPrevTime = it->second;
        ptrFP = this->OpenBitmapFile(niHStr(ahspRes), niHStr(ahspBasePath));
        if (ptrFP.IsOK()) {
          Ptr<iTime> ptrFileTime = ni::GetLang()->GetCurrentTime()->Clone();
          if (ptrFP->GetTime(eFileTime_LastWrite,ptrFileTime)) {
            if (ptrPrevTime->Compare(ptrFileTime) != 0) {
              niLog(Info,niFmt("Already loaded texture '%s' but it has changed, it will be reloaded.", ahspRes));
              Ptr<iDeviceResourceManager> texResMan = this->GetTextureDeviceResourceManager();
              texResMan->Unregister(ptrTex);
              ptrTex = NULL;
            }
          }
        }
      }
    }
    if (ptrTex.IsOK())
#endif
    {
      return ptrTex;
    }
  }

  if (Video_FileNameIsVideo(niHStr(ahspRes))) {
#ifdef NI_VIDEO
    //// Load a video ////
    ni::Ptr<ni::iFile> fp = ni::GetLang()->URLOpen(niHStr(ahspRes));
    if (!fp.IsOK()) {
      niError(niFmt(_A("Can't open video file '%s' !"),ahspRes));
      return NULL;
    }

    if (!this->IsVideoFile(fp)) {
      niWarning(niFmt(_A("File '%s' is not a supported video format !"),ahspRes));
      return NULL;
    }

    Ptr<iVideoDecoder> ptrDecoder = this->CreateVideoDecoder(ahspRes,fp,eVideoDecoderFlags_TargetTexture);
    if (!niIsOK(ptrDecoder)) {
      niWarning(niFmt(_A("Creation of the video decoder for '%s' failed !"),ahspRes));
      return NULL;
    }

    ptrTex = ptrDecoder->GetTargetTexture();
    if (!ptrTex.IsOK()) {
      niWarning(niFmt(_A("Retrieving the initial target of the video decoder for '%s' failed !"), ahspRes));
      return NULL;
    }
#else
    niError("No video decoder available.");
    return NULL;
#endif
  }
  else
  {
    if (!ptrFP.IsOK()) {
      ptrFP = this->OpenBitmapFile(niHStr(ahspRes), niHStr(ahspBasePath));
      niCheck(ptrFP.IsOK(),NULL);
    }

    QPtr<iBitmapBase> ptrBmp = this->LoadBitmap(ptrFP);
    niCheck(ptrBmp.IsOK(),NULL);

    ptrTex = CreateTextureFromBitmap(ahspRes,ptrBmp,aFlags);
    if (!ptrTex.IsOK()) {
      niError(niFmt("Can't create texture from bitmap resource '%s'.",ahspRes));
      return NULL;
    }

#ifdef CAN_RECREATE_TEXTURE_WHEN_CHANGED
    if (mbRecreateTextureWhenChanged) {
      Ptr<iTime> ptrFileTime = ni::GetLang()->GetCurrentTime()->Clone();
      if (ptrFP->GetTime(eFileTime_LastWrite,ptrFileTime)) {
        astl::upsert(mTextureTimeMap,ahspRes,ptrFileTime);
      }
    }
#endif
  }

  return ptrTex.GetRawAndSetNull();
}
void __stdcall cGraphics::SetRecreateTextureWhenChanged(tBool abEnabled) {
#ifdef CAN_RECREATE_TEXTURE_WHEN_CHANGED
  mbRecreateTextureWhenChanged = abEnabled;
#else
  niUnused(abEnabled);
#endif
}
tBool __stdcall cGraphics::GetRecreateTextureWhenChanged() const {
#ifdef CAN_RECREATE_TEXTURE_WHEN_CHANGED
  return mbRecreateTextureWhenChanged;
#else
  return eFalse;
#endif
}

///////////////////////////////////////////////
tBool __stdcall cGraphics::BlitBitmapToTexture(iBitmap2D* apSrc, iTexture* apDest, tU32 anDestLevel, const sRecti& aSrcRect, const sRecti& aDestRect, eTextureBlitFlags aFlags) {
  CHECKDRIVER(eFalse);
  niCheck(niIsOK(apSrc),eFalse);
  niCheck(niIsOK(apDest),eFalse);
  niCheck(apSrc->GetType() == eBitmapType_2D,eFalse);
  niCheck(apDest->GetType() == eBitmapType_2D,eFalse);

  sRecti srcRect, dstRect;
  if (!ClipBlitRect(srcRect,apSrc,0,aSrcRect)) {
    return eTrue;
  }
  if (!ClipBlitRect(dstRect,apDest,anDestLevel,aDestRect)) {
    return eTrue;
  }

  tU32 nNumMips = ni::Min(apSrc->GetNumMipMaps(),apDest->GetNumMipMaps());
  tBool bRet = eTrue;
  bRet = mptrDrv->BlitBitmapToTexture(apSrc,apDest,(anDestLevel==eInvalidHandle)?0:anDestLevel,srcRect,dstRect,aFlags);
  if (bRet && anDestLevel == eInvalidHandle &&
      (nNumMips && !niFlagIs(apDest->GetFlags(),eTextureFlags_AutoGenMipMaps))) {
    for (tU32 i = 0; i < nNumMips; ++i) {
      srcRect = _NextMip(srcRect);
      dstRect = _NextMip(dstRect);
      BlitBitmapToTexture((iBitmap2D*)apSrc->GetMipMap(i),apDest,i+1,srcRect,dstRect,aFlags);
    }
  }

  return bRet;
}

///////////////////////////////////////////////
tBool __stdcall cGraphics::BlitTextureToBitmap(iTexture* apSrc, tU32 anSrcLevel, iBitmap2D* apDest, const sRecti& aSrcRect, const sRecti& aDestRect, eTextureBlitFlags aFlags) {
  CHECKDRIVER(eFalse);
  niCheck(niIsOK(apSrc),eFalse);
  niCheck(niIsOK(apDest),eFalse);
  niCheck(apSrc->GetType() == eBitmapType_2D,eFalse);
  niCheck(apDest->GetType() == eBitmapType_2D,eFalse);

  sRecti srcRect, dstRect;
  if (!ClipBlitRect(srcRect, apSrc, anSrcLevel, aSrcRect))
    return eTrue;
  if (!ClipBlitRect(dstRect, apDest, 0, aDestRect))
    return eTrue;

  tU32 nNumMips = ni::Min(apSrc->GetNumMipMaps(),apDest->GetNumMipMaps());

  tBool bRet = eTrue;
  bRet = mptrDrv->BlitTextureToBitmap(apSrc,(anSrcLevel==eInvalidHandle)?0:anSrcLevel,apDest,srcRect,dstRect,aFlags);
  if (bRet && anSrcLevel == eInvalidHandle && nNumMips) {
    for (tU32 i = 0; i < nNumMips; ++i) {
      srcRect = _NextMip(srcRect);
      dstRect = _NextMip(dstRect);
      BlitTextureToBitmap(apSrc,i+1,(iBitmap2D*)apDest->GetMipMap(i),srcRect,dstRect,aFlags);
    }
  }
  return bRet;
}

///////////////////////////////////////////////
tBool __stdcall cGraphics::BlitTextureToTexture(iTexture* apSrc, tU32 anSrcLevel, iTexture* apDest, tU32 anDestLevel, const sRecti& aSrcRect, const sRecti& aDestRect, eTextureBlitFlags aFlags) {
  CHECKDRIVER(eFalse);
  niCheck(niIsOK(apSrc),eFalse);
  niCheck(niIsOK(apDest),eFalse);
  niCheck(apSrc->GetType() == eBitmapType_2D,eFalse);
  niCheck(apDest->GetType() == eBitmapType_2D,eFalse);

  sRecti srcRect, dstRect;
  if (!ClipBlitRect(srcRect, apSrc, anSrcLevel, aSrcRect))
    return eTrue;
  if (!ClipBlitRect(dstRect, apDest, anDestLevel, aDestRect))
    return eTrue;

  tU32 nNumMips = ni::Min(apSrc->GetNumMipMaps(),apDest->GetNumMipMaps());
  tBool bRet = eTrue;
  tBool bBlitAll = eFalse;
  if (anSrcLevel == eInvalidHandle || anDestLevel == eInvalidHandle) {
    bBlitAll = eTrue;
  }

  bRet = mptrDrv->BlitTextureToTexture(apSrc,bBlitAll?0:anSrcLevel,apDest,bBlitAll?0:anDestLevel,srcRect,dstRect,aFlags);
  if (bRet && bBlitAll && nNumMips &&
      !niFlagIs(apDest->GetFlags(),eTextureFlags_AutoGenMipMaps)) {
    sRecti destRect = aDestRect;
    sRecti srcRect = aSrcRect;
    for (tU32 i = 0; i < nNumMips; ++i) {
      srcRect = _NextMip(srcRect);
      dstRect = _NextMip(dstRect);
      BlitTextureToTexture(apSrc,i+1,apDest,i+1,srcRect,dstRect,aFlags);
    }
  }
  return bRet;
}

///////////////////////////////////////////////
tBool __stdcall cGraphics::BlitBitmap3DToTexture(iBitmap3D* apSrc, iTexture* apDest, tU32 anDestLevel, const sVec3i& aSrcMin, const sVec3i& aDestMin, const sVec3i& avSize, eTextureBlitFlags aFlags)
{
  CHECKDRIVER(eFalse);
  niCheck(niIsOK(apSrc),eFalse);
  niCheck(niIsOK(apDest),eFalse);
  niCheck(apSrc->GetType() == eBitmapType_3D,eFalse);
  niCheck(apDest->GetType() == eBitmapType_3D,eFalse);

  sVec3i srcP, srcS, dstP, dstS;
  if (!ClipBlitBox(srcP, srcS, apSrc, 0, aSrcMin, avSize))
    return eTrue;
  if (!ClipBlitBox(dstP, dstS, apDest, anDestLevel, aDestMin, avSize))
    return eTrue;
  sVec3i blitSize;
  ni::VecMinimize(blitSize,srcS,dstS);

  tU32 nNumMips = ni::Min(apSrc->GetNumMipMaps(),apDest->GetNumMipMaps());
  tBool bRet = eTrue;
  tBool bBlitAll = eFalse;
  if (anDestLevel == eInvalidHandle) {
    bBlitAll = eTrue;
  }

  bRet = mptrDrv->BlitBitmap3DToTexture(apSrc,
                                        apDest,
                                        bBlitAll?0:anDestLevel,
                                        srcP,dstP,blitSize,
                                        aFlags);
  if (bRet && bBlitAll && nNumMips &&
      !niFlagIs(apDest->GetFlags(),eTextureFlags_AutoGenMipMaps)) {
    for (tU32 i = 0; i < nNumMips; ++i) {
      srcP = _NextMip(srcP);
      dstP = _NextMip(dstP);
      blitSize = _NextMip(blitSize);
      BlitBitmap3DToTexture(apSrc,apDest,i+1,srcP,dstP,blitSize,aFlags);
    }
  }
  return bRet;
}

///////////////////////////////////////////////
tBool __stdcall cGraphics::BlitTextureToBitmap3D(iTexture* apSrc, tU32 anSrcLevel, iBitmap3D* apDest, const sVec3i& aSrcMin, const sVec3i& aDestMin, const sVec3i& avSize, eTextureBlitFlags aFlags)
{
  CHECKDRIVER(eFalse);
  niCheck(niIsOK(apSrc),eFalse);
  niCheck(niIsOK(apDest),eFalse);
  niCheck(apSrc->GetType() == eBitmapType_3D,eFalse);
  niCheck(apDest->GetType() == eBitmapType_3D,eFalse);

  sVec3i srcP, srcS, dstP, dstS;
  if (!ClipBlitBox(srcP, srcS, apSrc, anSrcLevel, aSrcMin, avSize))
    return eTrue;
  if (!ClipBlitBox(dstP, dstS, apDest, 0, aDestMin, avSize))
    return eTrue;
  sVec3i blitSize;
  ni::VecMinimize(blitSize,srcS,dstS);

  tU32 nNumMips = ni::Min(apSrc->GetNumMipMaps(),apDest->GetNumMipMaps());
  tBool bRet = eTrue;
  tBool bBlitAll = eFalse;
  if (anSrcLevel == eInvalidHandle) {
    bBlitAll = eTrue;
  }

  bRet = mptrDrv->BlitTextureToBitmap3D(apSrc,
                                        bBlitAll?0:anSrcLevel,
                                        apDest,
                                        srcP,dstP,blitSize,
                                        aFlags);
  if (bRet && bBlitAll && nNumMips) {
    for (tU32 i = 0; i < nNumMips; ++i) {
      srcP = _NextMip(srcP);
      dstP = _NextMip(dstP);
      blitSize = _NextMip(blitSize);
      BlitTextureToBitmap3D(apSrc,i+1,
                            apDest->GetMipMap(i),
                            srcP,dstP,blitSize,
                            aFlags);
    }
  }
  return bRet;
}

///////////////////////////////////////////////
iBitmapBase* __stdcall cGraphics::CreateBitmapFromTexture(iTexture* apSrc) {
  niCheck(niIsOK(apSrc),NULL);

  Ptr<iBitmapBase> bmp = CreateBitmap(
      apSrc->GetDeviceResourceName(),
      apSrc->GetType(),
      apSrc->GetPixelFormat()->GetFormat(),
      apSrc->GetNumMipMaps(),
      apSrc->GetWidth(),
      apSrc->GetHeight(),
      apSrc->GetDepth()
                                      );
  niCheck(bmp.IsOK(),NULL);

  switch (bmp->GetType()) {
    case eBitmapType_2D:
      {
        QPtr<iBitmap2D> bmp2d = bmp.ptr();
        niCheck(bmp2d.IsOK(),NULL);
        if (!BlitTextureToBitmap(apSrc,eInvalidHandle,bmp2d)) {
          niError(_A("Can't blit texture to bitmap."));
          return NULL;
        }
        break;
      }
    case eBitmapType_3D:
      {
        QPtr<iBitmap3D> bmp3d = bmp.ptr();
        niCheck(bmp3d.IsOK(),NULL);
        if (!BlitTextureToBitmap3D(apSrc,eInvalidHandle,bmp3d)) {
          niError(_A("Can't blit texture to bitmap3d."));
          return NULL;
        }
        break;
      }
    case eBitmapType_Cube:
      {
        QPtr<iBitmapCube> bmpCube = bmp.ptr();
        niCheck(bmpCube.IsOK(),NULL);
        niLoop(i,6) {
          Ptr<iBitmap2D>  bmpFace = bmpCube->GetFace((eBitmapCubeFace)i);
          niCheck(bmpFace.IsOK(),NULL);
          Ptr<iTexture>   texFace = apSrc->GetSubTexture(i);
          niCheck(texFace.IsOK(),NULL);
          if (!BlitTextureToBitmap(texFace,eInvalidHandle,bmpFace)) {
            niError(_A("Can't blit cube texture to bitmap."));
            return NULL;
          }
        }
        break;
      }
  }

  return bmp.GetRawAndSetNull();
}
