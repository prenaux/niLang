#ifndef __SCRIPTOBJECT_40560020_H__
#define __SCRIPTOBJECT_40560020_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "sqobject.h"
// #define SCRIPT_OBJECT_DEBUGID

//////////////////////////////////////////////////////////////////////////////////////////////
// cScriptObject declaration.

//! Script object implementation.
class cScriptObject : public cIUnknownImpl<iScriptObject>
{
  niBeginClass(cScriptObject);

 public:
  //! Constructor.
  cScriptObject(cScriptVM* apVM, int idx, int numpop, tBool abHoldRef = eTrue);
  //! Destructor.
  ~cScriptObject();

  //! Zeros all the class members.
  void ZeroMembers();

  //! Sanity check.
  tBool __stdcall IsOK() const;
  //! Invalidate the object.
  void __stdcall Invalidate();

  //// iScriptObject ////////////////////////////
  iScriptVM* __stdcall GetVM() const;
  eScriptObjectType __stdcall GetType() const;
  tBool __stdcall Push();
  iScriptObject* __stdcall Clone(tBool abHoldRef) const;
  tScriptObjectPtrCVec* __stdcall EnumObjects() const;

  tBool __stdcall Get(int idx, int numpop);

  tI32 __stdcall GetInt() const;
  tF64 __stdcall GetFloat() const;
  const achar* __stdcall GetString() const;
  iHString* __stdcall GetHString() const;
  iUnknown* __stdcall GetIUnknown() const;
  iUnknown* __stdcall GetIUnknownEx(const tUUID& aIID) const;
  sVec2f __stdcall GetVec2() const;
  sVec3f __stdcall GetVec3() const;
  sVec4f __stdcall GetVec4() const;
  sMatrixf __stdcall GetMatrix() const;
  const tUUID& __stdcall GetUUID() const;

  tU32 __stdcall GetNumParameters() const;
  tU32 __stdcall GetNumFreeVars() const;
  tU32 __stdcall GetNumCallParameters() const;
  //// iScriptObject ////////////////////////////

  inline tBool HoldRef() const { return mbHoldRef; }
  inline HSQOBJECT& GetSQObject() { return mObject; }

  inline tBool __stdcall Is(eScriptObjectType aType) const { return GetType() == aType; }
  inline tBool __stdcall IsUnknown() const { return Is(eScriptObjectType_Unknown); }
  inline tBool __stdcall IsFunction() const { return Is(eScriptObjectType_Function); }
  inline tBool __stdcall IsTable() const { return Is(eScriptObjectType_Table); }
  inline tBool __stdcall IsNull() const { return Is(eScriptObjectType_Null); }
  inline tBool __stdcall IsInt() const { return Is(eScriptObjectType_Int); }
  inline tBool __stdcall IsFloat() const { return Is(eScriptObjectType_Float); }
  inline tBool __stdcall IsString() const { return Is(eScriptObjectType_String); }
  inline tBool __stdcall IsUserData() const { return Is(eScriptObjectType_UserData); }
  inline tBool __stdcall IsVM() const { return Is(eScriptObjectType_VM); }
  inline tBool __stdcall IsIUnknown() const { return Is(eScriptObjectType_IUnknown); }
  inline tBool __stdcall IsVec2() const { return Is(eScriptObjectType_Vec2); }
  inline tBool __stdcall IsVec3() const { return Is(eScriptObjectType_Vec3); }
  inline tBool __stdcall IsVec4() const { return Is(eScriptObjectType_Vec4); }
  inline tBool __stdcall IsMatrix() const { return Is(eScriptObjectType_Matrix); }
  inline tBool __stdcall IsUUID() const { return Is(eScriptObjectType_UUID); }

 private:
#ifdef SCRIPTOBJECT_INVALIDATEPOLICY
  cScriptVM*  mpVM;
#else
  Ptr<cScriptVM>  mpVM;
#endif
  eScriptObjectType mType;
  HSQOBJECT mObject;
  tBool   mbHoldRef;
  tU32    mnNumParameters;
  tU32    mnNumFreeVars;

#ifdef SCRIPT_OBJECT_DEBUGID
 public:
  tU32    mnDebugID;
#endif

  niEndClass(cScriptObject);
};


/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __SCRIPTOBJECT_40560020_H__
