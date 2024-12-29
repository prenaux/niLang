// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "NUSpline.h"
#include "API/niUI_ModuleDef.h"

//////////////////////////////////////////////////////////////////////////////////////////////
// cNUSpline implementation.

///////////////////////////////////////////////
cNUSpline::cNUSpline(eNUSplineType aType)
{
  niAssert(aType < eNUSplineType_Last);
  ZeroMembers();
  mType = aType;
}

///////////////////////////////////////////////
cNUSpline::~cNUSpline()
{
}

///////////////////////////////////////////////
//! Zeros all the class members.
void cNUSpline::ZeroMembers()
{
  mfMaxDist = 0.0f;
  mbDirty = eTrue;
}

///////////////////////////////////////////////
//! Sanity check.
ni::tBool __stdcall cNUSpline::IsOK() const
{
  niClassIsOK(cNUSpline);
  return ni::eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cNUSpline::SetType(eNUSplineType aType)
{
  if (aType >= eNUSplineType_Last) return eFalse;
  mType = aType;
  mbDirty = eTrue;
  return eTrue;
}

///////////////////////////////////////////////
eNUSplineType __stdcall cNUSpline::GetType() const
{
  return mType;
}

///////////////////////////////////////////////
void cNUSpline::Clear()
{
  mvNodes.clear();
  mbDirty = eTrue;
}

///////////////////////////////////////////////
void cNUSpline::AddNode(const sVec4f &avNode)
{
  for (tNodeVecIt itN = mvNodes.begin(); itN != mvNodes.end(); ++itN)
    if (itN->mvPos == ((sVec3f&)avNode))
      return;

  if (mType == eNUSplineType_Timed && !mvNodes.empty())
    mvNodes.back().mfDist = avNode.w;

  sNode node;
  node.mvPos = ((sVec3f&)avNode);
  node.mfTime = avNode.w;
  mvNodes.push_back(node);
  mbDirty = eTrue;
}

///////////////////////////////////////////////
tBool cNUSpline::RemoveNode(tU32 anIndex)
{
  if (anIndex >= mvNodes.size()) return eFalse;
  mvNodes.erase(mvNodes.begin()+anIndex);
  mbDirty = eTrue;
  return eTrue;
}

///////////////////////////////////////////////
tU32 cNUSpline::GetNumNodes() const
{
  return (tU32)mvNodes.size();
}

///////////////////////////////////////////////
tBool __stdcall cNUSpline::SetNode(tU32 anIndex, const sVec4f& avNode)
{
  if (anIndex >= mvNodes.size()) return eFalse;
  if (mType == eNUSplineType_Timed && !mvNodes.empty())
    mvNodes[anIndex-1].mfDist = avNode.w;
  mvNodes[anIndex].mvPos = ((sVec3f&)avNode);
  mvNodes[anIndex].mfTime = avNode.w;
  mbDirty = eTrue;
  return eTrue;
}

///////////////////////////////////////////////
sVec4f cNUSpline::GetNode(tU32 anIndex) const
{
  if (anIndex >= mvNodes.size())
    return sVec4f::Zero();
  sVec4f ret;
  ret.x = mvNodes[anIndex].mvPos.x;
  ret.y = mvNodes[anIndex].mvPos.y;
  ret.z = mvNodes[anIndex].mvPos.z;
  ret.w = mvNodes[anIndex].mfTime;
  return ret;
}

///////////////////////////////////////////////
tF32 __stdcall cNUSpline::GetNodeDistance(tU32 anIndex) const
{
  if (anIndex >= mvNodes.size()) return ni::kMaxF32;
  return mvNodes[anIndex].mfDist;
}

///////////////////////////////////////////////
tF32 __stdcall cNUSpline::GetLength() const
{
  if (mbDirty) {
    if (!niThis(cNUSpline)->UpdateSpline())
      return 1.0f;
  }
  return mfMaxDist;
}

///////////////////////////////////////////////
sVec3f cNUSpline::GetPosition(tF32 afTime)
{
  if (mvNodes.empty())
    return (sVec3f&)(sVec3f::Zero());

  if (mbDirty)
  {
    if (!UpdateSpline())
      return (sVec3f&)(sVec3f::Zero());
  }

  tF32 dist = afTime * mfMaxDist;
  tF32 curDist = 0.0f;
  tU32 i = 0;
  while ((curDist+mvNodes[i].mfDist < dist) && (i < mvNodes.size()-1))
  {
    curDist += mvNodes[i].mfDist;
    ++i;
  }

  float t = dist-curDist;
  t *= ni::FInvert(mvNodes[i].mfDist); // scale t in range 0 - 1

  sVec3f r;
  if (mvNodes.size() < 3 || mType == eNUSplineType_Linear)
  {
    return VecLerp(r, mvNodes[i].mvPos, mvNodes[i+1].mvPos, t);
  }
  else
  {
    sVec3f startVel = mvNodes[i].mvVel * mvNodes[i].mfDist;
    sVec3f endVel = mvNodes[i+1].mvVel * mvNodes[i].mfDist;
    return VecHermite(r, mvNodes[i].mvPos, startVel, mvNodes[i+1].mvPos, endVel, t);
  }
}

///////////////////////////////////////////////
sVec3f __stdcall cNUSpline::GetRelativePosition(tF32 afTime)
{
  if (mvNodes.empty())
    return (sVec3f&)(sVec3f::Zero());

  return GetPosition(afTime)-mvNodes[0].mvPos;
}

///////////////////////////////////////////////
tBool cNUSpline::UpdateSpline()
{
  if (mvNodes.size() < 2) return eFalse;

  tU32 i;

  mfMaxDist = 0.0f;
  if (mType == eNUSplineType_Timed)
  {
    for (i = 0; i < mvNodes.size(); ++i)
      mfMaxDist += mvNodes[i].mfDist;
  }
  else
  {
    mvNodes[0].mfDist = 0.0f;
    for (i = 0; i < mvNodes.size()-1; ++i)
    {
      mvNodes[i].mfDist = VecLength(mvNodes[i].mvPos-mvNodes[i+1].mvPos);
      mfMaxDist += mvNodes[i].mfDist;
    }
  }

  for (i = 1; i < mvNodes.size()-1; ++i)
  {
    sVec3f np; VecNormalize(np,mvNodes[i+1].mvPos-mvNodes[i].mvPos);
    sVec3f pp; VecNormalize(pp,mvNodes[i-1].mvPos-mvNodes[i].mvPos);
    VecNormalize(mvNodes[i].mvVel, np-pp);
  }

  mvNodes[0].mvVel = GetStartVel(0);
  mvNodes.back().mvVel = GetEndVel(mvNodes.size()-1);
  mbDirty = eFalse;

  if (mType != eNUSplineType_Rounded)
    Smooth(3);

  return eTrue;
}

///////////////////////////////////////////////
sVec3f cNUSpline::GetStartVel(tU32 anIndex) const
{
  niAssert(anIndex < mvNodes.size()-1);
  sVec3f temp = (3.f*(mvNodes[anIndex+1].mvPos-mvNodes[anIndex].mvPos))*ni::FInvert(mvNodes[anIndex].mfDist);
  return (temp - mvNodes[anIndex+1].mvVel)*0.5f;
}

///////////////////////////////////////////////
sVec3f cNUSpline::GetEndVel(tU32 anIndex) const
{
  niAssert(anIndex > 0);
  sVec3f temp = (3.f*(mvNodes[anIndex].mvPos-mvNodes[anIndex-1].mvPos))*ni::FInvert(mvNodes[anIndex-1].mfDist);
  return (temp - mvNodes[anIndex-1].mvVel)*0.5f;
}

///////////////////////////////////////////////
void cNUSpline::Smooth(tU32 anCount)
{
  niAssert(mvNodes.size() >= 2);
  for (tU32 count = 0; count < anCount; ++count)
  {
    sVec3f newVel;
    sVec3f oldVel = GetStartVel(0);
    for (tU32 i = 1; i < mvNodes.size()-1; ++i)
    {
      newVel = GetEndVel(i)*mvNodes[i].mfDist + GetStartVel(i)*mvNodes[i-1].mfDist;
      newVel *= ni::FInvert(mvNodes[i-1].mfDist + mvNodes[i].mfDist);
      mvNodes[i-1].mvVel = oldVel;
      oldVel = newVel;
    }
    mvNodes[mvNodes.size()-1].mvVel = GetEndVel(mvNodes.size()-1);
    mvNodes[mvNodes.size()-2].mvVel = oldVel;
    if (mType == eNUSplineType_Timed)
      Constrain();
  }
}

///////////////////////////////////////////////
void cNUSpline::Constrain()
{
  niAssert(mvNodes.size() >= 3);
  for (tU32 i = 1; i < mvNodes.size()-1; ++i)
  {
    float r0 = ni::FDiv(VecLength(mvNodes[i].mvPos-mvNodes[i-1].mvPos),mvNodes[i-1].mfDist);
    float r1 = ni::FDiv(VecLength(mvNodes[i+1].mvPos-mvNodes[i].mvPos),mvNodes[i].mfDist);
    mvNodes[i].mvVel *= ni::FDiv(4.f*r0*r1,((r0+r1)*(r0+r1)));
  }
}

///////////////////////////////////////////////
const achar* __stdcall cNUSpline::GetSerializeObjectTypeID() const
{
  return "niUI.NUSpline";
}

///////////////////////////////////////////////
tSize __stdcall cNUSpline::Serialize(iFile* apFile, eSerializeMode aMode)
{
  switch (aMode)
  {
    case eSerializeMode_Read:
    case eSerializeMode_ReadRaw:
      {
        tI64 nPos = apFile->Tell();
        tVersion ver = apFile->ReadLE32();
        if (ver != niMakeVersion(1,0,0))
        {
          niError(niFmt(_A("Invalid NUSpline version '%s'."),niFmtVersion(ver)));
          return eInvalidHandle;
        }
        mType = (eNUSplineType)apFile->Read8();
        mvNodes.resize(apFile->ReadLE32());
        for (tU32 i = 0; i < mvNodes.size(); ++i)
        {
          apFile->ReadF32Array(mvNodes[i].mvPos.ptr(),3);
          apFile->ReadF32Array(mvNodes[i].mvVel.ptr(),3);
          mvNodes[i].mfDist = apFile->ReadF32();
        }
        mbDirty = eTrue;
        return (tSize)(apFile->Tell()-nPos);
      }
    case eSerializeMode_Write:
    case eSerializeMode_WriteRaw:
      {
        tU32 nSize = 0;
        nSize += apFile->WriteLE32(niMakeVersion(1,0,0));
        nSize += apFile->Write8(mType);
        nSize += apFile->WriteLE32(mvNodes.size());
        for (tU32 i = 0; i < mvNodes.size(); ++i)
        {
          nSize += apFile->WriteF32Array(mvNodes[i].mvPos.ptr(),3);
          nSize += apFile->WriteF32Array(mvNodes[i].mvVel.ptr(),3);
          nSize += apFile->WriteF32(mvNodes[i].mfDist);
        }
        return nSize;
      }
  }
  return eInvalidHandle;
}

///////////////////////////////////////////////
tBool __stdcall cNUSpline::SerializeDataTable(iDataTable* apDT, tSerializeFlags aFlags)
{
  niCheck(niIsOK(apDT),eFalse);

  // Write
  if (niFlagIs(aFlags,eSerializeFlags_Write)) {
    Ptr<iDataTableWriteStack> dt = ni::GetLang()->CreateDataTableWriteStack(apDT);
    dt->SetEnum(_A("type"),niEnumExpr(eNUSplineType),this->GetType());
    dt->SetInt(_A("num_nodes"),this->GetNumNodes());
    dt->Push(_A("Nodes"));
    for (tU32 i = 0; i < this->GetNumNodes(); ++i) {
      dt->SetVec4(niFmt(_A("node%d"),i),this->GetNode(i));
    }
    dt->Pop();
    if (niFlagIs(aFlags,eSerializeFlags_TypeInfoMetadata)) {
      dt->SetMetadata(_A("type"),_H("enum[*eNUSplineType]"));
    }
  }
  // Read
  else {
    Ptr<iDataTableReadStack> dt = ni::CreateDataTableReadStack(apDT);
    this->SetType((eNUSplineType)dt->GetEnum(_A("type"),niEnumExpr(eNUSplineType)));
    tU32 numNodes = dt->GetIntDefault(_A("num_nodes"),this->GetNumNodes());
    tU32 i = 0;
    if (dt->PushFail(_A("Nodes"))) {
      Clear();
      for ( ; i < numNodes; ++i) {
        tU32 nPropIndex = dt->GetTop()->GetPropertyIndex(niFmt(_A("node%d"),i));
        if (nPropIndex == eInvalidHandle)
          break;
        AddNode(dt->GetTop()->GetVec4FromIndex(nPropIndex));
      }
      dt->Pop();
    }
    // generate the new nodes automatically
    sVec4f vCurrent = this->GetNumNodes()?this->GetNode(this->GetNumNodes()-1):Vec4<tF32>(0,0,0,1);
    for ( ; i < numNodes; ++i) {
      AddNode(vCurrent);
      vCurrent += Vec4(25.0f,0.0f,0.0f,1.0f);
    }
  }

  return eTrue;
}

//////////////////////////////////////////////////////////////////////////////////////////////
niExportFunc(iUnknown*) New_niUI_NUSpline(const Var& avarA, const Var& avarB)
{
  eNUSplineType type = (eNUSplineType)avarA.mU8;
  if (type >= eNUSplineType_Last) {
    niError(niFmt(_A("Invalid non-uniform spline type '%d'."),type));
    return NULL;
  }
  return static_cast<ni::iNUSpline*>(niNew cNUSpline(type));
}
