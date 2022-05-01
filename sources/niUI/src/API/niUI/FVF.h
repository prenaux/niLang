#ifndef __FVF_10591779_H__
#define __FVF_10591779_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/Utils/StringTokenizerImpl.h>

namespace ni {
/** \addtogroup niUI
 * @{
 */

//! Maximum vertex size.
const tU32 kMaxVertexSize = 256;

//! FVF components
enum eFVF
{
  //! 3D Position (float3).
  eFVF_Position = 0x00000002,
  //! 3D Position (float3), and one weight (float1).
  eFVF_PositionB1 = 0x00000006,
  //! 3D Position (float3), and two weights (float2).
  eFVF_PositionB2 = 0x00000008,
  //! 3D Position (float3), and three weights (float3).
  eFVF_PositionB3 = 0x0000000a,
  //! 3D Position (float3), and four weights (float4).
  eFVF_PositionB4 = 0x0000000c,

  //! 3D Normal (float3).
  eFVF_Normal   = 0x00000010,
  //! Point size (float).
  eFVF_PointSize  = 0x00000020,
  //! ColorA (ULColor).
  eFVF_ColorA   = 0x00000040,
  //! Reserved (was ColorB).
  eFVF_Reserved1  = 0x00000080,

  //! First texture coordinate (index 0).
  //! \remark The number of dimensions of the texture coordinate can be specified with eFVF_TexCooSizeN.
  //!     The default is 2 dimensions.
  eFVF_Tex1   = 0x00000100,
  //! Second texture coordinate (index 1).
  //! \remark The number of dimensions of the texture coordinate can be specified with eFVF_TexCooSizeN.
  //!     The default is 2 dimensions.
  eFVF_Tex2   = 0x00000200,
  //! Third texture coordinate (index 2).
  //! \remark The number of dimensions of the texture coordinate can be specified with eFVF_TexCooSizeN.
  //!     The default is 2 dimensions.
  eFVF_Tex3   = 0x00000300,
  //! Fourth texture coordinate (index 3).
  //! \remark The number of dimensions of the texture coordinate can be specified with eFVF_TexCooSizeN.
  //!     The default is 2 dimensions.
  eFVF_Tex4   = 0x00000400,
  //! Fifth texture coordinate (index 4).
  //! \remark The number of dimensions of the texture coordinate can be specified with eFVF_TexCooSizeN.
  //!     The default is 2 dimensions.
  eFVF_Tex5   = 0x00000500,
  //! Sixth texture coordinate (index 5).
  //! \remark The number of dimensions of the texture coordinate can be specified with eFVF_TexCooSizeN.
  //!     The default is 2 dimensions.
  eFVF_Tex6   = 0x00000600,
  //! Seventh texture coordinate (index 6).
  //! \remark The number of dimensions of the texture coordinate can be specified with eFVF_TexCooSizeN.
  //!     The default is 2 dimensions.
  eFVF_Tex7   = 0x00000700,
  //! Eigth texture coordinate (index 7).
  //! \remark The number of dimensions of the texture coordinate can be specified with eFVF_TexCooSizeN.
  //!     The default is 2 dimensions.
  eFVF_Tex8   = 0x00000800,

  //! Indices (ULColor).
  eFVF_Indices  = 0x00001000,

  //! \internal
  eFVF_ForceDWORD = 0xFFFFFFFF
};

//! FVF type.
typedef tU32    tFVF;

//! FVF 1D texture coordinate. CoordIndex is the index (0 to 7).
#define eFVF_TexCooSize1(CoordIndex) ni::eFVF((ni::tU32)(3 << (CoordIndex*2 + 16)))
//! FVF 2D texture coordinate. CoordIndex is the index (0 to 7).
#define eFVF_TexCooSize2(CoordIndex) ni::eFVF((ni::tU32)(0))
//! FVF 3D texture coordinate. CoordIndex is the index (0 to 7).
#define eFVF_TexCooSize3(CoordIndex) ni::eFVF((ni::tU32)(1 << (CoordIndex*2 + 16)))
//! FVF 4D texture coordinate. CoordIndex is the index (0 to 7).
#define eFVF_TexCooSize4(CoordIndex) ni::eFVF((ni::tU32)(2 << (CoordIndex*2 + 16)))
//! Get the number of dimensions of the specified texture coordinates. CoordIndex is the index (0 to 7).
#define eFVF_TexCooDim(fvf,CoordIndex)     ((ni::tU32)((0x1432 >> (((fvf) >> ((CoordIndex) * 2 + 14)) & 0xC)) & 0xF))
//! Get the number of texture coordinates of the specified FVF.
#define eFVF_TexNumCoo(fvf)      ((ni::tU32)(((fvf)&eFVF_TexCount_Mask)>>eFVF_TexCount_Shift))
//! FVF position mask.
#define eFVF_Position_Mask       0x00E
//! FVF texture count mask.
#define eFVF_TexCount_Mask       0xf00
//! FVF texture count shift.
#define eFVF_TexCount_Shift      8
//! FVF index based texture coordinate. (same as eFVF_TexN, but index based)
#define eFVF_TexUnit(x)        ((ni::tU32)(((x)+1)<<8))
//! FVF check whether has a position.
#define eFVF_HasPosition(fvf)    (((fvf) & eFVF_Position_Mask) != 0)
//! FVF check whether has weights.
#define eFVF_HasWeights(fvf)     ((((fvf)) & eFVF_Position_Mask) >= ni::eFVF_PositionB1)
//! FVF check number of weights.
#define eFVF_NumWeights(fvf)     (eFVF_HasWeights(fvf)?((((((fvf))-ni::eFVF_PositionB1)&eFVF_Position_Mask)>>1)+1):0)

//! FVF texture count mask.
const tU32 knFVFTexCountMask = 0xf00;
//! FVF texture count shift.
const tU32 knFVFTexCountShift = 0xf00;

//! Get a string that contains the FVF description.
inline cString FVFToString(tFVF aFVF)
{
  cString str;

  if (niFlagIs(aFVF,eFVF_PositionB4))  str += _A("PositionB4");
  else if (niFlagIs(aFVF,eFVF_PositionB3))  str += _A("PositionB3");
  else if (niFlagIs(aFVF,eFVF_PositionB2))  str += _A("PositionB2");
  else if (niFlagIs(aFVF,eFVF_PositionB1))  str += _A("PositionB1");
  else if (niFlagIs(aFVF,eFVF_Position))    str += _A("Position");

  if (niFlagIs(aFVF,eFVF_Indices))  str += _A("|Indices");

  if (niFlagIs(aFVF,eFVF_Normal)) str += _A("|Normal");
  if (niFlagIs(aFVF,eFVF_PointSize))  str += _A("|PSize");
  if (niFlagIs(aFVF,eFVF_ColorA)) str += _A("|ColorA");

  for (tU32 i = 0; i < eFVF_TexNumCoo(aFVF); ++i) {
    str += niFmt(_A("|Tex%d(%d)"),i+1,eFVF_TexCooDim(aFVF,i));
  }

  return str;
}

//! Get a FVF from the specified string.
inline tFVF FVFFromString(const achar* aaszString) {
  cString str(aaszString);
  str.TrimEx(_A(" \n\t[]()"));

  cFlagsStringTokenizer tok;
  astl::vector<cString> vToks;
  StringTokenize(str,vToks,&tok);

  tFVF fvf = 0;

  for (tU32 i = 0; i < vToks.size(); ++i) {
    if (vToks[i].IEq(_A("PositionB4"))) {
      fvf |= eFVF_PositionB4;
    }
    else if (vToks[i].IEq(_A("PositionB3"))) {
      fvf |= eFVF_PositionB3;
    }
    else if (vToks[i].IEq(_A("PositionB2"))) {
      fvf |= eFVF_PositionB2;
    }
    else if (vToks[i].IEq(_A("PositionB1"))) {
      fvf |= eFVF_PositionB1;
    }
    else if (vToks[i].IEq(_A("Position"))) {
      fvf |= eFVF_Position;
    }
    else if (vToks[i].IEq(_A("Indices"))) {
      fvf |= eFVF_Indices;
    }
    else if (vToks[i].IEq(_A("Normal"))) {
      fvf |= eFVF_Normal;
    }
    else if (vToks[i].IEq(_A("PSize")) || vToks[i].IEq(_A("PointSize"))) {
      fvf |= eFVF_PointSize;
    }
    else if (vToks[i].IEq(_A("ColorA")) || vToks[i].IEq(_A("Color0"))) {
      fvf |= eFVF_ColorA;
    }
    else if (vToks[i].StartsWith(_A("Tex"))) {
      const achar* e;
      const achar* p = vToks[i].Chars();
      p += 3; // skip Tex
      if (*p) {
        tU32 nIndex = (tU32)ni::StrToL(p,&e,10);
        if (p != e && nIndex >= 1 && nIndex <= 8) {
          ++p; // skip '('
          if (*p) {
            tU32 nDim = (tU32)ni::StrToL(p,&e,10);
            if (p != e) {
              switch (nDim) {
                case 1: fvf |= eFVF_TexUnit(nIndex-1)|eFVF_TexCooSize1(nIndex-1); break;
                case 2: fvf |= eFVF_TexUnit(nIndex-1)|eFVF_TexCooSize2(nIndex-1); break;
                case 3: fvf |= eFVF_TexUnit(nIndex-1)|eFVF_TexCooSize3(nIndex-1); break;
                case 4: fvf |= eFVF_TexUnit(nIndex-1)|eFVF_TexCooSize4(nIndex-1); break;
              }
            }
            else {
              fvf |= eFVF_TexUnit(nIndex-1)|eFVF_TexCooSize2(nIndex-1);
            }
          }
          else {
            fvf |= eFVF_TexUnit(nIndex-1)|eFVF_TexCooSize2(nIndex-1);
          }
        }
      }
    }
  }

  return fvf;
}

//! Get a string that contains the FVF description.
inline cString FVFToShortString(tFVF aFVF)
{
  cString str;
  if (niFlagIs(aFVF,eFVF_PositionB4))
    str += _A("PB4");
  else if (niFlagIs(aFVF,eFVF_PositionB3))
    str += _A("PB3");
  else if (niFlagIs(aFVF,eFVF_PositionB2))
    str += _A("PB2");
  else if (niFlagIs(aFVF,eFVF_PositionB1))
    str += _A("PB1");
  else if (niFlagIs(aFVF,eFVF_Position))
    str += _A("P");
  if (niFlagIs(aFVF,eFVF_Indices))
    str += _A("I");
  if (niFlagIs(aFVF,eFVF_Normal))
    str += _A("N");
  if (niFlagIs(aFVF,eFVF_PointSize))
    str += _A("S");
  if (niFlagIs(aFVF,eFVF_ColorA))
    str += _A("A");
  for (tU32 i = 0; i < eFVF_TexNumCoo(aFVF); ++i) {
    str += niFmt(_A("T%d%d"),i+1,eFVF_TexCooDim(aFVF,i));
  }
  return str;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//! FVF description class.
class cFVFDescription
{
 public:
  //! Constructor.
  inline cFVFDescription(tFVF aFVF = eFVF(0));
  //! Setup the FVF structure.
  inline void Setup(tFVF aFVF);

  //! Compare two FVF.
  inline tBool operator == (const cFVFDescription& fvf) const;
  //! Compare two FVF.
  inline tBool operator != (const cFVFDescription& fvf) const;

  //! Get the FVF description.
  inline tFVF  GetFVF()     const   { return mFVF;  }
  //! Get the FVF stride.
  inline tU16  GetStride()    const   { return munStride; }

  //! Check if the FVF has a position.
  inline tBool HasPosition()    const   { return munPositionSize >= 12; }
  //! Get the position offset.
  inline tI32  GetPositionOffset() const    { return munPositionSize >= 12 ? 0 : -1; }
  //! Get the position size.
  inline tU32  GetPositionSize()  const   { return munPositionSize; }

  //! Check if the FVF has 1 weight.
  inline tBool HasWeights1()    const   { return niFlagTest(mFVF,eFVF_PositionB1); }
  //! Check if the FVF has 2 weights.
  inline tBool HasWeights2()    const   { return niFlagTest(mFVF,eFVF_PositionB2); }
  //! Check if the FVF has 3 weights.
  inline tBool HasWeights3()    const   { return niFlagTest(mFVF,eFVF_PositionB3); }
  //! Check if the FVF has 4 weights.
  inline tBool HasWeights4()    const   { return niFlagTest(mFVF,eFVF_PositionB4); }
  //! Get the weight offset.
  inline tI16  GetWeightsOffset() const   { return (munPositionSize > 12) ? 12 : -1; }
  //! Get the number of weights.
  inline tU16  GetNumWeights()  const   { return HasWeights4()?4:HasWeights3()?3:HasWeights2()?2:HasWeights1()?1:0; }

  //! Check if the FVF has indices.
  inline tBool HasIndices()   const   { return mnOffIndices >= 0; }
  //! Get the indices offset.
  inline tI16  GetIndicesOffset() const   { return mnOffIndices; }

  //! Check if the FVF has normals.
  inline tBool HasNormal()    const   { return mnOffNormal >= 0; }
  //! Get the normal offset.
  inline tI16  GetNormalOffset()  const   { return mnOffNormal; }

  //! Check if the FVF has a point size.
  inline tBool HasPointSize()   const   { return mnOffPointSize >= 0; }
  //! Get the offset of the point size.
  inline tI16  GetPointSizeOffset() const   { return mnOffPointSize; }

  //! Check if the FVF has a color A.
  inline tBool HasColorA()    const   { return mnOffColorA >= 0; }
  //! Get the offset of the color A.
  inline tI16  GetColorAOffset()  const   { return mnOffColorA; }

  //! Get the number of texture coordinates.
  inline tU16  GetNumTexCoos()        const { return munNumTexCoos; }
  //! Check if the FVF has the specified texture coordinate. (index based)
  inline tBool HasTexCoo(tU32 idx = 0)    const { return idx < munNumTexCoos; }
  //! Get the number of dimensions of the specified texture coordinates. (index based).
  inline tU16  GetTexCooDim(tU32 idx = 0)   const { return munTexSize[idx]>>2; }
  //! Get the offset of the specified texture coordinates. (index based)
  inline tU16  GetTexCooOffset(tU32 idx = 0)  const { return munOffTex[idx]; }

 private:
  //! FVF of the structure.
  tFVF  mFVF;
  //! Stride of the FVF in bytes.
  tU16  munStride;
  //! Size in bytes of the Position component, 0 if it don't exists.
  tU16  munPositionSize;
  //! Offset of the indices component, -1 if it don't exists.
  tI16  mnOffIndices;
  //! Offset of the normal component, -1 if it don't exists.
  tI16  mnOffNormal;
  //! Offset of the point size component, -1 if it don't exists.
  tI16  mnOffPointSize;
  //! Offset the the colora component, -1 if it don't exists.
  tI16  mnOffColorA;
  //! Number of texture coordinates.
  tU16  munNumTexCoos;
  //! Offset of the texture coordinates.
  tU16  munOffTex[8];
  //! Size in bytes of the texture coordinates.
  tU16  munTexSize[8];
};

//////////////////////////////////////////////////////////////////////////////////////////////
//! FVF vertex.
class cFVFVertex
{
 public:
  inline cFVFVertex();
  inline cFVFVertex(const cFVFDescription* FVF, tPtr pCurrent = NULL);

  inline cFVFVertex& SetEx(const cFVFDescription* FVF, tPtr pCurrent = NULL);
  inline cFVFVertex& Set(tPtr pCurrent);

  inline const cFVFDescription&   GetFVF()    const   { return *mpFVF; }
  inline tPtr       GetBase()   const   { return mpCurrent; }

  inline sVec3f& Position()    const   { return *((sVec3f*)(mpCurrent)); }
  inline tF32&    Weights1()    const   { return *((tF32*)(mpCurrent+12)); }
  inline sVec2f& Weights2()    const   { return *((sVec2f*)(mpCurrent+12)); }
  inline sVec3f& Weights3()    const   { return *((sVec3f*)(mpCurrent+12)); }
  inline sVec4f& Weights4()    const   { return *((sVec4f*)(mpCurrent+12)); }
  inline tU32&    Indices()   const   { return *((tU32*)(mpCurrent+mpFVF->GetIndicesOffset())); }
  inline sVec3f& Normal()    const   { return *((sVec3f*)(mpCurrent+mpFVF->GetNormalOffset())); }
  inline tF32&    PointSize()   const   { return *((tF32*)(mpCurrent+mpFVF->GetPointSizeOffset())); }
  inline tU32&    ColorA()    const   { return *((tU32*)(mpCurrent+mpFVF->GetColorAOffset())); }
  inline tF32&    TexCoo1(int idx = 0) const  { return *((tF32*)(mpCurrent+mpFVF->GetTexCooOffset(idx))); }
  inline sVec2f& TexCoo2(int idx = 0) const  { return *((sVec2f*)(mpCurrent+mpFVF->GetTexCooOffset(idx))); }
  inline sVec3f& TexCoo3(int idx = 0) const  { return *((sVec3f*)(mpCurrent+mpFVF->GetTexCooOffset(idx))); }
  inline sVec4f& TexCoo4(int idx = 0) const  { return *((sVec4f*)(mpCurrent+mpFVF->GetTexCooOffset(idx))); }

  inline const sVec3f& GetPosition() const   { return *((sVec3f*)(mpCurrent)); }
  inline const tF32&    GetWeights1() const   { return *((tF32*)(mpCurrent+12)); }
  inline const sVec2f& GetWeights2() const   { return *((sVec2f*)(mpCurrent+12)); }
  inline const sVec3f& GetWeights3() const   { return *((sVec3f*)(mpCurrent+12)); }
  inline const sVec4f& GetWeights4() const   { return *((sVec4f*)(mpCurrent+12)); }
  inline const tU32&    GetIndices()  const   { return *((tU32*)(mpCurrent+mpFVF->GetIndicesOffset())); }
  inline const sVec3f& GetNormal()   const   { return *((sVec3f*)(mpCurrent+mpFVF->GetNormalOffset())); }
  inline const tF32&    GetPointSize()  const   { return *((tF32*)(mpCurrent+mpFVF->GetPointSizeOffset())); }
  inline const tU32&    GetColorA()   const   { return *((tU32*)(mpCurrent+mpFVF->GetColorAOffset())); }
  inline tF32       GetTexCoo1(int idx = 0) const { return *((tF32*)(mpCurrent+mpFVF->GetTexCooOffset(idx))); }
  inline const sVec2f& GetTexCoo2(int idx = 0) const { return *((sVec2f*)(mpCurrent+mpFVF->GetTexCooOffset(idx))); }
  inline const sVec3f& GetTexCoo3(int idx = 0) const { return *((sVec3f*)(mpCurrent+mpFVF->GetTexCooOffset(idx))); }
  inline const sVec4f& GetTexCoo4(int idx = 0) const { return *((sVec4f*)(mpCurrent+mpFVF->GetTexCooOffset(idx))); }

  inline void SetPosition(const sVec3f& aV)  { *((sVec3f*)(mpCurrent)) = aV; }
  inline void SetWeights1(const tF32&  aV)    { *((tF32*)(mpCurrent+12)) = aV; }
  inline void SetWeights2(const sVec2f& aV)  { *((sVec2f*)(mpCurrent+12)) = aV; }
  inline void SetWeights3(const sVec3f& aV)  { *((sVec3f*)(mpCurrent+12)) = aV; }
  inline void SetWeights4(const sVec4f& aV)  { *((sVec4f*)(mpCurrent+12)) = aV; }
  inline void SetIndices(const tU32&   aV)    { *((tU32*)(mpCurrent+mpFVF->GetIndicesOffset())) = aV; }
  inline void SetNormal(const sVec3f& aV)    { *((sVec3f*)(mpCurrent+mpFVF->GetNormalOffset())) = aV; }
  inline void SetPointSize(const tF32&   aV)  { *((tF32*)(mpCurrent+mpFVF->GetPointSizeOffset())) = aV; }
  inline void SetColorA(const tU32&  aV)    { *((tU32*)(mpCurrent+mpFVF->GetColorAOffset())) = aV; }
  inline void SetTexCoo1(int idx, tF32 aV) { *((tF32*)(mpCurrent+mpFVF->GetTexCooOffset(idx))) = aV; }
  inline void SetTexCoo2(int idx, const sVec2f& aV) { *((sVec2f*)(mpCurrent+mpFVF->GetTexCooOffset(idx))) = aV; }
  inline void SetTexCoo3(int idx, const sVec3f& aV) { *((sVec3f*)(mpCurrent+mpFVF->GetTexCooOffset(idx))) = aV; }
  inline void SetTexCoo4(int idx, const sVec4f& aV) { *((sVec4f*)(mpCurrent+mpFVF->GetTexCooOffset(idx))) = aV; }

  inline void Copy(const cFVFVertex& src);
  // Interpolate this vertex and B, and copy the result in this
  inline void Lerp(const cFVFVertex& B, tF32 fF);

 private:
  const cFVFDescription* mpFVF;
  tPtr    mpCurrent;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//! FVF stream class.
class cFVFStream
{
 public:
  inline cFVFStream();
  inline cFVFStream(const cFVFDescription* FVF, tPtr pBase = NULL, tU32 ulCount = 0);

  inline void Setup(const cFVFDescription* FVF, tPtr pBase = NULL, tU32 ulCount = 0);
  inline void SetBase(tPtr pBase, tU32 ulCount);

  inline tBool IsOK() const { return mpBase != NULL; }

  inline tSize          GetNumVertices()  const { return (mpEnd-mpBase)/mpFVF->GetStride(); }
  inline const cFVFDescription& GetFVFDescription() const { return *mpFVF; }
  inline tPtr           GetBase()     const { return mpBase; }

  inline void   Reset()         { mpCurrent = mpBase; mVertex.Set(mpCurrent); }
  inline void   SetCurrentIndex(tSize idx){ mpCurrent = mpBase+(mpFVF->GetStride()*idx); mVertex.Set(mpCurrent); }
  inline tSize  GetCurrentIndex() const { return (mpCurrent-mpBase)/mpFVF->GetStride(); }
  inline tBool  End() const       { return mpCurrent >= mpEnd; }
  inline void   Next()          { mpCurrent += mpFVF->GetStride(); mVertex.Set(mpCurrent); }
  inline void   Prev()          { mpCurrent -= mpFVF->GetStride(); mVertex.Set(mpCurrent); }
  inline tPtr   Current() const     { return mpCurrent; }
  inline tPtr   GetVertex(tU32 ulIdx) const   { return mpBase+(ulIdx*mpFVF->GetStride()); }

  inline cFVFVertex& Vertex() const { return mVertex; }

  inline void Copy(cFVFStream& src);
  inline void Lerp(cFVFStream& B, tF32 fF);

 private:
  const cFVFDescription* mpFVF;
  tPtr    mpBase;
  tPtr    mpEnd;
  tPtr    mpCurrent;
  mutable cFVFVertex  mVertex;
};

//////////////////////////////////////////////////////////////////////////////////////////////
// cFVFDescription

//! Constructor.
inline cFVFDescription::cFVFDescription(tFVF aFVF)
{
  if (aFVF)
    Setup(aFVF);
}

//! Setup the FVF structure.
inline void cFVFDescription::Setup(tFVF aFVF)
{
  mFVF = aFVF;
  munPositionSize = 0;
  munNumTexCoos = 0;
  mnOffIndices = -1;
  mnOffNormal = -1;
  mnOffPointSize = -1;
  mnOffColorA = -1;
  memset(munOffTex, 0, sizeof(tU16)*8);
  memset(munTexSize, 0, sizeof(tU16)*8);

  tU16 unCurrentOff = 0;
  if (niFlagTest(mFVF,eFVF_PositionB4))  munPositionSize = 28;
  else if (niFlagTest(mFVF,eFVF_PositionB3))  munPositionSize = 24;
  else if (niFlagTest(mFVF,eFVF_PositionB2))  munPositionSize = 20;
  else if (niFlagTest(mFVF,eFVF_PositionB1))  munPositionSize = 16;
  else if (niFlagTest(mFVF,eFVF_Position))  munPositionSize = 12;
  unCurrentOff += munPositionSize;

  if (niFlagTest(mFVF,eFVF_Indices))
  {
    mnOffIndices = unCurrentOff;
    unCurrentOff += 4;
  }

  if (niFlagTest(mFVF,eFVF_Normal))
  {
    mnOffNormal = unCurrentOff;
    unCurrentOff += 12;
  }

  if (niFlagTest(mFVF,eFVF_PointSize))
  {
    mnOffPointSize = unCurrentOff;
    unCurrentOff += 4;
  }

  if (niFlagTest(mFVF,eFVF_ColorA))
  {
    mnOffColorA = unCurrentOff;
    unCurrentOff += 4;
  }

  munNumTexCoos  = (tU16)eFVF_TexNumCoo(mFVF);
  for (tU32 i = 0; i < munNumTexCoos; ++i) {
    munOffTex[i] = unCurrentOff;
    munTexSize[i] = (tU16)(eFVF_TexCooDim(mFVF,i)<<2);
    unCurrentOff += munTexSize[i];
  }

  munStride = unCurrentOff;
}

//! Compare two FVF
inline tBool cFVFDescription::operator == (const cFVFDescription& fvf) const
{
  return mFVF == fvf.mFVF && munStride == fvf.munStride;
}

//! Compare two FVF
inline tBool cFVFDescription::operator != (const cFVFDescription& fvf) const
{
  return mFVF != fvf.mFVF || munStride != fvf.munStride;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// cFVFVertex

///////////////////////////////////////////////
inline cFVFVertex::cFVFVertex()
{
  mpFVF = NULL;
  mpCurrent = NULL;
}

///////////////////////////////////////////////
inline cFVFVertex::cFVFVertex(const cFVFDescription* FVF, tPtr pCurrent)
{
  SetEx(FVF, pCurrent);
}

///////////////////////////////////////////////
inline cFVFVertex& cFVFVertex::SetEx(const cFVFDescription* FVF, tPtr pCurrent)
{
  mpFVF = FVF;
  if (pCurrent)
    mpCurrent = pCurrent;
  return *this;
}

///////////////////////////////////////////////
inline cFVFVertex& cFVFVertex::Set(tPtr pCurrent)
{
  mpCurrent = pCurrent;
  return *this;
}

///////////////////////////////////////////////
inline void cFVFVertex::Copy(const cFVFVertex& src)
{
  if (GetFVF() == src.GetFVF())
  {
    memcpy(GetBase(), src.GetBase(), GetFVF().GetStride());
  }
  else
  {
    const cFVFDescription& srcFVF = src.GetFVF();
    if (mpFVF->HasPosition() && srcFVF.HasPosition())
      Position() = src.Position();

    if (mpFVF->HasWeights4() && srcFVF.HasWeights4())
      Weights4() = src.Weights4();
    else if (mpFVF->HasWeights3() && srcFVF.HasWeights3())
      Weights3() = src.Weights3();
    else if (mpFVF->HasWeights2() && srcFVF.HasWeights2())
      Weights2() = src.Weights2();
    else if (mpFVF->HasWeights1() && srcFVF.HasWeights1())
      Weights1() = src.Weights1();

    if (mpFVF->HasIndices() && srcFVF.HasIndices())
      Indices() = src.Indices();

    if (mpFVF->HasNormal() && srcFVF.HasNormal())
      Normal() = src.Normal();

    if (mpFVF->HasPointSize() && srcFVF.HasPointSize())
      PointSize() = src.PointSize();

    if (mpFVF->HasColorA() && srcFVF.HasColorA())
      ColorA() = src.ColorA();

    tU32 ulNumTexCoos = ni::Min(mpFVF->GetNumTexCoos(),srcFVF.GetNumTexCoos());
    for (tU32 i = 0; i < ulNumTexCoos; ++i)
    {
      if (mpFVF->GetTexCooDim(i) != srcFVF.GetTexCooDim(i))
        continue;

      if (mpFVF->GetTexCooDim(i) == 4)
        TexCoo4(i) = src.TexCoo4(i);
      else if (mpFVF->GetTexCooDim(i) == 3)
        TexCoo3(i) = src.TexCoo3(i);
      else if (mpFVF->GetTexCooDim(i) == 2)
        TexCoo2(i) = src.TexCoo2(i);
    }
  }
}

///////////////////////////////////////////////
// Interpolate this vertex and B, and copy the result in this
inline void cFVFVertex::Lerp(const cFVFVertex& B, tF32 fF)
{
  const cFVFDescription& BFVF = B.GetFVF();
  if (mpFVF->HasPosition() && BFVF.HasPosition())
    Position() = ni::Lerp(Position(), B.Position(), fF);

  if (mpFVF->HasWeights4() && BFVF.HasWeights4())
    Weights4() = ni::Lerp(Weights4(), B.Weights4(), fF);
  else if (mpFVF->HasWeights3() && BFVF.HasWeights3())
    Weights3() = ni::Lerp(Weights3(), B.Weights3(), fF);
  else if (mpFVF->HasWeights2() && BFVF.HasWeights2())
    Weights2() = ni::Lerp(Weights2(), B.Weights2(), fF);
  else if (mpFVF->HasWeights1() && BFVF.HasWeights1())
    Weights1() = ni::Lerp(Weights1(), B.Weights1(), fF);

  //    if (mpFVF->HasIndices() && BFVF.HasIndices())
  //    Interpolating matrix indices just don't make sense

  if (mpFVF->HasNormal() && BFVF.HasNormal())
    Normal() = ni::Lerp(Normal(), B.Normal(), fF);

  if (mpFVF->HasPointSize() && BFVF.HasPointSize())
    PointSize() = ni::Lerp(PointSize(), B.PointSize(), fF);

  if (mpFVF->HasColorA() && BFVF.HasColorA())
    ColorA() = ULColorLerp(ColorA(), B.ColorA(), fF);

  tU32 ulNumTexCoos = ni::Min(mpFVF->GetNumTexCoos(),BFVF.GetNumTexCoos());
  for (tU32 i = 0; i < ulNumTexCoos; ++i)
  {
    tU32 ulTexCooDim = mpFVF->GetTexCooDim(i);
    if (ulTexCooDim != BFVF.GetTexCooDim(i))
      continue;

    if (ulTexCooDim == 4)
      TexCoo4(i) = ni::Lerp(TexCoo4(i), B.TexCoo4(i), fF);
    else if (ulTexCooDim == 3)
      TexCoo3(i) = ni::Lerp(TexCoo3(i), B.TexCoo3(i), fF);
    else if (ulTexCooDim == 2)
      TexCoo2(i) = ni::Lerp(TexCoo2(i), B.TexCoo2(i), fF);
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
// cFVFStream

///////////////////////////////////////////////
inline cFVFStream::cFVFStream()
{
  mpFVF = NULL;
  mpCurrent = mpBase = mpEnd = NULL;
}

///////////////////////////////////////////////
inline cFVFStream::cFVFStream(const cFVFDescription* FVF, tPtr pBase, tU32 ulCount)
{
  Setup(FVF,pBase,ulCount);
}

///////////////////////////////////////////////
inline void cFVFStream::Setup(const cFVFDescription* FVF, tPtr pBase, tU32 ulCount)
{
  mpFVF = FVF;
  mpCurrent = mpBase = pBase;
  if (mpFVF) {
    mpEnd = mpBase+(mpFVF->GetStride()*ulCount);
  }
  else {
    mpEnd = mpBase;
  }
  mVertex.SetEx(FVF, mpCurrent);
}

///////////////////////////////////////////////
inline void cFVFStream::SetBase(tPtr pBase, tU32 ulCount)
{
  mpCurrent = mpBase = pBase;
  if (mpFVF) {
    mpEnd = mpBase+(mpFVF->GetStride()*ulCount);
  }
  else {
    mpEnd = mpBase;
  }
  mVertex.Set(mpCurrent);
}

///////////////////////////////////////////////
inline void cFVFStream::Copy(cFVFStream& src)
{
  if (GetFVFDescription() == src.GetFVFDescription())
  {
    tSize ulNumVert = ni::Min(GetNumVertices(),src.GetNumVertices());
    memcpy(GetBase(), src.GetBase(), mpFVF->GetStride()*ulNumVert);
  }
  else
  {
    for (Reset(), src.Reset(); !End() && !src.End(); Next(), src.Next())
      Vertex().Copy(src.Vertex());
  }
}

///////////////////////////////////////////////
inline void cFVFStream::Lerp(cFVFStream& B, tF32 fF)
{
  for (Reset(), B.Reset(); !End() && !B.End(); Next(), B.Next())
    Vertex().Lerp(B.Vertex(),fF);
}

///////////////////////////////////////////////
//! Get the offset of a component of the given FVF.
inline tU16 FVFGetComponentOffset(tFVF aFVF, eFVF aC)
{
  tU16 off = 0;

  if(aC == eFVF_Position ||
     aC == eFVF_PositionB4 || aC == eFVF_PositionB3 ||
     aC == eFVF_PositionB2 || aC == eFVF_PositionB1)
    return 0;

  if (niFlagTest(aFVF,eFVF_PositionB4)) {
    off += 28;
  }
  else if(niFlagTest(aFVF,eFVF_PositionB3)) {
    off += 24;
  }
  else if(niFlagTest(aFVF,eFVF_PositionB2)) {
    off += 20;
  }
  else if(niFlagTest(aFVF,eFVF_PositionB1)) {
    off += 16;
  }
  else if(niFlagTest(aFVF,eFVF_Position)) {
    off += 12;
  }

#define COMP(fvf,sz) if(aC == fvf) return off; if(niFlagTest(aFVF,fvf)) off += sz;
  COMP(eFVF_Indices, 4);
  COMP(eFVF_Normal, 12);
  COMP(eFVF_PointSize, 4);
  COMP(eFVF_ColorA, 4);

  tU32 ulNumTexCoos  = eFVF_TexNumCoo(aFVF);
  for (tU32 i = 0; i < ulNumTexCoos; ++i)
  {
    if (aC == eFVF(eFVF_TexUnit(i)))
      return off;
    off += (tU16)(eFVF_TexCooDim(aFVF,i)<<2);
  }

#undef COMP

  return off;
}

///////////////////////////////////////////////
//! Get the size of a FVF.
inline tU16 FVFGetStride(tFVF aFVF)
{
  tU16 off = 0;

  if (niFlagTest(aFVF,eFVF_PositionB4))  off += 28;
  else if (niFlagTest(aFVF,eFVF_PositionB3))  off += 24;
  else if (niFlagTest(aFVF,eFVF_PositionB2))  off += 20;
  else if (niFlagTest(aFVF,eFVF_PositionB1))  off += 16;
  else if (niFlagTest(aFVF,eFVF_Position))  off += 12;

  if (niFlagTest(aFVF,eFVF_Indices)) off += 4;
  if (niFlagTest(aFVF,eFVF_Normal)) off += 12;
  if (niFlagTest(aFVF,eFVF_PointSize)) off += 4;
  if (niFlagTest(aFVF,eFVF_ColorA)) off += 4;

  tU32 ulNumTexCoos  = eFVF_TexNumCoo(aFVF);
  for (tU32 i = 0; i < ulNumTexCoos; ++i)
    off += (tU16)(eFVF_TexCooDim(aFVF,i)<<2);

  return off;
}

///////////////////////////////////////////////
//! Copy FVF stream of the same or different format.
inline void FVFCopy(tPtr pDest, const cFVFDescription& DestFVF, tPtr pSrc, const cFVFDescription& SrcFVF, tU32 ulCount)
{
  if (DestFVF == SrcFVF)
  { // Same FVF so just need a simple memcpy
    memcpy(pDest, pSrc, DestFVF.GetStride()*ulCount);
    return;
  }

  tU8 *pD, *pS;
  tU32 i = 0;
  tU32 ulDestStride = DestFVF.GetStride();
  tU32 ulSrcStride = SrcFVF.GetStride();
  if (DestFVF.HasPosition() && SrcFVF.HasPosition())
  {
    pD = (tU8*)pDest;
    pS = (tU8*)pSrc;
    tU32 ulPositionSize = Min(DestFVF.GetPositionSize(), SrcFVF.GetPositionSize());
    i = ulCount;
    if (ulPositionSize == 12)
    {
      while (i--)
      {
        *((sVec3f*)(pD)) = *((sVec3f*)(pS));
        pD += ulDestStride;
        pS += ulSrcStride;
      }
    }
    else if (ulPositionSize == 16)
    {
      while (i--)
      {
        *((sVec4f*)(pD)) = *((sVec4f*)(pS));
        pD += ulDestStride;
        pS += ulSrcStride;
      }
    }
    else if (ulPositionSize == 20)
    {
      while (i--)
      {
        *((sVec3f*)(pD)) = *((sVec3f*)(pS)); pD += 12; pS += 12;
        *((sVec2f*)(pD)) = *((sVec2f*)(pS));
        pD += ulDestStride-12;
        pS += ulSrcStride-12;
      }
    }
    else if (ulPositionSize == 24)
    {
      while (i--)
      {
        *((sVec3f*)(pD)) = *((sVec3f*)(pS)); pD += 12; pS += 12;
        *((sVec3f*)(pD)) = *((sVec3f*)(pS));
        pD += ulDestStride-12;
        pS += ulSrcStride-12;
      }
    }
    else if (ulPositionSize == 28)
    {
      while (i--)
      {
        *((sVec3f*)(pD)) = *((sVec3f*)(pS)); pD += 12; pS += 12;
        *((sVec4f*)(pD)) = *((sVec4f*)(pS));
        pD += ulDestStride-12;
        pS += ulSrcStride-12;
      }
    }
  }

  if (DestFVF.HasIndices() && SrcFVF.HasIndices())
  {
    pD = ((tU8*)pDest)+DestFVF.GetIndicesOffset();
    pS = ((tU8*)pSrc)+SrcFVF.GetIndicesOffset();
    i = ulCount;
    while (i--)
    {
      *((tU32*)(pD)) = *((tU32*)(pS));
      pD += ulDestStride;
      pS += ulSrcStride;
    }
  }

  if (DestFVF.HasNormal() && SrcFVF.HasNormal())
  {
    pD = ((tU8*)pDest)+DestFVF.GetNormalOffset();
    pS = ((tU8*)pSrc)+SrcFVF.GetNormalOffset();
    i = ulCount;
    while (i--)
    {
      *((sVec3f*)(pD)) = *((sVec3f*)(pS));
      pD += ulDestStride;
      pS += ulSrcStride;
    }
  }

  if (DestFVF.HasPointSize() && SrcFVF.HasPointSize())
  {
    pD = ((tU8*)pDest)+DestFVF.GetPointSizeOffset();
    pS = ((tU8*)pSrc)+SrcFVF.GetPointSizeOffset();
    i = ulCount;
    while (i--)
    {
      *((tF32*)(pD)) = *((tF32*)(pS));
      pD += ulDestStride;
      pS += ulSrcStride;
    }
  }

  if (DestFVF.HasColorA() && SrcFVF.HasColorA())
  {
    pD = ((tU8*)pDest)+DestFVF.GetColorAOffset();
    pS = ((tU8*)pSrc)+SrcFVF.GetColorAOffset();
    i = ulCount;
    while (i--)
    {
      *((tU32*)(pD)) = *((tU32*)(pS));
      pD += ulDestStride;
      pS += ulSrcStride;
    }
  }

  if (DestFVF.GetNumTexCoos() && SrcFVF.GetNumTexCoos())
  {
    tU32 ulNumTexCoos = Min(DestFVF.GetNumTexCoos(), SrcFVF.GetNumTexCoos());
    for (tU32 j = 0; j < ulNumTexCoos; ++j)
    {
      pD = ((tU8*)pDest)+DestFVF.GetTexCooOffset(j);
      pS = ((tU8*)pSrc)+SrcFVF.GetTexCooOffset(j);
      tU32 ulTexCooDim = Min(DestFVF.GetTexCooDim(j),SrcFVF.GetTexCooDim(j));
      i = ulCount;
      if (ulTexCooDim == 1)
      {
        while (i--)
        {
          *((tF32*)(pD)) = *((tF32*)(pS));
          pD += ulDestStride;
          pS += ulSrcStride;
        }
      }
      else if (ulTexCooDim == 2)
      {
        while (i--)
        {
          *((sVec2f*)(pD)) = *((sVec2f*)(pS));
          pD += ulDestStride;
          pS += ulSrcStride;
        }
      }
      else if (ulTexCooDim == 3)
      {
        while (i--)
        {
          *((sVec3f*)(pD)) = *((sVec3f*)(pS));
          pD += ulDestStride;
          pS += ulSrcStride;
        }
      }
      else if (ulTexCooDim == 4)
      {
        while (i--)
        {
          *((sVec4f*)(pD)) = *((sVec4f*)(pS));
          pD += ulDestStride;
          pS += ulSrcStride;
        }
      }
    }
  }
}

///////////////////////////////////////////////
//! Lerp two vertex of the same FVF.
inline tPtr FVFLerp(tFVF aFVF, tPtr _pOut, const tPtr _pA, const tPtr _pB, tF32 fF)
{
  tU8* pOut = (tU8*)_pOut;
  tU8* pA = (tU8*)_pA;
  tU8* pB = (tU8*)_pB;

  if (aFVF == tFVF(eFVF_Position)) {
    *((sVec3f*)pOut) = ni::Lerp(*((sVec3f*)pA), *((sVec3f*)pB), fF);
    return pOut;
  }

#define INCR(off) pOut += off; pA += off; pB += off;
  if(niFlagTest(aFVF,eFVF_PositionB4)) {
    *((sVec3f*)pOut) = ni::Lerp(*((sVec3f*)pA), *((sVec3f*)pB), fF); INCR(12);
    *((sVec4f*)pOut) = ni::Lerp(*((sVec4f*)pA), *((sVec4f*)pB), fF); INCR(16);
  }
  else if(niFlagTest(aFVF,eFVF_PositionB3)) {
    *((sVec3f*)pOut) = ni::Lerp(*((sVec3f*)pA), *((sVec3f*)pB), fF); INCR(12);
    *((sVec3f*)pOut) = ni::Lerp(*((sVec3f*)pA), *((sVec3f*)pB), fF); INCR(12);
  }
  else if(niFlagTest(aFVF,eFVF_PositionB2)) {
    *((sVec3f*)pOut) = ni::Lerp(*((sVec3f*)pA), *((sVec3f*)pB), fF); INCR(12);
    *((sVec2f*)pOut) = ni::Lerp(*((sVec2f*)pA), *((sVec2f*)pB), fF); INCR(8);
  }
  else if(niFlagTest(aFVF,eFVF_PositionB1)) {
    *((sVec4f*)pOut) = ni::Lerp(*((sVec4f*)pA), *((sVec4f*)pB), fF); INCR(16);
  }
  else if(niFlagTest(aFVF,eFVF_Position)) {
    *((sVec3f*)pOut) = ni::Lerp(*((sVec3f*)pA), *((sVec3f*)pB), fF); INCR(12);
  }

  if (niFlagTest(aFVF,eFVF_Indices)) {
    *((tU32*)(pOut)) = *((tU32*)(pA)); INCR(4);
  }

  if (niFlagTest(aFVF,eFVF_Normal)) {
    *((sVec3f*)pOut) = ni::Lerp(*((sVec3f*)pA), *((sVec3f*)pB), fF); INCR(12);
  }

  if (niFlagTest(aFVF,eFVF_PointSize)) {
    *((tF32*)(pOut)) = Lerp(*((tF32*)(pA)), *((tF32*)(pB)), fF); INCR(4);
  }

  if (niFlagTest(aFVF,eFVF_ColorA)) {
    *((tU32*)(pOut)) = ULColorLerp(*((tU32*)(pA)), *((tU32*)(pB)), fF); INCR(4);
  }

  tU32 ulNumTexCoos = eFVF_TexNumCoo(aFVF);
  for (tU32 i = 0; i <= ulNumTexCoos; ++i)
  {
    if(niFlagTest(aFVF,eFVF_TexCooSize1(i))) {
      *((tF32*)(pOut)) = ni::Lerp(*((tF32*)(pA)), *((tF32*)(pB)), fF); INCR(4);
    }
    if(niFlagTest(aFVF,eFVF_TexCooSize3(i))) {
      *((sVec3f*)pOut) = ni::Lerp(*((sVec3f*)pA), *((sVec3f*)pB), fF); INCR(12);
    }
    if(niFlagTest(aFVF,eFVF_TexCooSize4(i))) {
      *((sVec4f*)pOut) = ni::Lerp(*((sVec4f*)pA), *((sVec4f*)pB), fF); INCR(16);
    }
    else {
      *((sVec2f*)pOut) = ni::Lerp(*((sVec2f*)pA), *((sVec2f*)pB), fF); INCR(8);
    }
  }

#undef INCR
  return _pOut;
}

//////////////////////////////////////////////////////////////////////////////////////////////
enum eVertexFormat {
  eVertexFormat_P = eFVF_Position,
  eVertexFormat_PA = eFVF_Position|eFVF_ColorA,
  eVertexFormat_PN = eFVF_Position|eFVF_Normal,
  eVertexFormat_PNA = eFVF_Position|eFVF_Normal|eFVF_ColorA,
  eVertexFormat_PNT1 = eFVF_Position|eFVF_Normal|eFVF_Tex1,
  eVertexFormat_PNAT1 = eFVF_Position|eFVF_Normal|eFVF_ColorA|eFVF_Tex1,
  eVertexFormat_PNT2 = eFVF_Position|eFVF_Normal|eFVF_Tex2,
  eVertexFormat_PNAT2 = eFVF_Position|eFVF_Normal|eFVF_ColorA|eFVF_Tex2,
  eVertexFormat_PB4INT1 = eFVF_PositionB4|eFVF_Indices|eFVF_Normal|eFVF_Tex1,
  eVertexFormat_PB4INAT1 = eFVF_PositionB4|eFVF_Indices|eFVF_Normal|eFVF_ColorA|eFVF_Tex1,
  //! \internal
  eVertexFormat_ForceDWORD = 0xFFFFFFFF
};

struct sVertexP {
  enum { eFVF = eVertexFormat_P };
  sVec3f pos;
};

struct sVertexPA {
  enum { eFVF = eVertexFormat_PA };
  sVec3f pos;
  tU32   colora;
};

struct sVertexPN {
  enum { eFVF = eVertexFormat_PN };
  sVec3f pos;
  sVec3f normal;
};

struct sVertexPNA {
  enum { eFVF = eVertexFormat_PNA };
  sVec3f pos;
  sVec3f normal;
  tU32   colora;
};

struct sVertexPNT1 {
  enum { eFVF = eVertexFormat_PNT1 };
  sVec3f pos;
  sVec3f normal;
  sVec2f tex1;
};

struct sVertexPNAT1 {
  enum { eFVF = eVertexFormat_PNAT1 };
  sVec3f pos;
  sVec3f normal;
  tU32   colora;
  sVec2f tex1;
};
typedef sVertexPNAT1 tVertexCanvas;

struct sVertexPNT2 {
  enum { eFVF = eVertexFormat_PNT2 };
  sVec3f pos;
  sVec3f normal;
  sVec2f tex1;
  sVec2f tex2;
};

struct sVertexPNAT2 {
  enum { eFVF = eVertexFormat_PNAT2 };
  sVec3f pos;
  sVec3f normal;
  tU32   colora;
  sVec2f tex1;
  sVec2f tex2;
};
typedef sVertexPNAT2 tVertexRigid;

struct sVertexPB4INT1 {
  enum { eFVF = eVertexFormat_PB4INT1 };
  sVec3f pos;
  tF32   weights[4];
  union {
    tU32 indices;
    tU8  i[4];
  };
  sVec3f normal;
  sVec2f tex1;
};

struct sVertexPB4INAT1 {
  enum { eFVF = eVertexFormat_PB4INAT1 };
  sVec3f pos;
  tF32   weights[4];
  union {
    tU32 indices;
    tU8  i[4];
  };
  sVec3f normal;
  tU32   colora;
  sVec2f tex1;
};
typedef sVertexPB4INAT1 tVertexSkin;

enum eVertexStreamIndex
{
  eVertexStreamIndex_Position = 0,
  eVertexStreamIndex_Weights = 11,
  eVertexStreamIndex_Indices = 12,
  eVertexStreamIndex_Normal = 9,
  eVertexStreamIndex_ColorA = 10,
  eVertexStreamIndex_Tex1 = 1,
  eVertexStreamIndex_Tex2 = 2,
  eVertexStreamIndex_Tex3 = 3,
  eVertexStreamIndex_Tex4 = 4,
  eVertexStreamIndex_Tex5 = 5,
  eVertexStreamIndex_Tex6 = 6,
  eVertexStreamIndex_Tex7 = 7,
  eVertexStreamIndex_Tex8 = 8,
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}; // End of ni
#endif // __FVF_10591779_H__
