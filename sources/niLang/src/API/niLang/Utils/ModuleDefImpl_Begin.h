#ifndef __MODULEDEFIMPL_BEGIN_329018490_H__
#define __MODULEDEFIMPL_BEGIN_329018490_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

#ifdef USE_NICRT
#ifdef niMSVC
#pragma niNote("## Using ni CRT")
#pragma comment(linker, "/nodefaultlib")
#endif
#endif

#ifndef _DEBUG

#ifdef niConfig_MinModuleDefSize
#pragma niNote("Minimum Size ModuleDef")
#ifdef niMSVC
#pragma optimize("s",on)
#endif
#endif

#endif

// #define niConfig_OnlyObjectTypesIDL
// #define niConfig_NoXCALL

// Meant to be called in a Modules's ModuleDef GetModule() implementation
#define niModuleDefImpl_GetModule_Register()

/**@}*/
/**@}*/

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __MODULEDEFIMPL_BEGIN_329018490_H__
