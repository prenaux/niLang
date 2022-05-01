// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "Intersection.h"

niExportFunc(iUnknown*) New_niUI_Intersection(const Var&, const Var&) {
  return niNew cIntersection();
}

//////////////////////////////////////////////////////////////////////////////////////////////
// cIntersection implementation.

///////////////////////////////////////////////
cIntersection::cIntersection()
{
  ZeroMembers();
}

///////////////////////////////////////////////
cIntersection::~cIntersection()
{
}

///////////////////////////////////////////////
//! Zeros all the class members.
void cIntersection::ZeroMembers()
{
  mResult = eIntersectionResult_None;
  mvPosition = sVec3f::Zero();
  mvBaryCentric = sVec2f::Zero();
  mnPolygonIndex = 0;
}

///////////////////////////////////////////////
//! Sanity check.
ni::tBool __stdcall cIntersection::IsOK() const
{
  niClassIsOK(cIntersection);
  return ni::eTrue;
}


///////////////////////////////////////////////
void __stdcall cIntersection::SetResult(eIntersectionResult aResult)
{
  mResult = aResult;
}

///////////////////////////////////////////////
eIntersectionResult __stdcall cIntersection::GetResult() const
{
  return mResult;
}

///////////////////////////////////////////////
void cIntersection::SetPosition(const sVec3f &avPos)
{
  mvPosition = avPos;
}

///////////////////////////////////////////////
sVec3f cIntersection::GetPosition() const
{
  return mvPosition;
}

///////////////////////////////////////////////
void cIntersection::SetBaryCentric(const sVec2f &avBC)
{
  mvBaryCentric = avBC;
}

///////////////////////////////////////////////
sVec2f cIntersection::GetBaryCentric() const
{
  return mvBaryCentric;
}

///////////////////////////////////////////////
void cIntersection::SetPolygonIndex(tU32 anIndex)
{
  mnPolygonIndex = anIndex;
}

///////////////////////////////////////////////
tU32 cIntersection::GetPolygonIndex() const
{
  return mnPolygonIndex;
}
