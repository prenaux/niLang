// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "Lang.h"
#include "DataTable.h"
#include "DataTablePath.h"
#include "API/niLang/Utils/STLVectorTypes.h"
#include "API/niLang/Utils/ParseType.h"
#include <niLang/STL/map.h>
#include <niLang/Utils/QPtr.h>
#include <niLang/Utils/IndexedMap.h>

using namespace ni;

// This breaks in subtle ways, its kept only for reference right now
// #define SINK_CALL_ONLY_IF_VALUE_CHANGED

/*
  Memory Usage difference as measured with the MemBlock counter

  ### Map ###

  --- Gaya - TestCases App opened (~130Levels)
  TSPlayerw_x86_da.exe  TSPlayerw_x86_da.exe  4920    TSPlayerw Application   High  Gaya - TestCases - New Level  369,524 K 185,580 K 0:01:15.005 32-bit  199,688 K

  - niLang.DataTable.STL.raw_allocator_dt_map(223488) : 7151616
  - niLang.DataTable.STL.raw_allocator_list(35540) : 428016
  - Total (636580) : 33595848

  --- 005_StaticCharRotate.gpk
  - niLang.DataTable.STL.raw_allocator_dt_map(2555) : 81760
  - niLang.DataTable.STL.raw_allocator_list(1549) : 20124
  - Total (48079) : 4114198

  ### HashMap ###

  --- Gaya - TestCases App opened (~130Levels)
  TSPlayerw_x86_da.exe  TSPlayerw_x86_da.exe  4268    TSPlayerw Application   High  Gaya - TestCases - New Level  399,352 K 210,716 K 0:01:26.705 32-bit  224,632 K

  - niLang.DataTable.STL.raw_allocator_dt_map(223488) : 30453616
  - niLang.DataTable.STL.raw_allocator_list(35540) : 428016
  - Total (636561) : 58097116

  --- 005_StaticCharRotate.gpk
  - niLang.DataTable.STL.raw_allocator_dt_hmap(2555) : 643676
  - niLang.DataTable.STL.raw_allocator_list(1549) : 20124
  - Total (48079) : 4691310
*/

//////////////////////////////////////////////////////////////////////////////////////////////
// cDataTable declaration.
class cDataTable : public ni::cIUnknownImpl<ni::iDataTable,ni::eIUnknownImplFlags_Default,ni::iSerializable>
{
  niBeginClass(cDataTable);

  typedef astl::vector<Ptr<iDataTable> >  tDataTableVec;
  typedef tDataTableVec::iterator         tDataTableVecIt;
  typedef tDataTableVec::const_iterator     tDataTableVecCIt;

  struct sProperty {
    Var             mVar;
    tHStringPtr         mhspMetadata;
  };

  typedef IndexedMap<cString,sProperty> tPropertyIDMap;

 public:
  //! Constructor.
  cDataTable();
  //! Destructor.
  ~cDataTable();

  //! Sanity check.
  ni::tBool __stdcall IsOK() const;

  void __stdcall Invalidate();

  //// ni::iDataTable //////////////////////////
  void __stdcall SetName(const achar *aaszName);
  const achar * __stdcall GetName() const;
  void __stdcall SetParent(ni::iDataTable* apParent);
  ni::iDataTable * __stdcall GetParent() const;
  iDataTable* __stdcall GetRoot() const;
  iDataTable* __stdcall GetPrevSibling() const;
  iDataTable* __stdcall GetNextSibling() const;
  tU32 __stdcall GetIndex() const;
  ni::iDataTable * __stdcall Clone() const;
  ni::iDataTable * __stdcall CloneEx(ni::tDataTableCopyFlags aMode) const;
  void __stdcall Clear();
  void __stdcall ClearChildren();
  tU32 __stdcall GetNumChildren() const;
  tU32 __stdcall GetChildIndex(const achar *aaszName) const;
  ni::iDataTable * __stdcall GetChild(const achar *aaszName) const;
  ni::iDataTable * __stdcall GetChildFromIndex(tU32 anIndex) const;
  tBool __stdcall Copy(const ni::iDataTable *apSource, ni::tDataTableCopyFlags aMode);
  tBool __stdcall AddChild(ni::iDataTable *apTable);
  tBool __stdcall RemoveChild(tU32 anIndex);
  void __stdcall ClearProperties();
  tBool __stdcall RemoveProperty(const achar* aaszName);
  tBool __stdcall RemovePropertyFromIndex(tU32 anIndex);
  tU32 __stdcall GetNumProperties() const;
  tU32 __stdcall GetPropertyIndex(const achar *aaszName) const;
  tBool __stdcall HasProperty(const achar* aaszName) const {
    return this->GetPropertyIndex(aaszName) != eInvalidHandle;
  }
  const achar * __stdcall GetPropertyName(tU32 anIndex) const;
  ni::eDataTablePropertyType __stdcall GetPropertyTypeFromIndex(tU32 anIndex) const;
  cString __stdcall GetStringFromIndex(tU32 anIndex) const;
  tHStringPtr __stdcall GetHStringFromIndex(tU32 anIndex) const {
    return _H(GetStringFromIndex(anIndex));
  }
  tI64 __stdcall GetIntFromIndex(tU32 anIndex) const;
  tBool __stdcall GetBoolFromIndex(tU32 anIndex) const;
  tF64 __stdcall GetFloatFromIndex(tU32 anIndex) const;
  sVec2f __stdcall GetVec2FromIndex(tU32 anIndex) const;
  sVec3f __stdcall GetVec3FromIndex(tU32 anIndex) const;
  sVec4f __stdcall GetVec4FromIndex(tU32 anIndex) const;
  sMatrixf  __stdcall GetMatrixFromIndex(tU32 anIndex) const;
  iUnknown * __stdcall GetIUnknownFromIndex(tU32 anIndex) const;

  Var __stdcall _GetVarFromIndex(tU32 anIndex, const tBool abRaw) const;
  Var __stdcall GetVarFromIndex(tU32 anIndex) const {
    return _GetVarFromIndex(anIndex, eFalse);
  }
  Var __stdcall GetRawVarFromIndex(tU32 anIndex) const {
    return _GetVarFromIndex(anIndex, eTrue);
  }

  ni::eDataTablePropertyType __stdcall GetPropertyType(const achar *aaszProp) const;

  cString __stdcall GetString(const achar *aaszProp) const;
  cString __stdcall GetStringDefault(const achar *aaszProp, const achar * aDefault) const {
    tU32 idx = this->GetPropertyIndex(aaszProp);
    if (idx == eInvalidHandle) return aDefault;
    return this->GetStringFromIndex(idx);
  }

  Ptr<iHString> __stdcall GetHString(const achar *aaszProp) const;
  Ptr<iHString> __stdcall GetHStringDefault(const achar *aaszProp, iHString* aDefault) const {
    tU32 idx = this->GetPropertyIndex(aaszProp);
    if (idx == eInvalidHandle) return aDefault;
    return this->GetHStringFromIndex(idx);
  }

  tI64 __stdcall GetInt(const achar *aaszProp) const;
  tI64 __stdcall GetIntDefault(const achar *aaszProp, tI64 aDefault) const {
    tU32 idx = this->GetPropertyIndex(aaszProp);
    if (idx == eInvalidHandle) return aDefault;
    return this->GetIntFromIndex(idx);
  }

  tBool __stdcall GetBool(const achar *aaszProp) const;
  tBool __stdcall GetBoolDefault(const achar *aaszProp, tBool aDefault) const {
    tU32 idx = this->GetPropertyIndex(aaszProp);
    if (idx == eInvalidHandle) return aDefault;
    return this->GetBoolFromIndex(idx);
  }

  tF64 __stdcall GetFloat(const achar *aaszProp) const;
  tF64 __stdcall GetFloatDefault(const achar *aaszProp, tF64 aDefault) const {
    tU32 idx = this->GetPropertyIndex(aaszProp);
    if (idx == eInvalidHandle) return aDefault;
    return this->GetFloatFromIndex(idx);
  }

  sVec2f __stdcall GetVec2(const achar *aaszProp) const;
  sVec2f __stdcall GetVec2Default(const achar *aaszProp, const sVec2f& aDefault) const {
    tU32 idx = this->GetPropertyIndex(aaszProp);
    if (idx == eInvalidHandle) return aDefault;
    return this->GetVec2FromIndex(idx);
  }

  sVec3f __stdcall GetVec3(const achar *aaszProp) const;
  sVec3f __stdcall GetVec3Default(const achar *aaszProp, const sVec3f& aDefault) const {
    tU32 idx = this->GetPropertyIndex(aaszProp);
    if (idx == eInvalidHandle) return aDefault;
    return this->GetVec3FromIndex(idx);
  }

  sVec4f __stdcall GetVec4(const achar *aaszProp) const;
  sVec4f __stdcall GetVec4Default(const achar *aaszProp, const sVec4f& aDefault) const {
    tU32 idx = this->GetPropertyIndex(aaszProp);
    if (idx == eInvalidHandle) return aDefault;
    return this->GetVec4FromIndex(idx);
  }

  sMatrixf __stdcall GetMatrix(const achar *aaszProp) const;
  sMatrixf __stdcall GetMatrixDefault(const achar *aaszProp, const sMatrixf& aDefault) const {
    tU32 idx = this->GetPropertyIndex(aaszProp);
    if (idx == eInvalidHandle) return aDefault;
    return this->GetMatrixFromIndex(idx);
  }

  iUnknown* __stdcall GetIUnknown(const achar *aaszProp) const;
  iUnknown* __stdcall GetIUnknownDefault(const achar *aaszProp, iUnknown* aDefault) const {
    tU32 idx = this->GetPropertyIndex(aaszProp);
    if (idx == eInvalidHandle) return aDefault;
    return this->GetIUnknownFromIndex(idx);
  }

  Var __stdcall _GetVar(const achar* aaszProp, const tBool abRaw) const {
    return _GetVarFromIndex(midmapProperties.GetKeyIndex(aaszProp),abRaw);
  }
  Var __stdcall GetVar(const achar* aaszProp) const { return _GetVar(aaszProp, eFalse); }
  Var __stdcall GetRawVar(const achar* aaszProp) const { return _GetVar(aaszProp, eTrue); }
  Var __stdcall GetVarDefault(const achar* aaszProp, const Var& aDefault) const {
    tU32 idx = this->GetPropertyIndex(aaszProp);
    if (idx == eInvalidHandle) return aDefault;
    return this->_GetVarFromIndex(idx, eFalse);
  }
  Var __stdcall GetRawVarDefault(const achar* aaszProp, const Var& aDefault) const {
    tU32 idx = this->GetPropertyIndex(aaszProp);
    if (idx == eInvalidHandle) return aDefault;
    return this->_GetVarFromIndex(idx, eTrue);
  }

  void __stdcall SetStringFromIndex(tU32 anIndex, const achar *aaszValue);
  void __stdcall SetHStringFromIndex(tU32 anIndex, iHString* ahspValue) {
    return this->SetStringFromIndex(anIndex,HStringGetStringEmpty(ahspValue));
  }
  void __stdcall SetIntFromIndex(tU32 anIndex, tI64 anValue);
  void __stdcall SetBoolFromIndex(tU32 anIndex, tBool anValue);
  void __stdcall SetFloatFromIndex(tU32 anIndex, tF64 afValue);
  void __stdcall SetVec2FromIndex(tU32 anIndex, const sVec2f &avValue);
  void __stdcall SetVec3FromIndex(tU32 anIndex, const sVec3f &avValue);
  void __stdcall SetVec4FromIndex(tU32 anIndex, const sVec4f &avValue);
  void __stdcall SetMatrixFromIndex(tU32 anIndex, const sMatrixf &amtxValue);
  void __stdcall SetIUnknownFromIndex(tU32 anIndex, iUnknown *apObject);
  __forceinline void __stdcall _SetVarFromIndex(tU32 anIndex, const Var& aVar, tBool abRaw);
  void __stdcall SetVarFromIndex(tU32 anIndex, const Var& aVar) { _SetVarFromIndex(anIndex,aVar,eFalse); }
  void __stdcall SetRawVarFromIndex(tU32 anIndex, const Var& aVar) { _SetVarFromIndex(anIndex,aVar,eTrue); }

  tU32 __stdcall SetString(const achar *aaszName, const achar *aaszValue);
  tU32 __stdcall SetHString(const achar *aaszName, iHString* ahspValue) {
    return this->SetString(aaszName,HStringGetStringEmpty(ahspValue));
  }
  tU32 __stdcall SetInt(const achar *aaszName, tI64 anValue);
  tU32 __stdcall SetBool(const achar *aaszName, tBool anValue);
  tU32 __stdcall SetFloat(const achar *aaszName, tF64 afValue);
  tU32 __stdcall SetVec2(const achar *aaszName, const sVec2f &avValue);
  tU32 __stdcall SetVec3(const achar *aaszName, const sVec3f &avValue);
  tU32 __stdcall SetVec4(const achar *aaszName, const sVec4f &avValue);
  tU32 __stdcall SetMatrix(const achar *aaszName, const sMatrixf &amtxValue);
  tU32 __stdcall SetIUnknown(const achar *aaszName, iUnknown *apObject);
  tU32 __stdcall SetVar(const achar* aaszName, const Var& aVar) {
    tU32 nIndex = _GetNewProperty(aaszName);
    _SetVarFromIndex(nIndex,aVar,eFalse);
    return nIndex;
  }
  tU32 __stdcall SetRawVar(const achar* aaszName, const Var& aVar) {
    tU32 nIndex = _GetNewProperty(aaszName);
    _SetVarFromIndex(nIndex,aVar,eTrue);
    return nIndex;
  }

  sVec3f __stdcall GetCol3(const achar *aaszName) const {
    return GetVec3Default(aaszName,sVec3f::One());
  }
  sVec3f __stdcall GetCol3FromIndex(tU32 anIndex) const {
    if (anIndex >= GetNumProperties()) return sVec3f::One();
    return GetVec3FromIndex(anIndex);
  }

  sVec4f __stdcall GetCol4(const achar *aaszName) const {
    return GetVec4Default(aaszName,sVec4f::One());
  }
  sVec4f __stdcall GetCol4FromIndex(tU32 anIndex) const {
    if (anIndex >= GetNumProperties()) return sVec4f::One();
    return GetVec4FromIndex(anIndex);
  }

  tBool __stdcall GetHasSink() const;
  tDataTableSinkLst* __stdcall GetSinkList() const;
  tBool __stdcall SetMetadataFromIndex(tU32 anIndex, iHString* ahspData);
  Ptr<iHString> __stdcall GetMetadataFromIndex(tU32 anIndex) const;
  tBool __stdcall SetMetadata(const achar* aaszProp, iHString* ahspData);
  Ptr<iHString> __stdcall GetMetadata(const achar* aaszProp) const;

  tU32 __stdcall GetEnumFromIndex(tU32 anIndex, const sEnumDef* apEnumDef, tEnumToStringFlags aFlags) const;
  tU32 __stdcall GetEnum(const achar* aaszProp, const sEnumDef* apEnumDef, tEnumToStringFlags aFlags) const;
  tU32 __stdcall GetEnumDefault(const achar* aaszProp, const sEnumDef* apEnumDef, tEnumToStringFlags aFlags, tU32 anDefault) const {
    tU32 idx = GetPropertyIndex(aaszProp);
    if (idx == eInvalidHandle) return anDefault;
    return GetEnumFromIndex(idx,apEnumDef,aFlags);
  }

  tU32 __stdcall SetEnum(const achar* aaszName, const sEnumDef* apEnumDef, tEnumToStringFlags aFlags, tU32 anValue) niImpl;
  void __stdcall SetEnumFromIndex(tU32 anIndex, const sEnumDef* apEnumDef, tEnumToStringFlags aFlags, tU32 anValue) niImpl;

  tBool __stdcall NewVarFromPath(const achar* aPath, const Var& aVal);
  tBool __stdcall SetVarFromPath(const achar* aPath, const Var& aVal);
  Var __stdcall GetVarFromPath(const achar* aPath, const Var& aDefault) const;
  //// ni::iDataTable //////////////////////////

  //// iSerializable ////////////////////////////
  const achar* __stdcall GetSerializeObjectTypeID() const;
  tSize __stdcall Serialize(iFile* apFile, eSerializeMode aMode);
  //// iSerializable ////////////////////////////

  __forceinline tU32 _GetNewProperty(const achar* aaszName) const {
    tU32 nIndex = midmapProperties.GetKeyIndex(aaszName);
    if (nIndex == eInvalidHandle) {
      sProperty p;
      nIndex = midmapProperties.Set(aaszName,p);
    }
    return nIndex;
  }

  __forceinline Ptr<SinkList<iDataTableSink> > _GetRootSinkList() const {
    Ptr<cDataTable> pThis = this;
    Ptr<SinkList<iDataTableSink> > pLst = NULL;
    do {
      pLst = pThis->mlstSinks;
      if (pLst.IsOK())
        break;
      pThis = static_cast<const cDataTable*>(pThis->GetParent());
    } while (pThis.IsOK());
    return pLst;
  }

  __forceinline void __stdcall CallSink_SetName(iDataTable* apDT) {
    Ptr<SinkList<iDataTableSink> > pLst = _GetRootSinkList();
    niCallSinkVoid_(DataTableSink,pLst,SetName,(apDT));
  }

  __forceinline void __stdcall CallSink_AddChild(iDataTable* apDT, iDataTable* apChild) {
    Ptr<SinkList<iDataTableSink> > pLst = _GetRootSinkList();
    niCallSinkVoid_(DataTableSink,pLst,AddChild,(apDT,apChild));
  }

  __forceinline void __stdcall CallSink_RemoveChild(iDataTable* apDT, iDataTable* apChild) {
    Ptr<SinkList<iDataTableSink> > pLst = _GetRootSinkList();
    niCallSinkVoid_(DataTableSink,pLst,RemoveChild,(apDT,apChild));
  }

  __forceinline void __stdcall CallSink_SetProperty(iDataTable* apDT, tU32 anProperty) {
    Ptr<SinkList<iDataTableSink> > pLst = _GetRootSinkList();
    niCallSinkVoid_(DataTableSink,pLst,SetProperty,(apDT,anProperty));
  }

  __forceinline void __stdcall CallSink_SetMetadata(iDataTable* apDT, tU32 anProperty) {
    Ptr<SinkList<iDataTableSink> > pLst = _GetRootSinkList();
    niCallSinkVoid_(DataTableSink,pLst,SetMetadata,(apDT,anProperty));
  }

  __forceinline void __stdcall CallSink_RemoveProperty(iDataTable* apDT, tU32 anProperty) {
    Ptr<SinkList<iDataTableSink> > pLst = _GetRootSinkList();
    niCallSinkVoid_(DataTableSink,pLst,RemoveProperty,(apDT,anProperty));
  }

  __forceinline iExpressionVariable* __stdcall _EvaluateExpression(const achar* aaszExpr) const {
    return ni::GetLang()->GetExpressionContext()->Eval(aaszExpr);
  }

  tBool __stdcall _FindChildDataTable(iDataTable* apThis, iDataTable* apToFind) const;

  inline void _FetchPropertyVar(Var& aVar, tU32 anIndex) const {
    Ptr<SinkList<iDataTableSink> > pLst = _GetRootSinkList();
    niLoopImmutableIterator(SinkList<iDataTableSink>, it, pLst) {
      aVar = it->_Value()->OnDataTableSink_GetProperty(const_cast<cDataTable*>(this),anIndex);
      if (!aVar.IsNull())
        return;
    }
    aVar = midmapProperties.GetItem(anIndex).mVar;
  }
  inline void _FetchRawPropertyVar(Var& aVar, tU32 anIndex) const {
    aVar = midmapProperties.GetItem(anIndex).mVar;
  }

 private:
  WeakPtr<iDataTable> mpwParent;
  tDataTableVec mvChildren;
  cString     mstrName;
  mutable tPropertyIDMap      midmapProperties;
  mutable Ptr<tDataTableSinkLst>  mlstSinks;

  niEndClass(cDataTable);
};

//////////////////////////////////////////////////////////////////////////////////////////////
// cDataTable implementation.

///////////////////////////////////////////////
cDataTable::cDataTable()
{
}

///////////////////////////////////////////////
cDataTable::~cDataTable()
{
}

///////////////////////////////////////////////
//! Sanity check.
ni::tBool __stdcall cDataTable::IsOK() const
{
  niClassIsOK(cDataTable);
  return ni::eTrue;
}

///////////////////////////////////////////////
void __stdcall cDataTable::Invalidate()
{
  Clear();
}

///////////////////////////////////////////////
void cDataTable::SetName(const achar *aaszName)
{
  mstrName = aaszName;
  CallSink_SetName(this);
}

///////////////////////////////////////////////
const achar * cDataTable::GetName() const
{
  return mstrName.Chars();
}

///////////////////////////////////////////////
void cDataTable::SetParent(ni::iDataTable* apParent)
{
  if (_FindChildDataTable(this,apParent)) {
    niError(_A("Can't set a child data table as parent."));
    niAssertUnreachable("Can't set a child data table as parent.");
    return;
  }
  mpwParent.Swap(apParent);
}

///////////////////////////////////////////////
ni::iDataTable * cDataTable::GetParent() const
{
  QPtr<iDataTable> ptrParent = mpwParent;
  return ptrParent;
}

///////////////////////////////////////////////
iDataTable* __stdcall cDataTable::GetRoot() const
{
  iDataTable* pCur = (iDataTable*)this;
  while (pCur->GetParent()) {
    pCur = pCur->GetParent();
  }
  return pCur;
}

///////////////////////////////////////////////
iDataTable* __stdcall cDataTable::GetPrevSibling() const
{
  QPtr<iDataTable> ptrParent = mpwParent;
  if (!ptrParent.IsOK())
    return NULL;

  tU32 i = GetIndex();
  if (i == eInvalidHandle || i <= 0)
    return NULL;

  return ptrParent->GetChildFromIndex(i-1);
}

///////////////////////////////////////////////
iDataTable* __stdcall cDataTable::GetNextSibling() const
{
  QPtr<iDataTable> ptrParent = mpwParent;
  if (!ptrParent.IsOK())
    return NULL;

  tU32 i = GetIndex();
  if (i == eInvalidHandle || i >= ptrParent->GetNumChildren()-1)
    return NULL;

  return ptrParent->GetChildFromIndex(i+1);
}

///////////////////////////////////////////////
tU32 __stdcall cDataTable::GetIndex() const
{
  QPtr<iDataTable> ptrParent = mpwParent;
  if (!ptrParent.IsOK())
    return eInvalidHandle;

  for (tU32 i = 0; i < ptrParent->GetNumChildren(); ++i) {
    if (ptrParent->GetChildFromIndex(i) == this)
      return i;
  }
  return eInvalidHandle;
}

///////////////////////////////////////////////
ni::iDataTable * cDataTable::Clone() const
{
  return CloneEx(eDataTableCopyFlags_Clone|eDataTableCopyFlags_Recursive|eDataTableCopyFlags_AppendAllChildren);
}

///////////////////////////////////////////////
ni::iDataTable * cDataTable::CloneEx(ni::tDataTableCopyFlags aMode) const
{
  Ptr<cDataTable> pNew = niNew cDataTable();
  pNew->Copy(this,aMode);
  return pNew.GetRawAndSetNull();
}

///////////////////////////////////////////////
void cDataTable::Clear()
{
  ClearProperties();
  ClearChildren();
}

///////////////////////////////////////////////
void cDataTable::ClearChildren()
{
  Ptr<SinkList<iDataTableSink> > pLst = _GetRootSinkList();
  for (tDataTableVecIt it = mvChildren.begin(); it != mvChildren.end(); ++it) {
    niCallSinkVoid_(DataTableSink,pLst,RemoveChild,(this,*it));
    if ((*it)->GetParent() == this)
      ((cDataTable*)(*it).ptr())->SetParent(NULL);
  }
  mvChildren.clear();
}

///////////////////////////////////////////////
tU32 cDataTable::GetNumChildren() const
{
  return (tU32)mvChildren.size();
}

///////////////////////////////////////////////
tU32 cDataTable::GetChildIndex(const achar *aaszName) const
{
  for (tDataTableVecCIt it = mvChildren.begin(); it != mvChildren.end(); ++it)
  {
    if (ni::StrCmp((*it)->GetName(),aaszName) == 0)
      return astl::const_iterator_index(mvChildren,it);
  }
  return eInvalidHandle;
}

///////////////////////////////////////////////
ni::iDataTable * cDataTable::GetChild(const achar *aaszName) const
{
  tU32 nIndex = GetChildIndex(aaszName);
  if (nIndex == eInvalidHandle) return NULL;
  return mvChildren[nIndex];
}

///////////////////////////////////////////////
ni::iDataTable * cDataTable::GetChildFromIndex(tU32 anIndex) const
{
  if (anIndex >= GetNumChildren()) return NULL;
  return mvChildren[anIndex];
}
///////////////////////////////////////////////
tBool cDataTable::Copy(const ni::iDataTable *apSource, ni::tDataTableCopyFlags aMode)
{
  if (!niIsOK(apSource))
    return ni::eFalse;

  SetName(apSource->GetName());

  /// Properties Copy ////
  if (niFlagIs(aMode,eDataTableCopyFlags_Skip)) {
    for (tU32 i = 0; i < apSource->GetNumProperties(); ++i)
    {
      if (GetPropertyIndex(apSource->GetPropertyName(i)) != eInvalidHandle)
        continue;
      tU32 nIndex = SetVar(apSource->GetPropertyName(i),apSource->GetVarFromIndex(i));
      SetMetadataFromIndex(nIndex,apSource->GetMetadataFromIndex(i));
    }
  }
  else {
    for (tU32 i = 0; i < apSource->GetNumProperties(); ++i)
    {
      tU32 nIndex = SetVar(apSource->GetPropertyName(i),apSource->GetVarFromIndex(i));
      SetMetadataFromIndex(nIndex,apSource->GetMetadataFromIndex(i));
    }
  }

  //// Children Tables Copy ////
  if (niFlagIs(aMode,eDataTableCopyFlags_Recursive))
  {
    if (niFlagIs(aMode,eDataTableCopyFlags_UniqueChild))
    {
      for (tU32 i = 0; i < apSource->GetNumChildren(); ++i) {
        iDataTable* pChildSrc = apSource->GetChildFromIndex(i);
        if (!pChildSrc) continue;
        iDataTable* pChildDest = GetChild(pChildSrc->GetName());
        if (!pChildDest) {
          AddChild(niFlagIs(aMode,eDataTableCopyFlags_Clone) ? pChildSrc->Clone() : pChildSrc);
        }
        else {
          pChildDest->Copy(pChildSrc,aMode);
        }
      }
    }
    else {
      tU32 i = 0;
      if (niFlagIsNot(aMode,eDataTableCopyFlags_AppendAllChildren)) {
        tU32 commonNum = ni::Min(GetNumChildren(),apSource->GetNumChildren());
        tU8Vec vMerged;
        vMerged.resize(apSource->GetNumChildren());
        astl::fill(vMerged,(ni::tU8)ni::eFalse);
        for (i = 0; i < commonNum; ++i)
        {
          if (ni::StrCmp(GetChildFromIndex(i)->GetName(),apSource->GetChildFromIndex(i)->GetName()) == 0)
          {
            vMerged[i] = eTrue;
            GetChildFromIndex(i)->Copy(apSource->GetChildFromIndex(i),aMode);
          }
        }
        for (i = 0; i < commonNum; ++i)
        {
          if (vMerged[i]) continue;
          iDataTable* cdt = apSource->GetChildFromIndex(i);
          if (cdt) {
            AddChild(niFlagIs(aMode,eDataTableCopyFlags_Clone) ? cdt->Clone() : cdt);
          }
        }
      }
      while (i < apSource->GetNumChildren()) {
        iDataTable* cdt = apSource->GetChildFromIndex(i++);
        if (cdt) {
          AddChild(niFlagIs(aMode,eDataTableCopyFlags_Clone) ? cdt->Clone() : cdt);
        }
      }
    }
  }

  return ni::kTrue;
}

///////////////////////////////////////////////
tBool cDataTable::AddChild(ni::iDataTable *apTable)
{
  niCheckIsOK(apTable,eFalse);
  Ptr<cDataTable> ptrDT = static_cast<cDataTable*>(apTable);
  mvChildren.push_back(ptrDT.ptr());
  ptrDT->SetParent(this);
  CallSink_AddChild(this,ptrDT);
  return eTrue;
}

///////////////////////////////////////////////
tBool cDataTable::RemoveChild(tU32 anIndex)
{
  if (anIndex >= GetNumChildren()) return eFalse;
  CallSink_RemoveChild(this,mvChildren[anIndex]);
  if (mvChildren[anIndex]->GetParent() == this) {
    ((cDataTable*)mvChildren[anIndex].ptr())->SetParent(NULL);
  }
  mvChildren.erase(mvChildren.begin()+anIndex);
  return eTrue;
}

///////////////////////////////////////////////
void cDataTable::ClearProperties()
{
  Ptr<SinkList<iDataTableSink> > pLst = _GetRootSinkList();
  if (pLst.IsOK()) {
    for (tU32 i = 0; i < GetNumProperties(); ++i) {
      niCallSinkVoid_(DataTableSink,pLst,RemoveProperty,(this,i));
    }
  }
  midmapProperties.Clear();
}

///////////////////////////////////////////////
tBool __stdcall cDataTable::RemoveProperty(const achar* aaszName)
{
  return RemovePropertyFromIndex(GetPropertyIndex(aaszName));
}

///////////////////////////////////////////////
tBool __stdcall cDataTable::RemovePropertyFromIndex(tU32 anIndex)
{
  const tU32 numProps = GetNumProperties();
  if (anIndex >= numProps)
    return eFalse;
  if (numProps == 1) {
    // work around a deficency in the idmap...
    ClearProperties();
    return eTrue;
  }
  else {
    CallSink_RemoveProperty(this,anIndex);
    return midmapProperties.EraseIndex(anIndex);
  }
}

///////////////////////////////////////////////
tU32 cDataTable::GetNumProperties() const
{
  return midmapProperties.GetSize();
}

///////////////////////////////////////////////
tU32 cDataTable::GetPropertyIndex(const achar *aaszName) const
{
  return midmapProperties.GetKeyIndex(aaszName);
}

///////////////////////////////////////////////
const achar * cDataTable::GetPropertyName(tU32 anIndex) const
{
  if (!midmapProperties.HasIndex(anIndex)) { return AZEROSTR; }
  return midmapProperties.GetKey(anIndex).Chars();
}

///////////////////////////////////////////////
ni::eDataTablePropertyType cDataTable::GetPropertyTypeFromIndex(tU32 anIndex) const
{
  if (anIndex >= GetNumProperties()) return eDataTablePropertyType_Unknown;
  Var v; _FetchPropertyVar(v,anIndex);
  if (VarIsString(v))
    return eDataTablePropertyType_String;
  return (eDataTablePropertyType)v.GetType();
}

///////////////////////////////////////////////
cString cDataTable::GetStringFromIndex(tU32 anIndex) const
{
  if (anIndex >= GetNumProperties()) return AZEROSTR;

  Var v; _FetchPropertyVar(v,anIndex);

  const achar* pVarChars = VarGetCharsOrNull(v);
  if (pVarChars) {
    return pVarChars;
  }
  else {
    // not the same type, convert
    cString ret;
    switch (v.GetType()) {
      case eDataTablePropertyType_Int32:
        ret.Format(_A("%d"),v.GetI32());
        break;
      case eDataTablePropertyType_Int64:
        ret.Format(_A("%d"),v.GetI64());
        break;
      case eDataTablePropertyType_Float32:
        ret.Format(_A("%g"),v.GetF32());
        break;
      case eDataTablePropertyType_Float64:
        ret.Format(_A("%g"),v.GetF64());
        break;
      case eDataTablePropertyType_Vec2:
        ret.Set(v.GetVec2f());
        break;
      case eDataTablePropertyType_Vec3:
        ret.Set(v.GetVec3f());
        break;
      case eDataTablePropertyType_Vec4:
        ret.Set(v.GetVec4f());
        break;
      case eDataTablePropertyType_Matrix:
        ret.Set(v.GetMatrixf());
        break;
      default: {
        if (v.IsIntValue()) {
          ret.Format(_A("%d"),v.GetIntValue());
        }
        else if (v.IsFloatValue()) {
          ret.Format(_A("%g"),v.GetFloatValue());
        }
        else if (v.IsVec2i()) {
          ret.Set(v.GetVec2i());
        }
        else if (v.IsVec3i()) {
          ret.Set(v.GetVec3i());
        }
        else if (v.IsVec4i()) {
          ret.Set(v.GetVec4i());
        }
        else if (v.IsVec2f()) {
          ret.Set(v.GetVec2f());
        }
        else if (v.IsVec3f()) {
          ret.Set(v.GetVec3f());
        }
        else if (v.IsVec4f()) {
          ret.Set(v.GetVec4f());
        }
        else if (v.IsMatrixf()) {
          ret.Set(v.GetMatrixf());
        }
        else {
          return AZEROSTR;
        }
      }
    }
    return ret;
  }
}

///////////////////////////////////////////////
tI64 cDataTable::GetIntFromIndex(tU32 anIndex) const
{
  if (anIndex >= GetNumProperties()) return 0;

  Var v; _FetchPropertyVar(v,anIndex);
  if (niType(v.GetType()) == eType_I32 || v.GetType() == eDataTablePropertyType_Int32) {
    return v.GetI32();
  }
  else if (niType(v.GetType()) == eType_I64 || v.GetType() == eDataTablePropertyType_Int64) {
    return v.GetI64();
  }
  else {
    // not the same type, convert
    switch (v.GetType()) {
      case eDataTablePropertyType_Float32:
        return tI64(v.GetF32());
      case eDataTablePropertyType_Float64:
        return tI64(v.GetF64());
      case eDataTablePropertyType_String:
        {
          Ptr<iExpressionVariable> ptrV = _EvaluateExpression(v.GetString().Chars());
          if (ptrV.IsOK()) {
            if (ptrV->GetType() == eExpressionVariableType_Float) {
              return (tI64)ptrV->GetFloat();
            }
            else if (ptrV->GetType() == eExpressionVariableType_Vec3) {
              sVec3f col = ptrV->GetVec3();
              return (tI64)ULColorBuildf(col.x,col.y,col.z,1.0f);
            }
            else if (ptrV->GetType() == eExpressionVariableType_Vec4) {
              sVec4f col = ptrV->GetVec4();
              return (tI64)ULColorBuild(col);
            }
          }
          // jump to default
        }
      default:
        {
          if (VarConvertType(v,eDataTablePropertyType_Int64))
            return v.GetI64();
          else
            return 0;
        }
    }
  }
}

///////////////////////////////////////////////
tBool cDataTable::GetBoolFromIndex(tU32 anIndex) const
{
  return GetIntFromIndex(anIndex)?eTrue:eFalse;
}

///////////////////////////////////////////////
tF64 cDataTable::GetFloatFromIndex(tU32 anIndex) const
{
  if (anIndex >= GetNumProperties()) return 0.0f;

  Var v; _FetchPropertyVar(v,anIndex);
  if (niType(v.GetType()) == eType_F32 || v.GetType() == eDataTablePropertyType_Float32) {
    return v.GetF32();
  }
  else if (niType(v.GetType()) == eType_F64 || v.GetType() == eDataTablePropertyType_Float64) {
    return v.GetF64();
  }
  else {
    switch (v.GetType()) {
      case eDataTablePropertyType_Int32:
        return tF64(v.GetI32());
      case eDataTablePropertyType_Int64:
        return tF64(v.GetI64());
      case eDataTablePropertyType_String:
        {
          Ptr<iExpressionVariable> ptrV = _EvaluateExpression(v.GetString().Chars());
          if (ptrV.IsOK()) {
            return (tF32)ptrV->GetFloat();
          }
          // jump to default
        }
      default:
        {
          if (VarConvertType(v,eDataTablePropertyType_Float64))
            return v.GetF64();
          else
            return 0.0f;
        }
    }
  }
}

///////////////////////////////////////////////
sVec2f cDataTable::GetVec2FromIndex(tU32 anIndex) const
{
  if (anIndex >= GetNumProperties())
    return (sVec2f::Zero());

  Var v; _FetchPropertyVar(v,anIndex);
  if (v.GetType() == eDataTablePropertyType_Vec2) {
    // right type return it directly
    return v.GetVec2f();
  }
  else if (niType(v.GetType()) == eType_Vec2f) {
    return v.GetVec2f();
  }
  else {
    switch (v.GetType()) {
      case eDataTablePropertyType_String:
        {
          Ptr<iExpressionVariable> ptrV = _EvaluateExpression(v.GetString().Chars());
          if (ptrV.IsOK()) {
            return ptrV->GetVec2();
          }
          // jump to default
        }
      default: {
        if (VarConvertType(v,eDataTablePropertyType_Vec2))
          return v.GetVec2f();
        else
          return sVec2f::Zero();
      }
    }
  }
}

///////////////////////////////////////////////
sVec3f cDataTable::GetVec3FromIndex(tU32 anIndex) const
{
  if (anIndex >= GetNumProperties())
    return (sVec3f::Zero());

  Var v; _FetchPropertyVar(v,anIndex);
  if (v.GetType() == eDataTablePropertyType_Vec3) {
    // right type return it directly
    return v.GetVec3f();
  }
  else if (niType(v.GetType()) == eType_Vec3f) {
    return v.GetVec3f();
  }
  else {
    switch (v.GetType()) {
      case eDataTablePropertyType_String:
        {
          Ptr<iExpressionVariable> ptrV = _EvaluateExpression(v.GetString().Chars());
          if (ptrV.IsOK()) {
            return ptrV->GetVec3();
          }
          // jump to default
        }
      default: {
        if (VarConvertType(v,eDataTablePropertyType_Vec3))
          return v.GetVec3f();
        else
          return sVec3f::Zero();
      }
    }
  }
}

///////////////////////////////////////////////
sVec4f cDataTable::GetVec4FromIndex(tU32 anIndex) const
{
  if (anIndex >= GetNumProperties())
    return (sVec4f::Zero());

  Var v; _FetchPropertyVar(v,anIndex);
  if (v.GetType() == eDataTablePropertyType_Vec4) {
    // right type return it directly
    return v.GetVec4f();
  }
  else if (niType(v.GetType()) == eType_Vec4f) {
    return v.GetVec4f();
  }
  else {
    switch (v.GetType()) {
      case eDataTablePropertyType_String:
        {
          Ptr<iExpressionVariable> ptrV = _EvaluateExpression(v.GetString().Chars());
          if (ptrV.IsOK()) {
            return ptrV->GetVec4();
          }
          // jump to default
        }
      default: {
        if (VarConvertType(v,eDataTablePropertyType_Vec4))
          return v.GetVec4f();
        else
          return sVec4f::Zero();
      }
    }
  }
}

///////////////////////////////////////////////
sMatrixf cDataTable::GetMatrixFromIndex(tU32 anIndex) const
{
  if (anIndex >= GetNumProperties()) return (sMatrixf::Identity());

  Var v; _FetchPropertyVar(v,anIndex);
  if (v.GetType() == eDataTablePropertyType_Matrix) {
    // right type return it directly
    return v.GetMatrixf();
  }
  else if (niType(v.GetType()) == eType_Matrixf) {
    return v.GetMatrixf();
  }
  else {
    switch (v.GetType()) {
      case eDataTablePropertyType_String:
        {
          Ptr<iExpressionVariable> ptrV = _EvaluateExpression(v.GetString().Chars());
          if (ptrV.IsOK()) {
            return ptrV->GetMatrix();
          }
          // jump to default
        }
      default: {
        Var cv;
        if (VarConvertType(v,eDataTablePropertyType_Matrix))
          return cv.GetMatrixf();
        else
          return sMatrixf::Identity();
      }
    }
  }
}

///////////////////////////////////////////////
iUnknown * cDataTable::GetIUnknownFromIndex(tU32 anIndex) const
{
  if (anIndex >= GetNumProperties()) return NULL;
  Var v; _FetchPropertyVar(v,anIndex);
  if (v.GetType() != eDataTablePropertyType_IUnknown) return NULL;
  return v.GetIUnknownPointer();
}

///////////////////////////////////////////////
Var __stdcall cDataTable::_GetVarFromIndex(tU32 anIndex, const tBool abRaw) const
{
  if (anIndex >= GetNumProperties()) return (Var&)niVarNull;
  Var v;
  if (abRaw) {
    _FetchRawPropertyVar(v,anIndex);
  }
  else {
    _FetchPropertyVar(v,anIndex);
  }
  return v;
}

///////////////////////////////////////////////
ni::eDataTablePropertyType cDataTable::GetPropertyType(const achar *aaszProp) const
{
  return GetPropertyTypeFromIndex(midmapProperties.GetKeyIndex(aaszProp));
}

///////////////////////////////////////////////
cString cDataTable::GetString(const achar *aaszProp) const
{
  return GetStringFromIndex(midmapProperties.GetKeyIndex(aaszProp));
}

///////////////////////////////////////////////
tHStringPtr cDataTable::GetHString(const achar *aaszProp) const
{
  return _H(GetString(aaszProp));
}

///////////////////////////////////////////////
tI64 cDataTable::GetInt(const achar *aaszProp) const
{
  return GetIntFromIndex(midmapProperties.GetKeyIndex(aaszProp));
}

///////////////////////////////////////////////
tBool cDataTable::GetBool(const achar *aaszProp) const
{
  return GetBoolFromIndex(midmapProperties.GetKeyIndex(aaszProp));
}

///////////////////////////////////////////////
tF64 cDataTable::GetFloat(const achar *aaszProp) const
{
  return GetFloatFromIndex(midmapProperties.GetKeyIndex(aaszProp));
}

///////////////////////////////////////////////
sVec2f cDataTable::GetVec2(const achar *aaszProp) const
{
  return GetVec2FromIndex(midmapProperties.GetKeyIndex(aaszProp));
}

///////////////////////////////////////////////
sVec3f cDataTable::GetVec3(const achar *aaszProp) const
{
  return GetVec3FromIndex(midmapProperties.GetKeyIndex(aaszProp));
}

///////////////////////////////////////////////
sVec4f cDataTable::GetVec4(const achar *aaszProp) const
{
  return GetVec4FromIndex(midmapProperties.GetKeyIndex(aaszProp));
}

///////////////////////////////////////////////
sMatrixf cDataTable::GetMatrix(const achar *aaszProp) const
{
  return GetMatrixFromIndex(midmapProperties.GetKeyIndex(aaszProp));
}

///////////////////////////////////////////////
iUnknown * cDataTable::GetIUnknown(const achar *aaszProp) const
{
  return GetIUnknownFromIndex(midmapProperties.GetKeyIndex(aaszProp));
}

///////////////////////////////////////////////
void cDataTable::SetStringFromIndex(tU32 anIndex, const achar *aaszValue)
{
  if (anIndex >= GetNumProperties()) return;

  Ptr<SinkList<iDataTableSink> > pLst = _GetRootSinkList();

  sProperty& p = midmapProperties.GetItem(anIndex);
#ifdef SINK_CALL_ONLY_IF_VALUE_CHANGED
  if (pLst.IsOK() && p.mVar.GetType() == eDataTablePropertyType_String) {
    if (p.mVar.GetString() == aaszValue)
      return;
  }
#endif

  p.mVar.SetString(aaszValue);
  CallSink_SetProperty(this,anIndex);
}

///////////////////////////////////////////////
void cDataTable::SetIntFromIndex(tU32 anIndex, tI64 anValue)
{
  if (anIndex >= GetNumProperties()) return;
  Ptr<SinkList<iDataTableSink> > pLst = _GetRootSinkList();

  sProperty& p = midmapProperties.GetItem(anIndex);
#ifdef SINK_CALL_ONLY_IF_VALUE_CHANGED
  if (pLst.IsOK()) {
    if (p.mVar.GetType() == eDataTablePropertyType_Int32) {
      if (p.mVar.GetI32() == anValue)
        return;
    }
    else if (p.mVar.GetType() == eDataTablePropertyType_Int64) {
      if (p.mVar.GetI64() == anValue)
        return;
    }
  }
#endif

  if (anValue >= (tI64)niMinI32 && anValue <= (tI64)niMaxI32) {
    p.mVar.SetI32(anValue);
  }
  else {
    p.mVar.SetI64(anValue);
  }
  CallSink_SetProperty(this,anIndex);
}

///////////////////////////////////////////////
void cDataTable::SetBoolFromIndex(tU32 anIndex, tBool anValue)
{
  SetIntFromIndex(anIndex,anValue);
}

///////////////////////////////////////////////
void cDataTable::SetFloatFromIndex(tU32 anIndex, tF64 afValue)
{
  if (anIndex >= GetNumProperties()) return;
  Ptr<SinkList<iDataTableSink> > pLst = _GetRootSinkList();

  sProperty& p = midmapProperties.GetItem(anIndex);
#ifdef SINK_CALL_ONLY_IF_VALUE_CHANGED
  if (pLst.IsOK()) {
    if (p.mVar.GetType() == eDataTablePropertyType_Float32) {
      if (p.mVar.GetF32() == afValue)
        return;
    }
    else if (p.mVar.GetType() == eDataTablePropertyType_Float64) {
      if (p.mVar.GetF64() == afValue)
        return;
    }
  }
#endif

  p.mVar.SetF64(afValue);
  CallSink_SetProperty(this,anIndex);
}

///////////////////////////////////////////////
void cDataTable::SetVec2FromIndex(tU32 anIndex, const sVec2f &avValue)
{
  if (anIndex >= GetNumProperties()) return;
  Ptr<SinkList<iDataTableSink> > pLst = _GetRootSinkList();

  sProperty& p = midmapProperties.GetItem(anIndex);
#ifdef SINK_CALL_ONLY_IF_VALUE_CHANGED
  if (pLst.IsOK() && p.mVar.GetType() == eDataTablePropertyType_Vec2) {
    if (p.mVar.GetVec2f() == avValue)
      return;
  }
#endif

  p.mVar.SetVec2f(avValue);
  CallSink_SetProperty(this,anIndex);
}

///////////////////////////////////////////////
void cDataTable::SetVec3FromIndex(tU32 anIndex, const sVec3f &avValue)
{
  if (anIndex >= GetNumProperties()) return;
  Ptr<SinkList<iDataTableSink> > pLst = _GetRootSinkList();

  sProperty& p = midmapProperties.GetItem(anIndex);
#ifdef SINK_CALL_ONLY_IF_VALUE_CHANGED
  if (pLst.IsOK() && p.mVar.GetType() == eDataTablePropertyType_Vec3) {
    if (p.mVar.GetVec3f() == avValue)
      return;
  }
#endif

  p.mVar.SetVec3f(avValue);
  CallSink_SetProperty(this,anIndex);
}

///////////////////////////////////////////////
void cDataTable::SetVec4FromIndex(tU32 anIndex, const sVec4f &avValue)
{
  if (anIndex >= GetNumProperties()) return;
  Ptr<SinkList<iDataTableSink> > pLst = _GetRootSinkList();

  sProperty& p = midmapProperties.GetItem(anIndex);
#ifdef SINK_CALL_ONLY_IF_VALUE_CHANGED
  if (pLst.IsOK() && p.mVar.GetType() == eDataTablePropertyType_Vec4) {
    if (p.mVar.GetVec4f() == avValue)
      return;
  }
#endif

  p.mVar.SetVec4f(avValue);
  CallSink_SetProperty(this,anIndex);
}

///////////////////////////////////////////////
void cDataTable::SetMatrixFromIndex(tU32 anIndex, const sMatrixf &amtxValue)
{
  if (anIndex >= GetNumProperties()) return;
  Ptr<SinkList<iDataTableSink> > pLst = _GetRootSinkList();

  sProperty& p = midmapProperties.GetItem(anIndex);
#ifdef SINK_CALL_ONLY_IF_VALUE_CHANGED
  if (pLst.IsOK() && p.mVar.GetType() == eDataTablePropertyType_Matrix) {
    if (static_cast<sMatrixf&>(p.mVar.GetMatrixf()) == amtxValue)
      return;
  }
#endif

  p.mVar.SetMatrixf(amtxValue);
  CallSink_SetProperty(this,anIndex);
}

///////////////////////////////////////////////
void cDataTable::SetIUnknownFromIndex(tU32 anIndex, iUnknown *apObject)
{
  if (anIndex >= GetNumProperties()) return;
  Ptr<SinkList<iDataTableSink> > pLst = _GetRootSinkList();

  sProperty& p = midmapProperties.GetItem(anIndex);
#ifdef SINK_CALL_ONLY_IF_VALUE_CHANGED
  if (pLst.IsOK() && p.mVar.GetType() == eDataTablePropertyType_IUnknown) {
    if (p.mVar.GetIUnknownPointer() == apObject)
      return;
  }
#endif

  p.mVar.SetIUnknownPointer(apObject);
  CallSink_SetProperty(this,anIndex);
}

///////////////////////////////////////////////
void __stdcall cDataTable::_SetVarFromIndex(tU32 anIndex, const Var& aVar, const tBool abRaw)
{
  if (anIndex >= GetNumProperties())
    return;

  sProperty& p = midmapProperties.GetItem(anIndex);
  if (abRaw) {
    p.mVar = aVar;
  }
  else {
    Ptr<SinkList<iDataTableSink> > pLst = _GetRootSinkList();
#ifdef SINK_CALL_ONLY_IF_VALUE_CHANGED
    if (pLst.IsOK() && (p.mVar == aVar)) {
      return;
    }
#endif
    p.mVar = aVar;
    CallSink_SetProperty(this,anIndex);
  }
}

///////////////////////////////////////////////
tU32 __stdcall cDataTable::SetString(const achar *aaszName, const achar *aaszValue)
{
  tU32 nIndex = _GetNewProperty(aaszName);
  SetStringFromIndex(nIndex,aaszValue);
  return nIndex;
}

///////////////////////////////////////////////
tU32 cDataTable::SetInt(const achar *aaszName, tI64 anValue)
{
  tU32 nIndex = _GetNewProperty(aaszName);
  SetIntFromIndex(nIndex,anValue);
  return nIndex;
}

///////////////////////////////////////////////
tU32 cDataTable::SetBool(const achar *aaszName, tBool anValue)
{
  tU32 nIndex = _GetNewProperty(aaszName);
  SetBoolFromIndex(nIndex,anValue);
  return nIndex;
}

///////////////////////////////////////////////
tU32 cDataTable::SetFloat(const achar *aaszName, tF64 afValue)
{
  tU32 nIndex = _GetNewProperty(aaszName);
  SetFloatFromIndex(nIndex,afValue);
  return nIndex;
}

///////////////////////////////////////////////
tU32 cDataTable::SetVec2(const achar *aaszName, const sVec2f &avValue)
{
  tU32 nIndex = _GetNewProperty(aaszName);
  SetVec2FromIndex(nIndex,avValue);
  return nIndex;
}

///////////////////////////////////////////////
tU32 cDataTable::SetVec3(const achar *aaszName, const sVec3f &avValue)
{
  tU32 nIndex = _GetNewProperty(aaszName);
  SetVec3FromIndex(nIndex,avValue);
  return nIndex;
}

///////////////////////////////////////////////
tU32 cDataTable::SetVec4(const achar *aaszName, const sVec4f &avValue)
{
  tU32 nIndex = _GetNewProperty(aaszName);
  SetVec4FromIndex(nIndex,avValue);
  return nIndex;
}

///////////////////////////////////////////////
tU32 cDataTable::SetMatrix(const achar *aaszName, const sMatrixf &amtxValue)
{
  tU32 nIndex = _GetNewProperty(aaszName);
  SetMatrixFromIndex(nIndex,amtxValue);
  return nIndex;
}

///////////////////////////////////////////////
tU32 cDataTable::SetIUnknown(const achar *aaszName, iUnknown *apObject)
{
  tU32 nIndex = _GetNewProperty(aaszName);
  SetIUnknownFromIndex(nIndex,apObject);
  return nIndex;
}

///////////////////////////////////////////////
tBool __stdcall cDataTable::GetHasSink() const
{
  return mlstSinks.IsOK() && !mlstSinks->IsEmpty();
}

///////////////////////////////////////////////
tDataTableSinkLst* __stdcall cDataTable::GetSinkList() const
{
  if (!mlstSinks.IsOK()) {
    mlstSinks = SinkList<iDataTableSink>::Create();
  }
  return mlstSinks;
}

///////////////////////////////////////////////
tBool __stdcall cDataTable::SetMetadataFromIndex(tU32 anIndex, iHString* ahspData)
{
  if (anIndex >= GetNumProperties()) return eFalse;
  sProperty& p = midmapProperties.GetItem(anIndex);
  p.mhspMetadata = ahspData;
  CallSink_SetMetadata(this,anIndex);
  return eTrue;
}

///////////////////////////////////////////////
Ptr<iHString> __stdcall cDataTable::GetMetadataFromIndex(tU32 anIndex) const
{
  if (anIndex >= GetNumProperties()) return NULL;
  const sProperty& p = midmapProperties.GetItem(anIndex);
  return p.mhspMetadata;
}

///////////////////////////////////////////////
tBool __stdcall cDataTable::SetMetadata(const achar* aaszProp, iHString* ahspData)
{
  tU32 nIndex = _GetNewProperty(aaszProp);
  sProperty& p = midmapProperties.GetItem(nIndex);
  p.mhspMetadata = ahspData;
  CallSink_SetMetadata(this,nIndex);
  return eTrue;
}

///////////////////////////////////////////////
Ptr<iHString> __stdcall cDataTable::GetMetadata(const achar* aaszProp) const
{
  return GetMetadataFromIndex(midmapProperties.GetKeyIndex(aaszProp));
}

///////////////////////////////////////////////
tU32 __stdcall cDataTable::GetEnumFromIndex(tU32 anIndex, const sEnumDef* apEnumDef, tEnumToStringFlags aFlags) const
{
  if (anIndex >= GetNumProperties())
    return 0;

  Var v; _FetchPropertyVar(v,anIndex);
  const achar* aszExpr = VarGetCharsOrNull(v);
  if (aszExpr) {
    // always eval string as expression
    return ni::GetLang()->StringToEnum(aszExpr,apEnumDef,aFlags);
  }
  else if (v.IsEnumValue()) {
    return v.GetEnumValue();
  }
  else {
    return 0;
  }
}

///////////////////////////////////////////////
tU32 __stdcall cDataTable::GetEnum(const achar* aaszProp, const sEnumDef* apEnumDef, tEnumToStringFlags aFlags) const
{
  return GetEnumFromIndex(midmapProperties.GetKeyIndex(aaszProp),apEnumDef,aFlags);
}

///////////////////////////////////////////////
void __stdcall cDataTable::SetEnumFromIndex(tU32 anIndex, const sEnumDef* apEnumDef, tEnumToStringFlags aFlags, tU32 anValue)
{
  if (anIndex >= GetNumProperties())
    return;

  if (apEnumDef) {
    SetStringFromIndex(anIndex, ni::GetLang()->EnumToString(anValue, apEnumDef, aFlags).Chars());
  }
  else {
    SetIntFromIndex(anIndex, anValue);
  }
}

///////////////////////////////////////////////
tU32 __stdcall cDataTable::SetEnum(const achar* aaszProp, const sEnumDef* apEnumDef, tEnumToStringFlags aFlags, tU32 anValue)
{
  tU32 nIndex = _GetNewProperty(aaszProp);
  SetEnumFromIndex(nIndex,apEnumDef,aFlags,anValue);
  return nIndex;
}

///////////////////////////////////////////////
tBool __stdcall cDataTable::_FindChildDataTable(iDataTable* apThis, iDataTable* apToFind) const
{
  tU32 i;
  if (apThis == apToFind)
    return ni::eTrue;
  for (i = 0; i < apThis->GetNumChildren(); ++i) {
    if (apThis->GetChildFromIndex(i) == apToFind)
      return ni::eTrue;
  }
  for (i = 0; i < apThis->GetNumChildren(); ++i) {
    if (_FindChildDataTable(apThis->GetChildFromIndex(i),apToFind))
      return ni::eTrue;
  }
  return ni::eFalse;
}

//--------------------------------------------------------------------------------------------
//
//  Path properties
//
//--------------------------------------------------------------------------------------------
cString __stdcall cLang::GetAbsoluteDataTablePath(iDataTable* apDT, tU32 anPropIndex)
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

static tBool _GetPathProperty(iDataTable* apDT, const achar* aPath, Ptr<iDataTable>& resDT, tU32& resIndex, tBool abCreate) {
  tHStringPtr hspPath = _H(aPath);
  Ptr<cDataTablePath> path = niNew cDataTablePath(hspPath);
  if (abCreate) {
    if (!path->CreatePathInDataTable(apDT,eFalse))
      return eFalse;
  }
  else {
    if (!path->Evaluate(apDT))
      return eFalse;
  }
  if (path->GetResultType() != eDataTablePathResultType_Property)
    return eFalse;
  resDT = path->GetResultDataTable();
  resIndex = path->GetResultPropertyIndex();
  return eTrue;
}

tBool __stdcall cDataTable::NewVarFromPath(const achar* aPath, const Var& aVal)  {
  Ptr<iDataTable> dt; tU32 prop;
  if (!_GetPathProperty(this,aPath,dt,prop,eTrue))
    return eFalse;
  dt->SetVarFromIndex(prop,aVal);
  return eTrue;
}
tBool __stdcall cDataTable::SetVarFromPath(const achar* aPath, const Var& aVal) {
  Ptr<iDataTable> dt; tU32 prop;
  if (!_GetPathProperty(this,aPath,dt,prop,eFalse))
    return eFalse;
  dt->SetVarFromIndex(prop,aVal);
  return eTrue;
}
Var __stdcall cDataTable::GetVarFromPath(const achar* aPath, const Var& aDefault) const {
  Ptr<iDataTable> dt; tU32 prop;
  if (!_GetPathProperty((iDataTable*)this,aPath,dt,prop,eFalse))
    return aDefault;
  return dt->GetVarFromIndex(prop);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Serialize implementation

// implemented in DataTable_Serialize.cpp
tSize DataTableSerialize_Write(iFile* apFile, iDataTable* apTable, const tU32 anVersion);
tSize DataTableSerialize_Read(iFile* apFile, iDataTable* apTable);

///////////////////////////////////////////////
const achar* __stdcall cDataTable::GetSerializeObjectTypeID() const {
  return "niLang.DataTable";
}

///////////////////////////////////////////////
tSize __stdcall cDataTable::Serialize(iFile* apFile, eSerializeMode aMode)
{
  tSize size = 0;
  switch (aMode)
  {
    case eSerializeMode_Write:
    case eSerializeMode_WriteRaw:
      {
        size = DataTableSerialize_Write(apFile,this,_knDataTableObjectSerializeVersion);
        if (size == eInvalidHandle) {
          niError(_A("Can't write this table."));
          return eInvalidHandle;
        }
        break;
      }
    case eSerializeMode_Read:
    case eSerializeMode_ReadRaw:
      {
        if (DataTableSerialize_Read(apFile,this) == eInvalidHandle) {
          niError(_A("Can't read this table."));
          return eInvalidHandle;
        }
        break;
      }
  }
  return size;
}

///////////////////////////////////////////////
iDataTable* __stdcall cLang::CreateDataTable(const achar* aaszName) {
  return ni::CreateDataTable(aaszName);
}

//////////////////////////////////////////////////////////////////////////////////////////////
namespace ni {

///////////////////////////////////////////////
niExportFunc(iDataTable*) CreateDataTable(const achar* aaszName) {
  Ptr<iDataTable> pDT = niNew cDataTable();
  if (niIsOK(pDT) && niStringIsOK(aaszName)) {
    pDT->SetName(aaszName);
  }
  return pDT.GetRawAndSetNull();
}

///////////////////////////////////////////////
niExportFunc(iUnknown*) New_niLang_DataTable(const Var& avarA, const Var& avarB) {
  ni::Ptr<iDataTable> ptrDT = niNew cDataTable();
  if (ptrDT.IsOK()) {
    cString name = VarGetString(avarA);
    if (!name.empty())
      ptrDT->SetName(name.Chars());
  }
  return ptrDT.GetRawAndSetNull();
}
}
