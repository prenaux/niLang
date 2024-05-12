// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Graphics.h"
#include "Image.h"
#include "ImagePacker.h"
#include <niUI/IImageMap.h>

#define CHECKVALID(RET)   if (!mptrGraphics.IsOK()) return RET;

static const tBool _kbInsertFiltered = eFalse;
#define PACKER_INSERT_PADDING (abUseMipMaps ? 2 : 0)

///
/// cImageMap
///
class cImageMap : public ImplRC<ni::iImageMap,eImplFlags_Default>
{
  niBeginClass(cImageMap);

  struct sPage {
    Ptr<cImagePacker>  packer;
  };

 public:
  ///////////////////////////////////////////////
  cImageMap(iGraphics* apGraphics, const achar* aaszBaseName, const achar* aaszPxf) {
    ZeroMembers();
    mptrGraphics = apGraphics;
    mhspBaseName = _H(aaszBaseName);
    if (niStringIsOK(aaszPxf))
      mhspPxf = _H(aaszPxf);
    else
      mhspPxf = _H(IMAGE_PIXELFORMAT);
    SetPageSize(512);
  }

  ///////////////////////////////////////////////
  ~cImageMap() {
    Invalidate();
  }

  ///////////////////////////////////////////////
  void __stdcall Invalidate() {
    if (mptrGraphics.IsOK()) {
      Clear();
      mptrGraphics = NULL;
    }
  }

  ///////////////////////////////////////////////
  void __stdcall ZeroMembers() {
    mptrGraphics = NULL;
    mnMaxNumPages = 0;
    mnPageSize = 0;
    mnPageMipMaps = 1;
    mbComputeMipMapsPerPage = eFalse;
    mhspPageFormat = _H("R8G8B8A8");
    mbDefaultImageFiltering = eFalse;
    mDefaultImageBlendMode = eBlendMode_NoBlending;
    mbShouldSerialize = eFalse;
  }

  ///////////////////////////////////////////////
  ni::tBool __stdcall IsOK() const {
    return ni::eTrue;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall Clear() {
    niLoop(i,mvImages.size()) {
      mvImages[i]->Invalidate();
    }
    niLoop(i,mvPages.size()) {
      mvPages[i].packer->Invalidate();
    }
    mvImages.clear();
    mmapImages.clear();
    mvPages.clear();
    mbShouldSerialize = eTrue;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetMaxNumPages(tU32 anMax) {
    mnMaxNumPages = anMax;
  }

  ///////////////////////////////////////////////
  virtual tU32 __stdcall GetMaxNumPages() const {
    return mnMaxNumPages;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetDefaultImageBlendMode(eBlendMode aMode) {
    mDefaultImageBlendMode = aMode;
  }
  virtual eBlendMode __stdcall GetDefaultImageBlendMode() const {
    return mDefaultImageBlendMode;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetDefaultImageFilter(tBool abFiltering) {
    mbDefaultImageFiltering = abFiltering;
  }
  virtual tBool __stdcall GetDefaultImageFilter() const {
    return mbDefaultImageFiltering;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetPageSize(tU32 anSize) {
    anSize = ni::Min(anSize,mptrGraphics->GetDriverCaps(eGraphicsCaps_Texture2DMaxSize));
    mnPageSize = anSize;
    if (mnPageMipMaps) {
      mnPageMipMaps = ni::ComputeNumPow2Levels(mnPageSize>>1,mnPageSize>>1);
    }
  }
  virtual tU32 __stdcall GetPageSize() const {
    return mnPageSize;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetPageFormat(iHString* ahspFormat) {
    mhspPageFormat = ahspFormat;
  }
  virtual iHString* __stdcall GetPageFormat() const {
    return mhspPageFormat;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetPageMipMaps(tU32 anNumMipMaps) {
    mnPageMipMaps = ni::Min(anNumMipMaps,ni::ComputeNumPow2Levels(mnPageSize>>1,mnPageSize>>1));
  }
  virtual tU32 __stdcall GetPageMipMaps() const {
    return mnPageMipMaps;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetComputeMipMapsPerPage(tBool abComputeMipMapsPerPage) {
    mbComputeMipMapsPerPage = abComputeMipMapsPerPage;
  }
  virtual tBool __stdcall GetComputeMipMapsPerPage() const {
    return mbComputeMipMapsPerPage;
  }

  ///////////////////////////////////////////////
  virtual tU32 __stdcall GetNumPages() const {
    return mvPages.size();
  }

  ///////////////////////////////////////////////
  virtual iTexture* __stdcall GetPage(tU32 anIndex) const {
    niCheckSilent(anIndex < mvPages.size(),NULL);
    return mvPages[anIndex].packer->_GetSourceTexture();
  }
  virtual iBitmap2D* __stdcall GetPageBitmap(tU32 anIndex) const {
    niCheckSilent(anIndex < mvPages.size(),NULL);
    return mvPages[anIndex].packer->_GetSourceBitmap();
  }

  ///////////////////////////////////////////////
  virtual tU32 __stdcall GetNumImages() const {
    return mvImages.size();
  }
  virtual iOverlay* __stdcall GetImage(tU32 anIndex) const {
    niCheckSilent(anIndex < mvImages.size(),NULL);
    return mvImages[anIndex];
  }
  virtual tU32 __stdcall GetImageIndex(iOverlay* apImage) const {
    niLoop(i,mvImages.size()) {
      if (mvImages[i] == apImage)
        return i;
    }
    return eInvalidHandle;
  }
  virtual iOverlay* __stdcall GetImageFromName(iHString* ahspName) const {
    tImageMap::const_iterator it = mmapImages.find(ahspName);
    if (it == mmapImages.end())
      return NULL;
    return it->second;
  }

  ///////////////////////////////////////////////
  cImagePacker* __stdcall _CreateImagePacker(tU32 anIndex)
  {
    const tBool abUseMipMaps = (mnPageMipMaps > 0);
    cImagePacker* packer = niNew cImagePacker(
        mptrGraphics,mnPageSize,mnPageSize,
        abUseMipMaps,
        niHStr(mhspBaseName),niHStr(mhspPxf));
    if (niIsOK(packer)) {
      Image_SetDebugName(packer->mptrImage,niFmt(_A("IMAGEMAP%p_Page%d"),(tIntPtr)this,anIndex));
    }
    return packer;
  }
  virtual iOverlay* __stdcall AddImage(iHString* ahspName, iBitmap2D* apBitmap) {
    CHECKVALID(NULL);
    niCheckIsOK(apBitmap,NULL);

    Ptr<iOverlay> image = GetImageFromName(ahspName);
    if (image.IsOK())
      return image;

    // get the bitmap
    Ptr<iBitmap2D> bmp = apBitmap;
    if (bmp->GetWidth() > mnPageSize || bmp->GetHeight() > mnPageSize) {
      bmp = bmp->CreateResized(ni::Min(bmp->GetWidth(),mnPageSize),
                               ni::Min(bmp->GetHeight(),mnPageSize));
    }

    // look in the existing pages if the bitmap finds a place
    const tBool abUseMipMaps = (mnPageMipMaps > 0);
    niLoopit(tPageVec::iterator,it,mvPages) {
      sPage& page = *it;
      if (page.packer->GetIsFull())
        continue;
      sRecti rect = page.packer->InsertBitmap(bmp,_kbInsertFiltered,PACKER_INSERT_PADDING);
      if (rect != sRecti::Null()) {
        // managed to pack in the bitmap, create the image
        image = _AddImage(ahspName,rect,page);
        if (!image.IsOK()) {
          niError(_A("Can't add packed image."));
          return NULL;
        }
        return image.GetRawAndSetNull();
      }
    }

    // not been able to pack in an existing page, so see if we can create a new page,
    // if not return an error, if we can then we do so...
    if (mnMaxNumPages && mvPages.size() > mnMaxNumPages) {
      niError(niFmt(_A("Max number of pages '%d' reached, cant create an additional page."),mnMaxNumPages));
      return NULL;
    }

    sPage page;
    page.packer = _CreateImagePacker(mvPages.size());
    if (!page.packer.IsOK()) {
      niError(_A("Can't create the new page's packer."));
      return NULL;
    }

    sRecti rect = page.packer->InsertBitmap(bmp,_kbInsertFiltered,PACKER_INSERT_PADDING);
    if (rect == sRecti::Null()) {
      niError(_A("Can't insert the bitmap in the new page."));
      return NULL;
    }

    mvPages.push_back(page);
    image = _AddImage(ahspName,rect,page);
    if (!image.IsOK()) {
      niError(_A("Can't add packed image."));
      return NULL;
    }

    return image.GetRawAndSetNull();
  }

  ///////////////////////////////////////////////
  virtual iOverlay* __stdcall AddImageFromResource(iHString* ahspName, iHString* ahspRes) {
    CHECKVALID(NULL);
    tHStringPtr hspName;
    if (HStringIsEmpty(ahspName)) {
      hspName = ahspRes;
    }
    else {
      hspName = ahspName;
    }
    Ptr<iOverlay> image = GetImageFromName(ahspName);
    if (image.IsOK())
      return image;

    Ptr<iFile> fp = mptrGraphics->OpenBitmapFile(niHStr(ahspRes));
    QPtr<iBitmap2D> bmp = mptrGraphics->LoadBitmap(fp);
    if (!bmp.IsOK()) {
      niError(niFmt(_A("Can't open resource '%s'."),niHStr(ahspRes)));
      return NULL;
    }
    image = AddImage(hspName,bmp);
    if (!image.IsOK()) {
      niError(niFmt(_A("Can't add image '%s' from resource '%s'."),niHStr(ahspName),niHStr(ahspRes)));
      return NULL;
    }
    return image.GetRawAndSetNull();
  }

  ///////////////////////////////////////////////
  virtual iOverlay* __stdcall AddImageFromIconSet(iHString* ahspName, iHString* ahspFolder, iHString* ahspRes, tU32 anMaxRes, tU32 anMinRes) {
    CHECKVALID(NULL);

    tHStringPtr hspName;
    if (HStringIsEmpty(ahspName)) {
      cPath path((cString(niHStr(ahspFolder))+_A("/")+cString(niHStr(ahspRes))).Chars());
      hspName = _H(path.GetPath());
    }
    else {
      hspName = ahspName;
    }
    Ptr<iOverlay> image = GetImageFromName(hspName);
    if (image.IsOK())
      return image;

    tU32 curRes = ni::Min(256,anMaxRes);
    Ptr<iBitmap2D> bmp;
    Ptr<iBitmap2D> prevBmp;
    anMinRes = ni::Max(4,anMinRes);
    tU32 curMip = 0;

    while (curRes >= anMinRes) {
      Ptr<iFile> fp;
      QPtr<iBitmap2D> curBmp;
      cPath buildPath(niHStr(ahspFolder),NULL);
      buildPath.AddDirectoryBack(niFmt(_A("%dx%d/"),curRes,curRes));
      buildPath.SetPath((buildPath.GetPath() + cString(niHStr(ahspRes))).Chars());
      buildPath.SetExtension("png"); // icons are all in png
      cString foundPath = mptrGraphics->URLFindBitmapFilePath(buildPath.Chars(),NULL);
      if (foundPath.IsNotEmpty()) {
        fp = mptrGraphics->OpenBitmapFile(foundPath.Chars());
        if (fp.IsOK()) {
          curBmp = mptrGraphics->LoadBitmap(fp);
          if (!curBmp.IsOK()) {
            niError(niFmt(_A("Can't open image '%s' resolution '%d' from resource '%s', icon set '%s'."),niHStr(hspName),curRes,niHStr(ahspRes),niHStr(ahspFolder)));
            return NULL;
          }
        }
      }

      if (bmp.IsOK()) {
        if (!curBmp.IsOK()) {
          niAssert(prevBmp.IsOK());
          curBmp = prevBmp->CreateResized(prevBmp->GetWidth()/2,prevBmp->GetHeight()/2);
        }
        niAssert(curBmp.IsOK());
        bmp->GetMipMap(curMip)->Blit(curBmp,0,0,0,0,curBmp->GetWidth(),curBmp->GetHeight());
        prevBmp = curBmp;
        ++curMip;
        if (curMip >= bmp->GetNumMipMaps()) {
          break;
        }
      }
      else if (curBmp.IsOK()) {
        bmp = curBmp;
        if (!bmp->CreateMipMaps(0,eFalse)) {
          niError(niFmt(_A("Can't create mipmaps for image '%s' resolution '%d' from resource '%s', icon set '%s'."),niHStr(hspName),curRes,niHStr(ahspRes),niHStr(ahspFolder)));
          return NULL;
        }
        prevBmp = curBmp;
      }
      curRes /= 2;
    }

    if (!bmp.IsOK()) {
      niError(niFmt(_A("Can't open image '%s' at any resolution from resource '%s', icon set '%s'."),niHStr(hspName),niHStr(ahspRes),niHStr(ahspFolder)));
      return NULL;
    }

    for ( ; curMip < bmp->GetNumMipMaps(); ++curMip) {
      niAssert(prevBmp.IsOK());
      Ptr<iBitmap2D> curBmp = prevBmp->CreateResized(prevBmp->GetWidth()/2,prevBmp->GetHeight()/2);
      niAssert(curBmp.IsOK());
      bmp->GetMipMap(curMip)->Blit(curBmp,0,0,0,0,curBmp->GetWidth(),curBmp->GetHeight());
      prevBmp = curBmp;
    }

    image = AddImage(hspName,bmp);
    if (!image.IsOK()) {
      niError(niFmt(_A("Can't add image '%s' from resource '%s', icon set '%s'."),niHStr(hspName),niHStr(ahspRes),niHStr(ahspFolder)));
      return NULL;
    }
    return image.GetRawAndSetNull();
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall RemoveImage(iOverlay* apImage) {
#pragma niTodo("Implement")
    CHECKVALID(eFalse);
    return eFalse;
  }

  ///////////////////////////////////////////////
  iOverlay* _AddImage(iHString* ahspName, const sRecti& rect, const sPage& page) {
    Ptr<iOverlay> image = mptrGraphics->CreateOverlayImage(page.packer->GetImage());
    niCheckIsOK(image,NULL);
    sRectf mapping = rect.ToFloat();
    image->SetSize(mapping.GetSize());
    mapping.x1() = (mapping.x1())/tF32(page.packer->GetWidth());
    mapping.x2() = (mapping.x2())/tF32(page.packer->GetWidth());
    mapping.y1() = (mapping.y1())/tF32(page.packer->GetHeight());
    mapping.y2() = (mapping.y2())/tF32(page.packer->GetHeight());
    image->SetMapping(mapping);
    image->SetFiltering(mbDefaultImageFiltering);
    image->SetBlendMode(mDefaultImageBlendMode);
    if (mbComputeMipMapsPerPage) {
      page.packer->GetImage()->RecomputeBitmapMipmapsBeforeCopyToTexture();
    }
    return _AddImageEx(ahspName,image);
  }
  iOverlay* _AddImageEx(iHString* ahspName, ni::iOverlay* apOverlay) {
    mbShouldSerialize = eTrue;
    mvImages.push_back(apOverlay);
    if (!HStringIsEmpty(ahspName)) {
      astl::upsert(mmapImages,ahspName,apOverlay);
    }
    return apOverlay;
  }

  ///////////////////////////////////////////////
  void _RemoveImage(iOverlay* apImage) {
    astl::find_erase(mvImages,apImage);
    niLoopit(tImageMap::iterator,it,mmapImages) {
      if (it->second == apImage) {
        mmapImages.erase(it);
        break;
      }
    }
  }

  ///////////////////////////////////////////////
  tBool __stdcall GetShouldSerialize() const {
    return mbShouldSerialize;
  }

  ///////////////////////////////////////////////
  tBool __stdcall Serialize(ni::iFile* apFile, tImageMapSerializeFlags aFlags) {
    niCheck(mptrGraphics.IsOK(),eFalse);

    iGraphics* pGraphics = mptrGraphics;

    /// Read ///////////////////////////////////////////
    if (niFlagIs(aFlags,eImageMapSerializeFlags_Read)) {
      tU32 numImages = 0;
      tU32 numPages = 0;

      // Header
      {
        cString header = apFile->ReadString();
        if (!header.IEq("niImageMap")) {
          niError(_A("Invalid header marker 1."));
          return eFalse;
        }
        apFile->BeginReadBits();
        const tBool bDefaultImageFiltering = apFile->ReadBit();
        const eBlendMode defaultImageBlendMode = (eBlendMode)apFile->ReadBitsPackedU32();
        /*const tU32 nMaxNumPages =*/ apFile->ReadBitsPackedU32();
        const tU32 nPageSize = apFile->ReadBitsPackedU32();
        const tU32 nPageMipMaps = apFile->ReadBitsPackedU32();
        numPages = apFile->ReadBitsPackedU32();
        numImages = apFile->ReadBitsPackedU32();
        tHStringPtr hspPageFormat = _H(apFile->ReadBitsString());
        apFile->EndReadBits();
        header = apFile->ReadString();
        if (!header.IEq("niImageMap")) {
          niError(_A("Invalid header marker 2."));
          return eFalse;
        }

        if (nPageSize > (tU32)pGraphics->GetDriverCaps(eGraphicsCaps_Texture2DMaxSize)) {
          niError(niFmt(_A("Page size '%d' is too big for this graphics driver (max is '%d')"),
                        nPageSize,pGraphics->GetDriverCaps(eGraphicsCaps_Texture2DMaxSize)));
          return eFalse;
        }

        SetDefaultImageFilter(bDefaultImageFiltering);
        SetDefaultImageBlendMode(defaultImageBlendMode);
        SetMaxNumPages(mnMaxNumPages);
        SetPageSize(nPageSize);
        SetPageMipMaps(nPageMipMaps);
        SetPageFormat(hspPageFormat);
      }

      // Clear now only, if the header is wrong we dont want to change anything...
      Clear();

      // Pages
      niLoop(i,numPages) {
        sPage page;
        page.packer = _CreateImagePacker(mvPages.size());
        if (!page.packer.IsOK()) {
          niError(niFmt(_A("Can't create packer '%d'."),i));
          Clear();
          return eFalse;
        }

        QPtr<iBitmap2D> bmp = pGraphics->LoadBitmapEx("abm",apFile);
        if (!bmp.IsOK()) {
          niError(niFmt(_A("Can't load bitmap for packer '%d'."),i));
          Clear();
          return eFalse;
        }

        QPtr<iBitmap2D> targetBmp = page.packer->_GetTargetBitmap(sRecti::Null());
        if (!targetBmp.IsOK()) {
          niError(niFmt(_A("Can't get the target bitmap.")));
          Clear();
          return eFalse;
        }

        if (!bmp->GetPixelFormat()->IsSamePixelFormat(targetBmp->GetPixelFormat())) {
          niError(niFmt(_A("Mismatch of the number of pixel format: loaded '%s', page is '%s'."),
                        bmp->GetPixelFormat()->GetFormat(),
                        targetBmp->GetPixelFormat()->GetFormat()));
          Clear();
          return eFalse;
        }
        if (bmp->GetNumMipMaps() != targetBmp->GetNumMipMaps()) {
          niError(niFmt(_A("Mismatch of the number of mipmaps: loaded '%d', page is '%d'."),
                        bmp->GetNumMipMaps(),targetBmp->GetNumMipMaps()));
          Clear();
          return eFalse;
        }
        if (bmp->GetWidth() != targetBmp->GetWidth()) {
          niError(niFmt(_A("Mismatch of the bitmap width: loaded '%d', page is '%d'."),
                        bmp->GetWidth(),targetBmp->GetWidth()));
          Clear();
          return eFalse;
        }
        if (bmp->GetHeight() != targetBmp->GetHeight()) {
          niError(niFmt(_A("Mismatch of the bitmap height: loaded '%d', page is '%d'."),
                        bmp->GetHeight(),targetBmp->GetHeight()));
          Clear();
          return eFalse;
        }

        niLoop(j,bmp->GetNumMipMaps()+1) {
          Ptr<iBitmap2D> srcLevel = bmp->GetLevel(j);
          if (!srcLevel.IsOK()) {
            niError(niFmt(_A("Can't get level '%d' of loaded bitmap."),j));
            Clear();
            return eFalse;
          }
          Ptr<iBitmap2D> dstLevel = targetBmp->GetLevel(j);
          if (!dstLevel.IsOK()) {
            niError(niFmt(_A("Can't get level '%d' of target bitmap."),j));
            Clear();
            return eFalse;
          }
          dstLevel->Blit(srcLevel,0,0,0,0,srcLevel->GetWidth(),srcLevel->GetHeight());
        }

        const tBool abUseMipMaps = (mnPageMipMaps > 0);
        tU32 numRects = apFile->ReadLE32();
        niLoop(j,numRects) {
          sRecti rect;
          apFile->ReadLE32Array((tU32*)rect.ptr(),4);
          if (page.packer->InsertRect(rect,PACKER_INSERT_PADDING) == sRecti::Null()) {
            niError(niFmt(_A("Can't insert rect '%d' to texture for packer '%d'."),j,i));
            Clear();
            return eFalse;
          }
        }
        mvPages.push_back(page);
      }

      // Images
      niLoop(i,numImages) {
        tHStringPtr hspName = _H(apFile->ReadBitsString());
        sRectf mapping;
        apFile->ReadF32Array(mapping.ptr(),4);
        tBool bFiltering = apFile->Read8();
        tU32 blendMode = apFile->ReadLE32();
        tU32 pageIndex = apFile->ReadLE32();
        if (pageIndex >= mvPages.size()) {
          niError(niFmt(_A("Invalid page index '%d' when loading image '%d."),pageIndex,i));
          Clear();
          return eFalse;
        }
        const sPage& page = mvPages[pageIndex];
        Ptr<iOverlay> image = mptrGraphics->CreateOverlayImage(page.packer->GetImage());
        image->SetFiltering(bFiltering);
        image->SetBlendMode((eBlendMode)blendMode);
        image->SetMapping(mapping);
        _AddImageEx(hspName,image);
      }

      // done
      mbShouldSerialize = eFalse;
    }
    /// Write //////////////////////////////////////////
    else if (niFlagIs(aFlags,eImageMapSerializeFlags_Write)) {
      // Header
      apFile->WriteStringZ(_A("niImageMap"));
      apFile->BeginWriteBits();
      apFile->WriteBit(mbDefaultImageFiltering);
      apFile->WriteBitsPackedU32(mDefaultImageBlendMode);
      apFile->WriteBitsPackedU32(mnMaxNumPages);
      apFile->WriteBitsPackedU32(mnPageSize);
      apFile->WriteBitsPackedU32(mnPageMipMaps);
      apFile->WriteBitsPackedU32(mvPages.size());
      apFile->WriteBitsPackedU32(mmapImages.size());
      apFile->WriteBitsString(niHStr(mhspPageFormat));
      apFile->EndWriteBits();
      apFile->WriteStringZ(_A("niImageMap"));

      // Pages
      niLoop(i,mvPages.size()) {
        const sPage& p = mvPages[i];
        Ptr<iBitmap2D> bmp = p.packer->_GetSourceBitmap();
        if (!pGraphics->SaveBitmapEx(_A("abm"),apFile,bmp,0)) {
          niError(niFmt(_A("Can't save packer '%d'."),i));
          return eFalse;
        }
        apFile->WriteLE32(p.packer->GetNumRects());
        niLoop(j,p.packer->GetNumRects()) {
          apFile->WriteLE32Array((tU32*)p.packer->GetRect(j).ptr(),4);
        }
      }

      // Images
      niLoopit(tImageMap::iterator,it,mmapImages) {
        iHString* name = it->first;
        iOverlay* image = it->second;
        apFile->WriteBitsString(niHStr(name));
        apFile->WriteF32Array(image->GetMapping().ptr(),4);
        apFile->Write8(image->GetFiltering());
        apFile->WriteLE32(image->GetBlendMode());
        tU32 pageIndex = eInvalidHandle;
        niLoop(i,mvPages.size()) {
          const sPage& p = mvPages[i];
          if (p.packer->GetImage() == image->GetImage()) {
            pageIndex = i;
            break;
          }
        }
        if (pageIndex == eInvalidHandle) {
          niError(niFmt(_A("Can't find page index for image '%s'"),
                        niHStr(name)));
        }
        apFile->WriteLE32(pageIndex);
      }

      // done
      mbShouldSerialize = eFalse;
    }
    else {
      niError(_A("Invalid serialization flags."));
      return eFalse;
    }
    return eTrue;
  }

 private:
  Ptr<iGraphics> mptrGraphics;
  tBool          mbDefaultImageFiltering;
  eBlendMode     mDefaultImageBlendMode;
  tU32           mnMaxNumPages;
  tU32           mnPageSize;
  tU32           mnPageMipMaps;
  tHStringPtr    mhspPageFormat;
  tBool          mbShouldSerialize;
  tBool          mbComputeMipMapsPerPage;

  typedef astl::vector<Ptr<iOverlay> >            tImageVec;
  typedef astl::hstring_hash_map<Ptr<iOverlay> >  tImageMap;
  tImageVec mvImages;
  tImageMap mmapImages;

  typedef astl::vector<sPage> tPageVec;
  tPageVec                    mvPages;

  tHStringPtr mhspPxf;
  tHStringPtr mhspBaseName;

  niEndClass(cImageMap);
};

///////////////////////////////////////////////
iImageMap* __stdcall cGraphics::CreateImageMap(const achar* aaszBaseName, const achar* aaszPxf)
{
  return niNew cImageMap(this,aaszBaseName,aaszPxf);
}
