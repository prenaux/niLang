#ifndef __SCRIPTTYPES_37182345_H__
#define __SCRIPTTYPES_37182345_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "sqobject.h"
#include "sqvm.h"
#include <niLang/Utils/Hash.h>
#include <niLang/Utils/UUID.h>

typedef astl::list<Ptr<iScriptObject> >        tScriptObjectPtrLst;
typedef tScriptObjectPtrLst::iterator     tScriptObjectPtrLstIt;
typedef tScriptObjectPtrLst::const_iterator tScriptObjectPtrLstCIt;

//////////////////////////////////////////////////////////////////////////////////////////////
// sInterfaceDef type

struct sScriptTypeInterfaceDef : SQ_USERDATA_BASE(sScriptTypeInterfaceDef)
{
  virtual ni::iUnknown* __stdcall QueryInterface(const ni::tUUID& aIID) {
    return this->_DoQueryInterface(this,aIID);
  }

  const sInterfaceDef*  pInterfaceDef;

  sScriptTypeInterfaceDef(
    const SQSharedState& aSS,
    const sInterfaceDef* apInterfaceDef)
      : pInterfaceDef(apInterfaceDef)
  {
    SetDelegate(_ddel(aSS,interface));
  }
  ~sScriptTypeInterfaceDef() {}

  static int _GetType() { return eScriptType_InterfaceDef; }
  virtual int __stdcall GetSize() const { return sizeof(sScriptTypeInterfaceDef); }
  virtual int __stdcall GetType() const { return _GetType(); }
  virtual size_t __stdcall Hash() const {
    return (size_t)pInterfaceDef;
  }
  virtual bool __stdcall Eq(SQUserData* r) const {
    if (CmpType(r) != 0) return false;
    sScriptTypeInterfaceDef* b = (sScriptTypeInterfaceDef*)r;
    return pInterfaceDef == b->pInterfaceDef;
  }
  virtual int __stdcall Cmp(SQUserData* r) const {
    int res = CmpType(r);
    if (res != 0) return res;
    sScriptTypeInterfaceDef* b = (sScriptTypeInterfaceDef*)r;
    return ni::CmpByVal(pInterfaceDef,b->pInterfaceDef);
  }
  virtual SQUserData* __stdcall CloneData(SQSharedState& aSS, tSQDeepCloneGuardSet* apDeepClone) const {
    return niNew sScriptTypeInterfaceDef(aSS,pInterfaceDef);
  }
  virtual cString __stdcall GetTypeString() const {
    return niFmt("interface_def<%s>", pInterfaceDef ? pInterfaceDef->maszName : "NULL");
  }
};

//////////////////////////////////////////////////////////////////////////////////////////////
// sMethodDef type

struct sScriptTypeMethodDef : SQ_USERDATA_BASE(sScriptTypeMethodDef)
{
  virtual ni::iUnknown* __stdcall QueryInterface(const ni::tUUID& aIID) {
    return this->_DoQueryInterface(this,aIID);
  }

  const sInterfaceDef*  pInterfaceDef;
  const sMethodDef*   pMethodDef;

  sScriptTypeMethodDef(
    const SQSharedState& aSS,
    const sInterfaceDef* apInterfaceDef,
    const sMethodDef* apMethodDef)
      : pInterfaceDef(apInterfaceDef)
      , pMethodDef(apMethodDef)
  {
    SetDelegate(_ddel(aSS,method));
  }
  ~sScriptTypeMethodDef() {}

  static int _GetType() { return eScriptType_MethodDef; }
  virtual int __stdcall GetSize() const { return sizeof(sScriptTypeMethodDef); }
  virtual int __stdcall GetType() const { return _GetType(); }
  virtual size_t __stdcall Hash() const {
    return (size_t)((size_t)pInterfaceDef+(size_t)pMethodDef);
  }
  virtual bool __stdcall Eq(SQUserData* r) const {
    if (CmpType(r) != 0) return false;
    sScriptTypeMethodDef* b = (sScriptTypeMethodDef*)r;
    return
        pInterfaceDef == b->pInterfaceDef &&
        pMethodDef == b->pMethodDef;
  }
  virtual int __stdcall Cmp(SQUserData* r) const {
    int res = CmpType(r);
    if (res != 0) return res;
    sScriptTypeMethodDef* b = (sScriptTypeMethodDef*)r;
    res = ni::CmpByVal(pInterfaceDef,b->pInterfaceDef);
    if (res != 0) return res;
    return ni::CmpByVal(pMethodDef,b->pMethodDef);
  }
  virtual SQUserData* __stdcall CloneData(SQSharedState& aSS, tSQDeepCloneGuardSet* apDeepClone) const {
    return niNew sScriptTypeMethodDef(aSS,pInterfaceDef,pMethodDef);
  }
  virtual cString __stdcall GetTypeString() const {
    return niFmt("method_def<%s,%s/%d>",
                 pInterfaceDef ? pInterfaceDef->maszName : "NULL",
                 pMethodDef ? pMethodDef->maszName : "NULL",
                 pMethodDef ? pMethodDef->mnNumParameters : 0);
  }
};


//////////////////////////////////////////////////////////////////////////////////////////////
// PropertyDef type

struct sPropertyMethods
{
  const sMethodDef* pSet;
  const sMethodDef* pGet;
  sPropertyMethods(const sMethodDef* apSet = NULL, const sMethodDef* apGet = NULL)
      : pSet(apSet), pGet(apGet) {}
};

struct sScriptTypePropertyDef : SQ_USERDATA_BASE(sScriptTypePropertyDef)
{
  virtual ni::iUnknown* __stdcall QueryInterface(const ni::tUUID& aIID) {
    return this->_DoQueryInterface(this,aIID);
  }

  const sInterfaceDef*  pInterfaceDef;
  const sMethodDef*   pSetMethodDef;
  const sMethodDef*   pGetMethodDef;

  sScriptTypePropertyDef(
      const sInterfaceDef* apInterfaceDef,
      const sMethodDef* apSetMethodDef,
      const sMethodDef* apGetMethodDef)
      : pInterfaceDef(apInterfaceDef)
      , pSetMethodDef(apSetMethodDef)
      , pGetMethodDef(apGetMethodDef)
  {}

  ~sScriptTypePropertyDef() {}

  cString GetName() const {
    cString ret;
    if (pSetMethodDef)
      return StringToPropertyName(ret,pSetMethodDef->maszName);
    else
      return StringToPropertyName(ret,pGetMethodDef->maszName);
  }

  static int _GetType() { return eScriptType_PropertyDef; }
  virtual int __stdcall GetSize() const { return sizeof(sScriptTypePropertyDef); }
  virtual int __stdcall GetType() const { return _GetType(); }
  virtual size_t __stdcall Hash() const {
    return (size_t)((size_t)pInterfaceDef+(size_t)pSetMethodDef+(size_t)pGetMethodDef);
  }
  virtual bool __stdcall Eq(SQUserData* r) const {
    if (CmpType(r) != 0) return false;
    sScriptTypePropertyDef* b = (sScriptTypePropertyDef*)r;
    return
        pInterfaceDef == b->pInterfaceDef &&
        pSetMethodDef == b->pSetMethodDef &&
        pGetMethodDef == b->pGetMethodDef;
  }
  virtual int __stdcall Cmp(SQUserData* r) const {
    int res = CmpType(r);
    if (res != 0) return res;
    sScriptTypePropertyDef* b = (sScriptTypePropertyDef*)r;
    res = ni::CmpByVal(pInterfaceDef,b->pInterfaceDef);
    if (res != 0) return res;
    res = ni::CmpByVal(pSetMethodDef,b->pSetMethodDef);
    if (res != 0) return res;
    return ni::CmpByVal(pGetMethodDef,b->pGetMethodDef);
  }
  virtual SQUserData* __stdcall CloneData(SQSharedState& aSS, tSQDeepCloneGuardSet* apDeepClone) const {
    return niNew sScriptTypePropertyDef(pInterfaceDef,pSetMethodDef,pGetMethodDef);
  }

  virtual cString __stdcall GetTypeString() const {
    return niFmt("property_def<%s,set:%s,get:%s>",
                 pInterfaceDef ? pInterfaceDef->maszName : "NULL",
                 pSetMethodDef ? pSetMethodDef->maszName : "NULL",
                 pGetMethodDef ? pGetMethodDef->maszName : "NULL");
  }
};

//////////////////////////////////////////////////////////////////////////////////////////////
// Indexed property type

struct sScriptTypeIndexedProperty : SQ_USERDATA_BASE(sScriptTypeIndexedProperty)
{
  virtual ni::iUnknown* __stdcall QueryInterface(const ni::tUUID& aIID) {
    return this->_DoQueryInterface(this,aIID);
  }

  Ptr<iUnknown>       pObject;
  Ptr<sScriptTypePropertyDef> pProp;

  sScriptTypeIndexedProperty(
    const SQSharedState& aSS,
    iUnknown* apObject, const sScriptTypePropertyDef* apProp)
      : pObject(apObject)
      , pProp(apProp)
  {
    SetDelegate(_ddel(aSS,idxprop));
  }

  ~sScriptTypeIndexedProperty() {
  }

  static int _GetType() { return eScriptType_IndexedProperty; }
  virtual int __stdcall GetSize() const { return sizeof(sScriptTypeIndexedProperty); }
  virtual int __stdcall GetType() const { return _GetType(); }
  virtual size_t __stdcall Hash() const {
    return (size_t)((size_t)pObject.ptr()+(size_t)pProp.ptr());
  }
  virtual bool __stdcall Eq(SQUserData* r) const {
    if (CmpType(r) != 0) return false;
    sScriptTypeIndexedProperty* b = (sScriptTypeIndexedProperty*)r;
    return
        pObject == b->pObject &&
        pProp == b->pProp;
  }
  virtual int __stdcall Cmp(SQUserData* r) const {
    tIntPtr res = CmpType(r);
    if (res != 0) return res;
    sScriptTypeIndexedProperty* b = (sScriptTypeIndexedProperty*)r;
    res = ni::CmpByVal(pObject.ptr(),b->pObject.ptr());
    if (res != 0) return res;
    return ni::CmpByVal(pProp.ptr(),b->pProp.ptr());
  }
  virtual SQUserData* __stdcall CloneData(SQSharedState& aSS, tSQDeepCloneGuardSet* apDeepClone) const {
    return niNew sScriptTypeIndexedProperty(aSS,pObject,pProp);
  }

  virtual cString __stdcall GetTypeString() const {
    return niFmt("indexed_property<%s>",
                 pProp.IsOK() ? pProp->GetTypeString() : "NULL");
  }
};

//////////////////////////////////////////////////////////////////////////////////////////////
// sEnumDef type

struct sScriptTypeEnumDef : SQ_USERDATA_BASE(sScriptTypeEnumDef)
{
  virtual ni::iUnknown* __stdcall QueryInterface(const ni::tUUID& aIID) {
    return this->_DoQueryInterface(this,aIID);
  }

private:
  SQObjectPtr objTable;

public:
  const sEnumDef*   pEnumDef;

  sScriptTypeEnumDef(SQSharedState& aSS, const sEnumDef* apEnumDef)
      : pEnumDef(apEnumDef)
  {
    SetDelegate(_ddel(aSS,enum));
  }

  ~sScriptTypeEnumDef() {}

  static int _GetType() { return eScriptType_EnumDef; }

  SQTable* _GetTable(SQSharedState& aSS) {
    if (objTable.IsNull()) {
      objTable = aSS.GetEnumDefTable(pEnumDef);
    }
    return _table(objTable);
  }

  virtual int __stdcall GetSize() const { return sizeof(sScriptTypeEnumDef); }
  virtual int __stdcall GetType() const { return _GetType(); }
  virtual size_t __stdcall Hash() const {
    return (size_t)pEnumDef;
  }
  virtual bool __stdcall Eq(SQUserData* r) const {
    if (CmpType(r) != 0) return false;
    sScriptTypeEnumDef* b = (sScriptTypeEnumDef*)r;
    return pEnumDef == b->pEnumDef;
  }
  virtual int __stdcall Cmp(SQUserData* r) const {
    int res = CmpType(r);
    if (res != 0) return res;
    sScriptTypeEnumDef* b = (sScriptTypeEnumDef*)r;
    return ni::CmpByVal(pEnumDef,b->pEnumDef);
  }
  virtual SQUserData* __stdcall CloneData(SQSharedState& aSS, tSQDeepCloneGuardSet* apDeepClone) const {
    sScriptTypeEnumDef* ed = niNew sScriptTypeEnumDef(aSS,pEnumDef);
    ed->objTable = this->objTable;
    return ed;
  }

  virtual cString __stdcall GetTypeString() const {
    return niFmt("enum_def<%s>",
                 pEnumDef ? pEnumDef->maszName : "NULL");
  }

#ifndef NO_GARBAGE_COLLECTOR
  virtual void __stdcall Mark(SQCollectable** chain) {
    START_MARK()
        if (!objTable.IsNull()) {
          _table(objTable)->Mark(chain);
        }
    END_MARK(chain)
        }
#endif
};

//////////////////////////////////////////////////////////////////////////////////////////////
// Math types
struct sScriptTypeMathFloats : public SQUserData {
  sScriptTypeMathFloats() {}
  virtual ni::tF32* __stdcall GetData() const = 0;
};

template <eScriptType TYPE, typename T, typename BASE>
struct sScriptTypeMathBase : public BASE {
  T _val;

  sScriptTypeMathBase(const T& v)
      : _val(v)
  {}

  static int _GetType() { return TYPE; }
  virtual int __stdcall GetSize() const { return sizeof(*this); }
  virtual int __stdcall GetType() const { return _GetType(); }
  virtual bool __stdcall Eq(SQUserData* r) const {
    if (this->CmpType(r) != 0) return false;
    sScriptTypeMathBase* b = (sScriptTypeMathBase*)r;
    return ni::VectorEq(_val.ptr(),b->_val.ptr(),_val.size());
  }
  virtual int __stdcall Cmp(SQUserData* r) const {
    int res = this->CmpType(r);
    if (res != 0) return res;
    sScriptTypeMathBase* b = (sScriptTypeMathBase*)r;
    return ni::VectorCmp(_val.ptr(),b->_val.ptr(),_val.size());
  }
  ni::tF32* __stdcall GetData() const { return (ni::tF32*)_val.ptr(); }
};

struct sScriptTypeVec2f :
    public sScriptTypeMathBase<eScriptType_Vec2,ni::sVec2f,sScriptTypeMathFloats>,
    public SQ_USERDATA_ALLOC(sScriptTypeVec2f)
{
  virtual ni::iUnknown* __stdcall QueryInterface(const ni::tUUID& aIID) {
    return this->_DoQueryInterface(this,aIID);
  }

  sScriptTypeVec2f(const SQSharedState& aSS, const sVec2f& v)
      : sScriptTypeMathBase<eScriptType_Vec2,ni::sVec2f,sScriptTypeMathFloats>(v)
  { SetDelegate(_ddel(aSS,vec2f)); }
  virtual size_t __stdcall Hash() const {
    return ni::HashVec2(_val.ptr());
  }
  virtual SQUserData* __stdcall CloneData(SQSharedState& aSS, tSQDeepCloneGuardSet* apDeepClone) const {
    return niNew sScriptTypeVec2f(aSS,_val);
  }

  virtual cString __stdcall GetTypeString() const {
    return niFmt("%s", _val);
  }
};

struct sScriptTypeVec3f :
    public sScriptTypeMathBase<eScriptType_Vec3,ni::sVec3f,sScriptTypeMathFloats>,
    public SQ_USERDATA_ALLOC(sScriptTypeVec3f)
{
  virtual ni::iUnknown* __stdcall QueryInterface(const ni::tUUID& aIID) {
    return this->_DoQueryInterface(this,aIID);
  }

  sScriptTypeVec3f(const SQSharedState& aSS, const sVec3f& v)
      : sScriptTypeMathBase<eScriptType_Vec3,ni::sVec3f,sScriptTypeMathFloats>(v)
  { SetDelegate(_ddel(aSS,vec3f)); }
  virtual size_t __stdcall Hash() const {
    return ni::HashVec3(_val.ptr());
  }
  virtual SQUserData* __stdcall CloneData(SQSharedState& aSS, tSQDeepCloneGuardSet* apDeepClone) const {
    return niNew sScriptTypeVec3f(aSS,_val);
  }

  virtual cString __stdcall GetTypeString() const {
    return niFmt("%s", _val);
  }
};

struct sScriptTypeVec4f :
    public sScriptTypeMathBase<eScriptType_Vec4,ni::sVec4f,sScriptTypeMathFloats>,
    public SQ_USERDATA_ALLOC(sScriptTypeVec4f)
{
  virtual ni::iUnknown* __stdcall QueryInterface(const ni::tUUID& aIID) {
    return this->_DoQueryInterface(this,aIID);
  }

  sScriptTypeVec4f(const SQSharedState& aSS, const sVec4f& v)
      : sScriptTypeMathBase<eScriptType_Vec4,ni::sVec4f,sScriptTypeMathFloats>(v)
  { SetDelegate(_ddel(aSS,vec4f)); }
  virtual size_t __stdcall Hash() const {
    return ni::HashVec4(_val.ptr());
  }
  virtual SQUserData* __stdcall CloneData(SQSharedState& aSS, tSQDeepCloneGuardSet* apDeepClone) const {
    return niNew sScriptTypeVec4f(aSS,_val);
  }

  virtual cString __stdcall GetTypeString() const {
    return niFmt("%s", _val);
  }
};

struct sScriptTypeMatrixf :
    public sScriptTypeMathBase<eScriptType_Matrix,ni::sMatrixf,sScriptTypeMathFloats>,
    public SQ_USERDATA_ALLOC(sScriptTypeMatrixf)
{
  virtual ni::iUnknown* __stdcall QueryInterface(const ni::tUUID& aIID) {
    return this->_DoQueryInterface(this,aIID);
  }

  sScriptTypeMatrixf(const SQSharedState& aSS, const sMatrixf& v)
      : sScriptTypeMathBase<eScriptType_Matrix,ni::sMatrixf,sScriptTypeMathFloats>(v)
  { SetDelegate(_ddel(aSS,matrixf)); }
  virtual size_t __stdcall Hash() const {
    return ni::HashVec4(_val.ptr());
  }
  virtual SQUserData* __stdcall CloneData(SQSharedState& aSS, tSQDeepCloneGuardSet* apDeepClone) const {
    return niNew sScriptTypeMatrixf(aSS,_val);
  }

  virtual cString __stdcall GetTypeString() const {
    return niFmt("%s", _val);
  }
};

//////////////////////////////////////////////////////////////////////////////////////////////
// UUID

struct sScriptTypeUUID : SQ_USERDATA_BASE(sScriptTypeUUID)
{
  virtual ni::iUnknown* __stdcall QueryInterface(const ni::tUUID& aIID) {
    return this->_DoQueryInterface(this,aIID);
  }

  sUUID mUUID;

  sScriptTypeUUID(const SQSharedState& aSS, const tUUID& aUUID)
      : mUUID(aUUID)
  { SetDelegate(_ddel(aSS,uuid)); }

  ~sScriptTypeUUID()  {}

  static int _GetType() { return eScriptType_UUID; }
  virtual int __stdcall GetSize() const { return sizeof(sScriptTypeUUID); }
  virtual int __stdcall GetType() const { return _GetType(); }
  virtual size_t __stdcall Hash() const {
    return (size_t)ni::HashUUID(mUUID);
  }
  virtual bool __stdcall Eq(SQUserData* r) const {
    if (CmpType(r) != 0) return false;
    sScriptTypeUUID* b = (sScriptTypeUUID*)r;
    return !!(mUUID == b->mUUID);
  }
  virtual int __stdcall Cmp(SQUserData* r) const {
    int res = CmpType(r);
    if (res != 0) return res;
    sScriptTypeUUID* b = (sScriptTypeUUID*)r;
    return mUUID.Compare(b->mUUID);
  }
  virtual SQUserData* __stdcall CloneData(SQSharedState& aSS, tSQDeepCloneGuardSet* apDeepClone) const {
    return niNew sScriptTypeUUID(aSS,mUUID);
  }

  virtual cString __stdcall GetTypeString() const {
    return niFmt("UUID<%s>", cString(mUUID));
  }
};

//////////////////////////////////////////////////////////////////////////////////////////////
// sErrorCode type

struct sScriptTypeErrorCode : SQ_USERDATA_BASE(sScriptTypeErrorCode)
{
  virtual ni::iUnknown* __stdcall QueryInterface(const ni::tUUID& aIID) {
    return this->_DoQueryInterface(this,aIID);
  }

  NN<iHString> _hspKind;
  cString _strDesc;

  sScriptTypeErrorCode(
    const SQSharedState& aSS,
    ain_nn<iHString> aKind,
    ain<tChars> aDesc)
      : _hspKind(aKind)
      , _strDesc(aDesc)
  {
    SetDelegate(_ddel(aSS,error_code));
  }
  ~sScriptTypeErrorCode() {}

  static int _GetType() { return eScriptType_ErrorCode; }
  virtual int __stdcall GetSize() const { return sizeof(sScriptTypeErrorCode); }
  virtual int __stdcall GetType() const { return _GetType(); }
  virtual size_t __stdcall Hash() const {
    return (size_t)_hspKind.raw_ptr();
  }
  virtual bool __stdcall Eq(SQUserData* r) const {
    if (CmpType(r) != 0) return false;
    sScriptTypeErrorCode* b = (sScriptTypeErrorCode*)r;
    return _hspKind == b->_hspKind;
  }
  virtual int __stdcall Cmp(SQUserData* r) const {
    int res = CmpType(r);
    if (res != 0) return res;
    sScriptTypeErrorCode* b = (sScriptTypeErrorCode*)r;
    return ni::CmpByVal(_hspKind,b->_hspKind);
  }
  virtual SQUserData* __stdcall CloneData(SQSharedState& aSS, tSQDeepCloneGuardSet* apDeepClone) const {
    return niNew sScriptTypeErrorCode(aSS,_hspKind,_strDesc.c_str());
  }

  virtual cString __stdcall GetTypeString() const {
    if (_strDesc.IsEmpty()) {
      return niFmt("error_code<%s>", _hspKind);
    }
    else {
      return niFmt("error_code<%s,%s>", _hspKind, _strDesc);
    }
  }
};

//////////////////////////////////////////////////////////////////////////////////////////////
// sResolvedType type

struct sScriptTypeResolvedType : SQ_USERDATA_BASE(sScriptTypeResolvedType)
{
  virtual ni::iUnknown* __stdcall QueryInterface(const ni::tUUID& aIID) {
    return this->_DoQueryInterface(this,aIID);
  }

  const eScriptType _scriptType;

  sScriptTypeResolvedType(
    const SQSharedState& aSS,
    const eScriptType aType)
      : _scriptType(aType)
  {
    SetDelegate(_ddel(aSS,resolved_type));
  }
  ~sScriptTypeResolvedType() {}

  static int _GetType() { return eScriptType_ResolvedType; }
  virtual int __stdcall GetSize() const { return sizeof(sScriptTypeResolvedType); }
  virtual int __stdcall GetType() const { return _GetType(); }
  virtual size_t __stdcall Hash() const {
    return (size_t)_scriptType;
  }
  virtual bool __stdcall Eq(SQUserData* r) const {
    if (CmpType(r) != 0) return false;
    sScriptTypeResolvedType* b = (sScriptTypeResolvedType*)r;
    return _scriptType == b->_scriptType;
  }
  virtual int __stdcall Cmp(SQUserData* r) const {
    int res = CmpType(r);
    if (res != 0) return res;
    sScriptTypeResolvedType* b = (sScriptTypeResolvedType*)r;
    return ni::CmpByVal(_scriptType,b->_scriptType);
  }
  virtual SQUserData* __stdcall CloneData(SQSharedState& aSS, tSQDeepCloneGuardSet* apDeepClone) const {
    return niNew sScriptTypeResolvedType(aSS,_scriptType);
  }

  virtual cString __stdcall GetTypeString() const;
};

//////////////////////////////////////////////////////////////////////////////////////////////

niExportFunc(int) sqa_pushPropertyDef(HSQUIRRELVM v, const ni::sInterfaceDef* apInterfaceDef, const ni::sMethodDef* apSetMethDef, const ni::sMethodDef* apGetMethDef);
niExportFunc(int) sqa_getPropertyDef(HSQUIRRELVM v, int idx, const ni::sInterfaceDef** appInterfaceDef, const ni::sMethodDef** appSetMethDef, const ni::sMethodDef** appGetMethDef);

niExportFunc(int) sqa_pushIndexedProperty(HSQUIRRELVM v, iUnknown* apObject, const sScriptTypePropertyDef* apProp);
niExportFunc(int) sqa_getIndexedProperty(HSQUIRRELVM v, int idx, iUnknown** appObject, const sScriptTypePropertyDef** appProp);

niExportFunc(int) sqa_pushEnumDef(HSQUIRRELVM v, const sEnumDef* apEnumDef);
niExportFunc(int) sqa_getEnumDef(HSQUIRRELVM v, int idx, const sEnumDef** appEnumDef);

niExportFunc(int) sqa_pushIndexedProperty(HSQUIRRELVM v, iUnknown* apObject, const sScriptTypePropertyDef* apProp);
niExportFunc(int) sqa_getIndexedProperty(HSQUIRRELVM v, int idx, iUnknown** appObject, const sScriptTypePropertyDef** appProp);

niExportFunc(int) sqa_pushErrorCode(HSQUIRRELVM v, iHString* ahspType, iHString* ahspReason);
niExportFuncCPP(int) sqa_getErrorCode(HSQUIRRELVM v, int idx, aout<NN<sScriptTypeErrorCode>> aOut);

niExportFunc(int) sqa_pushResolvedType(HSQUIRRELVM v, ain<eScriptType> aType);
niExportFunc(int) sqa_getResolvedType(HSQUIRRELVM v, int idx, aout<eScriptType> aType);

// int sqa_callmethod_closure(HSQUIRRELVM v);
// int sqa_callmethod_delegate(HSQUIRRELVM v);

niExportFunc(eScriptType) sqa_getscriptobjtype(const SQObjectPtr& obj);
niExportFunc(eScriptType) sqa_getscripttype(HSQUIRRELVM v, int idx);
niExportFunc(eScriptType) sqa_type2scripttype(const tType aType);
niExportFunc(iHString*) sqa_getscripttypename(eScriptType aType);

bool iunknown_nexti(HSQUIRRELVM v, iUnknown* apObj, const SQObjectPtr &refpos, SQObjectPtr &outkey, SQObjectPtr &outval, SQObjectPtr &outitr);

struct sGetIUnknown {
  iUnknown* pObject;
};
#define GET_IUNKNOWNUD(NAME,INDEX)                                \
  sGetIUnknown get_##NAME;                                        \
  sGetIUnknown* p##NAME = &get_##NAME;                            \
  if (!SQ_SUCCEEDED(sq_getiunknown(v,INDEX,&get_##NAME.pObject))) \
    return SQ_ERROR;

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __SCRIPTTYPES_37182345_H__
