#pragma once
#ifndef __OBJMODEL_4180864_H__
#define __OBJMODEL_4180864_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "Types.h"
#include "Var.h"

namespace ni {
struct iModuleDef;
struct sEnumDef;
struct sInterfaceDef;

/** \addtogroup niLang
 * @{
 */

//! Create a dispatch wrapper
typedef iUnknown* (__ni_export_call_decl *tpfnCreateDispatchWrapper)(iDispatch* apDispatch);
//! Create an object instance
typedef iUnknown* (__ni_export_call_decl *tpfnCreateObjectInstance)(const Var& aVarA, const Var& aVarB);
//! Get a module definition.
typedef const iModuleDef* (__ni_export_call_decl *tpfnGetModuleDef)();
//! Get an enum definition.
typedef const sEnumDef* (__ni_export_call_decl *tpfnGetEnumDef)();
//! Get an interface definition.
typedef const sInterfaceDef* (__ni_export_call_decl *tpfnGetInterfaceDef)();

struct sConstantDef
{
 public:
  const achar* const maszName;
  const Var        mvarValue;

  sConstantDef(const achar* aaszName,
               const Var&  avarValue)
      : maszName(aaszName),
        mvarValue(avarValue)
  {}
};

struct sEnumValueDef {
  const achar* const maszName;
  const tU32         mnValue;
};

struct sEnumDef {
  const achar* const         maszName;
  const tU32                 mnNumValues;
  const sEnumValueDef* const mpValues;
};

struct sParameterDef {
  const achar* const  maszName;
  const tType         mType;
  const tUUID* const  mTypeUUID;
  const achar* const  mTypeName;
};

struct sMethodDef {
  const achar* const  maszName;
  const tType         mReturnType;
  const tUUID* const  mReturnTypeUUID;
  const char* const   mReturnTypeName;
  const tU32          mnNumParameters;
  const sParameterDef* const mpParameters;
#if !defined niConfig_NoXCALL
  const tpfnVMCall    mpVMCall;
#endif
};

struct sInterfaceDef
{
  const achar* const              maszName;
  const tUUID* const              mUUID;
  const tU32                      mnNumBases;
  const tUUID* const*             mpBases;
  const tU32                      mnNumMethods;
  const sMethodDef* const*        mpMethods;
  const tpfnCreateDispatchWrapper mpfnCreateDispatchWrapper;
};

//! Object type definition interface.
struct iObjectTypeDef : public iUnknown
{
  niDeclareInterfaceUUID(iObjectTypeDef,0xc26d991e,0xe51f,0x4d84,0x86,0x6a,0xe7,0x7f,0xaf,0xd1,0xb8,0xe2)
  //! Get the object type's name.
  virtual const achar* __stdcall GetName() const = 0;
  //! Create an instance of the object.
  virtual iUnknown* __stdcall CreateInstance(const Var& aVarA, const Var& aVarB) const = 0;
};

//! Module definition interface.
struct iModuleDef : public iUnknown
{
  niDeclareInterfaceUUID(iModuleDef,0x80463a66,0xfc9e,0x4490,0xab,0x81,0x00,0xb9,0x15,0x2c,0x17,0xf3)

  //! Get the module's name.
  virtual const achar* __stdcall GetName() const = 0;
  //! Get the module's version.
  virtual const achar* __stdcall GetVersion() const = 0;
  //! Get the module's description.
  virtual const achar* __stdcall GetDesc() const = 0;
  //! Get the module's author.
  virtual const achar* __stdcall GetAuthor() const = 0;
  //! Get the module's copyright.
  virtual const achar* __stdcall GetCopyright() const = 0;
  //! Get the number of dependencies.
  virtual const tU32 __stdcall GetNumDependencies() const = 0;
  //! Get the dependency at the given index.
  //! \remark The dependency are usually another module, but could be something else
  //!     like only one interface, that's why it is stored as a string and not
  //!     as a ModuleID.
  virtual const achar* __stdcall GetDependency(tU32 anIndex) const = 0;
  //! Get the number of interfaces.
  virtual const tU32 __stdcall GetNumInterfaces() const = 0;
  //! Get the interface at the given index.
  //! {NoAutomation}
  virtual const sInterfaceDef* __stdcall GetInterface(tU32 anIndex) const = 0;
  //! Get the number of enumerations.
  virtual const tU32 __stdcall GetNumEnums() const = 0;
  //! Get the enumeration at the given index.
  //! {NoAutomation}
  virtual const sEnumDef* __stdcall GetEnum(tU32 anIndex) const = 0;
  //! Get the number of constants.
  virtual const tU32 __stdcall GetNumConstants() const = 0;
  //! Get the constant at the given index.
  //! {NoAutomation}
  virtual const sConstantDef* __stdcall GetConstant(tU32 anIndex) const = 0;
  //! Get the number of object types.
  virtual const tU32 __stdcall GetNumObjectTypes() const = 0;
  //! Get the object type at the given index,
  virtual const iObjectTypeDef* __stdcall GetObjectType(tU32 anIndex) const = 0;
};

static inline tBool VarIsType(const Var* apVar, const tType aType, const tBool abCheckTypeFlags) {
  if (!aType)
    return apVar->IsNull();
  if (abCheckTypeFlags ?
      (apVar->mType != aType) :
      (niType(apVar->mType) != niType(aType)))
    return eFalse;
  return eTrue;
}

/**@}*/
/// EOF //////////////////////////////////////////////////////////////////////////////////////
}; // End of ni
#endif // __OBJMODEL_4180864_H__
