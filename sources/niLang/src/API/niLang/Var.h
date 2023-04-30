#pragma once
#ifndef __VAR_H_84696F7E_67DE_46FE_853C_D441EF7275C5__
#define __VAR_H_84696F7E_67DE_46FE_853C_D441EF7275C5__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "Types.h"

#ifdef __cplusplus
#include "IHString.h"
#include "StringBase.h"
#include "Utils/SmartPtr.h"
#include "Utils/WeakPtr.h"
#endif

#ifdef __cplusplus
namespace ni {
struct iHString;
struct iFile;
#endif

/** \addtogroup niLang
 * @{
 */

#define niVarDataSize 16

#if niPragmaPack
#pragma niPackPush(4)
#endif
struct VarData {
  tType mType;
  union {
    tBool mBool;
    tI8   mI8;
    tU8   mU8;
    tI16  mI16;
    tU16  mU16;
    tI32  mI32;
    tU32  mU32;
    tI64  mI64;
    tU64  mU64;
    tInt  mInt;
    tUInt mUInt;
    tIntPtr mIntPtr;
    tUIntPtr  mUIntPtr;
    tUUID mUUID;
    tF32  mF32;
    tF64  mF64;
    tF32  mC3F[3];
    tF32  mC4F[4];
    tU8   mC3UB[3];
    tU8   mC4UB[4];
    tI32  mV2L[2];
    tI32  mV3L[3];
    tI32  mV4L[4];
    tF32  mV2F[2];
    tF32  mV3F[3];
    tF32  mV4F[4];
    tF32  mRectf[4];
    tI32  mRecti[4];
    tF32  mQuat[4];
    tF32* mpFloats;
    const achar* mcpAChar;
    struct iUnknown*  mpIUnknown;
    const struct iUnknown*  mcpIUnknown;
    void*   mpPointer;
#ifdef __cplusplus
    sVec2i   mVec2i;
    sVec3i   mVec3i;
    sVec4i   mVec4i;
    sVec2f   mVec2f;
    sVec3f   mVec3f;
    sVec4f   mVec4f;
    cString*    mpString;
    sMatrixf*   mpMatrixf;
#endif
    tU8   mData[niVarDataSize];
    tU32  mDataU32[niVarDataSize/4];
  } niPacked(4);
} niAligned(4) niPacked(4);
#if !defined __INTELLISENSE__
niCAssert(sizeof(VarData) == 20);
#endif

struct Var : public VarData
{
#ifdef __cplusplus
  bool operator < (const Var& aR) const {
    return VarCompare(this,&aR) < 0;
  }
  bool operator > (const Var& aR) const {
    return VarCompare(this,&aR) > 0;
  }
  bool operator <= (const Var& aR) const {
    return VarCompare(this,&aR) <= 0;
  }
  bool operator >= (const Var& aR) const {
    return VarCompare(this,&aR) >= 0;
  }
  bool operator == (const Var& aR) const {
    return VarCompare(this,&aR) == 0;
  }
  bool operator != (const Var& aR) const {
    return VarCompare(this,&aR) != 0;
  }

  Var() {
    mType = eType_Null;
    mpPointer = NULL;
  }
#if defined niTypeIntIsOtherType
  Var(int n)  {
    mType = eType_Null;
    mpPointer = NULL;
    SetInt(n);
  }
  Var(unsigned int n) {
    mType = eType_Null;
    mpPointer = NULL;
    SetUInt(n);
  }
#endif
  Var(const void* n)  {
    mType = eType_Null;
    mpPointer = NULL;
    SetUIntPtr((tUIntPtr)n);
  }

  Var(tType aType, tU8* apData, tU32 anSize)
  {
    mType = eType_Null;
    Set(aType, apData, anSize);
  }

  void Set(tType aType, tU8* apData,  tU32 anSize)
  {
    VarDestruct(this);
    mType = aType;
    if (apData) {
      niAssert(anSize <= niVarDataSize);
      memcpy(mData,apData,anSize);
    }
  }

#define CPY_VPNEW(CT,T) {                       \
    Set##CT(*aV.mp##CT);                        \
  }

#define C_V_SETGET(T,TN,RT)                                             \
  inline void Set##TN(T anV)  { VarDestruct(this); mType = tType(eType_##TN); m##TN = anV; } \
  inline RT Get##TN() const { niAssert(niType(mType) == eType_##TN && !niFlagTest(mType,eTypeFlags_Pointer)); return m##TN; } \
  inline tBool Is##TN() const { return (niType(mType) == eType_##TN && !niFlagTest(mType,eTypeFlags_Pointer)); }

#define C_V_CPYC(T,TN,RT)                                 \
  Var(T anV)      { mType = eType_Null; Set##TN(anV); }   \
  Var& operator = (T anV) { Set##TN(anV); return *this; } \

#define C_V(T,TN,RT)                            \
  C_V_SETGET(T,TN,RT);                          \
  C_V_CPYC(T,TN,RT);

#define C_V_NC(T,TN,RT)                         \
  C_V_SETGET(T,TN,RT);

#define C_P_SETGET(T,TN)                                                \
  inline void Set##TN##Pointer(T* anV)  { VarDestruct(this); mType = tType(eType_##TN|eTypeFlags_Pointer); mp##TN = anV; } \
  inline T* Get##TN##Pointer() const    { niAssert(niType(mType) == eType_##TN && niFlagTest(mType,eTypeFlags_Pointer) && !niFlagTest(mType,eTypeFlags_Constant)); return mp##TN;  } \
  inline tBool Is##TN##Pointer() const  { return (niType(mType) == eType_##TN && niFlagTest(mType,eTypeFlags_Pointer) && !niFlagTest(mType,eTypeFlags_Constant)); }

#define C_CP_SETGET(T,TN)                                               \
  inline void Set##TN##ConstPointer(const T* anV) { VarDestruct(this); mType = tType(eType_##TN|eTypeFlags_Pointer|eTypeFlags_Constant); mcp##TN = anV; } \
  inline const T* Get##TN##ConstPointer() const { niAssert(niType(mType) == eType_##TN && niFlagTest(mType,eTypeFlags_Pointer)); return mcp##TN;  } \
  inline tBool Is##TN##ConstPointer() const { return (niType(mType) == eType_##TN && niFlagTest(mType,eTypeFlags_Pointer) && niFlagTest(mType,eTypeFlags_Constant)); }

#define C_CP_CPYC(T,TN)                                                 \
  Var(const T* anV) { mType = eType_Null; Set##TN##ConstPointer(anV); } \
  Var& operator = (const T* anV)  { Set##TN##ConstPointer(anV); return *this; }

#define C_P_CPYC(T,TN)                                                  \
  Var(T* anV)         { mType = eType_Null; Set##TN##Pointer(anV); }    \
  Var& operator = (T* anV)    { Set##TN##Pointer(anV); return *this; }

#define C_P(T,TN)                               \
  C_P_SETGET(T,TN);                             \
  C_P_CPYC(T,TN);
#define C_P_NC(T,TN)                            \
  C_P_SETGET(T,TN);

#define C_CP(T,TN)                              \
  C_CP_SETGET(T,TN);                            \
  C_CP_CPYC(T,TN);
#define C_CP_NC(T,TN)                           \
  C_CP_SETGET(T,TN);

#define C_VPNEW_SETGET(T,TN)                                            \
  inline void Set##TN(const T& anV) {                                   \
    VarDestruct(this);                                                       \
    mType = tType(eType_##TN);                                          \
    mpPointer = niMalloc(sizeof(T));                                    \
    new(mpPointer) T(anV);                                              \
  }                                                                     \
  inline T& Get##TN() const {                                           \
    niAssert(niType(mType) == eType_##TN && !niFlagTest(mType,eTypeFlags_Pointer)); \
    return *mp##TN;                                                     \
  }                                                                     \
  inline tBool Is##TN() const {                                         \
    return (niType(mType) == eType_##TN && !niFlagTest(mType,eTypeFlags_Pointer)); \
  }

#define C_VPNEW_CPYC(T,TN)                                        \
  Var(const T& anV) { mType = eType_Null; Set##TN(anV); }         \
  Var& operator = (const T& anV)  { Set##TN(anV); return *this; }

#define C_VPNEW(T,TN)                           \
  C_VPNEW_SETGET(T,TN);                         \
  C_VPNEW_CPYC(T,TN);

#define C_VPNEW_NC(T,TN)                        \
  C_VPNEW_SETGET(T,TN);

#define VAR_ASSERT_NOT_CONVERTIBLE(KIND)           \
  {                                                \
    char bufType[32];                              \
    ni::cString msg = #KIND ": var type not convertible: "; \
    msg += GetTypeString(bufType, this->mType);    \
    niAssertUnreachable(msg.c_str());              \
  }

  void SetVar(const Var& aV)
  {
    if (&aV == this)
      return;
    VarDestruct(this);
    switch (niType(aV.mType)) {
      case eType_Null: {
        mpPointer = NULL;
        break;
      }
      case eType_Matrixf: {
        CPY_VPNEW(Matrixf,sMatrixf);
        break;
      }
      case eType_String: {
        CPY_VPNEW(String,cString);
        break;
      }
      case eType_IUnknown: {
        niAssert((aV.mType)&eTypeFlags_Pointer);
        SetIUnknownPointer(aV.mpIUnknown);
        break;
      }
      default: {
        memcpy(mData,aV.mData,niVarDataSize);
        break;
      }
    }
    mType = aV.mType;
  }

  Var(const Var& aV) {
    mType = eType_Null;
    SetVar(aV);
  }
  Var& operator = (const Var& aV) {
    SetVar(aV);
    return *this;
  }
  ~Var() {
    VarDestruct(this);
  }

 public:
  void SetNull() {
    Set(eType_Null,NULL,0);
  }
  tBool IsNull() const {
    return niType(mType)==eType_Null;
  }
  tBool IsNullPointer() const {
    return IsNull() || !niFlagTest(mType,eTypeFlags_Pointer) || mpIUnknown == NULL;
  }

  tType GetType() const { return mType; }

  C_V(tI8,I8,tI8);
  C_V(tU8,U8,tU8);
  C_V(tI16,I16,tI16);
  C_V(tU16,U16,tU16);
  C_V(tI32,I32,tI32);
  C_V(tU32,U32,tU32);
  C_V(tI64,I64,tI64);
  C_V(tU64,U64,tU64);
  C_V(tUUID,UUID,tUUID);
  C_V(tF32,F32,tF32);
  C_V(tF64,F64,tF64);
#if defined niTypeIntIsOtherType
  C_V_NC(tInt,Int,tInt);
  C_V_NC(tUInt,UInt,tUInt);
#endif
#if defined niTypeIntPtrIsOtherType
  C_V(tIntPtr,IntPtr,tIntPtr);
  C_V(tUIntPtr,UIntPtr,tUIntPtr);
#else
  C_V_NC(tIntPtr,IntPtr,tIntPtr);
  C_V_NC(tUIntPtr,UIntPtr,tUIntPtr);
#endif

  C_CP(achar,AChar);

  C_V(sVec2i,Vec2i,sVec2i);
  C_V(sVec2f,Vec2f,sVec2f);
  C_V(sVec3i,Vec3i,sVec3i);
  C_V(sVec3f,Vec3f,sVec3f);
  C_V(sVec4i,Vec4i,sVec4i);
  C_V(sVec4f,Vec4f,sVec4f);

  C_VPNEW(sMatrixf,Matrixf);
  C_VPNEW(cString,String);
  inline const achar* GetChars() const {
    if (IsString())           return GetString().Chars();
    if (IsACharConstPointer())  return GetACharConstPointer();
    return NULL;
  }

  Var(const tHStringPtr& v) { mType = eType_Null; SetIUnknownPointer(v.ptr());  }
  Var& operator = (const tHStringPtr& v)  { SetIUnknownPointer(v.ptr());  return *this; }

  Var(iHString* anV) {  mType = eType_Null; SetIUnknownPointer(anV);  }
  Var& operator = (iHString* anV) { SetIUnknownPointer(anV);  return *this; }

  Var(const iUnknown* anV) {
    mType = eType_Null;
    SetIUnknownPointer(anV);
  }
  Var& operator = (const iUnknown* anV) {
    SetIUnknownPointer(anV);
    return *this;
  }

  template <typename S>
  Var(const Ptr<S>& aP) {
    mType = eType_Null;
    SetIUnknownPointer(aP.ptr());
  }
  template <typename S>
  Var& operator = (const Ptr<S>& aP) {
    SetIUnknownPointer(aP.ptr());
    return *this;
  }

  template <typename S>
  Var(const astl::non_null<S>& aP) {
    mType = eType_Null;
    SetIUnknownPointer(aP.raw_ptr());
  }
  template <typename S>
  Var& operator = (const astl::non_null<S>& aP) {
    SetIUnknownPointer(aP.raw_ptr());
    return *this;
  }

  template <typename S>
  Var(const Nonnull<S>& aP) {
    mType = eType_Null;
    SetIUnknownPointer(aP.raw_ptr());
  }
  template <typename S>
  Var& operator = (const Nonnull<S>& aP) {
    SetIUnknownPointer(aP.raw_ptr());
    return *this;
  }

  inline void SetIUnknownPointer(iUnknown* anV) {
    if (IsIUnknownPointer() && mpIUnknown == anV)
      return;
    if (anV) {
      anV->AddRef();
    }
    VarDestruct(this);
    mType = tType(eType_IUnknown|eTypeFlags_Pointer);
    mpIUnknown = anV;
  }
  inline void SetIUnknownPointer(const iUnknown* anV) {
    SetIUnknownPointer(const_cast<iUnknown*>(anV));
  }
  inline tBool IsIUnknownPointer() const  {
    return niType(mType) == eType_IUnknown && niFlagTest(mType,eTypeFlags_Pointer);
  }
  inline iUnknown* GetIUnknownPointer() const {
    niAssert(niType(mType) == eType_IUnknown && niFlagTest(mType,eTypeFlags_Pointer));
    return mpIUnknown;
  }
  inline iUnknown* QueryInterface(const tUUID& aIID) const {
    if (niType(mType) != eType_IUnknown || !niFlagTest(mType,eTypeFlags_Pointer) || IsNullPointer())
      return NULL;
    return mpIUnknown->QueryInterface(aIID);
  }
  inline iUnknown* GetRawIUnknownPointerAndSetNull() {
    niAssert(niType(mType) == eType_IUnknown && niFlagTest(mType,eTypeFlags_Pointer));
    iUnknown* rawPtr = mpIUnknown;
    if (!rawPtr) return NULL;
    rawPtr->SetNumRefs(rawPtr->GetNumRefs()+1000);
    this->SetNull();
    rawPtr->SetNumRefs(rawPtr->GetNumRefs()-1000);
    return rawPtr;
  }

  //! Check whether the variant is a pointer type value.
  inline tBool IsPtrValue() const {
    if (niFlagTest(mType,eTypeFlags_Pointer))
      return eTrue;
    switch (niType(mType)) {
      case eType_IntPtr:
        // case eType_UIntPtr: // same as eType_Ptr
      case eType_Ptr:
        return eTrue;
      default:
        return eFalse;
    }
  }
  //! Get the variant's pointer type value.
  inline tIntPtr GetPtrValue() const {
    if (IsPtrValue()) {
      return this->mIntPtr;
    }
    else if (niType(mType) == eType_Null) {
      return 0;
    }
    else {
      VAR_ASSERT_NOT_CONVERTIBLE(GetPtrValue);
      return 0;
    }
  }

  //! Check whether the variant is a number type value.
  inline tBool IsIntValue() const {
    switch (niType(mType)) {
      case eType_I8:
      case eType_U8:
      case eType_I16:
      case eType_U16:
      case eType_I32:
      case eType_U32:
      case eType_I64:
      case eType_U64:
      case eType_F32:
      case eType_F64:
        return eTrue;
      default:
        return eFalse;
    }
  }
  //! Get the variant's number value.
  inline tI64 GetIntValue() const {
    switch (niType(mType)) {
      case eType_I8:  return (tI64)this->mI8;
      case eType_I16: return (tI64)this->mI16;
      case eType_I32: return (tI64)this->mI32;
      case eType_I64: return (tI64)this->mI64;
      case eType_U8:  return (tI64)this->mU8;
      case eType_U16: return (tI64)this->mU16;
      case eType_U32: return (tI64)this->mU32;
      case eType_U64: return (tI64)this->mU64;
      case eType_F32: return (tI64)this->mF32;
      case eType_F64: return (tI64)this->mF64;
      default:
        VAR_ASSERT_NOT_CONVERTIBLE(GetIntValue);
      case eType_Null:
        return 0;
    }
  }

  //! Check whether the variant is a float type value.
  inline tBool IsFloatValue() const {
    switch (niType(mType)) {
      case eType_I8:
      case eType_U8:
      case eType_I16:
      case eType_U16:
      case eType_I32:
      case eType_U32:
      case eType_I64:
      case eType_U64:
      case eType_F32:
      case eType_F64:
        return eTrue;
      default:
        return eFalse;
    }
  }
  //! Get the variant's float value.
  inline tF64 GetFloatValue() const {
    switch (niType(mType)) {
      case eType_I8:  return (tF64)this->mI8;
      case eType_I16: return (tF64)this->mI16;
      case eType_I32: return (tF64)this->mI32;
      case eType_I64: return (tF64)this->mI64;
      case eType_U8:  return (tF64)this->mU8;
      case eType_U16: return (tF64)this->mU16;
      case eType_U32: return (tF64)this->mU32;
      case eType_U64: return (tF64)this->mU64;
      case eType_F32: return (tF64)this->mF32;
      case eType_F64: return (tF64)this->mF64;
      default:
        VAR_ASSERT_NOT_CONVERTIBLE(GetFloatValue);
      case eType_Null:
        return 0;
    }
  }

  //! Check whether the variant is an enum value.
  inline tBool IsEnumValue() const {
    return IsIntValue();
  }
  //! Get the variant's enum value.
  inline tU32 GetEnumValue() const {
    return (tU32)GetIntValue();
  }

  //! Check whether the variant is a vec2f type value.
  inline tBool IsVec2fValue() const {
    switch (niType(mType)) {
      case eType_Vec2f:
      case eType_Vec2i:
        return eTrue;
      default:
        return eFalse;
    }
  }
  //! Check whether the variant is a vec2i type value.
  inline tBool IsVec2iValue() const {
    return IsVec2fValue();
  }
  //! Get the variant's vec2f type value.
  inline sVec2f GetVec2fValue() const {
    switch (niType(mType)) {
      case eType_Vec2f:
        return this->mVec2f;
      case eType_Vec2i:
        return Vec2<tF32>(
            (tF32)this->mVec2i.x,
            (tF32)this->mVec2i.y);
      default:
        VAR_ASSERT_NOT_CONVERTIBLE(GetVec2fValue);
      case eType_Null:
        return sVec2f::Zero();
    }
  }
  //! Get the variant's vec2i type value.
  inline sVec2i GetVec2iValue() const {
    switch (niType(mType)) {
      case eType_Vec2i:
        return this->mVec2i;
      case eType_Vec2f:
        return Vec2<tI32>(
            (tI32)this->mVec2f.x,
            (tI32)this->mVec2f.y);
      default:
        VAR_ASSERT_NOT_CONVERTIBLE(GetVec2iValue);
      case eType_Null:
        return sVec2i::Zero();
    }
  }

  //! Check whether the variant is a vec3f type value.
  inline tBool IsVec3fValue() const {
    switch (niType(mType)) {
      case eType_Vec3f:
      case eType_Vec3i:
        return eTrue;
      default:
        return eFalse;
    }
  }
  //! Check whether the variant is a vec3i type value.
  inline tBool IsVec3iValue() const {
    return IsVec3fValue();
  }
  //! Get the variant's vec3f type value.
  inline sVec3f GetVec3fValue() const {
    switch (niType(mType)) {
      case eType_Vec3f:
        return this->mVec3f;
      case eType_Vec3i:
        return Vec3<tF32>(
            (tF32)this->mVec3i.x,
            (tF32)this->mVec3i.y,
            (tF32)this->mVec3i.z);
      default:
        VAR_ASSERT_NOT_CONVERTIBLE(GetVec3fValue);
      case eType_Null:
        return sVec3f::Zero();
    }
  }
  //! Get the variant's vec3i type value.
  inline sVec3i GetVec3iValue() const {
    switch (niType(mType)) {
      case eType_Vec3i:
        return this->mVec3i;
      case eType_Vec3f:
        return Vec3<tI32>(
            (tI32)this->mVec3f.x,
            (tI32)this->mVec3f.y,
            (tI32)this->mVec3f.z);
      default:
        VAR_ASSERT_NOT_CONVERTIBLE(GetVec3iValue);
      case eType_Null:
        return sVec3i::Zero();
    }
  }

  //! Check whether the variant is a vec4f type value.
  inline tBool IsVec4fValue() const {
    switch (niType(mType)) {
      case eType_Vec4f:
      case eType_Vec4i:
        return eTrue;
      default:
        return eFalse;
    }
  }
  //! Check whether the variant is a vec4i type value.
  inline tBool IsVec4iValue() const {
    return IsVec4fValue();
  }
  //! Get the variant's vec4f type value.
  inline sVec4f GetVec4fValue() const {
    switch (niType(mType)) {
      case eType_Vec4f:
        return this->mVec4f;
      case eType_Vec4i:
        return Vec4<tF32>(
            (tF32)this->mVec4i.x,
            (tF32)this->mVec4i.y,
            (tF32)this->mVec4i.z,
            (tF32)this->mVec4i.w);
      default:
        VAR_ASSERT_NOT_CONVERTIBLE(GetVec4fValue);
      case eType_Null:
        return sVec4f::Zero();
    }
  }
  //! Get the variant's vec4i type value.
  inline sVec4i GetVec4iValue() const {
    switch (niType(mType)) {
      case eType_Vec4i:
        return this->mVec4i;
      case eType_Vec4f:
        return Vec4<tI32>(
            (tI32)this->mVec4f.x,
            (tI32)this->mVec4f.y,
            (tI32)this->mVec4f.z,
            (tI32)this->mVec4f.w);
      default:
        VAR_ASSERT_NOT_CONVERTIBLE(GetVec4iValue);
      case eType_Null:
        return sVec4i::Zero();
    }
  }

  //! Check whether the variant is a bool type value.
  inline tBool IsBoolValue() const {
    return IsIntValue();
  }
  //! Get the variant's bool type value. Returns abDefault if the variant is not a boolean type.
  inline tBool GetBoolValue(tBool abDefault = eFalse) const {
  	if (!IsIntValue()) {
  	  return abDefault;
  	}
  	return !!GetIntValue();
  }

#undef CPY_VPNEW
#undef C_
#undef C_V
#undef C_P
#undef C_CP
#undef C_VPNEW

 private:
  // Forbidden casts
  Var(const gchar*) {
  }
  Var(const xchar*) {
  }

#endif // __cplusplus
} niAligned(4) niPacked(4);
#if !defined __INTELLISENSE__
niCAssert(sizeof(Var) == 20);
#endif
niCAssert(sizeof(cString) <= (sizeof(VarData)-sizeof(tType)));
niCAssert((offsetof(Var,mVec3f)&0x3) == 0);

#if niPragmaPack
#pragma niPackPop()
#endif

#ifdef __cplusplus
///////////////////////////////////////////////
static inline tBool VarIsStringType(tType aDestType) {
  tType dstType = niType(aDestType);
  return
      dstType == eType_AChar ||
      dstType == eType_String;
}

///////////////////////////////////////////////
static inline tBool VarIsIntType(tType aDestType) {
  tType dstType = niType(aDestType);
  return dstType == eType_I8 ||
      dstType == eType_U8 ||
      dstType == eType_I16 ||
      dstType == eType_U16 ||
      dstType == eType_I32 ||
      dstType == eType_U32 ||
      dstType == eType_I64 ||
      dstType == eType_U64;
}

///////////////////////////////////////////////
static inline tBool VarIsFloatType(tType aDestType) {
  tType dstType = niType(aDestType);
  return dstType == eType_F32 ||
      dstType == eType_F64;
}

niExportFunc(tBool) VarIsString(const Var& aVar);
niExportFunc(tBool) VarConvertToInt(Var& aVar, tI64 aV, tType aDestType);
niExportFunc(tBool) VarConvertToFloat(Var& aVar, tF64 aV, tType aDestType);
niExportFunc(tBool) VarConvertType(Var& aVar, tType aDestType);
niExportFunc(const achar*) VarGetCharsOrNull(const Var& aVar);
niExportFuncCPP(tHStringPtr) VarGetHString(const Var& aVar);
niExportFunc(tBool) VarSerializeWrite(iFile* apFile, const Var& aVar);
niExportFunc(tBool) VarSerializeRead(iFile* apFile, Var& aVar);
niExportFunc(tBool) VarSerializeWriteBitsPacked(iFile* apFile, const Var& aVar);
niExportFunc(tBool) VarSerializeReadBitsPacked(iFile* apFile, Var& aVar);
niExportFunc(tBool) VarSerializeWriteRaw(iFile* apFile, const Var& aVar);
niExportFunc(tBool) VarSerializeReadRaw(iFile* apFile, Var& aVar);
niExportFunc(void)  VarToString(cString& str, const Var& aVar);

///////////////////////////////////////////////
static inline cString VarGetString(const Var& aVar) {
  return VarGetCharsOrNull(aVar);
}

///////////////////////////////////////////////
static inline const achar* VarGetChars(const Var& aVar) {
  const achar* p = VarGetCharsOrNull(aVar);
  return p ? p : AZEROSTR;
}

///////////////////////////////////////////////
static inline cString& VarGetConvertedString(cString& aOut, const Var& aVar) {
  const achar* p = VarGetCharsOrNull(aVar);
  if (p) {
    aOut = p;
  }
  else {
    Var v = aVar;
    VarConvertType(v,eType_String);
    aOut = v.GetString();
  }
  return aOut;
}

///////////////////////////////////////////////
template <typename T>
inline T* VarQueryInterface(const Var& aVar)
{
  if (niType(aVar.mType) != eType_IUnknown || niFlagIsNot(aVar.mType,eTypeFlags_Pointer))
    return NULL;
  return QueryInterface<T>(const_cast<iUnknown*>(aVar.mpIUnknown));
}

///////////////////////////////////////////////
static inline Var VarWeakPtr(iUnknown* apObject) {
  if (!apObject) {
    return niVarNull;
  }
  return Var(ni_object_get_weak_ptr(apObject));
}
#endif

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
#ifdef __cplusplus
}
#endif

#endif // __VAR_H_84696F7E_67DE_46FE_853C_D441EF7275C5__
