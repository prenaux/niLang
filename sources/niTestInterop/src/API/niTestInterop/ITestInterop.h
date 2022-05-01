#ifndef __ITESTINTEROP_18501792_H__
#define __ITESTINTEROP_18501792_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

namespace ni {
/** \addtogroup niTestInterop
 * @{
 */

//! Test interop enum.
#if niMinFeatures(1)
enum eTestInterop
{
  eTestInterop_A = 0,
  eTestInterop_B = 1,
#if niMinFeatures(30)
  eTestInterop_C = 2,
#endif
#if niMinFeatures(200)
  eTestInterop_NeverThere = 3,
#endif
};
#endif

enum eTestInteropFlags
{
  //! This is the flag A.
  //! \remark This test remarks...
  eTestInteropFlags_A = niBit(0),
  //! This is the flag B.
  //! \remark This test remarks...
  eTestInteropFlags_B = niBit(1),
  //! This is the flag C.
  //! \remark This test remarks...
  eTestInteropFlags_C = niBit(2),
  //! Combines all flags.
  eTestInteropFlags_All = eTestInteropFlags_A|eTestInteropFlags_B|eTestInteropFlags_C,
  //! \internal
  eTestInteropFlags_ForceDWORD = 0xFFFFFFFF
};

typedef tU32 tTestInteropFlags;

//! Test interop struct.
struct sTestInterop
{
  tU32 mnInt;
  tF32 mfFloat;
  sVec3f mvVec3;
};

//! Test interop object
struct iTestInteropObject : public iUnknown
{
  niDeclareInterfaceUUID(iTestInteropObject,0x488da121,0x4087,0x4d75,0x95,0x7a,0x74,0xa8,0xc6,0x07,0x07,0x03);
  //! Set the number of items for indexed properties.
  //! {Property}
  //! \remark Default is 100.
  virtual void __stdcall SetNumItems(tSize aNumItems) = 0;
  //! Get the number of items for indexed properties.
  //! {Property}
  virtual tSize __stdcall GetNumItems() const = 0;

  //! Set values from the test interop structure.
  virtual void __stdcall SetTestInterop(sTestInterop* apStruct) = 0;
  //! Get values from the test interop structure.
  virtual sTestInterop* __stdcall GetTestInterop() const = 0;
  //! Set values from the test interop structure.
  virtual void __stdcall SetTestInteropConst(const sTestInterop* apStruct) = 0;
  //! Get values from the test interop structure.
  virtual const sTestInterop* __stdcall GetTestInteropConst() const = 0;

  //! Set values from the test interop structure reference.
  virtual void __stdcall SetTestInteropRef(sTestInterop& aStruct) = 0;
  //! Get values from the test interop structure.
  virtual sTestInterop& __stdcall GetTestInteropRef() const = 0;
  //! Set values from the test interop structure reference.
  virtual void __stdcall SetTestInteropConstRef(const sTestInterop& aStruct) = 0;
  //! Get values from the test interop structure.
  virtual const sTestInterop& __stdcall GetTestInteropConstRef() const = 0;

  //! Set an enumeration value.
  //! {Property}{Serialize1}
  virtual void __stdcall SetEnum(eTestInterop aV) = 0;
  //! Get an enumeration value.
  //! {Property}{Serialize1}
  virtual eTestInterop __stdcall GetEnum() const = 0;

  //! Set flags value.
  //! {Property}{Serialize1}
  virtual void __stdcall SetFlags(tTestInteropFlags aV) = 0;
  //! Get flags value.
  //! {Property}{Serialize1}
  virtual tTestInteropFlags __stdcall GetFlags() const = 0;

  //! Set a IntPtr value.
  //! {Property}{Serialize1}
  virtual void __stdcall SetIntPtr(tIntPtr aV) = 0;
  //! Get a IntPtr value.
  //! {Property}{Serialize1}
  virtual tIntPtr __stdcall GetIntPtr() const = 0;
  //! Set a UIntPtr value.
  //! {Property}{Serialize1}
  virtual void __stdcall SetUIntPtr(tIntPtr aV) = 0;
  //! Get a UIntPtr value.
  //! {Property}{Serialize1}
  virtual tIntPtr __stdcall GetUIntPtr() const = 0;

  //! Set a I8 value.
  //! {Property}{Serialize1}
  virtual void __stdcall SetI8(tI8 aV) = 0;
  //! Get a I8 value.
  //! {Property}{Serialize1}
  virtual tI8 __stdcall GetI8() const = 0;
  //! Set a U8 value.
  //! {Property}{Serialize1}
  virtual void __stdcall SetU8(tU8 aV) = 0;
  //! Get a U8 value.
  //! {Property}{Serialize1}
  virtual tU8 __stdcall GetU8() const = 0;
  //! Set a I16 value.
  //! {Property}{Serialize1}
  virtual void __stdcall SetI16(tI16 aV) = 0;
  //! Get a I16 value.
  //! {Property}{Serialize1}
  virtual tI16 __stdcall GetI16() const = 0;
  //! Set a U16 value.
  //! {Property}{Serialize1}
  virtual void __stdcall SetU16(tU16 aV) = 0;
  //! Get a U16 value.
  //! {Property}{Serialize1}
  virtual tU16 __stdcall GetU16() const = 0;
  //! Set a I32 value.
  //! {Property}{Serialize1}
  virtual void __stdcall SetI32(tI32 aV) = 0;
  //! Get a I32 value.
  //! {Property}{Serialize1}
  virtual tI32 __stdcall GetI32() const = 0;
  //! Set a U32 value.
  //! {Property}{Serialize1}
  virtual void __stdcall SetU32(tU32 aV) = 0;
  //! Get a U32 value.
  //! {Property}{Serialize1}
  virtual tU32 __stdcall GetU32() const = 0;
  //! Set a I64 value.
  //! {Property}{Serialize1}
  virtual void __stdcall SetI64(tI64 aV) = 0;
  //! Get a I64 value.
  //! {Property}{Serialize1}
  virtual tI64 __stdcall GetI64() const = 0;
  //! Set a U64 value.
  //! {Property}{Serialize1}
  virtual void __stdcall SetU64(tU64 aV) = 0;
  //! Get a U64 value.
  //! {Property}{Serialize1}
  virtual tU64 __stdcall GetU64() const = 0;
  //! Set a F32 value.
  //! {Property}{Serialize1}
  virtual void __stdcall SetF32(tF32 aV) = 0;
  //! Get a F32 value.
  //! {Property}{Serialize1}
  virtual tF32 __stdcall GetF32() const = 0;
  //! Set a F64 value.
  //! {Property}{Serialize1}
  virtual void __stdcall SetF64(tF64 aV) = 0;
  //! Get a F64 value.
  //! {Property}{Serialize1}
  virtual tF64 __stdcall GetF64() const = 0;

  //! Set a Variant value.
  //! {Property}
  virtual void __stdcall SetVar(const Var& aV) = 0;
  //! Get a Variant value.
  //! {Property}
  virtual Var __stdcall GetVar() const = 0;

  //! Set a Vec2i value.
  //! {Property}{Serialize1}
  virtual void __stdcall SetVec2l(const sVec2i& aV) = 0;
  //! Get a Vec2i value.
  //! {Property}{Serialize1}
  virtual sVec2i __stdcall GetVec2l() const = 0;
  //! Set a Vec2f value.
  //! {Property}{Serialize1}
  virtual void __stdcall SetVec2f(const sVec2f& aV) = 0;
  //! Get a Vec2f value.
  //! {Property}{Serialize1}
  virtual sVec2f __stdcall GetVec2f() const = 0;
  //! Set a Vec3i value.
  //! {Property}{Serialize1}
  virtual void __stdcall SetVec3l(const sVec3i& aV) = 0;
  //! Get a Vec3i value.
  //! {Property}{Serialize1}
  virtual sVec3i __stdcall GetVec3l() const = 0;
  //! Set a Vec3f value.
  //! {Property}{Serialize1}
  virtual void __stdcall SetVec3f(const sVec3f& aV) = 0;
  //! Get a Vec3f value.
  //! {Property}{Serialize1}
  virtual sVec3f __stdcall GetVec3f() const = 0;
  //! Set a Vec4i value.
  //! {Property}{Serialize1}
  virtual void __stdcall SetVec4l(const sVec4i& aV) = 0;
  //! Get a Vec4i value.
  //! {Property}{Serialize1}
  virtual sVec4i __stdcall GetVec4l() const = 0;
  //! Set a Vec4f value.
  //! {Property}{Serialize1}
  virtual void __stdcall SetVec4f(const sVec4f& aV) = 0;
  //! Get a Vec4f value.
  //! {Property}{Serialize1}
  virtual sVec4f __stdcall GetVec4f() const = 0;
  //! Set a Matrixf value.
  //! {Property}{Serialize1}
  virtual void __stdcall SetMatrixf(const sMatrixf& aV) = 0;
  //! Get a Matrixf value.
  //! {Property}{Serialize1}
  virtual sMatrixf __stdcall GetMatrixf() const = 0;
  //! Set the matrix
  virtual ni::tBool __stdcall SetMatrixfValues(
      ni::tF32 a00,ni::tF32 a01,ni::tF32 a02,ni::tF32 a03,
      ni::tF32 a10,ni::tF32 a11,ni::tF32 a12,ni::tF32 a13,
      ni::tF32 a20,ni::tF32 a21,ni::tF32 a22,ni::tF32 a23,
      ni::tF32 a30,ni::tF32 a31,ni::tF32 a32,ni::tF32 a33) = 0;

  //! Set a achar stringz value.
  //! {Property}{Serialize1}
  virtual void __stdcall SetACharStrZ(const achar* aV) = 0;
  //! Get a achar stringz value.
  //! {Property}{Serialize1}
  virtual const achar* __stdcall GetACharStrZ() const = 0;

  //! Set a string value.
  //! {Property}{Serialize1}
  virtual void __stdcall SetString(cString aStr) = 0;
  //! Get a string value.
  //! {Property}{Serialize1}
  virtual cString __stdcall GetString() const = 0;
  //! Set a string const ref value.
  //! {Property}{Serialize1}
  virtual void __stdcall SetStringCRef(const cString& aStr) = 0;
  //! Get a string const ref value.
  //! {Property}{Serialize1}
  virtual const cString& __stdcall GetStringCRef() const = 0;

  //! Set a string value.
  //! {Property}{Serialize1}
  virtual void __stdcall SetHString(iHString* ahspStr) = 0;
  //! Get a string value.
  //! {Property}{Serialize1}
  virtual iHString* __stdcall GetHString() const = 0;

  //! Set an indexed U32 value.
  //! {Property}
  virtual void __stdcall SetIndexedU32(tSize aIndex, tU32 aV) = 0;
  //! Get an indexed U32 value.
  //! {Property}
  virtual tU32 __stdcall GetIndexedU32(tSize aIndex) const = 0;
  //! Set a 'mapped' U32 value.
  //! {Property}
  virtual void __stdcall SetMappedU32(const achar* aaszKey, tU32 aV) = 0;
  //! Get a 'mapped' U32 value.
  //! {Property}
  virtual tU32 __stdcall GetMappedU32(const achar* aaszKey) const = 0;

  //! Set a Ptr value.
  //! {Property}
  virtual void __stdcall SetPtr(tPtr aV) = 0;
  //! Get a Ptr value.
  //! {Property}
  virtual tPtr __stdcall GetPtr() const = 0;

  //! Write to the internal raw buffer.
  //! \remark The buffer is 4K in size.
  virtual ni::tBool __stdcall WriteRawBuffer(const tPtr aData, tSize anSize) = 0;
  //! Read from the internal raw buffer.
  //! \remark The buffer is 4K in size.
  virtual ni::tBool __stdcall ReadRawBuffer(tPtr aData, tSize anSize) = 0;

  //! Write to the internal raw buffer.
  //! \remark The buffer is 4K in size.
  virtual ni::tBool __stdcall WriteRawBufferVoid(const void* apIn, tSize anSize) = 0;
  //! Read from the internal raw buffer.
  //! \remark The buffer is 4K in size.
  virtual ni::tBool __stdcall ReadRawBufferVoid(void* apOut, tSize anSize) = 0;

  //! Write to the internal raw buffer.
  //! \remark The buffer is 4K in size.
  virtual ni::tBool __stdcall WriteRawBufferU32(const tU32* apIn, tSize anSize) = 0;
  //! Read from the internal raw buffer.
  //! \remark The buffer is 4K in size.
  virtual ni::tBool __stdcall ReadRawBufferU32(tU32* apOut, tSize anSize) = 0;

  //! Set an indexed IUnknown value.
  //! {Property}
  virtual void __stdcall SetIndexedIUnknown(tSize aIndex, ni::iUnknown* aV) = 0;
  //! Get an indexed IUnknown value.
  //! {Property}
  virtual ni::iUnknown* __stdcall GetIndexedIUnknown(tSize aIndex) const = 0;
  //! Set a 'mapped' IUnknown value.
  //! {Property}
  virtual void __stdcall SetMappedIUnknown(const achar* aaszKey, ni::iUnknown* aV) = 0;
  //! Get a 'mapped' IUnknown value.
  //! {Property}
  virtual ni::iUnknown* __stdcall GetMappedIUnknown(const achar* aaszKey) const = 0;

  //! Set a TestInteropObject value.
  //! {Property}
  virtual void __stdcall SetTestInteropObject(iTestInteropObject* aV) = 0;
  //! Get a TestInteropObject value.
  //! {Property}
  virtual iTestInteropObject* __stdcall GetTestInteropObject() const = 0;
  //! Set an indexed TestInteropObject value.
  //! {Property}
  virtual void __stdcall SetIndexedTestInteropObject(tSize aIndex, iTestInteropObject* aV) = 0;
  //! Get an indexed TestInteropObject value.
  //! {Property}
  virtual iTestInteropObject* __stdcall GetIndexedTestInteropObject(tSize aIndex) const = 0;
  //! Set a 'mapped' TestInteropObject value.
  //! {Property}
  virtual void __stdcall SetMappedTestInteropObject(const achar* aaszKey, iTestInteropObject* aV) = 0;
  //! Get a 'mapped' TestInteropObject value.
  //! {Property}
  virtual iTestInteropObject* __stdcall GetMappedTestInteropObject(const achar* aaszKey) const = 0;

  //! Set the object's U32Vec
  //! {Property}
  virtual void __stdcall SetU32Vec(tU32CVec* apVec) = 0;
  //! Get the object's U32Vec
  //! {Property}
  virtual tU32CVec* __stdcall GetU32Vec() const = 0;

  //! Set the object's StringMap
  //! {Property}
  virtual void __stdcall SetStringMap(tStringCMap* apMap) = 0;
  //! Get the object's StringMap
  //! {Property}
  virtual tStringCMap* __stdcall GetStringMap() const = 0;

  //! Function with 32 parameters
  virtual ni::tBool __stdcall Func32(
      ni::tU32 a00,ni::tU32 a01,ni::tU32 a02,ni::tU32 a03,
      ni::tU32 a10,ni::tU32 a11,ni::tU32 a12,ni::tU32 a13,
      ni::tU32 a20,ni::tU32 a21,ni::tU32 a22,ni::tU32 a23,
      ni::tU32 a30,ni::tU32 a31,ni::tU32 a32,ni::tU32 a33,
      ni::tU32 a40,ni::tU32 a41,ni::tU32 a42,ni::tU32 a43,
      ni::tU32 a50,ni::tU32 a51,ni::tU32 a52,ni::tU32 a53,
      ni::tU32 a60,ni::tU32 a61,ni::tU32 a62,ni::tU32 a63,
      ni::tU32 a70,ni::tU32 a71,ni::tU32 a72,ni::tU32 a73) = 0;
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
      ni::tU32 firstOops) = 0;
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
      ni::tU32 a90,ni::tU32 a91,ni::tU32 a92,ni::tU32 a93) = 0;
};

//! Test interop sink.
//! {DispatchWrapper}
struct iTestInteropSink : public iUnknown
{
  niDeclareInterfaceUUID(iTestInteropSink,0xecd3e0d2,0x95af,0x4f70,0xa5,0xd0,0xca,0x8c,0xf2,0x2c,0x0d,0x89);

  virtual void __stdcall OnTestInteropSink_Void() = 0;

  //! {Optional}
  virtual void __stdcall OnTestInteropSink_Void_I32(ni::tI32 v) = 0;
  //! {Optional}
  virtual void __stdcall OnTestInteropSink_Void_I64(ni::tI64 v) = 0;
  //! {Optional}
  virtual void __stdcall OnTestInteropSink_Void_Ptr(ni::tIntPtr v) = 0;
  //! {Optional}
  virtual ni::Var __stdcall OnTestInteropSink_Var_Var(const ni::Var& v) = 0;
  //! {Optional}
  virtual sVec3f __stdcall OnTestInteropSink_Vec3f_String(const ni::achar* aaszStr) = 0;
  //! {Optional}
  virtual sVec3f __stdcall OnTestInteropSink_Vec3f_CObject(const iTestInteropObject* apObj) = 0;
  //! {Optional}
  virtual sVec3f __stdcall OnTestInteropSink_Vec3f_Object(iTestInteropObject* apObj) = 0;
  //! {Optional}
  virtual iTestInteropObject* __stdcall OnTestInteropSink_Object_CObject(const iTestInteropObject* apObj) = 0;
  //! {Optional}
  virtual iTestInteropObject* __stdcall OnTestInteropSink_Object_Object(iTestInteropObject* apObj) = 0;
  //! {Optional}
  virtual iTestInteropObject* __stdcall OnTestInteropSink_Object_Object2(ni::tU32 a, ni::tF32 b, iTestInteropObject* apObj) = 0;
  //! {Optional}
  virtual iTestInteropObject* __stdcall OnTestInteropSink_Object_ObjectVec3f(iTestInteropObject* apObj, const ni::sVec3f& c) = 0;
};

//! Test interop foreach interface.
struct iTestInteropForEach : public iUnknown
{
  niDeclareInterfaceUUID(iTestInteropForEach,0xef4eebbd,0x8394,0x488e,0xa7,0xf9,0xbe,0x9c,0x06,0xa3,0x6e,0x2d);


  //! Call the sink for each object, adds the values returned, if any.
  virtual void __stdcall ForEachObject_OnTestInteropSink_Void(iTestInteropSink* apSink) = 0;
  //! Call the sink for each object, adds the values returned, if any.
  virtual void __stdcall ForEachObject_OnTestInteropSink_Void_I32(iTestInteropSink* apSink, ni::tI32 v) = 0;
  //! Call the sink for each object, adds the values returned, if any.
  virtual void __stdcall ForEachObject_OnTestInteropSink_Void_I64(iTestInteropSink* apSink, ni::tI64 v) = 0;
  //! Call the sink for each object, adds the values returned, if any.
  virtual void __stdcall ForEachObject_OnTestInteropSink_Void_Ptr(iTestInteropSink* apSink, ni::tPtr v) = 0;
  //! Call the sink for each object, adds the values returned, if any.
  virtual Var __stdcall ForEachObject_OnTestInteropSink_Var_Var(iTestInteropSink* apSink, Var v) = 0;
  //! Call the sink for each object, adds the values returned, if any.
  virtual sVec3f __stdcall ForEachObject_OnTestInteropSink_Vec3f_String(iTestInteropSink* apSink, const ni::achar* aaszStr) = 0;
  //! Call the sink for each object, adds the values returned, if any.
  virtual sVec3f __stdcall ForEachObject_OnTestInteropSink_Vec3f_CObject(iTestInteropSink* apSink) = 0;
  //! Call the sink for each object, adds the values returned, if any.
  virtual sVec3f __stdcall ForEachObject_OnTestInteropSink_Vec3f_Object(iTestInteropSink* apSink) = 0;
  //! Call the sink for each object, adds the values returned, if any.
  virtual iTestInteropObject* __stdcall ForEachObject_OnTestInteropSink_Object_CObject(iTestInteropSink* apSink) = 0;
  //! Call the sink for each object, adds the values returned, if any.
  virtual iTestInteropObject* __stdcall ForEachObject_OnTestInteropSink_Object_Object(iTestInteropSink* apSink) = 0;
  //! Call the sink for each object, adds the values returned, if any.
  virtual iTestInteropObject* __stdcall ForEachObject_OnTestInteropSink_Object_Object2(iTestInteropSink* apSink, ni::tU32 a, ni::tF32 b) = 0;
  //! Call the sink for each object, adds the values returned, if any.
  virtual iTestInteropObject* __stdcall ForEachObject_OnTestInteropSink_Object_ObjectVec3f(iTestInteropSink* apSink, const ni::sVec3f& v) = 0;
};

//! Test interop interface.
struct iTestInterop : public iTestInteropForEach
{
  niDeclareInterfaceUUID(iTestInterop,0xd9724bed,0xe665,0x4aee,0x87,0x6c,0xbe,0xc9,0xe1,0x15,0xba,0x96);

#if niMinFeatures(200)
  virtual void __stdcall FeatureSetNeverThere() = 0;
#endif
#if niMinFeatures(30)
  virtual void __stdcall FeatureSetFull() = 0;
#endif
#if niMinFeatures(20)
  virtual void __stdcall FeatureSetEmbedded() = 0;
#endif
  virtual void __stdcall FeatureSetLight() = 0;

  //! Emulate a crash.
  //! \see CrashRpt.h
  virtual tInt __stdcall GenerateCrash(ni::tU32 anExceptionType) = 0;

  //! Get the number of test interop objects created.
  //! {Property}
  virtual tSize __stdcall GetNumTestInteropObjects() const = 0;
  //! Get the test interop object at the specified index.
  //! {Property}
  virtual iTestInteropObject* __stdcall GetTestInteropObject(ni::tSize anIndex) const = 0;
  //! Create a test interop object.
  //! \remark This is here mostly to test that IDLGen prints a warning on raw pointer factory functions.
  virtual iTestInteropObject* __stdcall CreateTestInteropObject(ni::tU32 anNumItems) = 0;
  //! Create a test interop object.
  virtual Ptr<iTestInteropObject> __stdcall CreateTestInteropObjectPtr(ni::tU32 anNumItems) = 0;
};

niExportFunc(ni::iUnknown*) New_niTestInterop_TestInterop(const ni::Var&,const ni::Var&);

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __ITESTINTEROP_18501792_H__
