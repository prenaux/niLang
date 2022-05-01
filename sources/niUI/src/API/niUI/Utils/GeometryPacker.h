#ifndef __GEOMETRYPACKER_29385760_H__
#define __GEOMETRYPACKER_29385760_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "VertexArrayInitializer.h"

namespace ni {
/** \addtogroup niUI
 * @{
 */
/** \addtogroup niUI_Utils
 * @{
 */

//! Pack mesh reference
struct sPackMeshRef {
  tU32 firstIdx;
  tU32 numIndices;
  tU32 matIndex;
  tU32 geomIndex;
};

//! Pack mesh
struct sPackMesh
{
  sPackMesh() {
    _mesh = NULL;
    _fvf = 0;
    _chunk = 0;
  }
  sPackMeshRef* _mesh;
  tFVF  _fvf;
  tU64  _chunk;
};

//! Pack mesh vector.
typedef astl::vector<sPackMesh> tPackMeshVec;

//! Pack a geometry into several geometries.
//! \remark mesh format is (tU32:FirstIdx, tU32:NumIndices, tU32:MaterialIndex, tU32:Geom)
inline tU32 __stdcall PackGeometry(iGraphics* apGraphics,
                                   iGeometry* apGeom,
                                   tPtr apMeshes,
                                   tU32 anMeshStride,
                                   tU32 anNumMeshes,
                                   tU32 anGeomIndexBase,
                                   iMaterialLibrary* apMatLib,
                                   tIUnknownCVec& aOutput,
                                   eArrayUsage aUsage,
                                   tFVF aDefaultFVF,
                                   tBool abOneFVF)
{
  sVertexArrayInitializer vaInit(apGraphics);

  // Collect meshes infos
  tPackMeshVec  packMeshes;
  packMeshes.resize(anNumMeshes);

  // Lock the geometry
  tPtr  vaPtr = apGeom->GetVertexArray()->Lock(0,0,eLock_Normal);
  tIndex* iaPtr = (tIndex*)apGeom->GetIndexArray()->Lock(0,0,eLock_Normal);
  cFVFDescription vertFVF(apGeom->GetVertexArray()->GetFVF());
  cFVFStream    vertStream(&vertFVF,vaPtr,apGeom->GetVertexArray()->GetNumVertices());

  // Detect the FVF
  niLoop(i,anNumMeshes) {
    sPackMesh& packMesh = packMeshes[i];
    packMesh._mesh = (sPackMeshRef*)(apMeshes+anMeshStride*i);
    if (abOneFVF) {
      packMesh._fvf = aDefaultFVF;
    }
    else {
      iMaterial* mat = apMatLib->GetMaterial(packMesh._mesh->matIndex);
      if (mat) {
        tU32 fvf = eFVF_Position;
        if (vertFVF.HasNormal()) {
          fvf |= eFVF_Normal;
        }
        if (vertFVF.HasColorA()) {
          const tSize coloraOffset = vertFVF.GetColorAOffset();
          const tIndex* idx = &iaPtr[packMesh._mesh->firstIdx];
          niLoop(i,packMesh._mesh->numIndices) {
            tU32* colora = (tU32*)(vertStream.GetVertex(*idx)+coloraOffset);
            if (*colora != 0xFFFFFFFF) {
              fvf |= eFVF_ColorA;
              break;
            }
          }
        }

        tBool hasTex2 = eFalse;
        if (vertFVF.GetTexCooDim(1) == 2) {
          const tSize tex2offset = vertFVF.GetTexCooOffset(1);
          const tIndex* idx = &iaPtr[packMesh._mesh->firstIdx];
          niLoop(i,packMesh._mesh->numIndices) {
            sVec2f* texCoo2 = (sVec2f*)(vertStream.GetVertex(*idx)+tex2offset);
            if (!niFloatIsZero(texCoo2->x) || !niFloatIsZero(texCoo2->y)) {
              hasTex2 = eTrue;
              break;
            }
          }
        }
        if (hasTex2) {
          fvf |= eFVF_Tex2;
        }
        else if (vertFVF.GetTexCooDim(0) == 2) {
          fvf |= eFVF_Tex1;
        }

        packMesh._fvf = fvf;
      }
      else {
        packMesh._fvf = aDefaultFVF;
      }
    }
  }

  // const tU32 numVertices = apGeom->GetVertexArray()->GetNumVertices();
  const tU32 numIndices = apGeom->GetIndexArray()->GetNumIndices();

  // Init the index array output
  astl::vector<tIndex>  destIA;
  destIA.reserve(apGeom->GetIndexArray()->GetNumIndices());

  // Fill up and init the chunks
  niLoop(i,anNumMeshes) {
    sPackMesh& packMesh = packMeshes[i];
    if (packMesh._mesh->firstIdx >= numIndices)
      continue; // invalid index
    if (packMesh._mesh->firstIdx+packMesh._mesh->numIndices > numIndices)
      continue; // invalid index

    tU32 chunk = vaInit.GetNextChunk(packMesh._fvf,packMesh._mesh->numIndices);
    niAssert(chunk != eInvalidHandle);
    packMesh._chunk = vaInit.GetHash(packMesh._fvf,chunk);

    const tIndex* idx = &iaPtr[packMesh._mesh->firstIdx];
    packMesh._mesh->firstIdx = destIA.size();
    niLoop(i,packMesh._mesh->numIndices) {
      tIndex newIndex = vaInit.PushVertex(packMesh._fvf,chunk,*idx,vertFVF.GetFVF(),vertStream.GetVertex(*idx));
      niAssert(newIndex < vertStream.GetNumVertices());
      destIA.push_back(newIndex);
      ++idx;
    }
  }
  apGeom->GetIndexArray()->Unlock();
  apGeom->GetVertexArray()->Unlock();

  // output arrays
  astl::vector<Ptr<iVertexArray> >  vaOutput;
  Ptr<iIndexArray>          iaOutput;

  tU32 nNewGeomIndex = 0;

  // get the new VAs
  for (sVertexArrayInitializer::tVBHMap::iterator it = vaInit.GetVBBeginIt(); it != vaInit.GetVBEndIt(); ++it) {
    Ptr<iVertexArray> va = vaInit.GetVA(apGraphics,it,aUsage);
    niCheck(va.IsOK(),0);
    // assign the new geom index
    niLoop(i,anNumMeshes) {
      if (packMeshes[i]._chunk == it->first) {
        sPackMesh& packMesh = packMeshes[i];
        packMesh._mesh->geomIndex = anGeomIndexBase+nNewGeomIndex;
      }
    }
    vaOutput.push_back(va);
    ++nNewGeomIndex;
  }

  // get the new IA
  iaOutput = apGraphics->CreateIndexArray(eGraphicsPrimitiveType_TriangleList,destIA.size(),vertStream.GetNumVertices(),eArrayUsage_Static);
  niCheck(iaOutput.IsOK(),0);
  tPtr iaOutputPtr = iaOutput->Lock(0,0,eLock(eLock_Discard|eLock_NoOverwrite));
  ni::MemCopy(iaOutputPtr,(tPtr)&destIA[0],destIA.size()*sizeof(tIndex));
  iaOutput->Unlock();

  // output the new geometries
  niLoop(i,vaOutput.size()) {
    Ptr<iGeometry> geomOutput = apGraphics->CreateGeometryPolygonalEx(vaOutput[i],iaOutput);
    niCheck(geomOutput.IsOK(),0);
    aOutput.push_back(geomOutput.ptr());
  }

  return aOutput.size();
}

//! Pack meshes into a single geometry.
inline iGeometry* __stdcall PackSingleGeometry(iGraphics* apGraphics,
                                                        iGeometry* apGeom,
                                                        tPtr apMeshes,
                                                        tU32 anMeshStride,
                                                        tU32 anNumMeshes,
                                                        tU32 anGeomIndexBase,
                                                        iMaterialLibrary* apMatLib,
                                                        eArrayUsage aUsage,
                                                        tFVF aDefaultFVF)
{
  Ptr<tIUnknownCVec> output = tIUnknownCVec::Create();
  if (PackGeometry(apGraphics,apGeom,
                   apMeshes,anMeshStride,
                   anNumMeshes,anGeomIndexBase,
                   apMatLib,
                   *output,
                   aUsage,
                   aDefaultFVF,
                   eTrue) != 1)
  {
    return NULL;
  }
  QPtr<iGeometry> geom = output->at(0).ptr();
  output->clear(); // make sure references are cleared...
  return geom.GetRawAndSetNull();
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __GEOMETRYPACKER_29385760_H__
