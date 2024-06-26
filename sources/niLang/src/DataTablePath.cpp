// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include <niLang/ILang.h>
#include "API/niLang/Utils/StringTokenizerImpl.h"
#include "API/niLang/ILang.h"
#include "DataTablePath.h"

using namespace ni;

//////////////////////////////////////////////////////////////////////////////////////////////
// cDataTablePathOp implementation.

///////////////////////////////////////////////
cDataTablePathOp::cDataTablePathOp(eDataTablePathOp aPathOp, const achar* aaszValue)
{
  ZeroMembers();
  mOp = aPathOp;
  mstrValue = aaszValue;
  mptrRegex = ni::CreateFilePatternRegex(aaszValue,NULL);
}

///////////////////////////////////////////////
cDataTablePathOp::~cDataTablePathOp()
{
}

///////////////////////////////////////////////
//! Zeros all the class members.
void cDataTablePathOp::ZeroMembers()
{
}

///////////////////////////////////////////////
//! Sanity check.
tBool __stdcall cDataTablePathOp::IsOK() const
{
  niClassIsOK(cDataTablePathOp);
  return niIsOK(mptrRegex);
}

///////////////////////////////////////////////
eDataTablePathOp __stdcall cDataTablePathOp::GetOp() const
{
  return mOp;
}

///////////////////////////////////////////////
iRegex* __stdcall cDataTablePathOp::GetRegex() const
{
  return mptrRegex;
}

///////////////////////////////////////////////
const cString& __stdcall cDataTablePathOp::GetValue() const
{
  return mstrValue;
}


//////////////////////////////////////////////////////////////////////////////////////////////
struct cDataTablePathParserTokenizer : public ImplRC<iStringTokenizer>
{
  cDataTablePathParserTokenizer() : mbFirstChar(eTrue) {}

  eStringTokenizerCharType __stdcall GetCharType(tU32 c) {
    eStringTokenizerCharType tokType = eStringTokenizerCharType_Normal;
    if (c == '/' || c == '\\') {
      tokType = mbFirstChar?eStringTokenizerCharType_SplitterAndToken:eStringTokenizerCharType_Splitter;
    }
    else if (c == '@' || c == ',') {
      tokType = eStringTokenizerCharType_SplitterAndToken;
    }
    mbFirstChar = eFalse;
    return tokType;
  }

  void __stdcall OnNewLine() {
  }

 private:
  tBool mbFirstChar;
};


tBool __stdcall ParseDataTablePathOp(const achar* aaszPath, tDataTablePathOpPtrCLst& aLst)
{
  astl::vector<cString> vToks;
  cString path(aaszPath);
  path = path.Trim();
  cDataTablePathParserTokenizer tokDT;
  StringTokenize(path,vToks,&tokDT);
  astl::vector<cString>::iterator it = vToks.begin();
  while (it != vToks.end())
  {
    // Root expression
    if (*it == _A("/") || *it == _A("\\")) {
      aLst.push_back(niNew cDataTablePathOp(eDataTablePathOp_Root,it->Chars()));
    }
    // Property
    else if (*it == _A("@")) {
      ++it;
      if (it == vToks.end())  {
        niError(niFmt(_A("Unexpected end of path in property declaration (%s)."),aaszPath));
        return eFalse;
      }
      aLst.push_back(niNew cDataTablePathOp(eDataTablePathOp_Property,it->Chars()));
      ++it;
      if (it != vToks.end())  {
        niError(niFmt(_A("End of path expected after property declaration (%s)."),aaszPath));
        return eFalse;
      }
      break;
    }
    // Previous
    else if (*it == _A("..")) {
      aLst.push_back(niNew cDataTablePathOp(eDataTablePathOp_Parent,it->Chars()));
    }
    // Current
    else if (*it == _A(".")) {
      aLst.push_back(niNew cDataTablePathOp(eDataTablePathOp_Current,it->Chars()));
    }
    // DataTable
    else {
      aLst.push_back(niNew cDataTablePathOp(eDataTablePathOp_DataTable,it->Chars()));
    }
    ++it;
  }

  return eTrue;
};

//////////////////////////////////////////////////////////////////////////////////////////////
// cDataTablePath implementation.

///////////////////////////////////////////////
cDataTablePath::cDataTablePath(iHString* ahspPath)
{
  ZeroMembers();
  SetPath(ahspPath);
}

///////////////////////////////////////////////
cDataTablePath::~cDataTablePath()
{
  ClearResult();
}

///////////////////////////////////////////////
//! Zeros all the class members.
void cDataTablePath::ZeroMembers()
{
  mResultType = eDataTablePathResultType_None;
  mptrResultDataTable = NULL;
  mnResultIndex = eInvalidHandle;
}

///////////////////////////////////////////////
//! Sanity check.
tBool __stdcall cDataTablePath::IsOK() const
{
  niClassIsOK(cDataTablePath);
  return eTrue;
}

///////////////////////////////////////////////
void __stdcall cDataTablePath::SetPath(iHString* ahspPath)
{
  ClearResult();
  mlstOps.clear();
  mhspPath = _H("");
  if (ni::HStringIsNotEmpty(ahspPath)) {
    const achar* aszPath = niHStr(ahspPath);
    ParseDataTablePathOp(aszPath,mlstOps);
    mhspPath = ahspPath;
  }
}

///////////////////////////////////////////////
iHString* __stdcall cDataTablePath::GetPath() const {
  return mhspPath;
}

///////////////////////////////////////////////
tBool __stdcall cDataTablePath::Evaluate(iDataTable* apDT)
{
  ClearResult();
  niCheckIsOK(apDT,eFalse);
  niCheck(!mlstOps.empty(),eFalse);

  iDataTable* pCur = apDT;
  for (tDataTablePathOpPtrCLstIt it = mlstOps.begin(); it != mlstOps.end(); )
  {
    cDataTablePathOp* pOp = *it;

    switch (pOp->GetOp())
    {
      case eDataTablePathOp_Root:
        {
          pCur = pCur->GetRoot();
          if (!niIsOK(pCur)) {
            niError(_A("Can't get the DataTable root."));
            return eFalse;
          }
          break;
        }
      case eDataTablePathOp_Current:
        {
          break;
        }
      case eDataTablePathOp_Parent:
        {
          pCur = pCur->GetParent();
          if (!niIsOK(pCur))
          {
            niError(_A("Can't get DataTable parent."));
            return eFalse;
          }
          break;
        }
      case eDataTablePathOp_DataTable:
        {
          tDataTablePathOpPtrCLstIt itBase = it;
          tU32 nS;
          for (nS = 0; nS < pCur->GetNumChildren(); ++nS)
          {
            pOp = (*it);
            iDataTable* pS = pCur->GetChildFromIndex(nS);
            if (pOp->GetRegex()->DoesMatch(pS->GetName())) {
              break;
            }
          }
          if (nS == pCur->GetNumChildren())
          {
#pragma niTodo("Better error report mech, for now it bloats the output, so its disabled.")
            /*          it = itBase;
                        pOp = *it;
                        cString strParams;
                        niError(niFmt(_A("Can't find child script '%s%s'."), (*itBase)->GetRegex()->GetRegexString(), strParams.Chars()));*/
            return eFalse;
          }
          pCur = pCur->GetChildFromIndex(nS);
          ++it;
          continue;
        }
      case eDataTablePathOp_Property:
        {
          tU32 nP;
          for (nP = 0; nP < pCur->GetNumProperties(); ++nP)
          {
            if (pOp->GetRegex()->DoesMatch(pCur->GetPropertyName(nP)))
              break;
          }
          if (nP == pCur->GetNumProperties()) {
#pragma niTodo("Better error report mech, for now it bloats the output, so its disabled.")
            //niError(niFmt(_A("Can't find script property '%s'."), pOp->GetRegex()->GetRegexString()));
            return eFalse;
          }
          mResultType = eDataTablePathResultType_Property;
          mptrResultDataTable = pCur;
          mnResultIndex = nP;
          return eTrue;
        }
      default:
        niError(niFmt(_A("Unknown operation '%d'."), pOp->GetOp()));
        return eFalse;
    }

    ++it;
  }

  mResultType = eDataTablePathResultType_DataTable;
  mptrResultDataTable = pCur;
  mnResultIndex = eInvalidHandle;
  return eTrue;
}

///////////////////////////////////////////////
tBool __stdcall cDataTablePath::EvaluateEx(iHString* ahspPath, iDataTable* apDT)
{
  SetPath(ahspPath);
  return Evaluate(apDT);
}

///////////////////////////////////////////////
eDataTablePathResultType __stdcall cDataTablePath::GetResultType() const
{
  return mResultType;
}

///////////////////////////////////////////////
iDataTable* __stdcall cDataTablePath::GetResultDataTable() const
{
  return mptrResultDataTable;
}

///////////////////////////////////////////////
tU32 __stdcall cDataTablePath::GetResultPropertyIndex() const
{
  return mResultType==eDataTablePathResultType_Property?mnResultIndex:eInvalidHandle;
}

///////////////////////////////////////////////
void __stdcall cDataTablePath::ClearResult()
{
  if (mResultType == eDataTablePathResultType_None) return;
  mptrResultDataTable = NULL;
  mnResultIndex = eInvalidHandle;
  mResultType = eDataTablePathResultType_None;
}

///////////////////////////////////////////////
cString __stdcall cDataTablePath::GetRootPathToDataTable(iDataTable* apDT, tU32 anPropIndex) const
{
  niCheck(niIsOK(apDT),AZEROSTR);

  cString ret;
  if (anPropIndex != eInvalidHandle) {
    ret += _A("@");
    ret += apDT->GetPropertyName(anPropIndex);
    niCheck(ret != _A("@"),AZEROSTR);
  }

  if  (apDT->GetParent()) {
    while (apDT->GetParent()) {
      cString cur;
      cur = _A("/");
      cur += apDT->GetName();
      cur += ret;
      ret = cur;
      apDT = apDT->GetParent();
    }
  }
  else {
    ret = cString(_A("/")) + ret;
  }

  return ret;
}

///////////////////////////////////////////////
tBool __stdcall cDataTablePath::CreatePathInDataTable(iDataTable* apDT, tBool abCompletlyNew)
{
  niCheckIsOK(apDT,eFalse);
  niCheck(!mlstOps.empty(),eFalse);

  ClearResult();
  iDataTable* pCur = apDT;
  for (tDataTablePathOpPtrCLstIt it = mlstOps.begin(); it != mlstOps.end(); )
  {
    cDataTablePathOp* pOp = *it;

    switch (pOp->GetOp())
    {
      case eDataTablePathOp_Current:
        break;  // empty
      case eDataTablePathOp_Root:
        {
          pCur = pCur->GetRoot();
          if (!niIsOK(pCur)) {
            niError(_A("Can't get the DataTable root."));
            return eFalse;
          }
          break;
        }
      case eDataTablePathOp_Parent:
        {
          pCur = pCur->GetParent();
          if (!niIsOK(pCur)) {
            niError(_A("Can't get DataTable parent."));
            return eFalse;
          }
          break;
        }
      case eDataTablePathOp_DataTable:
        {
          tBool bCreateNew = eTrue;
          cString strName = ((cDataTablePathOp*)pOp)->GetValue();
          if (!abCompletlyNew) {
            tU32 nChildIndex = pCur->GetChildIndex(strName.Chars());
            if (nChildIndex != eInvalidHandle) {
              pCur = pCur->GetChildFromIndex(nChildIndex);
              bCreateNew = eFalse;
            }
          }
          if (bCreateNew) {
            Ptr<iDataTable> dtNew = ni::GetLang()->CreateDataTable(strName.Chars());
            pCur->AddChild(dtNew);
            pCur = dtNew;
          }
          ++it;
          continue;
        }
      case eDataTablePathOp_Property:
        {
          // set the property to an empty string if it doesnt already exists
          cString strName = ((cDataTablePathOp*)pOp)->GetValue();
          tU32 nPropIndex = pCur->GetPropertyIndex(strName.Chars());
          if (nPropIndex == eInvalidHandle) {
            nPropIndex = pCur->SetString(strName.Chars(),_A(""));
          }
          mResultType = eDataTablePathResultType_Property;
          mptrResultDataTable = pCur;
          mnResultIndex = nPropIndex;
          return eTrue;
        }
      default:
        niError(niFmt(_A("Unknown operation '%d'."), pOp->GetOp()));
        return eFalse;
    }

    ++it;
  }

  mResultType = eDataTablePathResultType_DataTable;
  mptrResultDataTable = pCur;
  mnResultIndex = eInvalidHandle;
  return eTrue;
}

//////////////////////////////////////////////////////////////////////////////////////////////
namespace ni {
niExportFunc(iUnknown*) New_niLang_DataTablePath(const Var& avarA, const Var& avarB)
{
  const achar* path = NULL;
  if (avarA.IsString()) {
    path = avarA.GetString().Chars();
  }
  if (!path && avarA.IsACharConstPointer()) {
    path = avarA.GetACharConstPointer();
  }
  if (!path) {
    niError(_A("Parameter A is not a valid string."));
  }
  return niNew cDataTablePath(_H(path));
}
}
