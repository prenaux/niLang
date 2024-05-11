#ifndef __OBJMODELIMPL_19489664_H__
#define __OBJMODELIMPL_19489664_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../ObjModel.h"
#include "../Var.h"
#include "UnknownImpl.h"

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

struct sObjectTypeDef : public ImplLocal<iObjectTypeDef>
{
 private:
  const achar*  mName;
  tpfnCreateObjectInstance  mpfnCreateInstance;

 public:
  sObjectTypeDef(const achar* aName, tpfnCreateObjectInstance apfnCreateInstance)
      :   mName(aName), mpfnCreateInstance(apfnCreateInstance)
  {}

  const achar* __stdcall GetName() const niImpl { return mName; }
  iUnknown* __stdcall CreateInstance(const Var& aVarA, const Var& aVarB) const niOverride {
    return mpfnCreateInstance?mpfnCreateInstance((Var&)aVarA,(Var&)aVarB):NULL;
  }
};

struct sModuleDef : public ImplLocal<iModuleDef>
{
 private:
  const achar*  mName;
  const achar*  mVersion;
  const achar*  maszDesc;
  const achar*  maszAuthor;
  const achar*  maszCopyright;
  const tU32    mnNumDependencies;
  const achar* const* mpDependencies;
  const tU32    mnNumInterfaces;
  const sInterfaceDef* const*   mpInterfaces;
  const tU32    mnNumEnums;
  const sEnumDef* const*      mpEnums;
  const tU32    mnNumConstants;
  const sConstantDef* const*    mpConstants;
  const tU32    mnNumObjectTypes;
  const sObjectTypeDef* const*  mpObjectTypes;

 public:
  sModuleDef( const achar*  aName,
              const achar*  aVersion,
              const achar*  aaszDesc,
              const achar*  aaszAuthor,
              const achar*  aaszCopyright,
              const tU32    anNumDependencies,
              const achar* const* apDependencies,
              const tU32    anNumInterfaces,
              const sInterfaceDef* const*   apInterfaces,
              const tU32    anNumEnums,
              const sEnumDef* const*      apEnums,
              const tU32    anNumConstants,
              const sConstantDef* const*    apConstants,
              const tU32    anNumObjectTypes,
              const sObjectTypeDef* const*  apObjectTypes)
      : mName(aName), mVersion(aVersion),
        maszDesc(aaszDesc), maszAuthor(aaszAuthor), maszCopyright(aaszCopyright),
        mnNumDependencies(anNumDependencies), mpDependencies(apDependencies),
        mnNumInterfaces(anNumInterfaces), mpInterfaces(apInterfaces),
        mnNumEnums(anNumEnums), mpEnums(apEnums),
        mnNumConstants(anNumConstants), mpConstants(apConstants),
        mnNumObjectTypes(anNumObjectTypes), mpObjectTypes(apObjectTypes) {}

  //! Get the module's name.
  const achar* __stdcall GetName() const niOverride { return mName; }
  //! Get the module's version.
  const achar* __stdcall GetVersion() const niOverride { return mVersion; }
  //! Get the module's description.
  const achar* __stdcall GetDesc() const niOverride { return maszDesc; }
  //! Get the module's author.
  const achar* __stdcall GetAuthor() const niOverride { return maszAuthor; }
  //! Get the module's copyright.
  const achar* __stdcall GetCopyright() const niOverride { return maszCopyright; }
  //! Get the number of dependencies.
  const tU32 __stdcall GetNumDependencies() const niOverride { return mnNumDependencies; }
  //! Get the dependency at the given index.
  const achar* __stdcall GetDependency(tU32 anIndex) const niOverride { return (anIndex<mnNumDependencies)?mpDependencies[anIndex]:NULL; }
  //! Get the number of interfaces.
  const tU32 __stdcall GetNumInterfaces() const niOverride { return mnNumInterfaces; }
  //! Get the interface at the given index.
  const sInterfaceDef* __stdcall GetInterface(tU32 anIndex) const niOverride { return (anIndex<mnNumInterfaces)?mpInterfaces[anIndex]:NULL; }
  //! Get the number of enumerations.
  const tU32 __stdcall GetNumEnums() const niOverride { return mnNumEnums; }
  //! Get the enumeration at the given index.
  const sEnumDef* __stdcall GetEnum(tU32 anIndex) const niOverride { return (anIndex<mnNumEnums)?mpEnums[anIndex]:NULL; }
  //! Get the number of constants.
  const tU32 __stdcall GetNumConstants() const niOverride { return mnNumConstants; }
  //! Get the constant at the given index.
  const sConstantDef* __stdcall GetConstant(tU32 anIndex) const niOverride { return (anIndex<mnNumConstants)?mpConstants[anIndex]:NULL; }
  //! Get the number of object types.
  const tU32 __stdcall GetNumObjectTypes() const niOverride { return mnNumObjectTypes; }
  //! Get the object type at the given index,
  const sObjectTypeDef* __stdcall GetObjectType(tU32 anIndex) const niOverride { return (anIndex<mnNumObjectTypes)?mpObjectTypes[anIndex]:NULL; }
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __OBJMODELIMPL_19489664_H__
