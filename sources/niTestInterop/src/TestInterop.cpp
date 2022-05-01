// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "TestInterop.h"
#include <niLang/Utils/UnknownImpl.h>
#include <niLang/Utils/ThreadImpl.h>

static astl::vector<iTestInteropObject*> _allObjects;
static ni::ThreadMutex _mutexObjects;

///
/// TestInteropObject
///

//! Test interop object
struct sTestInteropObject : public cIUnknownImpl<iTestInteropObject>
{
  astl::vector<tU32> _vItems;
  astl::map<cString,tU32> _mapItems;
  astl::vector<Ptr<iUnknown> > _vItems2;
  astl::map<cString,Ptr<iUnknown> > _mapItems2;
  astl::vector<Ptr<iTestInteropObject> > _vItems3;
  astl::map<cString,Ptr<iTestInteropObject> > _mapItems3;
  astl::vector<tU8> _buffer;
  sTestInterop _testInterop;
  sTestInterop* _pTestInterop;
  struct {
    eTestInterop Enum;
    tTestInteropFlags Flags;
    tIntPtr IntPtr;
    tUIntPtr UIntPtr;
    tI8 I8;
    tU8 U8;
    tI16 I16;
    tU16 U16;
    tI32 I32;
    tU32 U32;
    tI64 I64;
    tU64 U64;
    tF32 F32;
    tF64 F64;
    sVec2i Vec2i;
    sVec2f Vec2f;
    sVec3i Vec3i;
    sVec3f Vec3f;
    sVec4i Vec4i;
    sVec4f Vec4f;
    sMatrixf Matrixf;
    tPtr Ptr;
    void* VoidP;
    tU32 U32Ref;
    sColor3ub col3ub;
    sColor4ub col4ub;
    Var var;
  } _v;
  cString _astr;
  cString _str;
  Ptr<iTestInteropObject> _obj3;
  tHStringPtr _hstr;
  Ptr<tU32CVec> _vecU32;
  Ptr<tStringCMap> _mapString;

  sTestInteropObject(tSize aNumItems) {
    _vecU32 = tU32CVec::Create();
    _mapString = tStringCMap::Create();
    memset(&_testInterop,0,sizeof(_testInterop));
    memset(&_v,0,sizeof(_v));
    _pTestInterop = NULL;
    _buffer.resize(4096);
    SetNumItems(aNumItems);
    {
      AutoThreadLock lock(_mutexObjects);
      _allObjects.push_back(this);
    }
  }
  ~sTestInteropObject() {
    {
      AutoThreadLock lock(_mutexObjects);
      astl::find_erase(_allObjects,this);
    }
  }

  virtual void __stdcall SetNumItems(tSize aNumItems) {
    _vItems.resize(aNumItems);
    _vItems2.resize(aNumItems);
    _vItems3.resize(aNumItems);
  }
  virtual tSize __stdcall GetNumItems() const {
    return _vItems.size();
  }

  virtual void __stdcall SetTestInterop(sTestInterop* apStruct) {
    _testInterop = *apStruct;
  }
  virtual sTestInterop* __stdcall GetTestInterop() const {
    return (sTestInterop*)&_testInterop;
  }
  virtual void __stdcall SetTestInteropConst(const sTestInterop* apStruct) {
    _testInterop = *apStruct;
  }
  virtual const sTestInterop* __stdcall GetTestInteropConst() const {
    return &_testInterop;
  }

  virtual void __stdcall SetTestInteropRef(sTestInterop& aStruct) {
    _testInterop = aStruct;
  }
  virtual sTestInterop& __stdcall GetTestInteropRef() const {
    return (sTestInterop&)_testInterop;
  }
  virtual void __stdcall SetTestInteropConstRef(const sTestInterop& aStruct) {
    _testInterop = aStruct;
  }
  virtual const sTestInterop& __stdcall GetTestInteropConstRef() const {
    return _testInterop;
  }

  virtual void __stdcall SetTestInteropPP(sTestInterop** apStruct) {
    _pTestInterop = *apStruct;
  }
  virtual sTestInterop** __stdcall GetTestInteropPP() const {
    return (sTestInterop**)&_pTestInterop;
  }
  virtual void __stdcall SetTestInteropConstPP(const sTestInterop** apStruct) {
    _pTestInterop = (sTestInterop*)*apStruct;
  }
  virtual const sTestInterop** __stdcall GetTestInteropConstPP() const {
    return (const sTestInterop**)&_pTestInterop;
  }

  virtual void __stdcall SetEnum(eTestInterop aV) {
    _v.Enum = aV;
  }
  virtual eTestInterop __stdcall GetEnum() const {
    return _v.Enum;
  }

  virtual void __stdcall SetFlags(tTestInteropFlags aV) {
    _v.Flags = aV;
  }
  virtual tTestInteropFlags __stdcall GetFlags() const {
    return _v.Flags;
  }

  virtual void __stdcall SetIntPtr(tIntPtr aV) {
    _v.IntPtr = aV;
  }
  virtual tIntPtr __stdcall GetIntPtr() const {
    return _v.IntPtr;
  }
  virtual void __stdcall SetUIntPtr(tIntPtr aV) {
    _v.UIntPtr = aV;
  }
  virtual tIntPtr __stdcall GetUIntPtr() const {
    return _v.UIntPtr;
  }

  virtual void __stdcall SetCol3ub(const sColor3ub& aV) {
    _v.col3ub = aV;
  }
  virtual const sColor3ub& __stdcall GetCol3ub() const {
    return _v.col3ub;
  }
  virtual void __stdcall SetCol4ub(const sColor4ub& aV) {
    _v.col4ub = aV;
  }
  virtual const sColor4ub& __stdcall GetCol4ub() const {
    return _v.col4ub;
  }

  virtual void __stdcall SetI8(tI8 aV) {
    _v.I8 = aV;
  }
  virtual tI8 __stdcall GetI8() const {
    return _v.I8;
  }
  virtual void __stdcall SetU8(tU8 aV) {
    _v.U8 = aV;
  }
  virtual tU8 __stdcall GetU8() const {
    return _v.U8;
  }
  virtual void __stdcall SetI16(tI16 aV) {
    _v.I16 = aV;
  }
  virtual tI16 __stdcall GetI16() const {
    return _v.I16;
  }
  virtual void __stdcall SetU16(tU16 aV) {
    _v.U16 = aV;
  }
  virtual tU16 __stdcall GetU16() const {
    return _v.U16;
  }
  virtual void __stdcall SetI32(tI32 aV) {
    _v.I32 = aV;
  }
  virtual tI32 __stdcall GetI32() const {
    return _v.I32;
  }
  virtual void __stdcall SetU32(tU32 aV) {
    _v.U32 = aV;
  }
  virtual tU32 __stdcall GetU32() const {
    return _v.U32;
  }
  virtual void __stdcall SetI64(tI64 aV) {
    _v.I64 = aV;
  }
  virtual tI64 __stdcall GetI64() const {
    return _v.I64;
  }
  virtual void __stdcall SetU64(tU64 aV) {
    _v.U64 = aV;
  }
  virtual tU64 __stdcall GetU64() const {
    return _v.U64;
  }
  virtual void __stdcall SetF32(tF32 aV) {
    _v.F32 = aV;
  }
  virtual tF32 __stdcall GetF32() const {
    return _v.F32;
  }
  virtual void __stdcall SetF64(tF64 aV) {
    _v.F64 = aV;
  }
  virtual tF64 __stdcall GetF64() const {
    return _v.F64;
  }

  virtual void __stdcall SetVar(const Var& aV) {
    _v.var = aV;
  }
  virtual Var __stdcall GetVar() const {
    return _v.var;
  }

  virtual void __stdcall SetVec2l(const sVec2i& aV) {
    _v.Vec2i = aV;
  }
  virtual sVec2i __stdcall GetVec2l() const {
    return _v.Vec2i;
  }
  virtual void __stdcall SetVec2f(const sVec2f& aV) {
    _v.Vec2f = aV;
  }
  virtual sVec2f __stdcall GetVec2f() const {
    return _v.Vec2f;
  }
  virtual void __stdcall SetVec3l(const sVec3i& aV) {
    _v.Vec3i = aV;
  }
  virtual sVec3i __stdcall GetVec3l() const {
    return _v.Vec3i;
  }
  virtual void __stdcall SetVec3f(const sVec3f& aV) {
    _v.Vec3f = aV;
  }
  virtual sVec3f __stdcall GetVec3f() const {
    return _v.Vec3f;
  }
  virtual void __stdcall SetVec4l(const sVec4i& aV) {
    _v.Vec4i = aV;
  }
  virtual sVec4i __stdcall GetVec4l() const {
    return _v.Vec4i;
  }
  virtual void __stdcall SetVec4f(const sVec4f& aV) {
    _v.Vec4f = aV;
  }
  virtual sVec4f __stdcall GetVec4f() const {
    return _v.Vec4f;
  }
  virtual void __stdcall SetMatrixf(const sMatrixf& aV) {
    _v.Matrixf = aV;
  }
  virtual sMatrixf __stdcall GetMatrixf() const {
    return _v.Matrixf;
  }
  virtual ni::tBool __stdcall SetMatrixfValues(
      ni::tF32 a00,ni::tF32 a01,ni::tF32 a02,ni::tF32 a03,
      ni::tF32 a10,ni::tF32 a11,ni::tF32 a12,ni::tF32 a13,
      ni::tF32 a20,ni::tF32 a21,ni::tF32 a22,ni::tF32 a23,
      ni::tF32 a30,ni::tF32 a31,ni::tF32 a32,ni::tF32 a33)
  {
    _v.Matrixf._11 = a00; _v.Matrixf._12 = a01; _v.Matrixf._13 = a02; _v.Matrixf._14 = a03;
    _v.Matrixf._21 = a10; _v.Matrixf._22 = a11; _v.Matrixf._23 = a12; _v.Matrixf._24 = a13;
    _v.Matrixf._31 = a20; _v.Matrixf._32 = a21; _v.Matrixf._33 = a22; _v.Matrixf._34 = a23;
    _v.Matrixf._41 = a30; _v.Matrixf._42 = a31; _v.Matrixf._43 = a32; _v.Matrixf._44 = a33;
    return eTrue;
  }

  virtual void __stdcall SetACharStrZ(const achar* aV) {
    _astr = aV;
  }
  virtual const achar* __stdcall GetACharStrZ() const {
    return _astr.Chars();
  }

  virtual void __stdcall SetString(cString aStr) {
    _str = aStr;
  }
  virtual cString __stdcall GetString() const {
    return _str;
  }

  virtual void __stdcall SetHString(iHString* ahspStr) {
    _hstr = ahspStr;
  }
  virtual iHString* __stdcall GetHString() const {
    return _hstr;
  }

  virtual void __stdcall SetStringCRef(const cString& aStr) {
    _str = aStr;
  }
  virtual const cString& __stdcall GetStringCRef() const {
    return _str;
  }

  virtual void __stdcall SetIndexedU32(tSize aIndex, tU32 aV) {
    if (aIndex >= _vItems.size()) return;
    _vItems[aIndex] = aV;
  }
  virtual tU32 __stdcall GetIndexedU32(tSize aIndex) const {
    if (aIndex >= _vItems.size()) return eInvalidHandle;
    return _vItems[aIndex];
  }
  virtual void __stdcall SetMappedU32(const achar* aaszKey, tU32 aV) {
    _mapItems[aaszKey] = aV;
  }
  virtual tU32 __stdcall GetMappedU32(const achar* aaszKey) const {
    astl::map<cString,tU32>::const_iterator it = _mapItems.find(_ASTR(aaszKey));
    if (it == _mapItems.end())
      return eInvalidHandle;
    return it->second;
  }

  virtual void __stdcall SetPtr(tPtr aV) {
    _v.Ptr = aV;
  }
  virtual tPtr __stdcall GetPtr() const {
    return _v.Ptr;
  }

  virtual void __stdcall SetVoidP(void* aV) {
    _v.VoidP = aV;
  }
  virtual void* __stdcall GetVoidP() const {
    return _v.VoidP;
  }

  virtual void __stdcall SetU32Ref(tU32& aV) {
    _v.U32Ref = aV;
  }
  virtual tU32& __stdcall GetU32Ref() const {
    return (tU32&)_v.U32Ref;
  }

  virtual void __stdcall SetU32Vec(tU32CVec* apVec)  {
    _vecU32 = apVec;
  }
  virtual tU32CVec* __stdcall GetU32Vec() const {
    return _vecU32;
  }

  virtual void __stdcall SetStringMap(tStringCMap* apMap) {
    _mapString = apMap;
  }
  virtual tStringCMap* __stdcall GetStringMap() const {
    return _mapString;
  }

  tPtr _Buf() {
    return (tPtr)&_buffer[0];
  }
  tPtr _CBuf() const {
    return (tPtr)&_buffer[0];
  }

  virtual ni::tBool __stdcall WriteRawBuffer(const tPtr aData, tSize anSize) {
    ni::MemCopy(_Buf(),aData,anSize);
    return eTrue;
  }
  virtual ni::tBool __stdcall ReadRawBuffer(tPtr aData, tSize anSize) {
    ni::MemCopy(aData,_Buf(),anSize);
    return ni::eTrue;
  }

  virtual ni::tBool __stdcall WriteRawBufferVoid(const void* apIn, tSize anSize) {
    ni::MemCopy(_Buf(),(tPtr)apIn,anSize);
    return eTrue;
  }
  virtual ni::tBool __stdcall ReadRawBufferVoid(void* apOut, tSize anSize) {
    ni::MemCopy((tPtr)apOut,_Buf(),anSize);
    return ni::eTrue;
  }

  virtual ni::tBool __stdcall WriteRawBufferU32(const tU32* apIn, tSize anSize) {
    ni::MemCopy(_Buf(),(tPtr)apIn,anSize);
    return eTrue;
  }
  virtual ni::tBool __stdcall ReadRawBufferU32(tU32* apOut, tSize anSize) {
    ni::MemCopy((tPtr)apOut,_Buf(),anSize);
    return ni::eTrue;
  }

  virtual void __stdcall SetIndexedIUnknown(tSize aIndex, ni::iUnknown* aV) {
    if (aIndex >= _vItems2.size()) return;
    _vItems2[aIndex] = aV;
  }
  virtual ni::iUnknown* __stdcall GetIndexedIUnknown(tSize aIndex) const {
    if (aIndex >= _vItems2.size()) return NULL;
    return _vItems2[aIndex];
  }
  virtual void __stdcall SetMappedIUnknown(const achar* aaszKey, ni::iUnknown* aV) {
    _mapItems2[aaszKey] = aV;
  }
  virtual ni::iUnknown* __stdcall GetMappedIUnknown(const achar* aaszKey) const {
    astl::map<cString,Ptr<iUnknown> >::const_iterator it = _mapItems2.find(aaszKey);
    if (it == _mapItems2.end())
      return NULL;
    return it->second;
  }

  virtual void __stdcall SetTestInteropObject(iTestInteropObject* aV) {
    _obj3 = aV;
  }
  virtual iTestInteropObject* __stdcall GetTestInteropObject() const {
    return _obj3;
  }
  virtual void __stdcall SetIndexedTestInteropObject(tSize aIndex, iTestInteropObject* aV) {
    if (aIndex >= _vItems3.size()) return;
    _vItems3[aIndex] = aV;
  }
  virtual iTestInteropObject* __stdcall GetIndexedTestInteropObject(tSize aIndex) const {
    if (aIndex >= _vItems3.size()) return NULL;
    return _vItems3[aIndex];
  }
  virtual void __stdcall SetMappedTestInteropObject(const achar* aaszKey, iTestInteropObject* aV) {
    _mapItems3[aaszKey] = aV;
  }
  virtual iTestInteropObject* __stdcall GetMappedTestInteropObject(const achar* aaszKey) const {
    astl::map<cString,Ptr<iTestInteropObject> >::const_iterator it = _mapItems3.find(aaszKey);
    if (it == _mapItems3.end())
      return NULL;
    return it->second;
  }

  //! Function with 32 parameters
  virtual ni::tBool __stdcall Func32(
      ni::tU32 a00,ni::tU32 a01,ni::tU32 a02,ni::tU32 a03,
      ni::tU32 a10,ni::tU32 a11,ni::tU32 a12,ni::tU32 a13,
      ni::tU32 a20,ni::tU32 a21,ni::tU32 a22,ni::tU32 a23,
      ni::tU32 a30,ni::tU32 a31,ni::tU32 a32,ni::tU32 a33,
      ni::tU32 a40,ni::tU32 a41,ni::tU32 a42,ni::tU32 a43,
      ni::tU32 a50,ni::tU32 a51,ni::tU32 a52,ni::tU32 a53,
      ni::tU32 a60,ni::tU32 a61,ni::tU32 a62,ni::tU32 a63,
      ni::tU32 a70,ni::tU32 a71,ni::tU32 a72,ni::tU32 a73)
  {
    return eTrue;
  }
  //! Function with 33 parameters
  virtual ni::tBool __stdcall Func33(
      ni::tU32 a00,ni::tU32 a01,ni::tU32 a02,ni::tU32 a03,
      ni::tU32 a10,ni::tU32 a11,ni::tU32 a12,ni::tU32 a13,
      ni::tU32 a20,ni::tU32 a21,ni::tU32 a22,ni::tU32 a23,
      ni::tU32 a30,ni::tU32 a31,ni::tU32 a32,ni::tU32 a33,
      ni::tU32 a40,ni::tU32 a41,ni::tU32 a42,ni::tU32 a43,
      ni::tU32 a50,ni::tU32 a51,ni::tU32 a52,ni::tU32 a53,
      ni::tU32 a60,ni::tU32 a61,ni::tU32 a62,ni::tU32 a63,
      ni::tU32 a70,ni::tU32 a71,ni::tU32 a72,ni::tU32 a73,
      ni::tU32 firstOops)
  {
    return eTrue;
  }
  //! Function with 40 parameters
  virtual ni::tBool __stdcall Func48(
      ni::tU32 a00,ni::tU32 a01,ni::tU32 a02,ni::tU32 a03,
      ni::tU32 a10,ni::tU32 a11,ni::tU32 a12,ni::tU32 a13,
      ni::tU32 a20,ni::tU32 a21,ni::tU32 a22,ni::tU32 a23,
      ni::tU32 a30,ni::tU32 a31,ni::tU32 a32,ni::tU32 a33,
      ni::tU32 a40,ni::tU32 a41,ni::tU32 a42,ni::tU32 a43,
      ni::tU32 a50,ni::tU32 a51,ni::tU32 a52,ni::tU32 a53,
      ni::tU32 a60,ni::tU32 a61,ni::tU32 a62,ni::tU32 a63,
      ni::tU32 a70,ni::tU32 a71,ni::tU32 a72,ni::tU32 a73,
      ni::tU32 a80,ni::tU32 a81,ni::tU32 a82,ni::tU32 a83,
      ni::tU32 a90,ni::tU32 a91,ni::tU32 a92,ni::tU32 a93)
  {
    return eTrue;
  }
};

///
/// TestInterop
///

///////////////////////////////////////////////
cTestInterop::cTestInterop() {
}


///////////////////////////////////////////////
cTestInterop::~cTestInterop() {
}


///////////////////////////////////////////////
void cTestInterop::ZeroMembers() {
}

///////////////////////////////////////////////
tBool __stdcall cTestInterop::IsOK() const {
  niClassIsOK(cTestInterop);
  return ni::eTrue;
}

///////////////////////////////////////////////
tInt __stdcall cTestInterop::GenerateCrash(ni::tU32 anExceptionType) {
  return 0;
}

///////////////////////////////////////////////
tSize __stdcall cTestInterop::GetNumTestInteropObjects() const {
  AutoThreadLock lock(_mutexObjects);
  return _allObjects.size();
}

///////////////////////////////////////////////
iTestInteropObject* __stdcall cTestInterop::GetTestInteropObject(ni::tSize anIndex) const {
  AutoThreadLock lock(_mutexObjects);
  if (anIndex >= _allObjects.size()) return NULL;
  return _allObjects[anIndex];
}

///////////////////////////////////////////////
iTestInteropObject* __stdcall cTestInterop::CreateTestInteropObject(ni::tU32 anNumItems) {
  return niNew sTestInteropObject(anNumItems);
}

///////////////////////////////////////////////
Ptr<iTestInteropObject> __stdcall cTestInterop::CreateTestInteropObjectPtr(ni::tU32 anNumItems) {
  return niNew sTestInteropObject(anNumItems);
}

///////////////////////////////////////////////
void __stdcall cTestInterop::ForEachObject_OnTestInteropSink_Void(iTestInteropSink* apSink) {
  niCheckSilent(niIsOK(apSink),;);
  niLoop(i,_allObjects.size()) {
    apSink->OnTestInteropSink_Void();
  }
}

///////////////////////////////////////////////
void __stdcall cTestInterop::ForEachObject_OnTestInteropSink_Void_I32(iTestInteropSink* apSink, ni::tI32 v) {
  niCheckSilent(niIsOK(apSink),;);
  niLoop(i,_allObjects.size()) {
    apSink->OnTestInteropSink_Void_I32(v);
  }
}

///////////////////////////////////////////////
void __stdcall cTestInterop::ForEachObject_OnTestInteropSink_Void_I64(iTestInteropSink* apSink, ni::tI64 v) {
  niCheckSilent(niIsOK(apSink),;);
  niLoop(i,_allObjects.size()) {
    apSink->OnTestInteropSink_Void_I64(v);
  }
}

///////////////////////////////////////////////
void __stdcall cTestInterop::ForEachObject_OnTestInteropSink_Void_Ptr(iTestInteropSink* apSink, ni::tPtr v) {
  niCheckSilent(niIsOK(apSink),;);
  niLoop(i,_allObjects.size()) {
    apSink->OnTestInteropSink_Void_Ptr((tIntPtr)v);
  }
}

///////////////////////////////////////////////
Var __stdcall cTestInterop::ForEachObject_OnTestInteropSink_Var_Var(iTestInteropSink* apSink, Var v) {
  Var r = niVarNull;
  niCheckSilent(niIsOK(apSink),r);
  niLoop(i,_allObjects.size()) {
    r = apSink->OnTestInteropSink_Var_Var(v);
  }
  return r;
}

///////////////////////////////////////////////
sVec3f __stdcall cTestInterop::ForEachObject_OnTestInteropSink_Vec3f_String(iTestInteropSink* apSink, const ni::achar* aaszStr) {
  sVec3f r = sVec3f::Zero();
  niCheckSilent(niIsOK(apSink),r);
  niLoop(i,_allObjects.size()) {
    r = apSink->OnTestInteropSink_Vec3f_String(aaszStr);
  }
  return r;
}

///////////////////////////////////////////////
sVec3f __stdcall cTestInterop::ForEachObject_OnTestInteropSink_Vec3f_CObject(iTestInteropSink* apSink) {
  sVec3f r = sVec3f::Zero();
  niCheckSilent(niIsOK(apSink),r);
  niLoop(i,_allObjects.size()) {
    iTestInteropObject* o = _allObjects[i];
    r = apSink->OnTestInteropSink_Vec3f_CObject(o);
  }
  return r;
}

///////////////////////////////////////////////
sVec3f __stdcall cTestInterop::ForEachObject_OnTestInteropSink_Vec3f_Object(iTestInteropSink* apSink) {
  sVec3f r = sVec3f::Zero();
  niCheckSilent(niIsOK(apSink),r);
  niLoop(i,_allObjects.size()) {
    iTestInteropObject* o = _allObjects[i];
    r = apSink->OnTestInteropSink_Vec3f_Object(o);
  }
  return r;
}

///////////////////////////////////////////////
iTestInteropObject* __stdcall cTestInterop::ForEachObject_OnTestInteropSink_Object_CObject(iTestInteropSink* apSink) {
  iTestInteropObject* r = NULL;
  niCheckSilent(niIsOK(apSink),r);
  niLoop(i,_allObjects.size()) {
    iTestInteropObject* o = _allObjects[i];
    r = apSink->OnTestInteropSink_Object_CObject(o);
  }
  return r;
}

///////////////////////////////////////////////
iTestInteropObject* __stdcall cTestInterop::ForEachObject_OnTestInteropSink_Object_Object(iTestInteropSink* apSink) {
  iTestInteropObject* r = NULL;
  niCheckSilent(niIsOK(apSink),r);
  niLoop(i,_allObjects.size()) {
    iTestInteropObject* o = _allObjects[i];
    r = apSink->OnTestInteropSink_Object_Object(o);
  }
  return r;
}

///////////////////////////////////////////////
iTestInteropObject* __stdcall cTestInterop::ForEachObject_OnTestInteropSink_Object_Object2(iTestInteropSink* apSink, ni::tU32 a, ni::tF32 b) {
  iTestInteropObject* r = NULL;
  niCheckSilent(niIsOK(apSink),r);
  niLoop(i,_allObjects.size()) {
    iTestInteropObject* o = _allObjects[i];
    r = apSink->OnTestInteropSink_Object_Object2(a,b,o);
  }
  return r;
}

///////////////////////////////////////////////
iTestInteropObject* __stdcall cTestInterop::ForEachObject_OnTestInteropSink_Object_ObjectVec3f(iTestInteropSink* apSink, const ni::sVec3f& v) {
  iTestInteropObject* r = NULL;
  niCheckSilent(niIsOK(apSink),r);
  niLoop(i,_allObjects.size()) {
    iTestInteropObject* o = _allObjects[i];
    r = apSink->OnTestInteropSink_Object_ObjectVec3f(o,v);
  }
  return r;
}

///////////////////////////////////////////////
niExportFunc(iUnknown*) New_niTestInterop_TestInterop(const Var& avarA, const Var& avarB) {
  return niNew cTestInterop();
}
