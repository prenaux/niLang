#ifndef __MESSAGEID_9C5A8773_2D32_459D_96E8_779A441AC097_H__
#define __MESSAGEID_9C5A8773_2D32_459D_96E8_779A441AC097_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/StringDef.h>

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

#define niMessageID_CharToBase(CHAR)                      \
  (((CHAR)=='_')?1:                                       \
   ((CHAR) >= 'a' && (CHAR) <= 'z')?(((CHAR)-'a')+2):     \
   ((CHAR) >= 'A' && (CHAR) <= 'Z')?(((CHAR)-'A')+28):    \
   ((CHAR) >= '0' && (CHAR) <= '9')?(((CHAR)-'0')+54):0)

#define niMessageID_PackBased(A,B,C,D,BYTE)         \
  (((BYTE))|((D)<<8)|((C)<<14)|((B)<<20)|((A)<<26))

#define niMessageID_PackChars(A,B,C,D,BYTE)         \
  niMessageID_PackBased(niMessageID_CharToBase(A),  \
                        niMessageID_CharToBase(B),  \
                        niMessageID_CharToBase(C),  \
                        niMessageID_CharToBase(D),  \
                        BYTE)

#define niMessageID(A,B,C,D,BYTE)               \
  niMessageID_PackChars(A,B,C,D,BYTE)

#define niMessageID_FromChars(CHARS,BYTE)                               \
  (niMessageID_PackChars((CHARS)[0],(CHARS)[1],(CHARS)[2],(CHARS)[3],BYTE))

#define niMessageID_BaseToChar(BASE)                \
  (((BASE)==1)?'_':                                 \
   ((BASE)>=2  && (BASE)<28)?(((BASE)-2)+'a'):      \
   ((BASE)>=28 && (BASE)<54)?(((BASE)-28)+'A'):     \
   ((BASE)>=54 && (BASE)<64)?(((BASE)-54)+'0'):'-')

#define niMessageID_GetBaseA(MSGID) (((MSGID)>>26)&0x3F)
#define niMessageID_GetBaseB(MSGID) (((MSGID)>>20)&0x3F)
#define niMessageID_GetBaseC(MSGID) (((MSGID)>>14)&0x3F)
#define niMessageID_GetBaseD(MSGID) (((MSGID)>>8)&0x3F)
#define niMessageID_GetBYTE(MSGID)  (((MSGID))&0xFF)

#define niMessageID_GetCharA(MSGID) niMessageID_BaseToChar(niMessageID_GetBaseA(MSGID))
#define niMessageID_GetCharB(MSGID) niMessageID_BaseToChar(niMessageID_GetBaseB(MSGID))
#define niMessageID_GetCharC(MSGID) niMessageID_BaseToChar(niMessageID_GetBaseC(MSGID))
#define niMessageID_GetCharD(MSGID) niMessageID_BaseToChar(niMessageID_GetBaseD(MSGID))

#define niMessageID_MaskA(MSGID)                      \
  niMessageID_PackBased(niMessageID_GetBaseA(MSGID),  \
                        0,0,0,0)

#define niMessageID_MaskAB(MSGID)                     \
  niMessageID_PackBased(niMessageID_GetBaseA(MSGID),  \
                        niMessageID_GetBaseB(MSGID),  \
                        0,0,0)

#define niMessageID_MaskABC(MSGID)                    \
  niMessageID_PackBased(niMessageID_GetBaseA(MSGID),  \
                        niMessageID_GetBaseB(MSGID),  \
                        niMessageID_GetBaseC(MSGID),  \
                        0,0)

#define niMessageID_MaskABCD(MSGID)                   \
  niMessageID_PackBased(niMessageID_GetBaseA(MSGID),  \
                        niMessageID_GetBaseB(MSGID),  \
                        niMessageID_GetBaseC(MSGID),  \
                        niMessageID_GetBaseD(MSGID),  \
                        0)


#define niMessageID_Fmt(MSGID)                  \
  niFmt(_A("%c%c%c%c%02X"),                     \
        niMessageID_GetCharA(MSGID),            \
        niMessageID_GetCharB(MSGID),            \
        niMessageID_GetCharC(MSGID),            \
        niMessageID_GetCharD(MSGID),            \
        niMessageID_GetBYTE(MSGID))

inline ni::tU32 MessageID_FromString(const ni::achar* aaszStr) {
  const ni::achar* p = aaszStr;
  if (!*p) return 0;
  const int a = *p++;
  if (!*p) return 0;
  const int b = *p++;
  if (!*p) return 0;
  const int c = *p++;
  if (!*p) return 0;
  const int d = *p++;
  if (!*p) return 0;
  ni::achar buf[3] = {0,0,0};
  buf[0] = *p++;
  if (!*p) return 0;
  buf[1] = *p++;
  if (buf[0] == '.') {
#ifdef niUnicode // this is to avoid the "comparison is always true" warning happening
    // with GCC on some platforms
    if (buf[1] > 32 && buf[1] <= 255)
#else
      if (buf[1] > 32)
#endif
        return niMessageID(a,b,c,d,buf[1]);
      else
        return 0; // invalid format, '.' has to be followed by a valid ascii character
    // excluding the space character
  }
  else {
    const ni::achar* e;
    const int byte = (int)ni::StrToL(buf,&e,16);
    if (byte > 255) return 0;
    return niMessageID(a,b,c,d,byte);
  }
}
inline const ni::achar* MessageID_ToString(ni::achar aaszOut[7], ni::tU32 anMessageID) {
  aaszOut[0] = (ni::achar)niMessageID_GetCharA(anMessageID);
  aaszOut[1] = (ni::achar)niMessageID_GetCharB(anMessageID);
  aaszOut[2] = (ni::achar)niMessageID_GetCharC(anMessageID);
  aaszOut[3] = (ni::achar)niMessageID_GetCharD(anMessageID);
  int byte = niMessageID_GetBYTE(anMessageID);
  if (byte > 32 && byte <= 255) {
    aaszOut[4] = '.';
    aaszOut[5] = (tU8)byte;
  }
  else {
    tU8 b = (tU8)byte;
    StrBytesToHexa(aaszOut+4,&b,1);
  }
  aaszOut[6] = 0;
  return aaszOut;
}
inline ni::cString MessageID_ToString(ni::tU32 anMessageID) {
  ni::achar buf[7] = {0,0,0,0,0,0,0};
  MessageID_ToString(buf,anMessageID);
  return buf;
}

//! Filter format : INCLUDE!EXCLUDE*. INCLUDE letters are matched, if
//! any of them does match returns TRUE. Then EXCLUDE letters are
//! matched, if any of them does match returns FALSE. The
//! default return value if a '!' is found is TRUE, otherwise it is
//! FALSE. This means that the string "!" will match ANY character (since the default
//! return value becomes TRUE.
template <typename T>
inline tBool MessageID_FilterChar(const T d, const T* p) {
  if (!p) return eFalse;
  tBool bRet = eFalse;
  // INCLUDES
  while (*p) {
    if (*p == '!') {
      bRet = eTrue;
      ++p; // skip the '!'
      break; // excludes
    }
    else if (*p == d)
      return eTrue;
    ++p;
  }
  // EXCLUDES
  while (*p) {
    if (*p == d)
      return eFalse;
    ++p;
  }
  return bRet;
}

template <typename T>
inline tBool MessageID_FilterD(tU32 anMsgID, const T* p) {
  return MessageID_FilterChar((T)niMessageID_GetCharD(anMsgID),p);
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __MESSAGEID_9C5A8773_2D32_459D_96E8_779A441AC097_H__
