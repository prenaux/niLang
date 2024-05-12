#ifndef __NUSPLINE_26378882_H__
#define __NUSPLINE_26378882_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/StringDef.h>
#include <niLang/Utils/UnknownImpl.h>
#include "API/niUI/INUSpline.h"

using namespace ni;

//////////////////////////////////////////////////////////////////////////////////////////////
// cNUSpline declaration.
class cNUSpline : public ni::ImplRC<ni::iNUSpline,ni::eImplFlags_Default,ni::iSerializable>
{
  niBeginClass(cNUSpline);

  struct sNode
  {
    sVec3f mvPos;
    sVec3f mvVel;
    tF32    mfTime;
    tF32    mfDist;
    sNode() { mfDist = 0.0f; }
  };
  typedef astl::vector<sNode>     tNodeVec;
  typedef tNodeVec::iterator      tNodeVecIt;
  typedef tNodeVec::const_iterator  tNodeVecCIt;

 public:

  //! Constructor.
  cNUSpline(eNUSplineType aType);
  //! Destructor.
  ~cNUSpline();

  //! Zeros all the class members.
  void ZeroMembers();
  //! Sanity check.
  ni::tBool __stdcall IsOK() const;

  //// ni::iSerializable ///////////////////////
  const achar* __stdcall GetSerializeObjectTypeID() const;
  tSize __stdcall Serialize(iFile* apFile, eSerializeMode aMode);
  //// ni::iSerializable ///////////////////////

  //// ni::iNUSpline ///////////////////////////
  tBool __stdcall SetType(eNUSplineType aType);
  eNUSplineType __stdcall GetType() const;
  void __stdcall Clear();
  void __stdcall AddNode(const sVec4f &avNode);
  tBool __stdcall RemoveNode(tU32 anIndex);
  tU32 __stdcall GetNumNodes() const;
  tBool __stdcall SetNode(tU32 anIndex, const sVec4f& avNode);
  sVec4f __stdcall GetNode(tU32 anIndex) const;
  tF32 __stdcall GetNodeDistance(tU32 anIndex) const;
  tF32 __stdcall GetLength() const;
  sVec3f __stdcall GetPosition(tF32 afTime);
  sVec3f __stdcall GetRelativePosition(tF32 afTime);
  tBool __stdcall SerializeDataTable(iDataTable* apDT, tSerializeFlags aFlags);
  //// ni::iNUSpline ///////////////////////////

 private:
  tBool UpdateSpline();
  sVec3f GetStartVel(tU32 anIndex) const;
  sVec3f GetEndVel(tU32 anIndex) const;
  void Smooth(tU32 anCount);
  void Constrain();

  eNUSplineType mType;
  tBool   mbDirty;
  tNodeVec  mvNodes;
  tF32    mfMaxDist;

  niEndClass(cNUSpline);
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __NUSPLINE_26378882_H__
