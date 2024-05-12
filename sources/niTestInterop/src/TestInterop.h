#ifndef __TestInterop_4519356_H__
#define __TestInterop_4519356_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

//! TestInterop implementation.
class cTestInterop : public ImplRC<iTestInterop,eImplFlags_DontInherit1,iTestInteropForEach>
{
  niBeginClass(cTestInterop);

 public:
  //! Constructor.
  cTestInterop();
  //! Destructor.
  ~cTestInterop();

  //! Zeros all the class members.
  void ZeroMembers();

  //! Sanity check.
  tBool __stdcall IsOK() const;

  //// iTestInterop /////////////////////////////////////
#if niMinFeatures(30)
  void __stdcall FeatureSetFull() { niDebugFmt(("Full")); }
#endif
#if niMinFeatures(20)
  void __stdcall FeatureSetEmbedded() { niDebugFmt(("Embedded")); }
#endif
  void __stdcall FeatureSetLight()  { niDebugFmt(("Light")); }

  tInt __stdcall GenerateCrash(ni::tU32 anExceptionType);

  tSize __stdcall GetNumTestInteropObjects() const;
  iTestInteropObject* __stdcall GetTestInteropObject(ni::tSize anIndex) const;
  iTestInteropObject* __stdcall CreateTestInteropObject(ni::tU32 anNumItems);
  Ptr<iTestInteropObject> __stdcall CreateTestInteropObjectPtr(ni::tU32 anNumItems);

  void __stdcall ForEachObject_OnTestInteropSink_Void(iTestInteropSink* apSink);
  void __stdcall ForEachObject_OnTestInteropSink_Void_I32(iTestInteropSink* apSink, ni::tI32 v);
  void __stdcall ForEachObject_OnTestInteropSink_Void_I64(iTestInteropSink* apSink, ni::tI64 v);
  void __stdcall ForEachObject_OnTestInteropSink_Void_Ptr(iTestInteropSink* apSink, ni::tPtr v);
  Var __stdcall ForEachObject_OnTestInteropSink_Var_Var(iTestInteropSink* apSink, Var v);
  sVec3f __stdcall ForEachObject_OnTestInteropSink_Vec3f_String(iTestInteropSink* apSink, const ni::achar* aaszStr);
  sVec3f __stdcall ForEachObject_OnTestInteropSink_Vec3f_CObject(iTestInteropSink* apSink);
  sVec3f __stdcall ForEachObject_OnTestInteropSink_Vec3f_Object(iTestInteropSink* apSink);
  iTestInteropObject* __stdcall ForEachObject_OnTestInteropSink_Object_CObject(iTestInteropSink* apSink);
  iTestInteropObject* __stdcall ForEachObject_OnTestInteropSink_Object_Object(iTestInteropSink* apSink);
  iTestInteropObject* __stdcall ForEachObject_OnTestInteropSink_Object_Object2(iTestInteropSink* apSink, ni::tU32 a, ni::tF32 b);
  iTestInteropObject* __stdcall ForEachObject_OnTestInteropSink_Object_ObjectVec3f(iTestInteropSink* apSink, const ni::sVec3f& v);
  //// iTestInterop /////////////////////////////////////

 private:
  niEndClass(cTestInterop);
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __TestInterop_4519356_H__
