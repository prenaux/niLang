// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "API/niUI_ModuleDef.h"

const tU32 _kfccMatLib0 = niFourCC('A','M','L','0');
const tU32 _kfccMatLib1 = niFourCC('A','M','L','1');
const tU32 _kfccMatTex = niFourCC('A','M','T','X');

//////////////////////////////////////////////////////////////////////////////////////////////
class cMaterialLibrary : public cIUnknownImpl<ni::iMaterialLibrary,eIUnknownImplFlags_Default>
{
  niBeginClass(cMaterialLibrary);

  struct sMaterial {
    ni::Ptr<ni::iMaterial>  _mat;
    sMaterial(ni::iMaterial* apMaterial) {
      _mat = apMaterial;
    }
    bool operator == (const sMaterial& m) const {
      return _mat == m._mat;
    }
  };
  astl::vector<sMaterial> mvMaterials;

  struct sTexture {
    tHStringPtr             _baseName;
    cString                 _finalName;
    Ptr<iTexture>           _tex;
    tTextureFlags           _texFlags;
    tMaterialSerializeFlags _matSerFlags;
    tBool                   _appliedChannel[eMaterialChannel_Last];

    sTexture() {
      _texFlags = 0;
      _matSerFlags = 0;
      ni::MemZero((tPtr)_appliedChannel,sizeof(_appliedChannel));
    }

    ///////////////////////////////////////////////
    tBool Read0(iGraphics* apGraphics, iFile* apFile) {
      _finalName = apFile->ReadBitsString();
      _texFlags = apFile->ReadLE32();

      //             niWarning(niFmt(_A("# READ BITMAP AT %d\n"),apFile->Tell()));

      Ptr<iBitmapBase> bmp = apGraphics->LoadBitmapEx("abm",apFile);
      niCheck(bmp.IsOK(),eFalse);
      _tex = apGraphics->CreateTextureFromBitmap(_H(_finalName),bmp,_texFlags);
      if (!_tex.IsOK()) {
        _tex = NULL;
      }

      //             niWarning(niFmt(_A("# READ BITMAP END AT %d\n"),apFile->Tell()));

      return eTrue;
    }

    ///////////////////////////////////////////////
    void InitWrite(iTexture* apTexture, iHString* ahspName, tMaterialSerializeFlags aFlags) {
      _tex = apTexture;
      _baseName = ahspName;
      _matSerFlags = aFlags;
      // Compression ratio...
      tU32 compression = eMaterialSerializeFlags_GetTextureCompression(aFlags);
      // Resize to max res ?
      tU32 maxRes = eMaterialSerializeFlags_GetTextureMaxResIndex(aFlags);
      if (maxRes) {
        maxRes = 1<<maxRes;
        // Check whether the bitmap needs to be resized, if not 'disable' maxRes
        if (maxRes && _tex->GetWidth() <= maxRes && _tex->GetHeight() <= maxRes)
          maxRes = 0;
      }
      // Generate the final texture name from the max res and compression parameters
      _finalName = niHStr(_baseName);
      if (maxRes || compression) _finalName << "_";
      if (compression) _finalName << "c" << compression;
      if (maxRes)      _finalName << "r" << maxRes;
    }

    ///////////////////////////////////////////////
    tBool WriteRaw(iGraphics* apGraphics, iFile* apFile) {
      niCheck(_tex.IsOK(),eFalse);

      Ptr<iBitmapBase> bmp = apGraphics->CreateBitmapFromTexture(_tex);
      niCheck(bmp.IsOK(),eFalse);

      // Compression ratio...
      tU32 compression = eMaterialSerializeFlags_GetTextureCompression(_matSerFlags);
      // Resize to max res ?
      tU32 maxRes = eMaterialSerializeFlags_GetTextureMaxResIndex(_matSerFlags);
      if (maxRes) {
        maxRes = 1<<maxRes;
        // Check whether the bitmap needs to be resized, if not 'disable' maxRes
        if (maxRes && bmp->GetWidth() <= maxRes && bmp->GetHeight() <= maxRes)
          maxRes = 0;
      }

      //             niWarning(niFmt(_A("# WRITE BITMAP AT %d[%08x]\n"),(tU32)apFile->Tell(),(tU32)apFile->Tell()));
      apFile->WriteLE32(_kfccMatTex);
      apFile->WriteStringZ(_finalName.Chars());
      apFile->WriteLE32(_texFlags);

      // Remove mipmaps, do it first so that if we resize we
      // don't needlessly recompute them...
      if (niFlagIs(_matSerFlags,eMaterialSerializeFlags_TextureNoMipMaps)) {
        bmp->RemoveMipMaps();
      }

      // Resize to max res ?
      if (maxRes) {
        if (bmp->GetPixelFormat()->GetCaps()&ePixelFormatCaps_BlockCompressed) {
          // We can't directly resize compressed textures, so we unpack to
          // RGB(A)8 first...
          Ptr<iPixelFormat> pxf = apGraphics->CreatePixelFormat(
              bmp->GetPixelFormat()->GetNumABits() > 1 ?
              _A("R8G8B8A8"):_A("R8G8B8"));
          bmp = bmp->CreateConvertedFormat(pxf);
          niCheckSilent(bmp.IsOK(),eFalse);
        }

        tF32 ratio = 1.0f;
        if (bmp->GetWidth() > bmp->GetHeight()) {
          ratio = (tF32)bmp->GetWidth()/(tF32)maxRes;
        }
        else {
          ratio = (tF32)bmp->GetHeight()/(tF32)maxRes;
        }
        tU32 newW = ni::GetNearestPow2((tU32)((tF32)bmp->GetWidth()/ratio),eTrue);
        tU32 newH = ni::GetNearestPow2((tU32)((tF32)bmp->GetHeight()/ratio),eTrue);
        tU32 newD = bmp->GetDepth();

        switch (bmp->GetType()) {
          case eBitmapType_2D: {
            bmp = ((iBitmap2D*)bmp.ptr())->CreateResized(newW,newH);
            break;
          }
          case eBitmapType_Cube: {
            bmp = ((iBitmapCube*)bmp.ptr())->CreateResized(newW);
            break;
          }
          case eBitmapType_3D: {
            bmp = ((iBitmap3D*)bmp.ptr())->CreateResized(newW,newH,newD);
            break;
          }
        }
        niCheck(bmp.IsOK(),eFalse);
      }

      if (!apGraphics->SaveBitmapEx(_A("abm"),apFile,bmp,compression)) {
        niError(_A("Can't write bitmap."));
        return eFalse;
      }

      return eTrue;
    }

    ///////////////////////////////////////////////
    tBool Read1(iGraphics* apGraphics, iFile* apFile) {
      //             niWarning(niFmt(_A("# READ BITMAP AT %d[%08x]\n"),(tU32)apFile->Tell(),(tU32)apFile->Tell()));
      if (apFile->ReadLE32() != _kfccMatTex) {
        niError(_A("Invalid texture FourCC."));
        return eFalse;
      }
      _finalName = apFile->ReadString();
      _texFlags = apFile->ReadLE32();

      //             tU32 size = apFile->ReadLE32();
      //      niWarning(niFmt(_A("# READ BITMAP SIZE: %d\n"),size));
      //             Ptr<iFile> fp = ni::CreateFileDynamicMemory(size,NULL);
      //             fp->WriteFile(apFile,size);
      //             fp->SeekSet(0);

      Ptr<iBitmapBase> bmp = apGraphics->LoadBitmapEx("abm",apFile);
      niCheck(bmp.IsOK(),eFalse);

      if (!bmp->GetNumMipMaps() && IsPow2(bmp->GetWidth()) && IsPow2(bmp->GetHeight())) {
        bmp->CreateMipMaps(0,eTrue);
      }

      _tex = apGraphics->CreateTextureFromBitmap(_H(_finalName),bmp,_texFlags);
      if (!_tex.IsOK()) {
        _tex = NULL;
      }

      //             niWarning(niFmt(_A("# READ BITMAP END AT %d[%08x]\n"),(tU32)apFile->Tell(),(tU32)apFile->Tell()));
      return eTrue;
    }
  };

  typedef astl::hstring_hash_map<sTexture>  tTextureMap;

  Nonnull<tMaterialLibrarySinkLst> _sinkList;
  Ptr<iGraphics>                   _graphics;
  tHStringPtr                      mhspBasePath;

 public:
  ///////////////////////////////////////////////
  cMaterialLibrary(iGraphics* apGraphics) :
      _sinkList(tMaterialLibrarySinkLst::Create())
  {
    ZeroMembers();
    _graphics = apGraphics;
  }

  ///////////////////////////////////////////////
  ~cMaterialLibrary() {
    Invalidate();
  }

  ///////////////////////////////////////////////
  void __stdcall Invalidate() {
    ClearMaterials();
  }

  ///////////////////////////////////////////////
  void __stdcall ZeroMembers() {
  }

  ///////////////////////////////////////////////
  ni::tBool __stdcall IsOK() const {
    niClassIsOK(cMaterialLibrary);
    return ni::eTrue;
  }

  ///////////////////////////////////////////////
  virtual tMaterialLibrarySinkLst* __stdcall GetSinkList() const {
    return _sinkList;
  }

  ///////////////////////////////////////////////
  virtual ni::tBool __stdcall Copy(const iMaterialLibrary* apSrc, tBool abCloneMats) {
    niCheckSilent(niIsOK(apSrc),eFalse);
    ClearMaterials();
    niLoop(i,apSrc->GetNumMaterials()) {
      Ptr<iMaterial> m = apSrc->GetMaterial(i);
      if (m.IsOK()) {
        if (abCloneMats) m = m->Clone();
        AddMaterial(m);
      }
    }
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual iMaterialLibrary* __stdcall Clone(tBool abCloneMats) const {
    cMaterialLibrary* pNew = niNew cMaterialLibrary(_graphics);
    pNew->Copy(this,abCloneMats);
    return pNew;
  }

  ///////////////////////////////////////////////
  virtual ni::tBool __stdcall ClearMaterials() {
    while (GetNumMaterials()) {
      RemoveMaterial(GetMaterial(GetNumMaterials()-1));
    }
    return ni::eTrue;
  }
  ///////////////////////////////////////////////
  virtual ni::tBool __stdcall AddMaterial(ni::iMaterial* apMaterial) {
    niCheckIsOK(apMaterial,ni::eFalse);
    if (astl::push_back_once(mvMaterials,apMaterial)) {
      niCallSinkVoid_(MaterialLibrarySink,_sinkList,MaterialAdded,(this,apMaterial));
      return eTrue;
    }
    return eFalse;
  }
  ///////////////////////////////////////////////
  virtual ni::tBool __stdcall RemoveMaterial(ni::iMaterial* apMaterial) {
    niCheckIsOK(apMaterial,ni::eFalse);

    ni::tU32 nIndex = GetMaterialIndex(apMaterial);
    if (nIndex == ni::eInvalidHandle)
      return ni::eFalse;

    ni::Ptr<ni::iMaterial> m = apMaterial;
    mvMaterials.erase(mvMaterials.begin()+nIndex);
    niCallSinkVoid_(MaterialLibrarySink,_sinkList,MaterialRemoved,(this,apMaterial));
    return ni::eTrue;
  }

  ///////////////////////////////////////////////
  virtual ni::tU32 __stdcall GetNumMaterials() const {
    return mvMaterials.size();
  }
  ///////////////////////////////////////////////
  virtual ni::iMaterial* __stdcall GetMaterial(ni::tU32 anIndex) const {
    if (anIndex >= mvMaterials.size())
      return NULL;
    return mvMaterials[anIndex]._mat;
  }
  ///////////////////////////////////////////////
  virtual ni::iMaterial* __stdcall GetMaterialFromName(ni::iHString* ahspName) const {
    niLoop(i,mvMaterials.size()) {
      if (mvMaterials[i]._mat->GetName() == ahspName)
        return mvMaterials[i]._mat;
    }
    return NULL;
  }
  ///////////////////////////////////////////////
  virtual ni::tU32 __stdcall GetMaterialIndex(ni::iMaterial* apMaterial) const {
    niLoop(i,mvMaterials.size()) {
      if (mvMaterials[i]._mat == apMaterial)
        return i;
    }
    return ni::eInvalidHandle;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetBasePath(iHString* ahspBasePath) {
    mhspBasePath = ahspBasePath;
  }
  virtual iHString* __stdcall GetBasePath() const {
    return mhspBasePath;
  }

  ///////////////////////////////////////////////
  virtual ni::tBool __stdcall SerializeDataTable(ni::iDataTable* apDT, tMaterialSerializeFlags anFlags) {
    if (!niIsOK(apDT)) {
      niError(_A("Invalid data table."));
      return ni::eFalse;
    }
    if (!ni::StrEq(apDT->GetName(),_A("Materials"))) {
      niError(_A("Not a Materials data table."));
      return ni::eFalse;
    }

    ///// READ /////
    if (niFlagIs(anFlags,eMaterialSerializeFlags_Read)) {
      niLoop(i,apDT->GetNumChildren()) {
        ni::Ptr<ni::iDataTable> c = apDT->GetChildFromIndex(i);
        if (ni::StrCmp(c->GetName(),_A("Material")) != 0)
          continue;

        ni::tHStringPtr hspName = _H(c->GetString(_A("name")));
        if (ni::HStringIsEmpty(hspName)) {
          continue;
        }

        ni::Ptr<ni::iMaterial> m = GetMaterialFromName(hspName);
        if (!m.IsOK()) {
          if (niFlagIs(anFlags,eMaterialSerializeFlags_ReadExistingOnly)) {
            // material doesn't exist, we don't it to be created by the serialization
            continue;
          }
          m = _graphics->CreateMaterial();
          if (!m.IsOK()) {
            niWarning(niFmt(_A("Can't find nor create material '%s'."),niHStr(hspName)));
            continue;
          }
          m->SetName(hspName);
          AddMaterial(m);
        }

        { // Read the material from the datatable
          if (!m->Serialize(ni::eSerializeMode_Read,c,mhspBasePath)) {
            niWarning(niFmt(_A("Can't serialize read material '%s'."),niHStr(hspName)));
            continue;
          }
        }
      }
    }
    ///// WRITE /////
    else {
      ni::Ptr<ni::iDataTableWriteStack> dt = ni::CreateDataTableWriteStack(apDT);
      niLoop(i,GetNumMaterials()) {
        dt->PushNew(_A("Material"));
        if (anFlags & eMaterialSerializeFlags_NamesOnly) {
          dt->SetHString(_A("name"),mvMaterials[i]._mat->GetName());
        }
        else {
          // Write the material to the datatable
          mvMaterials[i]._mat->Serialize(ni::eSerializeMode_Write,dt->GetTop(),mhspBasePath);
        }
        dt->Pop();
      }
    }

    return ni::eTrue;
  }

  ///////////////////////////////////////////////
  virtual ni::tBool __stdcall SerializeFile(ni::iFile* apFile, tMaterialSerializeFlags aFlags) {
    niCheckIsOK(apFile,eFalse);

    //// WRITE ////
    if (niFlagIs(aFlags,eMaterialSerializeFlags_Write)) {
      tTextureMap mapTexs;
      if (niFlagIs(aFlags,eMaterialSerializeFlags_Textures))
      {
        // build the set of textures
        niLoop(i,mvMaterials.size()) {
          iMaterial* m = mvMaterials[i]._mat;
          niLoop(j,eMaterialChannel_Last) {
            iTexture* tex = m->GetChannelTexture((eMaterialChannel)j);
            if (niIsOK(tex)) {
              tTextureMap::iterator it = mapTexs.find(tex->GetDeviceResourceName());
              if (it != mapTexs.end()) {
                it->second._appliedChannel[j] = eTrue;
              }
              else {
                sTexture t;
                t._baseName = tex->GetDeviceResourceName();
                t._tex = tex;
                t._texFlags = tex->GetFlags();
                t.InitWrite(tex,t._baseName,aFlags);
                t._appliedChannel[j] = eTrue;
                astl::upsert(mapTexs,t._baseName,t);
              }
            }
          }
        }
      }

      // Write the datatable
      {
        Ptr<iDataTable> dt = ni::CreateDataTable(_A("Materials"));
        SerializeDataTable(dt,aFlags);
        dt->SetEnum(_A("serialize_flags"),niFlagsExpr(eMaterialSerializeFlags),aFlags);
        // patch the texture names...
        if (!mapTexs.empty())
        {
          niLoop(i,mvMaterials.size()) {
            Ptr<iDataTable> dtMat = dt->GetChildFromIndex(i);
            if (!dtMat.IsOK()) {
              niError(niFmt("Can't get child table of material '%d'.",i));
              return eFalse;
            }
            Ptr<iDataTable> dtChans = dtMat->GetChild("Channels");
            if (!dtChans.IsOK()) {
              niError(niFmt("Can't get Channels table of material '%d'.",i));
              return eFalse;
            }
            iMaterial* m = mvMaterials[i]._mat;
            niLoop(j,eMaterialChannel_Last) {
              iTexture* tex = m->GetChannelTexture((eMaterialChannel)j);
              if (!niIsOK(tex)) continue;
              tTextureMap::iterator it = mapTexs.find(tex->GetDeviceResourceName());
              if (it != mapTexs.end()) {
                Ptr<iDataTable> dtCh = dtChans->GetChildFromIndex(j);
                if (!dtCh.IsOK()) {
                  niError(niFmt("Can't get Channel[%d] table of material '%d'.",j,i));
                  return eFalse;
                }
                dtCh->SetString("texture",it->second._finalName.Chars());
              }
            }
          }
        }
        // serialize
        if (!ni::SerializeDataTable(_A("dtz"),eSerializeMode_Write,dt,apFile)) {
          niError(_A("Can't write the data table."));
          return eFalse;
        }
      }
      // Textures
      if (niFlagIs(aFlags,eMaterialSerializeFlags_Textures))
      {
        apFile->WriteLE32(_kfccMatLib1);
        // write the number of textures
        apFile->WriteLE32(mapTexs.size());
        // write the textures
        niLoopit(tTextureMap::iterator,it,mapTexs) {
          sTexture& t = it->second;
          if (!t.WriteRaw(_graphics,apFile)) {
            niError(niFmt(_A("Can't write texture '%s'."),niHStr(t._baseName)));
            return eFalse;
          }
        }
      }
    }
    //// READ /////
    else if (niFlagIs(aFlags,eMaterialSerializeFlags_Read)) {

      // Read the datatable
      Ptr<iDataTable> dt = ni::CreateDataTable(_A("Materials"));
      {
        if (!ni::SerializeDataTable(_A("dtz"),eSerializeMode_Read,dt,apFile)) {
          niError(_A("Can't read the data table."));
          return eFalse;
        }
        if (!ni::StrEq(dt->GetName(),_A("Materials"))) {
          niError(_A("Not a Materials data table."));
          return ni::eFalse;
        }
      }

      // Texture map, keeps references to loaded textures
      tTextureMap mapTexs;
      // Get the serialize flags in the dt
      cString str = dt->GetString(_A("serialize_flags"));
      tMaterialSerializeFlags dtFlags = dt->GetEnum(_A("serialize_flags"),niFlagsExpr(eMaterialSerializeFlags));

      // Load the textures
      if (niFlagIs(dtFlags,eMaterialSerializeFlags_Textures))
      {
        const tU32 matLibVer = apFile->ReadLE32();
        switch (matLibVer) {
          case _kfccMatLib0: {
            tU32 numTexs = apFile->ReadLE32();
            niLoop(i,numTexs) {
              sTexture t;
              if (!t.Read0(_graphics,apFile)) {
                niError(niFmt(_A("Can't read texture '%d' (%s)."),i,t._finalName));
                return eFalse;
              }
              astl::upsert(mapTexs,_H(t._finalName),t);
            }
            break;
          }
          case _kfccMatLib1: {
            tU32 numTexs = apFile->ReadLE32();
            niLoop(i,numTexs) {
              sTexture t;
              if (!t.Read1(_graphics,apFile)) {
                niError(niFmt(_A("Can't read texture '%d' (%s)."),i,t._finalName));
                return eFalse;
              }
              astl::upsert(mapTexs,_H(t._finalName),t);
            }
            break;
          }
          default:
            niError(_A("Invalid material library fourCC."));
            return eFalse;
        }
      }
      {
        // Load the datatable
        if (!SerializeDataTable(dt,aFlags)) {
          niError(_A("Can't load the material library from the datatable."));
          return eFalse;
        }
      }
    }
    else {
      niError(_A("Invalid serialize flags."));
      return ni::eFalse;
    }

    return eTrue;
  }

  niEndClass(cMaterialLibrary);
};

iMaterialLibrary* __stdcall New_MaterialLibrary(iGraphics* apGraphics) {
  return niNew cMaterialLibrary(apGraphics);
}
