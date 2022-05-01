#ifndef __MODESELECTOR_14421631_H__
#define __MODESELECTOR_14421631_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "API/niUI/Utils/GraphicsDriverDX9.h"

//////////////////////////////////////////////////////////////////////////////////////////////
// cModeSelector declaration.

//! Mode selector class.
class cModeSelector : public cIUnknownImpl<iUnknown,eIUnknownImplFlags_Default>
{
  niBeginClass(cModeSelector);

 public:
  struct sFSMode {
    D3DDISPLAYMODE  mDispMode;
    D3DFORMAT   mfmtDS;
    astl::vector<D3DMULTISAMPLE_TYPE> mvMS;
    sFSMode(D3DDISPLAYMODE& aMode, D3DFORMAT afmtDS) {
      mDispMode = aMode;
      mfmtDS = afmtDS;
    }
  };

  struct sWMode {
    D3DFORMAT             mfmtDS;
    astl::vector<D3DMULTISAMPLE_TYPE> mvMS;
    sWMode(D3DFORMAT afmtDS) {
      mfmtDS = afmtDS;
    }
  };

  struct sAdapter {
    HMONITOR          mhMonitor;
    D3DCAPS9          mCaps;
    D3DADAPTER_IDENTIFIER9    mIdentifier;
    D3DDISPLAYMODE        mDispMode;
    astl::vector<sWMode>    mvWModes;
    astl::vector<sFSMode>   mvFSModes;
  };
  typedef astl::vector<sAdapter>    tAdapterVec;
  typedef tAdapterVec::iterator   tAdapterVecIt;
  typedef tAdapterVec::const_iterator tAdapterVecCIt;

 public:
  //! Constructor.
  cModeSelector();
  //! Destructor.
  ~cModeSelector();

  //! Zeros all the class members.
  void ZeroMembers();

  //! Sanity check.
  tBool __stdcall IsOK() const;

  //! Initialize the mode selector.
  tBool __stdcall Initialize(IDirect3D9* apD3D9, tBool abLimitedImpl);
  //! Get the number of adapters.
  tU32 __stdcall GetNumAdapters() const;
  //! Get the adapter at the specified index.
  const sAdapter* __stdcall GetAdapter(tU32 anIndex) const;
  //! Get the default adapter index.
  //! \remark The default adapter is generally what should be used.
  tU32 __stdcall GetDefaultAdapterIndex() const;
  //! Get the default adapter.
  //! \remark The default adapter is generally what should be used.
  const sAdapter* __stdcall GetDefaultAdapter() const;

  void LogModes();

 private:
  tAdapterVec mvAdapters;
  tU32    mnDefaultAdapter;
  niEndClass(cModeSelector);
};

BOOL D3D9_IsDepthFormatOk(
    IDirect3D9* pD3D,
    tU32 anAdapter, D3DDEVTYPE anDevType,
    D3DFORMAT DepthFormat,
    D3DFORMAT AdapterFormat,
    D3DFORMAT BackBufferFormat);

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __MODESELECTOR_14421631_H__
