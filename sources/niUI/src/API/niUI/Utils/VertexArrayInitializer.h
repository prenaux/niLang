#ifndef __VERTEXARRAYINITIALIZER_5907632_H__
#define __VERTEXARRAYINITIALIZER_5907632_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/Utils/STLVectorTypes.h>

namespace ni {
/** \addtogroup niUI
 * @{
 */
/** \addtogroup niUI_Utils
 * @{
 */

//! Large vertex array initializer.
struct sVertexArrayInitializer
{
  //! Index hash map.
  typedef astl::hash_map<tU32,tU32> tIndexHMap;

  //! Vertex buffer structure
  struct sVB {
    cFVFDescription fvfDesc;
    tU8Vec    vVerts;
    tU32    nCount;
    tIndexHMap  hmapIndices;
    Ptr<iVertexArray> ptrVA;
    sVB() : nCount(0) {}
  };

  //! Vertex buffer hash map.
  typedef astl::hash_map<tU64,sVB>  tVBHMap;
  //! Chunk hash map.
  typedef astl::hash_map<tFVF,tU32> tChunkHMap;

  tU32    mnMaxVertsPerVA;
  tVBHMap   mhmapVB;
  tChunkHMap  mhmapChunks;
  tU32    mnTotalVerts;
  tU64    mnTotalMemory;

  //! Constructor, pass the maximum vertex per vertex array manually
  sVertexArrayInitializer(tU32 anMaxVert) {
    mnMaxVertsPerVA = anMaxVert;
    mnTotalVerts = 0;
    mnTotalMemory = 0;
  }
  //! Constructor, pass the maximum vertex per vertex array using the graphics caps
  sVertexArrayInitializer(iGraphics* apGraphics) {
    mnMaxVertsPerVA = apGraphics->GetDriverCaps(eGraphicsCaps_MaxVertexIndex);
    if (!mnMaxVertsPerVA)
      mnMaxVertsPerVA = 0xFFFF;
    mnTotalVerts = 0;
    mnTotalMemory = 0;
  }
  //! Destructor
  ~sVertexArrayInitializer() {
    mhmapVB.clear();
    mhmapChunks.clear();
  }

  //! Hash for FVF|Chunk set
  inline tU64 GetHash(tFVF aFVF, tU32 anChunk) const {
    return tU64(aFVF)|(tU64(anChunk)<<32);
  }

  //! Get the vertex buffer of the specified FVF|Chunk pair
  tVBHMap::iterator GetVB(tFVF anFVF, tU32 anChunk) {
    tVBHMap::iterator itVB = mhmapVB.find(GetHash(anFVF,anChunk));
    if (itVB == mhmapVB.end()) {
      itVB = astl::upsert(mhmapVB,GetHash(anFVF,anChunk),sVB());
      sVB& vb = itVB->second;
      vb.fvfDesc.Setup(anFVF);
      vb.vVerts.reserve(256*1024);
    }
    return itVB;
  }

  //! Get a new chunk. The number of vertices should be smaller than the number of vertices per VA allowed.
  //! \remark anNumVerts is an estimate of the max number of vertices that will go into the chunk.
  //!     PushVertex will do the actual vertex adding.
  tU32 GetNextChunk(tFVF aFVF, tU32 anNumVerts)
  {
    niAssert(anNumVerts < mnMaxVertsPerVA);
    tChunkHMap::iterator itC = mhmapChunks.find(aFVF);
    if (itC == mhmapChunks.end()) {
      astl::upsert(mhmapChunks,aFVF,0);
      return 0;
    }
    tVBHMap::iterator itVB = GetVB(aFVF,itC->second);
    tU32 nCount = itVB->second.nCount;
    if (nCount && (nCount+anNumVerts >= mnMaxVertsPerVA)) {
      return ++itC->second;
    }
    return itC->second;
  }

  //! Add a vertex to the specified FVF|Chunk|Index.
  //! \remark Will convert the FVF from srcFVF to the FVF of the chunk.
  //! \return The index of the pushed vertex.
  tU32 PushVertex(tFVF anFVF, tU32 anChunk, tU32 anIndex, tFVF aSrcFVF, tPtr apVert) {
    cFVFDescription srcFVF(aSrcFVF);

    // get the VB
    tVBHMap::iterator itVB = GetVB(anFVF,anChunk);
    // add the vertex
    sVB& vb = itVB->second;
    niAssert(!vb.ptrVA.IsOK()); // cant add a vertex after initialization

    tIndexHMap::const_iterator it = vb.hmapIndices.find(anIndex);
    if (it == vb.hmapIndices.end()) {
      tU32 nCur = vb.vVerts.size();
      vb.vVerts.resize(vb.vVerts.size()+vb.fvfDesc.GetStride());
      memset(&vb.vVerts[nCur],0,vb.fvfDesc.GetStride());
      cFVFVertex destV(&vb.fvfDesc,&vb.vVerts[nCur]);
      cFVFVertex srcV(&srcFVF,apVert);
      destV.Copy(srcV);
      tU32 c = vb.nCount++;
      astl::upsert(vb.hmapIndices,anIndex,c);
      return c;
    }
    else {
      // vertex already in, dont add it
      return it->second;
    }
  }

  //! Get the index of the specified vertex in the final VA.
  tU32 GetVertexIndex(tFVF anFVF, tU32 anChunk, tU32 anIndex) const {
    tVBHMap::const_iterator itVB = mhmapVB.find(GetHash(anFVF,anChunk));
    niAssert(itVB != mhmapVB.end());
    const sVB& vb = itVB->second;
    tIndexHMap::const_iterator it = vb.hmapIndices.find(anIndex);
    niAssert(it != vb.hmapIndices.end());
    return it->second;
  }

  //! Get all vertices of the specified FVF|Chunk.
  const tU8* GetVerts(tFVF anFVF, tU32 anChunk) const {
    tVBHMap::const_iterator itVB = mhmapVB.find(GetHash(anFVF,anChunk));
    niAssert(itVB != mhmapVB.end());
    const sVB& vb = itVB->second;
    return &vb.vVerts[0];
  }

  //! Get the number of vertices of the specified FVF|Chunk.
  tU32 GetNumVerts(tFVF anFVF, tU32 anChunk) const {
    tVBHMap::const_iterator itVB = mhmapVB.find(GetHash(anFVF,anChunk));
    niAssert(itVB != mhmapVB.end());
    const sVB& vb = itVB->second;
    return vb.nCount;
  }

  //! Get the vertex array of the specified FVF|Chunk.
  iVertexArray* GetVA(iGraphics* apGraphics, tFVF anFVF, tU32 anChunk, eArrayUsage aUsage) {
    tVBHMap::iterator itVB = mhmapVB.find(GetHash(anFVF,anChunk));
    return GetVA(apGraphics,itVB,aUsage);
  }
  iVertexArray* GetVA(iGraphics* apGraphics, tVBHMap::iterator itVB, eArrayUsage aUsage) {
    niAssert(itVB != mhmapVB.end());
    sVB& vb = itVB->second;
    niAssert(vb.nCount);
    if (vb.nCount && !vb.ptrVA.IsOK()) {
      vb.ptrVA = apGraphics->CreateVertexArray(vb.nCount,vb.fvfDesc.GetFVF(),aUsage);
      if (!niIsOK(vb.ptrVA))
    	  return NULL;

      // copy VA content
      tPtr pVerts = vb.ptrVA->Lock(0,0,eLock_Normal);
      memcpy(pVerts,&vb.vVerts[0],vb.fvfDesc.GetStride()*vb.nCount);
      vb.ptrVA->Unlock();

      // check IA integrity
#ifdef _DEBUG
      niLoopit(tIndexHMap::const_iterator,it,vb.hmapIndices) {
        niAssert(it->second < vb.nCount);
      }
#endif

      tFVF fvf = vb.fvfDesc.GetFVF();
      niLog(Info,niFmt(_A("# FVF %s (stride %d), %d vertices, %.2f Kb"),FVFToString(fvf).Chars(),vb.fvfDesc.GetStride(),vb.nCount,tF32(vb.nCount*vb.fvfDesc.GetStride())/1024.0f));

      mnTotalVerts += vb.nCount;
      mnTotalMemory += vb.nCount*vb.fvfDesc.GetStride();
    }
    return vb.ptrVA;
  }

  //! Get the first VB iterator.
  tVBHMap::iterator GetVBBeginIt() {
    return mhmapVB.begin();
  }
  //! Get the end VB iterator.
  tVBHMap::iterator GetVBEndIt() {
    return mhmapVB.end();
  }

  //! Get the total number of vertices.
  //! \remark This is initialized by GetVA
  tU32 GetTotalNumVerts() const {
    return mnTotalVerts;
  }
  //! Get the total memory in bytes.
  //! \remark This is initialized by GetVA
  tU64 GetTotalMemory() const {
    return mnTotalMemory;
  }
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __VERTEXARRAYINITIALIZER_5907632_H__
