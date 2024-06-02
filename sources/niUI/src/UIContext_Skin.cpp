// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "UIContext.h"
#include "API/niUI_ModuleDef.h"

///////////////////////////////////////////////
tBool __stdcall cUIContext::SetErrorOverlay(iOverlay* apOverlay) {
  if (niIsOK(apOverlay)) {
    mptrErrorOverlay = apOverlay;
  }
  else {
    mptrErrorOverlay = mptrGraphics->CreateOverlayColor(sColor4f::Pink());
    mptrErrorOverlay->SetSize(Vec2<tF32>(16,16));
  }
  return eTrue;
}

///////////////////////////////////////////////
iOverlay* __stdcall cUIContext::GetErrorOverlay() const {
  return mptrErrorOverlay;
}

///////////////////////////////////////////////
void __stdcall cUIContext::ClearSkins()
{
  mmapSkins.clear();
}

///////////////////////////////////////////////
tBool __stdcall cUIContext::AddSkin(iDataTable* apDT)
{
  if (!niIsOK(apDT)) {
    niError(_A("Invalid datatable."));
    return eFalse;
  }

  if (!ni::StrEq(apDT->GetName(),_A("UISkin"))) {
    niError(_A("Not a UISkin data table."));
    return eFalse;
  }

  cString strName = apDT->GetString(_A("name"));
  if (strName.IsEmpty()) {
    niError(_A("No name defined in UISkin."));
    return eFalse;
  }

  if (!_InitializeSkinDataTable(apDT)) {
    niError(_A("Can't initialize the skin data table."));
    return eFalse;
  }

  // niDebugFmt(("... Added skin: %s", strName));
  astl::upsert(mmapSkins,_H(strName.Chars()),apDT);
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cUIContext::AddSkinFromRes(iHString* ahspRes)
{
  Ptr<iFile> fp = ni::GetLang()->URLOpen(niHStr(ahspRes));
  if (!fp.IsOK()) {
    niError(niFmt(_A("Can't open the skin resource '%s'."),niHStr(ahspRes)));
    return eFalse;
  }

  Ptr<iDataTable> ptrDT = ni::CreateDataTable(_A("UISkin"));
  if (!SerializeDataTable(NULL,eSerializeMode_Read,ptrDT,fp)) {
    niError(niFmt(_A("Can't read skin datatable from resource '%s'."),niHStr(ahspRes)));
    return eFalse;
  }
  ptrDT->SetHString("res_path", ahspRes);

  if (!AddSkin(ptrDT)) {
    niError(niFmt(_A("Can't add skin from resource '%s'."),niHStr(ahspRes)));
    return eFalse;
  }

  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cUIContext::RemoveSkin(iHString* ahspSkin)
{
  tSkinMap::iterator it = mmapSkins.find(ahspSkin);
  if (it == mmapSkins.end()) return eFalse;
  mmapSkins.erase(it);
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cUIContext::SetDefaultSkin(iHString* ahspName)
{
  tU32 nSkinIndex = GetSkinIndex(ahspName);
  if (nSkinIndex == eInvalidHandle) {
    niError(niFmt(_A("Can't find skin '%s' to set as default."),niHStr(ahspName)));
    return eFalse;
  }
  mhspDefaultSkin = ahspName;
  return eTrue;
}

///////////////////////////////////////////////
static void _ApplySkin(iWidget* apWidget, iHString* ahspName) {
  niLoop(i,apWidget->GetNumChildren()) {
    _ApplySkin(apWidget->GetChildFromIndex(i),ahspName);
  }
  apWidget->SetSkin(ahspName);
}
tBool __stdcall cUIContext::ApplySkin(iWidget* apWidget, iHString* ahspName)
{
  niCheckIsOK(apWidget,eFalse);

  tU32 nSkinIndex = GetSkinIndex(ahspName);
  if (nSkinIndex == eInvalidHandle) {
    niError(niFmt(_A("Can't find skin '%s'."),niHStr(ahspName)));
    return eFalse;
  }

  _ApplySkin(apWidget,ahspName);
  return eTrue;
}

///////////////////////////////////////////////
iHString* __stdcall cUIContext::GetDefaultSkin() const
{
  return mhspDefaultSkin;
}

///////////////////////////////////////////////
tU32 __stdcall cUIContext::GetNumSkins() const
{
  return (tU32)mmapSkins.size();
}

///////////////////////////////////////////////
iHString* __stdcall cUIContext::GetSkinName(tU32 anIndex) const
{
  tU32 nCount = 0;
  for (tSkinMap::const_iterator it = mmapSkins.begin(); it != mmapSkins.end(); ++it, ++nCount) {
    if (anIndex == nCount)
      return it->first;
  }
  return NULL;
}

///////////////////////////////////////////////
tU32 __stdcall cUIContext::GetSkinIndex(iHString* ahspName) const{
  tU32 nCount = 0;
  for (tSkinMap::const_iterator it = mmapSkins.begin(); it != mmapSkins.end(); ++it, ++nCount) {
    if (it->first == ahspName)
      return nCount;
  }
  return eInvalidHandle;
}

///////////////////////////////////////////////
iDataTable* __stdcall cUIContext::GetSkinDataTable(iHString* ahspSkin) const
{
  tSkinMap::const_iterator it = mmapSkins.find(ahspSkin);
  if (it == mmapSkins.end()) {
    // try the default skin
    it = mmapSkins.find(mhspDefaultSkin);
    if (it == mmapSkins.end()) {
      return mptrErrorSkin;
    }
  }
  return it->second;
}

///////////////////////////////////////////////
cString _GetSkinFontPath(const iHString* ahspClass, const iHString* ahspState, const iHString* ahspName)
{
  cString r;
  r += _A("/");
  r += ni::HStringIsNotEmpty(ahspClass) ? niHStr(ahspClass) : _A("Default");
  r += _A("/");
  r += ni::HStringIsNotEmpty(ahspState) ? niHStr(ahspState) : _A("Normal");
  r += _A("/Fonts/");
  r += niHStr(ahspName);
  r += _A("/@_font");
  return r;
}
iFont* __stdcall cUIContext::FindSkinFont(iHString* ahspSkin, iHString* ahspClass, iHString* ahspState, iHString* ahspName) const
{
  QPtr<iFont> font;
  Ptr<iDataTable> ptrDT = GetSkinDataTable(ahspSkin);
  if (ptrDT.IsOK() && ptrDT != mptrErrorSkin) {
    // Class/State/"Fonts"/Name
    if (!font.IsOK()) {
      font = ptrDT->GetVarFromPath(_GetSkinFontPath(ahspClass,ahspState,ahspName).Chars(),niVarNull);
    }

    // Class/"Normal"/"Fonts"/Name
    if (!font.IsOK()) {
      font = ptrDT->GetVarFromPath(_GetSkinFontPath(ahspClass,NULL,ahspName).Chars(),niVarNull);
    }

    // "Default"/State/"Fonts"/Name
    if (!font.IsOK()) {
      font = ptrDT->GetVarFromPath(_GetSkinFontPath(NULL,ahspState,ahspName).Chars(),niVarNull);
    }

    // "Default"/"Normal"/"Fonts"/Name
    if (!font.IsOK()) {
      font = ptrDT->GetVarFromPath(_GetSkinFontPath(NULL,NULL,ahspName).Chars(),niVarNull);
    }
  }

  if (!font.IsOK()) {
    font = mptrGraphics->LoadFont(ahspName);
    font->SetSizeAndResolution(Vec2f(22,22), 64, GetContentsScale());
  }

  if (!font.IsOK()) {
    font = mptrGraphics->LoadFont(_H("Default"));
  }

  niAssert(font.IsOK());
  return font.GetRawAndSetNull();
}

///////////////////////////////////////////////
cString _GetSkinCursorPath(const iHString* ahspClass, const iHString* ahspName)
{
  cString r;
  r += _A("/");
  r += ni::HStringIsNotEmpty(ahspClass) ? niHStr(ahspClass) : _A("Default");
  r += _A("/Cursors/");
  r += niHStr(ahspName);
  r += "/@_overlay";
  return r;
}
iOverlay* __stdcall cUIContext::FindSkinCursor(iHString* ahspSkin, iHString* ahspClass, iHString* ahspState, iHString* ahspName) const
{
  Ptr<iDataTable> ptrDT = GetSkinDataTable(ahspSkin);
  if (ptrDT.IsOK() && ptrDT != mptrErrorSkin) {
    cString wa;

    QPtr<iOverlay> overlay;

    // Class/"Cursors"/Name
    overlay = ptrDT->GetVarFromPath(_GetSkinCursorPath(ahspClass,ahspName).Chars(), niVarNull);
    if (overlay.IsOK())
      return overlay;

    // "Default"/"Cursors"/Name
    overlay = ptrDT->GetVarFromPath(_GetSkinCursorPath(NULL,ahspName).Chars(), niVarNull);
    if (overlay.IsOK())
      return overlay;
  }
  return mptrErrorOverlay;
}

///////////////////////////////////////////////
cString _GetSkinElementPath(const iHString* ahspClass, const iHString* ahspState, const iHString* ahspName)
{
  cString r;
  r += _A("/");
  r += ni::HStringIsNotEmpty(ahspClass) ? niHStr(ahspClass) : _A("Default");
  r += _A("/");
  r += ni::HStringIsNotEmpty(ahspState) ? niHStr(ahspState) : _A("Normal");
  r += _A("/");
  r += niHStr(ahspName);
  r += "/@_overlay";
  return r;
}
iOverlay* __stdcall cUIContext::FindSkinElement(iHString* ahspSkin, iHString* ahspClass, iHString* ahspState, iHString* ahspName) const {
  Ptr<iDataTable> ptrDT = GetSkinDataTable(ahspSkin);
  if (ptrDT.IsOK() && ptrDT != mptrErrorSkin) {
    QPtr<iOverlay> overlay;

    // Class/State/Name
    overlay = ptrDT->GetVarFromPath(_GetSkinElementPath(ahspClass, ahspState, ahspName).Chars(), niVarNull);
    if (overlay.IsOK())
      return overlay;

    // Class/"Normal"/Name
    overlay = ptrDT->GetVarFromPath(_GetSkinElementPath(ahspClass, NULL, ahspName).Chars(), niVarNull);
    if (overlay.IsOK())
      return overlay;

    // "Default"/State/Name
    overlay = ptrDT->GetVarFromPath(_GetSkinElementPath(NULL,ahspState,ahspName).Chars(), niVarNull);
    if (overlay.IsOK())
      return overlay;

    // "Default"/"Normal"/Name
    overlay = ptrDT->GetVarFromPath(_GetSkinElementPath(NULL,NULL,ahspName).Chars(), niVarNull);
    if (overlay.IsOK())
      return overlay;
  }

  return mptrErrorOverlay;
}

///////////////////////////////////////////////
sColor4f __stdcall cUIContext::FindSkinColor(const sColor4f& aDefault, iHString* ahspSkin, iHString* ahspClass, iHString* ahspState, iHString* ahspName) const
{
  iOverlay* pOvr = FindSkinElement(ahspSkin,ahspClass,ahspState,ahspName);
  if (!pOvr || pOvr == mptrErrorOverlay)
    return aDefault;
  return pOvr->GetColor();
}

#define OVERLAYSOURCE_UNKNOWN 0
#define OVERLAYSOURCE_BITMAP  1
struct sOverlaySource {
  tU32 nType;
  tHStringPtr hspPath;
  sOverlaySource() {
    nType = OVERLAYSOURCE_UNKNOWN;
  }
};

typedef astl::hstring_map_cmp<Ptr<iImage> > tLoadedImageMap;

///////////////////////////////////////////////
static Ptr<iOverlay> _ReadOverlayDT(
    iGraphics* apGraphics,
    const sOverlaySource& aTopSource,
    iDataTable* dt,
    iMaterialLibrary* apMatLib,
    iHString* ahspClassMaterial,
    tLoadedImageMap& mapImages)
{
  tU32 idx;
  Ptr<iOverlay> ptrOvr;

  {
    Ptr<iHString> hspMatName;
    idx = dt->GetPropertyIndex(_A("material"));
    if (idx != eInvalidHandle) {
      hspMatName = dt->GetHStringFromIndex(idx);
    }
    else {
      hspMatName = ahspClassMaterial;
    }
    if (!HStringIsEmpty(hspMatName)) {
      if (apMatLib) {
        Ptr<iMaterial> ptrMaterial = apMatLib->GetMaterialFromName(hspMatName);
        if (ptrMaterial.IsOK()) {
          // niDebugFmt(("... UISkin: CreateOverlayMaterial: %s", hspMatName));
          ptrOvr = apGraphics->CreateOverlayMaterial(ptrMaterial);
        }
        else {
          niWarning(niFmt(
              "Skin can't find specified material '%s' in the skin's material library.",
              hspMatName));
        }
      }
      else {
        niWarning("Skin specifies a material without a material library defined.");
      }
    }
  }

  idx = dt->GetPropertyIndex(_A("mapping"));
  if (idx != eInvalidHandle) {
    if (!ptrOvr.IsOK() && HStringIsNotEmpty(aTopSource.hspPath)) {
      Ptr<iImage> img;
      tLoadedImageMap::iterator it = mapImages.find(aTopSource.hspPath);
      if (it == mapImages.end()) {
        // niDebugFmt(("... UISkin: CreateOverlayImage: Load: %s", aTopSource.hspPath));
        img = apGraphics->CreateImageFromResource(aTopSource.hspPath);
        astl::upsert(mapImages,aTopSource.hspPath,img);
      }
      else {
        img = it->second.ptr();
      }
      // niDebugFmt(("... UISkin: CreateOverlayImage: %s", aTopSource.hspPath));
      ptrOvr = apGraphics->CreateOverlayImage(img);
    }
    // mapping and frames make sense only if there's already an overlay...
    if (ptrOvr.IsOK()) {
      sRectf rect = dt->GetVec4Default(_A("mapping"),ptrOvr->GetMapping());
      ptrOvr->SetMapping(rect);
      ptrOvr->SetSize(rect.GetSize());
    }
    else {
      niWarning("Skin specifies a mapping without a bitmap or material setup.");
    }
  }
  if (!ptrOvr.IsOK()) {
    ptrOvr = apGraphics->CreateOverlayColor(sColor4f::White());
  }
  ptrOvr->SetPivot(dt->GetVec2Default(_A("pivot"),ptrOvr->GetPivot()));
  ptrOvr->SetSize(dt->GetVec2Default(_A("size"),ptrOvr->GetSize()));
  ptrOvr->SetBlendMode((eBlendMode)dt->GetEnumDefault(_A("blendmode"),niEnumExpr(eBlendMode),eBlendMode_NoBlending));
  ptrOvr->SetFiltering(dt->GetBoolDefault(_A("filtering"),ptrOvr->GetFiltering()));
  ptrOvr->SetFrame(dt->GetVec4Default(_A("frame"),ptrOvr->GetFrame()));

  ptrOvr->SetColor(dt->GetVec4Default(_A("color"),ptrOvr->GetColor()));

  if (dt->HasProperty("color_top")) {
    ptrOvr->SetCornerColor(
        eRectCorners_Top,dt->GetVec4Default(_A("color_top"),ptrOvr->GetCornerColor(eRectCorners_Top)));
  }
  if (dt->HasProperty("color_bottom")) {
    ptrOvr->SetCornerColor(
        eRectCorners_Bottom,dt->GetVec4Default(_A("color_bottom"),ptrOvr->GetCornerColor(eRectCorners_Bottom)));
  }
  if (dt->HasProperty("color_left")) {
    ptrOvr->SetCornerColor(
        eRectCorners_Left,dt->GetVec4Default(_A("color_left"),ptrOvr->GetCornerColor(eRectCorners_Left)));
  }
  if (dt->HasProperty("color_right")) {
    ptrOvr->SetCornerColor(
        eRectCorners_Right,dt->GetVec4Default(_A("color_right"),ptrOvr->GetCornerColor(eRectCorners_Right)));
  }
  if (dt->HasProperty("color_top_left")) {
    ptrOvr->SetCornerColor(
        eRectCorners_TopLeft,dt->GetVec4Default(_A("color_top_left"),ptrOvr->GetCornerColor(eRectCorners_TopLeft)));
  }
  if (dt->HasProperty("color_top_right")) {
    ptrOvr->SetCornerColor(
        eRectCorners_TopRight,dt->GetVec4Default(_A("color_top_right"),ptrOvr->GetCornerColor(eRectCorners_TopRight)));
  }
  if (dt->HasProperty("color_bottom_left")) {
    ptrOvr->SetCornerColor(
        eRectCorners_BottomLeft,dt->GetVec4Default(_A("color_bottom_left"),ptrOvr->GetCornerColor(eRectCorners_BottomLeft)));
  }
  if (dt->HasProperty("color_bottom_right")) {
    ptrOvr->SetCornerColor(
        eRectCorners_BottomRight,dt->GetVec4Default(_A("color_bottom_right"),ptrOvr->GetCornerColor(eRectCorners_BottomRight)));
  }
  _UISkinTrace(niFmt(_A("Added element '%s' (%s) (%s)."),
                     ni::GetLang()->GetAbsoluteDataTablePath(dt,eInvalidHandle),
                     (idx != eInvalidHandle)?_A("Color"):_A("Bitmap"),
                     cString(ptrOvr->GetColor()).Chars()));
  return ptrOvr;
}

///////////////////////////////////////////////
tBool cUIContext::_InitializeSkinDataTable(iDataTable* apDT)
{
  Ptr<iDataTableReadStack> dt = ni::CreateDataTableReadStack(apDT);

  // parse and initialize the datatable
  astl::stack<sOverlaySource,astl::list<sOverlaySource> > stkOverlay;
  tLoadedImageMap mapImages;

  // Load the main bitmap overlay
  {
    sOverlaySource source;
    source.nType = OVERLAYSOURCE_BITMAP;
    if (apDT->GetPropertyIndex(_A("bitmap")) != eInvalidHandle) {
      source.hspPath = _H(apDT->GetString(_A("bitmap")));
    }
    stkOverlay.push(source);
  }

  // Load the material library
  Ptr<iHString> hspDefaultMaterial;
  QPtr<iMaterialLibrary> ptrMatLib;
  {
    Ptr<iDataTable> dtMatLib = apDT->GetChild("Materials");
    if (dtMatLib.IsOK()) {
      hspDefaultMaterial = dtMatLib->GetHString("default");
      ptrMatLib = dt->GetTop()->GetIUnknown("_material_library");
      if (!ptrMatLib.IsOK()) {
        ptrMatLib = mptrGraphics->CreateMaterialLibrary();
      }
      if (!ptrMatLib->SerializeDataTable(dtMatLib,ni::eMaterialSerializeFlags_Read)) {
        niWarning(niFmt(
            "Skin '%s', Couldn't load the material library.",
            apDT->GetString("name")));
      }
      else {
        niLoop(i,ptrMatLib->GetNumMaterials()) {
          Ptr<iMaterial> ptrMaterial = ptrMatLib->GetMaterial(i);
          if (!ptrMaterial.IsOK())
            continue;
          if (!ptrMaterial->GetFlags()) {
            ptrMaterial->SetFlags(
                eMaterialFlags_NoLighting|eMaterialFlags_DoubleSided|
                eMaterialFlags_Vertex|eMaterialFlags_DiffuseModulate|
                eMaterialFlags_Transparent|eMaterialFlags_Translucent);
          }
        }
        dt->GetTop()->SetIUnknown(_A("_material_library"),ptrMatLib);
      }
    }
  }

  // Widget Class
  for (tU32 i = 0; i < dt->GetNumChildren(); ++i) {
    dt->PushChild(i);
    Ptr<iHString> hspClassMaterial = dt->GetHStringDefault("material",hspDefaultMaterial);
    for (tU32 c = 0; c < dt->GetNumChildren(); ++c) {
      dt->PushChild(c);

      // ---- Cursors ----
      if (ni::StrEq(dt->GetName(),_A("Cursors"))) {
        for (tU32 j = 0; j < dt->GetNumChildren(); ++j) {
          dt->PushChild(j);
          Ptr<iOverlay> ptrOvr = _ReadOverlayDT(
              mptrGraphics,stkOverlay.top(),dt->GetTop(),ptrMatLib,hspClassMaterial,mapImages);
          dt->GetTop()->SetIUnknown(_A("_overlay"),ptrOvr);
          _UISkinTrace(niFmt(_A("Added cursor '%s'."), ni::GetLang()->GetAbsoluteDataTablePath(dt->GetTop(),eInvalidHandle)));
          dt->Pop();
        }
      }
      // ---- State ----
      else {
        for (tU32 j = 0; j < dt->GetNumChildren(); ++j) {
          dt->PushChild(j);
          // --- Fonts ---
          if (ni::StrEq(dt->GetName(),_A("Fonts"))) {
            for (tU32 k = 0; k < dt->GetNumChildren(); ++k) {
              dt->PushChild(k);
              tHStringPtr hspName = _H(dt->GetName());
              tHStringPtr hspPath = dt->GetHString(_A("path"));
              Ptr<iFont> ptrFont = mptrGraphics->LoadFont(hspPath);
              if (!ptrFont.IsOK()) {
                niError(niFmt(_A("Can't load font '%s'."),
                              niHStr(hspPath)));
                return eFalse;
              }
              Ptr<iMaterial> ptrMaterial;
              const tU32 nFontMaterialIdx = dt->GetTop()->GetPropertyIndex("material");
              if (ptrMatLib.IsOK() && nFontMaterialIdx != eInvalidHandle) {
                Ptr<iHString> hspFontMaterial = dt->GetTop()->GetHStringFromIndex(nFontMaterialIdx);
                if (!HStringIsEmpty(hspFontMaterial)) {
                  ptrMaterial = ptrMatLib->GetMaterialFromName(hspFontMaterial);
                  if (!ptrMaterial.IsOK()) {
                    niWarning(niFmt(
                        "Skin font '%s' can't find specified material '%s' in the skin's material library.",
                        hspName,hspFontMaterial));
                  }
                }
              }

              ptrFont = ptrFont->CreateFontInstance(ptrMaterial);

              tU32 nFontResolution = ptrFont->GetResolution();
              sVec2f vFontSize = sVec2f::Zero();
              if (dt->GetTop()->GetPropertyIndex(_A("resolution")) != eInvalidHandle) {
                nFontResolution = dt->GetIntDefault(_A("resolution"),nFontResolution);
              }
              else if (dt->GetTop()->GetPropertyIndex(_A("pixel_size")) != eInvalidHandle) {
                nFontResolution = dt->GetIntDefault(_A("pixel_size"),nFontResolution);
              }
              if (dt->GetTop()->GetPropertyIndex(_A("scaled_size")) != eInvalidHandle) {
                vFontSize = dt->GetVec2Default(_A("scaled_size"),vFontSize);
              }
              if (vFontSize == sVec2f::Zero()) {
                vFontSize = Vec2f(nFontResolution,nFontResolution);
              }
              ptrFont->SetSizeAndResolution(vFontSize,nFontResolution,mfContentsScale);

              if (dt->GetTop()->GetPropertyIndex(_A("line_spacing")) != eInvalidHandle) {
                ptrFont->SetLineSpacing(dt->GetFloatDefault(_A("line_spacing"),ptrFont->GetLineSpacing()));
              }

              ptrFont->SetColor(ULColorBuild(dt->GetVec4Default(_A("color"),Vec4<tF32>(ptrFont->GetColor()))));
              dt->GetTop()->SetIUnknown(_A("_font"),ptrFont);
              _UISkinTrace(niFmt(_A("Added font '%s' ('%s','%s')."),
                                 hspName,
                                 niHStr(ptrFont->GetName()),
                                 niHStr(ptrFont->GetFamilyName())));
              dt->Pop();
            }
          }
          // --- Element ---
          else {
            Ptr<iOverlay> ptrOvr = _ReadOverlayDT(
                mptrGraphics,stkOverlay.top(),dt->GetTop(),ptrMatLib,hspClassMaterial,mapImages);
            dt->GetTop()->SetIUnknown(_A("_overlay"),ptrOvr);
            _UISkinTrace(niFmt(_A("Added element '%s'."),ni::GetLang()->GetAbsoluteDataTablePath(dt->GetTop(),eInvalidHandle)));
          }
          dt->Pop();
        }
      }
      dt->Pop();
    }
    dt->Pop();
  }

  return eTrue;
}
