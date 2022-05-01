#ifndef __ISCRIPTOBJECT_3733125_H__
#define __ISCRIPTOBJECT_3733125_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/Utils/CollectionImpl.h>

#if niMinFeatures(15)

namespace ni {

//////////////////////////////////////////////////////////////////////////////////////////////
// Forward declaration
struct iScriptObject;
struct iScriptVM;

/** \addtogroup niLang
 * @{
 */

//////////////////////////////////////////////////////////////////////////////////////////////
//! Script object type.
enum eScriptObjectType
{
  //! Unknown object type.
  eScriptObjectType_Unknown = 0,
  //! Function object type.
  eScriptObjectType_Function = 1,
  //! Table object type.
  eScriptObjectType_Table = 2,
  //! Null object type.
  eScriptObjectType_Null = 3,
  //! Integer object type.
  eScriptObjectType_Int = 4,
  //! Float object type.
  eScriptObjectType_Float = 5,
  //! String object type.
  eScriptObjectType_String = 6,
  //! UserData object type.
  eScriptObjectType_UserData = 7,
  //! Array object type.
  eScriptObjectType_Array = 8,
  //! VM object type.
  eScriptObjectType_VM = 9,
  //! iUnknown object type.
  eScriptObjectType_IUnknown = 0x0400,
  //! Vec2 object type.
  eScriptObjectType_Vec2 = 11,
  //! Vec3 object type.
  eScriptObjectType_Vec3 = 12,
  //! Vec4 object type.
  eScriptObjectType_Vec4 = 13,
  //! Matrix object type.
  eScriptObjectType_Matrix = 14,
  //! UUID object type.
  eScriptObjectType_UUID = 15,
  //! \internal
  eScriptObjectType_Last = 16,
  //! \internal
  eScriptObjectType_ForceDWORD = 0xFFFFFFFF
};

//! Script object smart pointer vector.
typedef tInterfaceCVec<iScriptObject> tScriptObjectPtrCVec;

//! Script object reference interface.
struct iScriptObject : public iUnknown
{
  niDeclareInterfaceUUID(iScriptObject,0x8359934f,0xec0d,0x4bb3,0x81,0x68,0xcd,0x78,0x2c,0x3e,0x5f,0xad)

  //! Parent script VM.
  //! {Property}
  virtual iScriptVM* __stdcall GetVM() const = 0;
  //! Get the script object's type.
  //! {Property}
  virtual eScriptObjectType __stdcall GetType() const = 0;
  //! Clone the object reference.
  virtual iScriptObject* __stdcall Clone(tBool abHoldRef) const = 0;
  //! Enumerate all objects in this table.
  virtual tScriptObjectPtrCVec* __stdcall EnumObjects() const = 0;

  //########################################################################################
  //! \name Value getting.
  //########################################################################################
  //! @{

  //! Get the integer value.
  //! \return niMaxI32 if the type of the object is not integer.
  virtual tI32 __stdcall GetInt() const = 0;
  //! {Property}
  //! Get the float value.
  //! \return niMaxF64 if the type of the object is not float.
  //! {Property}
  virtual tF64 __stdcall GetFloat() const = 0;
  //! Get the string value.
  //! \return NULL if the type of the object is not string.
  //! {Property}
  virtual const achar* __stdcall GetString() const = 0;
  //! Get the hstring value.
  //! \return NULL if the type of the object is not string.
  //! {Property}
  virtual iHString* __stdcall GetHString() const = 0;
  //! Get the IUnknown pointer value.
  //! \return NULL if the type of the object is not iUnknown.
  //! \remark Make sure that the user data is really a IUnknown pointer otherwise the pointer
  //!     returned will be garbage.
  //! {Property}
  virtual iUnknown* __stdcall GetIUnknown() const = 0;
  //! Get the IUnknown object, will get the interface also from iDispatch table.
  virtual iUnknown* __stdcall GetIUnknownEx(const tUUID& aIID) const = 0;
  //! Get the Vec2 value.
  //! {Property}
  virtual sVec2f __stdcall GetVec2() const = 0;
  //! Get the Vec3 value.
  //! {Property}
  virtual sVec3f __stdcall GetVec3() const = 0;
  //! Get the Vec4 value.
  //! {Property}
  virtual sVec4f __stdcall GetVec4() const = 0;
  //! Get the Matrix value.
  //! {Property}
  virtual sMatrixf __stdcall GetMatrix() const = 0;
  //! Get the UUID value.
  //! {Property}
  virtual const tUUID& __stdcall GetUUID() const = 0;
  //! @}

  //########################################################################################
  //! \name Function specific
  //########################################################################################
  //! @{

  //! Get the number of parameters of the function.
  //! {Property}
  virtual tU32 __stdcall GetNumParameters() const = 0;
  //! Get the number of free variables of the function.
  //! {Property}
  virtual tU32 __stdcall GetNumFreeVars() const = 0;
  //! Get the number of parameters of the function minus the number of free variables.
  //! \remark This is the number of parameters to push on the stack to call the function.
  //! {Property}
  virtual tU32 __stdcall GetNumCallParameters() const = 0;
  //! @}
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // niMinFeatures
#endif // __ISCRIPTOBJECT_3733125_H__
