#ifndef __GENERATEUNIQUENAME_83193888_H__
#define __GENERATEUNIQUENAME_83193888_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../ILang.h"
#include <niLang/Utils/UUID.h>
#include <niLang/Utils/SmartPtr.h>

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

//! Unique name sink.
struct iIsUniqueNameSink : public iUnknown {
  niDeclareInterfaceUUID(iIsUniqueNameSink,0x99c0433a,0xd836,0x467f,0xb5,0xdb,0x6c,0x47,0x18,0x7d,0x0f,0x20);
  //! Return true if the name is unique, else false.
  virtual ni::tBool __stdcall OnIsUniqueNameSink(ni::iHString* ahspName) = 0;
};

//! Prediate on top of a unique name sink.
struct sIsUniqueNameSink {
  ni::Ptr<ni::iIsUniqueNameSink> mptrSink;
  sIsUniqueNameSink(ni::iIsUniqueNameSink* apSink) {
    mptrSink = apSink;
  }
  ni::tBool operator() (ni::iHString* ahspName) {
    return mptrSink->OnIsUniqueNameSink(ahspName);
  }
};

//! Generate a unique name, using a UUID as name generation base.
//! The predicate should return true if the name is unique, else false.
template <typename T>
ni::Ptr<ni::iHString> GenerateUniqueNameUUID(ni::iHString* ahspBaseName, const tUUID& aUUID, const T& aPredicate) {
  ni::tHStringPtr hspBaseName = ahspBaseName;
  if (HStringIsEmpty(hspBaseName)) {
    hspBaseName = _H("NoName");
  }
  if (aPredicate(hspBaseName))
    return hspBaseName;

  // get the name part, before the "#"
  ni::cString strName = niHStr(hspBaseName);
  ni::tSize pos = strName.rfind(_A("#"));
  if (pos != ni::cString::npos) {
    strName = strName.substr(0,pos);
  }

  const ni::sUUID& guid = (const ni::sUUID&)aUUID;
  // try with the first 16bits part
  {
    ni::tHStringPtr hspName = _H(niFmt(_A("%s#%04X"),
                                        strName.Chars(),
                                        guid.nData2));
    if (aPredicate(hspName)) return hspName;
  }
  // try with the 3rd 16bits part
  {
    ni::tHStringPtr hspName = _H(niFmt(_A("%s#%02X%02X"),
                                        strName.Chars(),
                                        guid.nData4[0],guid.nData4[1]));
    if (aPredicate(hspName)) return hspName;
  }
  // try with the 32bits part
  {
    ni::tHStringPtr hspName = _H(niFmt(_A("%s#%08X"),
                                        strName.Chars(),
                                        guid.nData1));
    if (aPredicate(hspName)) return hspName;
  }
  // try with the last 6bytes part
  {
    ni::tHStringPtr hspName = _H(niFmt(_A("%s#%02X%02X%02X%02X%02X%02X"),
                                        strName.Chars(),
                                        guid.nData4[2],guid.nData4[3],guid.nData4[4],
                                        guid.nData4[5],guid.nData4[6],guid.nData4[7]));
    if (aPredicate(hspName)) return hspName;
  }
  // worst case we happend the whole GUID
  {
    ni::tHStringPtr hspName = _H(niFmt(_A("%s#%s"),strName.Chars(),guid.ToString().Chars()));
    return hspName;
  }
}

//! Generate a unique name, using a counter as name generation base.
//! The predicate should return true if the name is unique, else false.
template <typename T>
ni::Ptr<ni::iHString> GenerateUniqueNameCounter(ni::iHString* ahspBaseName, const tU32 aMaxCount, const T& aPredicate) {
  ni::tHStringPtr hspBaseName = ahspBaseName;
  if (HStringIsEmpty(hspBaseName)) {
    hspBaseName = _H("NoName");
  }
  if (aPredicate(hspBaseName))
    return hspBaseName;

  // get the name part, before the "#"
  ni::cString strName = niHStr(hspBaseName);
  ni::tSize pos = strName.rfind(_A("#"));
  if (pos != ni::cString::npos) {
    strName = strName.substr(0,pos);
  }

  ni::tU32 nCount = 1;
  do {
    ni::tHStringPtr hspName = _H(niFmt(_A("%s#%d"),strName.Chars(),nCount));
    if (aPredicate(hspName))
      return hspName;
  } while (nCount++ < aMaxCount);

  return NULL;
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __GENERATEUNIQUENAME_83193888_H__
