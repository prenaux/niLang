#pragma once
#ifndef __BUFFERCACHE_H_58BB0AB1_7281_2E4C_BF3B_F6B3720C0458__
#define __BUFFERCACHE_H_58BB0AB1_7281_2E4C_BF3B_F6B3720C0458__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

namespace ni {
/** \addtogroup niUI
 * @{
 */
/** \addtogroup niUI_Utils
 * @{
 */

#ifndef TRACE_BUFFER_CACHE
#ifdef _DEBUG
#define TRACE_BUFFER_CACHE(X) niDebugFmt(X)
#else
#define TRACE_BUFFER_CACHE(X)
#endif
#endif

template <typename T, typename TDERIVED>
struct BufferCache
{
  typedef astl::vector<T> tElVec;

  inline BufferCache(const tU32 anReserve) : mnReserve(anReserve) {
    niAssert(mnReserve > 0);
    mnLastResetFrame = eInvalidHandle;
    mnBaseEl = mnCurrentEl = 0;
  }
  inline void __stdcall Invalidate() {
    static_cast<TDERIVED*>(this)->_Invalidate();
    mnBaseEl = 0;
    mnCurrentEl = 0;
  }

  //! Get the number of pending elements in the cache.
  inline tU32 GetNumPending() const {
    return mnCurrentEl - mnBaseEl;
  }

  //! Return eTrue if the cache has any pending element that haven't been updated.
  inline tBool HasPending() const {
    return GetNumPending() > 0;
  }

  //! Reset the cache.
  inline void Reset() {
    if (mnLastResetFrame != ni::GetLang()->GetFrameNumber()) {
      NextFrame();
    }
  }

  //! Notify that the current frame has been submitted.
  inline void NextFrame() {
    mnBaseEl = 0;
    mnCurrentEl = 0;
    mnLastResetFrame = ni::GetLang()->GetFrameNumber();
    static_cast<TDERIVED*>(this)->_NextFrame();
  }

  //! Add an element in the cache.
  inline void Add(const T& ulV) {
    if ((mnCurrentEl+1) >= mvEls.size()) {
      mvEls.resize(ni::AlignOffset<tU32>(mnReserve, (tU32)mvEls.size() + 1));
    }
    mvEls[mnCurrentEl] = ulV;
    ++mnCurrentEl;
  }

  //! Add an array of element in the cache.
  inline void Add(const T* apEls, tU32 anNumEls) {
    if (mnCurrentEl+anNumEls >= mvEls.size()) {
      mvEls.resize(ni::AlignOffset<tU32>(mnReserve, (tU32)mvEls.size() + anNumEls));
    }
    memcpy(&mvEls[mnCurrentEl], apEls, sizeof(T)*anNumEls);
    mnCurrentEl += anNumEls;
  }

  //! Add a vector of element in the cache.
  inline void Add(const tElVec& vVertices) {
    AddVertices(&vVertices[0], (tU32)vVertices.size());
  }

  //! Get the size of the cache.
  inline const tU32 GetCacheSize() const {
    return (tU32)mvEls.size();
  }
  //! Get an element in the cache.
  inline const T* GetEl(tU32 anIndex) const {
    niAssert(anIndex < mvEls.size());
    return &mvEls[anIndex];
  }
  //! Get an element in the cache.
  inline T* GetEl(tU32 anIndex) {
    niAssert(anIndex < mvEls.size());
    return &mvEls[anIndex];
  }

  //! Get the current element.
  inline tU32 GetCurrentEl() const {
    return mnCurrentEl;
  }

  //! Get the current index.
  inline tU32 GetCurrentIndex() const {
    return mnCurrentEl - mnBaseEl;
  }

  //! Update the buffer.
  inline sVec2i Update() {
    const tU32 baseEl = mnBaseEl;
    const tU32 numPending = GetNumPending();
    if (numPending <= 0) {
      return Vec2i(0,0);
    }

    tU32 upBase = mnBaseEl;
    tU32 upCount = GetNumPending();
    if (!static_cast<TDERIVED*>(this)->_HasCapacity(mnCurrentEl)) {
      if (!static_cast<TDERIVED*>(this)->_Init((tU32)mvEls.size())) {
        return Vec2i(0,0);
      }
      // new VA we need to upload everything
      upBase = 0;
      upCount = (tU32)mvEls.size();
    }

    if (static_cast<TDERIVED*>(this)->_Upload(mvEls.data(),upBase,upCount)) {
      mnBaseEl = mnCurrentEl;
      niAssert(baseEl <= ni::TypeMax<tI32>());
      niAssert(numPending <= ni::TypeMax<tI32>());
      return Vec2i((tI32)baseEl, (tI32)numPending);
    }
    else {
      return Vec2i(0,0);
    }
  }

 private:
  const tU32        mnReserve;
  tU32              mnBaseEl;
  tU32              mnCurrentEl;
  tU32              mnLastResetFrame;
  tElVec            mvEls;
};

template <typename TVERTEX>
struct BufferCacheVertex : public BufferCache<TVERTEX,BufferCacheVertex<TVERTEX> >, public cIUnknownImpl<iUnknown> {
  typedef BufferCache<TVERTEX,BufferCacheVertex<TVERTEX> > tBase;
  typedef TVERTEX tVertex;

  Ptr<iGraphics> mptrGraphics;
  Ptr<iVertexArray> mptrVA;

  BufferCacheVertex(iGraphics* apGraphics, tU32 anReserve) : tBase(anReserve) {
    mptrGraphics = apGraphics;
  }

  inline void _Invalidate() {
    mptrVA = NULL;
  }
  inline void _NextFrame() {
  }
  inline tBool _HasCapacity(const tU32 anNumEls) {
    return mptrVA.IsOK() && (mptrVA->GetNumVertices() >= anNumEls);
  }
  inline tBool _Init(const tU32 anNumEls) {
    mptrVA = mptrGraphics->CreateVertexArray(anNumEls, tVertex::eFVF, eArrayUsage_Dynamic);
    TRACE_BUFFER_CACHE(("... BufferCacheVertex<%s>[%p]::Init: %d vertices.",
                        FVFToString(tVertex::eFVF), (tIntPtr)this, anNumEls));
    return mptrVA.IsOK();
  }
  inline tBool _Upload(const TVERTEX* apVerts, tU32 aUpBase, tU32 aUpCount) {
    TVERTEX* pV = (TVERTEX*)mptrVA->Lock(aUpBase,aUpCount,eLock_Discard);
    niAssert(pV != NULL);
    if (!pV) {
      return eFalse;
    }
    memcpy(pV, apVerts+aUpBase, sizeof(tVertex)*aUpCount);
    mptrVA->Unlock();
    return eTrue;
  }
};

struct BufferCacheIndex : public BufferCache<tU32,BufferCacheIndex>, public cIUnknownImpl<iUnknown> {
  typedef BufferCache<tU32,BufferCacheIndex> tBase;
  typedef tU32 tIndex;

  Ptr<iGraphics> mptrGraphics;
  Ptr<iIndexArray> mptrIA;

  BufferCacheIndex(iGraphics* apGraphics, tU32 anReserve) : tBase(anReserve) {
    mptrGraphics = apGraphics;
  }

  inline void _Invalidate() {
    mptrIA = NULL;
  }
  inline void _NextFrame() {
  }
  inline tBool _HasCapacity(const tU32 anNumEls) {
    return mptrIA.IsOK() && (mptrIA->GetNumIndices() >= anNumEls);
  }
  inline tBool _Init(const tU32 anNumEls) {
    mptrIA = mptrGraphics->CreateIndexArray(eGraphicsPrimitiveType_TriangleList, anNumEls, ~0, eArrayUsage_Dynamic);
    TRACE_BUFFER_CACHE(("... BufferCacheIndex[%p]::Init: %d indices.",
                        (tIntPtr)this, anNumEls));
    return mptrIA.IsOK();
  }
  inline tBool _Upload(const tU32* apInds, tU32 aUpBase, tU32 aUpCount) {
    tIndex* pV = (tIndex*)mptrIA->Lock(aUpBase,aUpCount,eLock_Discard);
    niAssert(pV != NULL);
    if (!pV) {
      return eFalse;
    }
    memcpy(pV, apInds+aUpBase, sizeof(tU32)*aUpCount);
    mptrIA->Unlock();
    return eTrue;
  }
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __BUFFERCACHE_H_58BB0AB1_7281_2E4C_BF3B_F6B3720C0458__
