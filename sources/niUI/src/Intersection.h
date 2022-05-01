#ifndef __INTERSECTION_31197658_H__
#define __INTERSECTION_31197658_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

//////////////////////////////////////////////////////////////////////////////////////////////
// cIntersection declaration.
class cIntersection : public ni::cIUnknownImpl<ni::iIntersection,ni::eIUnknownImplFlags_Default>
{
  niBeginClass(cIntersection);

 public:
  //! Constructor.
  cIntersection();
  //! Destructor.
  ~cIntersection();

  //! Zeros all the class members.
  void ZeroMembers();
  //! Sanity check.
  ni::tBool __stdcall IsOK() const;

  //// ni::iIntersection ///////////////////////
  void __stdcall SetResult(eIntersectionResult aResult);
  eIntersectionResult __stdcall GetResult() const;
  void __stdcall SetPosition(const sVec3f &avPos);
  sVec3f __stdcall GetPosition() const;
  void __stdcall SetBaryCentric(const sVec2f &avBC);
  sVec2f __stdcall GetBaryCentric() const;
  void __stdcall SetPolygonIndex(tU32 anIndex);
  tU32 __stdcall GetPolygonIndex() const;
  //// ni::iIntersection ///////////////////////

 private:
  eIntersectionResult mResult;
  sVec3f mvPosition;
  sVec2f mvBaryCentric;
  tU32    mnPolygonIndex;
  niEndClass(cIntersection);
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __INTERSECTION_31197658_H__
