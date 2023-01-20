#ifndef __SCRIPTCPPIMPL_H_4E213FAF_3F6C_4512_8C38_7699B7BF02A6__
#define __SCRIPTCPPIMPL_H_4E213FAF_3F6C_4512_8C38_7699B7BF02A6__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include <niLang/Types.h>
#include <niLang/ILang.h>
#include <niLang/Utils/SmartPtr.h>
#include <niLang/Utils/QPtr.h>

/** \addtogroup niScriptCpp
 * @{
 */

//--------------------------------------------------------------------------------------------
//
// General macros
//
//--------------------------------------------------------------------------------------------

#define niScriptCppExportClass(CATEGORY,NAME,CLASS)     \
  niExportFunc(ni::iUnknown*) New_##CATEGORY##_##NAME(  \
    const ni::Var&,const ni::Var&)                      \
  {                                                     \
    return niNew CLASS();                               \
  }

#define niScriptCppExportFunc(CATEGORY,NAME,CONSTRUCTOR)                \
  niExportFunc(ni::iUnknown*) New_##CATEGORY##_##NAME(                  \
    const ni::Var&,const ni::Var&)                                      \
  {                                                                     \
    ni::Ptr<ni::iUnknown> scriptCppObjectInstance = CONSTRUCTOR().ptr(); \
    niCheck(scriptCppObjectInstance.IsOK(),NULL);                       \
    return scriptCppObjectInstance.GetRawAndSetNull();                  \
  }

#define niScriptCppExport(CATEGORY,CLASS)       \
  niScriptCppExportClass(CATEGORY,CLASS,CLASS)

/**@}*/

#endif // __SCRIPTCPPIMPL_H_4E213FAF_3F6C_4512_8C38_7699B7BF02A6__
