#ifndef __PARSETYPE_H_A194FF1F_4A0D_49FB_BB2D_1CD14712E923__
#define __PARSETYPE_H_A194FF1F_4A0D_49FB_BB2D_1CD14712E923__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/STL/vector.h>

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

/*

  NAME<TYPE>[X][Y][Z=a,W=b,A=(a,b,c),B,C]

*/

//! Parse type description flags enumeration
enum eParseTypeDescFlags {
  eParseTypeDescFlags_Name = niBit(0),
  eParseTypeDescFlags_Type = niBit(1),
  eParseTypeDescFlags_Flags = niBit(2),
  eParseTypeDescFlags_All = eParseTypeDescFlags_Name|eParseTypeDescFlags_Type|eParseTypeDescFlags_Flags
};

//! Parse type description flags type.
typedef tU32 tParseTypeDescFlags;

///////////////////////////////////////////////
template <typename T, typename TSTR>
static inline void ParseTypeDescTpl(const T* aaszInput,
                                    TSTR* apstrOutName,
                                    TSTR* apstrOutType,
                                    astl::vector<TSTR>* apvFlags)
{
  TSTR strName;
  TSTR strType;
  TSTR strTemp = aaszInput;
  const T* p = strTemp.Chars();

  // read the name
  while (*p) {
    if (*p == '[' || *p == '<')
      break;
    strName.appendChar(StrGetNextX(&p));
  }
  if (apstrOutName) {
    strName.Trim();
    *apstrOutName = strName;
  }

  // read the type
  if (*p == '<') {
    ++p; // skip '<'
    while (*p) {
      if (*p == '>')
        break;
      strType.appendChar(StrGetNextX(&p));
    }
  }
  if (apstrOutType) {
    strType.Trim();
    *apstrOutType = strType;
  }

  if (!apvFlags || !*p)
    return;

  while (1) {
    if (*p == '[') {
      StrGetNextX(&p);
      TSTR str;
      int depth = 0;
      while (*p) {
        const tU32 ch = StrGetNext(p);
        if (*p == '(') {
          ++depth;
          str.appendChar(ch);
        }
        else if (*p == ')') {
          --depth;
          str.appendChar(ch);
        }
        else if (*p == ']' && depth == 0)
          break;
        else if (*p == ',' && depth == 0) {
          str.Trim();
          apvFlags->push_back(str);
          str.Clear();
        }
        else {
          str.appendChar(ch);
        }
        StrGetNextX(&p);
      }
      if (str.IsNotEmpty()) {
        str.Trim();
        apvFlags->push_back(str);
      }
    }
    if (!*p)
      break;
    StrGetNextX(&p);
  }
}

static inline void ParseTypeDescEx(const achar* aaszInput, cString* apOutName, cString* apOutType, astl::vector<cString>* apOutFlags) {
  ParseTypeDescTpl(aaszInput,apOutName,apOutType,apOutFlags);
}

template <typename T, typename TSTR>
static inline void ParseKeyValueTpl(const T* aaszInput, TSTR* apKey, TSTR* apVal) {
  cString cur;
  const achar* p = aaszInput;

  // read the key, the first '=' determine when the value starts
  while (*p) {
    if (*p == '=') {
      break;
    }
    cur.appendChar(StrGetNextX(&p));
  }
  if (apKey) {
    // copy the key
    cur.Trim(); // removes leading and trailing spaces and nl
    *apKey = cur;
  }

  if (apVal && *p == '=') {
    cur.Clear();
    ++p; // skip '='
    // everything after '=' is a part of the value
    while (*p) {
      cur.appendChar(StrGetNextX(&p));
    }
    // copy the value
    cur.Trim(); // removes leading and trailing spaces and nl
    *apVal = cur;
  }
}

static inline void ParseKeyValueEx(const achar* aaszInput, cString* apKey, cString* apVal) {
  ParseKeyValueTpl(aaszInput,apKey,apVal);
}

template <typename MAPTYPE>
static inline void __stdcall ParseTypeToMap(MAPTYPE& aOut, const achar* aaszType, cString* apOutName = NULL, cString* apOutType = NULL) {
  astl::vector<cString> vProps;
  ParseTypeDescEx(aaszType,apOutName,apOutType,&vProps);
  for (tU32 i = 0; i < vProps.size(); ++i) {
    cString strKey, strVal;
    ParseKeyValueEx(vProps[i].Chars(),&strKey,&strVal);
    if (strKey.IsNotEmpty()) {
      aOut[strKey] = strVal;
    }
  }
}

#ifdef __IDATATABLE_15270737_H__
static inline iDataTable* __stdcall ParseTypeToDT(const achar* aaszType) {
  Ptr<iDataTable> ptrDTBase = ni::CreateDataTable(AZEROSTR);
  astl::vector<cString> vProps;
  ParseTypeDescEx(aaszType,NULL,NULL,&vProps);
  for (tU32 i = 0; i < vProps.size(); ++i) {
    cString strKey, strVal;
    ParseKeyValueEx(vProps[i].Chars(),&strKey,&strVal);
    if (strKey.IsNotEmpty()) {
      ptrDTBase->SetString(strKey.Chars(),strVal.Chars());
    }
  }
  return ptrDTBase.GetRawAndSetNull();
}
#endif

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __PARSETYPE_H_A194FF1F_4A0D_49FB_BB2D_1CD14712E923__
