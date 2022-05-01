#ifndef __OBJECTINTERFACECAST_18950084_H__
#define __OBJECTINTERFACECAST_18950084_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../IHString.h"

/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

// Has been tested with MSVC only...
#if !defined niMSVC && !defined niGCC && !defined niCLang
#define niSafeObjectInterfaceCast
#pragma niNote("Using SafeHString test, be sure to read the comments below.")
/*
  niSafeObjectInterfaceCast will use QueryInterface to determin if the object is a iHString.
  This is reliable on any platform but it ought to be quite slow. So it's a really
  good idea to make sure to implement the "unsafe" version on each new platform.
  The current unsafe version should actually work on any platform aswell, as the
  position of the vtable ough to be at the first place for binary compatibility
  between languages. But we still cant rely on that since the binary compatibility
  is only theory in the C++ world.
*/
#endif

//////////////////////////////////////////////////////////////////////
// Safe Object Interface cast
//////////////////////////////////////////////////////////////////////
#ifdef niSafeObjectInterfaceCast
namespace ni {

inline iHString* GetIUnknownHString(iUnknown* apObj) {
  return ni::QueryInterface<iHString>(apObj);
}

}
//////////////////////////////////////////////////////////////////////
// 'Unsafe' (if not tested) Object Interface cast
//////////////////////////////////////////////////////////////////////
#else

niExportFunc(void*) ni_get_vtable_object_iHString();

namespace ni {

#define niGetVTable_Object(OBJ)    (*((void**)(OBJ)))
#define niGetVTable_IHString()      niGetVTable_Object(ni_get_vtable_object_iHString())

__forceinline iHString* GetIUnknownHString(iUnknown* apObj) {
  return  ((apObj == NULL || niGetVTable_IHString() != niGetVTable_Object(apObj)) ?
           (ni::iHString*)NULL : (ni::iHString*)apObj);
}

}
#endif

#define niGetIUnknownHString(apObj) ni::GetIUnknownHString(apObj)

/**@}*/
/**@}*/

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __OBJECTINTERFACECAST_18950084_H__
