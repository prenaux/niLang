// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#ifdef GDRV_DX9

#include "GDRV_DX9_ModeSelector.h"
#include "GDRV_DX9_FormatMap.h"
#include <niLang/STL/set.h>

#pragma niTodo("Double-check the AdapterOrdinal value when having several monitors, and see if they are reported correctly.")

//////////////////////////////////////////////////////////////////////////////////////////////
static D3DFORMAT _ColorFormats[] = {
  D3DFMT_A2R10G10B10,
  D3DFMT_A8R8G8B8,
  D3DFMT_X8R8G8B8,
  D3DFMT_R5G6B5,
  D3DFMT_UNKNOWN
};

static D3DFORMAT _DepthFormats[] = {
  D3DFMT_D24S8,
  D3DFMT_D24X8,
  D3DFMT_D16,
  D3DFMT_D32,
  D3DFMT_UNKNOWN
};

BOOL D3D9_IsDepthFormatOk(
    IDirect3D9* pD3D,
    tU32 anAdapter, D3DDEVTYPE anDevType,
    D3DFORMAT DepthFormat,
    D3DFORMAT AdapterFormat,
    D3DFORMAT BackBufferFormat)
{

  // Verify that the depth format exists
  HRESULT hr = pD3D->CheckDeviceFormat(anAdapter,
                                       anDevType,
                                       AdapterFormat,
                                       D3DUSAGE_DEPTHSTENCIL,
                                       D3DRTYPE_SURFACE,
                                       DepthFormat);

  if(FAILED(hr)) return FALSE;

  // Verify that the depth format is compatible
  hr = pD3D->CheckDepthStencilMatch(anAdapter,
                                    anDevType,
                                    AdapterFormat,
                                    BackBufferFormat,
                                    DepthFormat);

  return SUCCEEDED(hr);
}

BOOL IsMultisampleOk(IDirect3D9* apD3D9, D3DCAPS9* apCaps, D3DFORMAT afmtCol, D3DFORMAT afmtDS, BOOL abWindowed, D3DMULTISAMPLE_TYPE aMS)
{
  if (SUCCEEDED(apD3D9->CheckDeviceMultiSampleType(apCaps->AdapterOrdinal,
                                                   apCaps->DeviceType, afmtCol,
                                                   abWindowed, aMS, NULL)) &&
      (afmtDS == D3DFMT_UNKNOWN || SUCCEEDED(apD3D9->CheckDeviceMultiSampleType(apCaps->AdapterOrdinal,
                                                                                apCaps->DeviceType, afmtDS,
                                                                                abWindowed, aMS, NULL))))
    return TRUE;
  return FALSE;
}

void GetModeMultisample(IDirect3D9* apD3D9, D3DCAPS9* apCaps, D3DFORMAT afmtCol, D3DFORMAT afmtDS, BOOL abWindowed, astl::vector<D3DMULTISAMPLE_TYPE>& avMS)
{
  for (tU32 i = 0; i < D3DMULTISAMPLE_16_SAMPLES; ++i)
  {
    if (IsMultisampleOk(apD3D9,apCaps,afmtCol,afmtDS,abWindowed,(D3DMULTISAMPLE_TYPE)i))
      avMS.push_back((D3DMULTISAMPLE_TYPE)i);
  }
}

cString GetMultisampleStr(const astl::vector<D3DMULTISAMPLE_TYPE>& avMS)
{
  cString str;
  for (tU32 i = 0; i < avMS.size(); ++i)
    str += niFmt(_A("%d;"),avMS[i]);
  return str;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// cModeSelector implementation.

///////////////////////////////////////////////
cModeSelector::cModeSelector()
{
  ZeroMembers();
}

///////////////////////////////////////////////
cModeSelector::~cModeSelector()
{
}

///////////////////////////////////////////////
//! Zeros all the class members.
void cModeSelector::ZeroMembers()
{
  mnDefaultAdapter = 0;
}

///////////////////////////////////////////////
//! Sanity check.
tBool __stdcall cModeSelector::IsOK() const
{
  niClassIsOK(cModeSelector);
  return eTrue;
}

typedef struct __ModeKey {
  tU32 dw; tU16 w;
  bool operator < (const __ModeKey& aKey) const  { return w < aKey.w || dw < aKey.dw; }
} tModeKey;

///////////////////////////////////////////////
tBool __stdcall cModeSelector::Initialize(IDirect3D9* apD3D9, tBool abLimitedImpl)
{
  mvAdapters.clear();
  astl::set<tModeKey> setFoundModes;

  const tBool bUseRef = ni::GetProperty("d3d9.UseRef","0").Bool();
  const tBool bUseNVPerfHUD = ni::GetProperty("d3d9.UseNVPrefHUD","0").Bool();

  const D3DDEVTYPE nDevType = bUseRef?D3DDEVTYPE_REF:D3DDEVTYPE_HAL;
  tU32 nAdapterCount = abLimitedImpl ? 1 : apD3D9->GetAdapterCount();
  mnDefaultAdapter = 0;
  for (tU32 i = 0; i < nAdapterCount; ++i)
  {
    sAdapter& a = astl::push_back(mvAdapters);

    // adapter identifier
    apD3D9->GetAdapterIdentifier(i,0,&a.mIdentifier);
    if (strstr(a.mIdentifier.Description,"PerfHUD"))
    {
#ifndef niRedist
      if (!bUseNVPerfHUD)
        continue;

      // adapter caps
      if (!SUCCEEDED(apD3D9->GetDeviceCaps(i,D3DDEVTYPE_REF,&a.mCaps))) {
        mvAdapters.erase(mvAdapters.begin()+(mvAdapters.size()-1));
        continue;
      }
      mnDefaultAdapter = i;
      a.mCaps.DeviceType = D3DDEVTYPE_REF;
      a.mCaps.AdapterOrdinal = i;
#else
      continue; // skip the NVIDIA NVPerfHUD in non development builds
#endif
    }
    else
    {
      // adapter caps
      if (!SUCCEEDED(apD3D9->GetDeviceCaps(i,nDevType,&a.mCaps))) {
        mvAdapters.erase(mvAdapters.begin()+(mvAdapters.size()-1));
        continue;
      }
    }
    if (abLimitedImpl) {
      a.mhMonitor = (HMONITOR)ni::GetLang()->GetMonitorHandle(0);
    }
    else {
      a.mhMonitor = apD3D9->GetAdapterMonitor(i);
    }

    // full-screen adapter modes
    for (tU32 ifmt = 0; _ColorFormats[ifmt] != D3DFMT_UNKNOWN; ++ifmt)
    {
      // get modes
      for (tU32 j = 0; j < apD3D9->GetAdapterModeCount(i,_ColorFormats[ifmt]); ++j)
      {
        D3DDISPLAYMODE mode;
        apD3D9->EnumAdapterModes(i,_ColorFormats[ifmt],j,&mode);
        mode.RefreshRate = 0;

        // check if the mode has already been added.
        tModeKey modekey;
        modekey.dw = (mode.Width<<16)|mode.Height;
        modekey.w = (tU16)mode.Format;
        if (setFoundModes.find(modekey) != setFoundModes.end()) continue;

        // add the mode
        setFoundModes.insert(modekey);

        // no depth buffer
        a.mvFSModes.push_back(sFSMode(mode,D3DFMT_UNKNOWN));
        GetModeMultisample(apD3D9,&a.mCaps,
                           a.mvFSModes.back().mDispMode.Format,
                           a.mvFSModes.back().mfmtDS,
                           FALSE,
                           a.mvFSModes.back().mvMS);

        // depth formats
        for (tU32 dfmt = 0; _DepthFormats[dfmt] != D3DFMT_UNKNOWN; ++dfmt) {
          if (D3D9_IsDepthFormatOk(apD3D9,i,a.mCaps.DeviceType,
                                   _DepthFormats[dfmt],
                                   mode.Format,
                                   _ColorFormats[ifmt]))
          {
            a.mvFSModes.push_back(sFSMode(mode,_DepthFormats[dfmt]));
            GetModeMultisample(apD3D9,&a.mCaps,
                               a.mvFSModes.back().mDispMode.Format,
                               a.mvFSModes.back().mfmtDS,
                               FALSE,
                               a.mvFSModes.back().mvMS);
          }
        }
      }
    }

    // windowed adapter modes
    apD3D9->GetAdapterDisplayMode(i,&a.mDispMode);
    a.mvWModes.push_back(sWMode(D3DFMT_UNKNOWN)); // do depth buffer
    GetModeMultisample(apD3D9,&a.mCaps,
                       a.mDispMode.Format,
                       a.mvWModes.back().mfmtDS,
                       TRUE,
                       a.mvWModes.back().mvMS);
    for (tU32 dfmt = 0; _DepthFormats[dfmt] != D3DFMT_UNKNOWN; ++dfmt) {
      if (D3D9_IsDepthFormatOk(apD3D9,i,a.mCaps.DeviceType,
                               _DepthFormats[dfmt],
                               a.mDispMode.Format,
                               a.mDispMode.Format))
      {
        a.mvWModes.push_back(sWMode(_DepthFormats[dfmt]));
        GetModeMultisample(apD3D9,&a.mCaps,
                           a.mDispMode.Format,
                           a.mvWModes.back().mfmtDS,
                           FALSE,
                           a.mvWModes.back().mvMS);
      }
    }
  }

  return eTrue;
}

///////////////////////////////////////////////
tU32 __stdcall cModeSelector::GetNumAdapters() const
{
  return (tU32)mvAdapters.size();
}

///////////////////////////////////////////////
const cModeSelector::sAdapter* __stdcall cModeSelector::GetAdapter(tU32 anIndex) const
{
  niAssert(anIndex < GetNumAdapters());
  return &mvAdapters[anIndex];
}

///////////////////////////////////////////////
tU32 __stdcall cModeSelector::GetDefaultAdapterIndex() const
{
  return mnDefaultAdapter;
}

///////////////////////////////////////////////
const cModeSelector::sAdapter* __stdcall cModeSelector::GetDefaultAdapter() const
{
  return GetAdapter(mnDefaultAdapter);
}

///////////////////////////////////////////////
void cModeSelector::LogModes()
{
  cString str;
  str += _A("=== D3D9 Infos ===\n");
  str += niFmt(_A("Num Adapters: %d\n"),GetNumAdapters());
  str += niFmt(_A("Default Adapter: %d\n"),GetDefaultAdapterIndex());
  for (tU32 ai = 0; ai < GetNumAdapters(); ++ai)
  {
    const sAdapter& a = *GetAdapter(ai);
    str += niFmt(_A("Adapter %d {\n"),ai);
    str += niFmt(_A("  Monitor: 0x%x\n"),a.mhMonitor);
    str += niFmt(_A("  Ordinal: %d\n"),a.mCaps.AdapterOrdinal);
    str += niFmt(_A("  DeviceType: %d\n"),a.mCaps.DeviceType);
    str += niFmt(_A("  Driver: %s\n"),cString(a.mIdentifier.Driver).Chars());
    str += niFmt(_A("  Desc: %s\n"),cString(a.mIdentifier.Description).Chars());
    str += niFmt(_A("  Name: %s\n"),cString(a.mIdentifier.DeviceName).Chars());
    str += niFmt(_A("  HiVer: %d\n"),a.mIdentifier.DriverVersion.HighPart);
    str += niFmt(_A("  LoVer: %d\n"),a.mIdentifier.DriverVersion.LowPart);
    str += niFmt(_A("  DispMode: %dx%d %dhz %s\n"),
                 a.mDispMode.Width,a.mDispMode.Height,a.mDispMode.RefreshRate,
                 GetD3DFormatMap()->GetPixelFormat(a.mDispMode.Format));

#if 0 // these are quite numerous usually and not very usefull in the log...
    str += _A("= Windowed Modes =====\n");
    for (tU32 wi = 0; wi < a.mvWModes.size(); ++wi)
    {
      const sWMode& wm = a.mvWModes[wi];
      str += niFmt(_A("  - DS:%s, MS:%s\n"),GetD3DFormatMap()->GetPixelFormat(wm.mfmtDS,NULL,_A("none")),GetMultisampleStr(wm.mvMS).Chars());
    }

    str += _A("= FullScreen Modes ===\n");
    for (tU32 fi = 0; fi < a.mvFSModes.size(); ++fi)
    {
      const sFSMode& fm = a.mvFSModes[fi];
      str += niFmt(_A("  - %dx%d %s, DS: %s, MS: %s\n"),
                   fm.mDispMode.Width,fm.mDispMode.Height,
                   GetD3DFormatMap()->GetPixelFormat(fm.mDispMode.Format),
                   GetD3DFormatMap()->GetPixelFormat(fm.mfmtDS,NULL,_A("none")),
                   GetMultisampleStr(fm.mvMS).Chars());
    }
#endif

    str += "}\n";
  }

  niLog(Info,str.Chars());
}

#endif
