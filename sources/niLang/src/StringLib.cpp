// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "API/niLang/StringLib.h"
#include "API/niLang/StringLibIt.h"
#include "API/niLang/STL/list.h"
#include "API/niLang/Utils/UnknownImpl.h"
#include "API/niLang/Utils/UTFImpl.h"
#include "API/niLang/ILang.h"
#include <stdio.h>

namespace ni {

#define STRLIB_ENC  utf8

static inline tU32 _GetC(const achar* s) {
  return STRLIB_ENC::peek_next(s);
}
static inline tU32 _GetX(const achar** s) {
  return STRLIB_ENC::next(*s);
}
static inline tU32 _GetX(achar** s) {
  const achar* p = *s;
  tU32 c = STRLIB_ENC::next(p);
  *s = (achar*)p;
  return c;
}
static inline tU32 _SetC(achar* s, tI32 c) {
  niAssert(s != NULL);
  achar* ps = s;
  s = STRLIB_ENC::append(c,s);
  return (s-ps);
}
static inline tU32 _PrevX(const achar** s) {
  return STRLIB_ENC::prior(*s);
}
static inline tU32 _PrevX(achar** s) {
  const achar* p = *s;
  tU32 c = STRLIB_ENC::prior(p);
  *s = (achar*)p;
  return c;
}

static _HDecl_(stringlib_FormatNoArg, "<MISSING_FMT_ARG>");

#define STRLIB_GETC(s)      _GetC(s)
#define STRLIB_GETX(s)      _GetX(s)
#define STRLIB_GETXC(s)     _GetX(s)
#define STRLIB_SETC(s,c)    _SetC(s,c)
#define STRLIB_UWIDTH(s)    STRLIB_ENC::sequence_length(s)
#define STRLIB_UCWIDTH(c)   STRLIB_ENC::char_width(c)

static char _HexDigitUpr[] = "0123456789ABCDEF";
static char _HexDigitLwr[] = "0123456789abcdef";

//! Get the size in bytes of the specified character.
niExportFunc(tU32) StrCharWidth(tU32 c) {
  return STRLIB_ENC::char_width(c);
}

//! Get the size in bytes of the next character in the specified string.
niExportFunc(tU32) StrSequenceLength(const achar* s) {
  niAssert(s != NULL);
  return STRLIB_ENC::sequence_length(s);
}

//! Get the number of characters between a and b.
//! \remark a and b must be pointers to the same string.
niExportFunc(tU32) StrCharDistance(const achar* a, const achar* b) {
  niAssert(a != NULL);
  niAssert(b != NULL);
  return STRLIB_ENC::distance(a,b);
}

//! Return the current character.
niExportFunc(tU32) StrGetNext(const achar* s) {
  niAssert(s != NULL);
  return STRLIB_ENC::peek_next(s);
}

//! Return the current character and set the pointer to the next.
niExportFunc(tU32) StrGetNextX(const achar** s) {
  niAssert(s != NULL);
  niAssert(*s != NULL);
  return STRLIB_ENC::next(*s);
}

//! Return the previous character.
niExportFunc(tU32) StrGetPrior(const achar* s) {
  niAssert(s != NULL);
  return STRLIB_ENC::peek_prior(s);
}

//! Return the previous character and set the pointer to it.
niExportFunc(tU32) StrGetPriorX(const achar** s) {
  niAssert(s != NULL);
  niAssert(*s != NULL);
  return STRLIB_ENC::prior(*s);
}

//! Set a character in the specified buffer. Buffer should be at least 4 bytes wide.
//! \return Size in bytes, of the character set.
niExportFunc(tU32) StrSetChar(achar* s, tI32 c) {
  niAssert(s != NULL);
  achar* ps = s;
  s = STRLIB_ENC::append(c,s);
  return (s-ps);
}

//! Set a character in the specified buffer and move the pointer after
//! it. Buffer should be at least 4 bytes wide.
//! \return Size in bytes, of the character set.
niExportFunc(tU32) StrSetCharX(achar** s, tI32 c) {
  niAssert(s != NULL);
  niAssert(*s != NULL);
  achar* ps = *s;
  *s = STRLIB_ENC::append(c,*s);
  return (*s-ps);
}

///////////////////////////////////////////////
niExportFunc(tBool) StrIsValidCodePoint(tU32 c) {
  return utf_is_code_point_valid(c);
}

///////////////////////////////////////////////
niExportFunc(tBool) StrIsValidSequence(const achar* b, const achar* e) {
  if (!b || !e || e < b) return eFalse;
  return utf8::is_valid(b,e);
}
niExportFunc(tBool) StrIsValidString(const achar* str) {
  if (!str) return eFalse;
  const tU32 size = StrSize(str);
  return utf8::is_valid(str,str+size);
}

///////////////////////////////////////////////
niExportFunc(tU32) StrOffset(const achar* _s, tI32 index)
{
  const achar* s = _s;
  const achar* orig = s;
  const achar* last;
  niAssert(s);

  if (index < 0)
    index += StrLen(s);

  while (index-- > 0) {
    last = s;
    if (!STRLIB_GETXC(&s)) {
      s = last;
      break;
    }
  }

  return ((tIntPtr)s - (tIntPtr)orig);
}

///////////////////////////////////////////////
niExportFunc(tI32) StrGetAt(const achar* s, tI32 index)
{
  niAssert(s);
  return STRLIB_GETC(s + StrOffset(s,index));
}

///////////////////////////////////////////////
niExportFunc(tI32) StrSetAt(achar* s, tI32 index, tI32 c)
{
  tI32 oldw, neww;
  niAssert(s);

  s += StrOffset( s,    index);

  oldw = STRLIB_UWIDTH(s);
  neww = STRLIB_UCWIDTH(c);

  if (oldw != neww)
    memmove(s+neww, s+oldw, StrSizeZ(s+oldw));

  STRLIB_SETC(s, c);

  return neww-oldw;
}

///////////////////////////////////////////////
niExportFunc(tI32) StrInsert(achar* s, tI32 index, tI32 c)
{
  if (!s) return 0;
  tI32 w = STRLIB_UCWIDTH(c);
  s += StrOffset( s,    index);
  memmove(s+w, s, StrSizeZ(s));
  STRLIB_SETC(s, c);
  return w;
}

///////////////////////////////////////////////
niExportFunc(tI32) StrRemove(achar* s, tI32 index)
{
  if (!s) return 0;
  tI32 w;
  s += StrOffset( s, index);
  w = STRLIB_UWIDTH(s);
  memmove(s, s+w, StrSizeZ(s+w));
  return -w;
}

///////////////////////////////////////////////
niExportFunc(tBool) StrIsUpper(tI32 c) {
  return (c >= 'A' && c <= 'Z');
}
niExportFunc(tBool) StrIsLower(tI32 c) {
  return (c >= 'a' && c <= 'z');
}

///////////////////////////////////////////////
niExportFunc(tI32) StrToLower(tI32 c)
{
  if ((c >= 65 && c <= 90) ||
      (c >= 192 && c <= 214) ||
      (c >= 216 && c <= 222) ||
      (c >= 913 && c <= 929) ||
      (c >= 931 && c <= 939) ||
      (c >= 1040 && c <= 1071))
    return c + 32;
  if ((c >= 393 && c <= 394))
    return c + 205;
  if ((c >= 433 && c <= 434))
    return c + 217;
  if ((c >= 904 && c <= 906))
    return c + 37;
  if ((c >= 910 && c <= 911))
    return c + 63;
  if ((c >= 1025 && c <= 1036) ||
      (c >= 1038 && c <= 1039))
    return c + 80;
  if ((c >= 1329 && c <= 1366) ||
      (c >= 4256 && c <= 4293))
    return c + 48;
  if ((c >= 7944 && c <= 7951) ||
      (c >= 7960 && c <= 7965) ||
      (c >= 7976 && c <= 7983) ||
      (c >= 7992 && c <= 7999) ||
      (c >= 8008 && c <= 8013) ||
      (c >= 8040 && c <= 8047) ||
      (c >= 8072 && c <= 8079) ||
      (c >= 8088 && c <= 8095) ||
      (c >= 8104 && c <= 8111) ||
      (c >= 8120 && c <= 8121) ||
      (c >= 8152 && c <= 8153) ||
      (c >= 8168 && c <= 8169))
    return c + -8;
  if ((c >= 8122 && c <= 8123))
    return c + -74;
  if ((c >= 8136 && c <= 8139))
    return c + -86;
  if ((c >= 8154 && c <= 8155))
    return c + -100;
  if ((c >= 8170 && c <= 8171))
    return c + -112;
  if ((c >= 8184 && c <= 8185))
    return c + -128;
  if ((c >= 8186 && c <= 8187))
    return c + -126;
  if ((c >= 8544 && c <= 8559))
    return c + 16;
  if ((c >= 9398 && c <= 9423))
    return c + 26;

  switch (c) {
    case 256: case 258: case 260: case 262: case 264: case 266:
    case 268: case 270: case 272: case 274: case 276: case 278:
    case 280: case 282: case 284: case 286: case 288: case 290:
    case 292: case 294: case 296: case 298: case 300: case 302:
    case 306: case 308: case 310: case 313: case 315: case 317:
    case 319: case 321: case 323: case 325: case 327: case 330:
    case 332: case 334: case 336: case 338: case 340: case 342:
    case 344: case 346: case 348: case 350: case 352: case 354:
    case 356: case 358: case 360: case 362: case 364: case 366:
    case 368: case 370: case 372: case 374: case 377: case 379:
    case 381: case 386: case 388: case 391: case 395: case 401:
    case 408: case 416: case 418: case 420: case 423: case 428:
    case 431: case 435: case 437: case 440: case 444: case 453:
    case 456: case 459: case 461: case 463: case 465: case 467:
    case 469: case 471: case 473: case 475: case 478: case 480:
    case 482: case 484: case 486: case 488: case 490: case 492:
    case 494: case 498: case 500: case 506: case 508: case 510:
    case 512: case 514: case 516: case 518: case 520: case 522:
    case 524: case 526: case 528: case 530: case 532: case 534:
    case 994: case 996: case 998: case 1000: case 1002: case 1004:
    case 1006: case 1120: case 1122: case 1124: case 1126: case 1128:
    case 1130: case 1132: case 1134: case 1136: case 1138: case 1140:
    case 1142: case 1144: case 1146: case 1148: case 1150: case 1152:
    case 1168: case 1170: case 1172: case 1174: case 1176: case 1178:
    case 1180: case 1182: case 1184: case 1186: case 1188: case 1190:
    case 1192: case 1194: case 1196: case 1198: case 1200: case 1202:
    case 1204: case 1206: case 1208: case 1210: case 1212: case 1214:
    case 1217: case 1219: case 1223: case 1227: case 1232: case 1234:
    case 1236: case 1238: case 1240: case 1242: case 1244: case 1246:
    case 1248: case 1250: case 1252: case 1254: case 1256: case 1258:
    case 1262: case 1264: case 1266: case 1268: case 1272: case 7680:
    case 7682: case 7684: case 7686: case 7688: case 7690: case 7692:
    case 7694: case 7696: case 7698: case 7700: case 7702: case 7704:
    case 7706: case 7708: case 7710: case 7712: case 7714: case 7716:
    case 7718: case 7720: case 7722: case 7724: case 7726: case 7728:
    case 7730: case 7732: case 7734: case 7736: case 7738: case 7740:
    case 7742: case 7744: case 7746: case 7748: case 7750: case 7752:
    case 7754: case 7756: case 7758: case 7760: case 7762: case 7764:
    case 7766: case 7768: case 7770: case 7772: case 7774: case 7776:
    case 7778: case 7780: case 7782: case 7784: case 7786: case 7788:
    case 7790: case 7792: case 7794: case 7796: case 7798: case 7800:
    case 7802: case 7804: case 7806: case 7808: case 7810: case 7812:
    case 7814: case 7816: case 7818: case 7820: case 7822: case 7824:
    case 7826: case 7828: case 7840: case 7842: case 7844: case 7846:
    case 7848: case 7850: case 7852: case 7854: case 7856: case 7858:
    case 7860: case 7862: case 7864: case 7866: case 7868: case 7870:
    case 7872: case 7874: case 7876: case 7878: case 7880: case 7882:
    case 7884: case 7886: case 7888: case 7890: case 7892: case 7894:
    case 7896: case 7898: case 7900: case 7902: case 7904: case 7906:
    case 7908: case 7910: case 7912: case 7914: case 7916: case 7918:
    case 7920: case 7922: case 7924: case 7926: case 7928:
      return c + 1;
    case 304:
      return c + -199;
    case 376:
      return c + -121;
    case 385:
      return c + 210;
    case 390:
      return c + 206;
    case 398:
      return c + 79;
    case 399:
      return c + 202;
    case 400:
      return c + 203;
    case 403:
      return c + 205;
    case 404:
      return c + 207;
    case 406:
    case 412:
      return c + 211;
    case 407:
      return c + 209;
    case 413:
      return c + 213;
    case 415:
      return c + 214;
    case 422: case 425: case 430:
      return c + 218;
    case 439:
      return c + 219;
    case 452: case 455: case 458: case 497:
      return c + 2;
    case 902:
      return c + 38;
    case 908:
      return c + 64;
    case 8025: case 8027: case 8029: case 8031:
      return c + -8;
    case 8124: case 8140: case 8188:
      return c + -9;
    case 8172:
      return c + -7;
    default:
      return c;
  }
}

///////////////////////////////////////////////
niExportFunc(tI32) StrToUpper(tI32 c)
{
  if ((c >= 97 && c <= 122) ||
      (c >= 224 && c <= 246) ||
      (c >= 248 && c <= 254) ||
      (c >= 945 && c <= 961) ||
      (c >= 963 && c <= 971) ||
      (c >= 1072 && c <= 1103))
    return c + -32;
  if ((c >= 598 && c <= 599))
    return c + -205;
  if ((c >= 650 && c <= 651))
    return c + -217;
  if ((c >= 941 && c <= 943))
    return c + -37;
  if ((c >= 973 && c <= 974))
    return c + -63;
  if ((c >= 1105 && c <= 1116) ||
      (c >= 1118 && c <= 1119))
    return c + -80;
  if ((c >= 1377 && c <= 1414))
    return c + -48;
  if ((c >= 7936 && c <= 7943) ||
      (c >= 7952 && c <= 7957) ||
      (c >= 7968 && c <= 7975) ||
      (c >= 7984 && c <= 7991) ||
      (c >= 8000 && c <= 8005) ||
      (c >= 8032 && c <= 8039) ||
      (c >= 8064 && c <= 8071) ||
      (c >= 8080 && c <= 8087) ||
      (c >= 8096 && c <= 8103) ||
      (c >= 8112 && c <= 8113) ||
      (c >= 8144 && c <= 8145) ||
      (c >= 8160 && c <= 8161))
    return c + 8;
  if ((c >= 8048 && c <= 8049))
    return c + 74;
  if ((c >= 8050 && c <= 8053))
    return c + 86;
  if ((c >= 8054 && c <= 8055))
    return c + 100;
  if ((c >= 8056 && c <= 8057))
    return c + 128;
  if ((c >= 8058 && c <= 8059))
    return c + 112;
  if ((c >= 8060 && c <= 8061))
    return c + 126;
  if ((c >= 8560 && c <= 8575))
    return c + -16;
  if ((c >= 9424 && c <= 9449))
    return c + -26;

  switch (c) {
    case 255:
      return c + 121;
    case 257: case 259: case 261: case 263: case 265: case 267:
    case 269: case 271: case 273: case 275: case 277: case 279:
    case 281: case 283: case 285: case 287: case 289: case 291:
    case 293: case 295: case 297: case 299: case 301: case 303:
    case 307: case 309: case 311: case 314: case 316: case 318:
    case 320: case 322: case 324: case 326: case 328: case 331:
    case 333: case 335: case 337: case 339: case 341: case 343:
    case 345: case 347: case 349: case 351: case 353: case 355:
    case 357: case 359: case 361: case 363: case 365: case 367:
    case 369: case 371: case 373: case 375: case 378: case 380:
    case 382: case 387: case 389: case 392: case 396: case 402:
    case 409: case 417: case 419: case 421: case 424: case 429:
    case 432: case 436: case 438: case 441: case 445: case 453:
    case 456: case 459: case 462: case 464: case 466: case 468:
    case 470: case 472: case 474: case 476: case 479: case 481:
    case 483: case 485: case 487: case 489: case 491: case 493:
    case 495: case 498: case 501: case 507: case 509: case 511:
    case 513: case 515: case 517: case 519: case 521: case 523:
    case 525: case 527: case 529: case 531: case 533: case 535:
    case 995: case 997: case 999: case 1001: case 1003: case 1005:
    case 1007: case 1121: case 1123: case 1125: case 1127: case 1129:
    case 1131: case 1133: case 1135: case 1137: case 1139: case 1141:
    case 1143: case 1145: case 1147: case 1149: case 1151: case 1153:
    case 1169: case 1171: case 1173: case 1175: case 1177: case 1179:
    case 1181: case 1183: case 1185: case 1187: case 1189: case 1191:
    case 1193: case 1195: case 1197: case 1199: case 1201: case 1203:
    case 1205: case 1207: case 1209: case 1211: case 1213: case 1215:
    case 1218: case 1220: case 1224: case 1228: case 1233: case 1235:
    case 1237: case 1239: case 1241: case 1243: case 1245: case 1247:
    case 1249: case 1251: case 1253: case 1255: case 1257: case 1259:
    case 1263: case 1265: case 1267: case 1269: case 1273: case 7681:
    case 7683: case 7685: case 7687: case 7689: case 7691: case 7693:
    case 7695: case 7697: case 7699: case 7701: case 7703: case 7705:
    case 7707: case 7709: case 7711: case 7713: case 7715: case 7717:
    case 7719: case 7721: case 7723: case 7725: case 7727: case 7729:
    case 7731: case 7733: case 7735: case 7737: case 7739: case 7741:
    case 7743: case 7745: case 7747: case 7749: case 7751: case 7753:
    case 7755: case 7757: case 7759: case 7761: case 7763: case 7765:
    case 7767: case 7769: case 7771: case 7773: case 7775: case 7777:
    case 7779: case 7781: case 7783: case 7785: case 7787: case 7789:
    case 7791: case 7793: case 7795: case 7797: case 7799: case 7801:
    case 7803: case 7805: case 7807: case 7809: case 7811: case 7813:
    case 7815: case 7817: case 7819: case 7821: case 7823: case 7825:
    case 7827: case 7829: case 7841: case 7843: case 7845: case 7847:
    case 7849: case 7851: case 7853: case 7855: case 7857: case 7859:
    case 7861: case 7863: case 7865: case 7867: case 7869: case 7871:
    case 7873: case 7875: case 7877: case 7879: case 7881: case 7883:
    case 7885: case 7887: case 7889: case 7891: case 7893: case 7895:
    case 7897: case 7899: case 7901: case 7903: case 7905: case 7907:
    case 7909: case 7911: case 7913: case 7915: case 7917: case 7919:
    case 7921: case 7923: case 7925: case 7927: case 7929:
      return c + -1;
    case 305:
      return c + -232;
    case 383:
      return c + -300;
    case 454:
    case 457:
    case 460:
    case 499:
      return c + -2;
    case 477:
    case 1010:
      return c + -79;
    case 595:
      return c + -210;
    case 596:
      return c + -206;
    case 601:
      return c + -202;
    case 603:
      return c + -203;
    case 608:
      return c + -205;
    case 611:
      return c + -207;
    case 616:
      return c + -209;
    case 617:
    case 623:
      return c + -211;
    case 626:
      return c + -213;
    case 629:
      return c + -214;
    case 640:
    case 643:
    case 648:
      return c + -218;
    case 658:
      return c + -219;
    case 837:
      return c + 84;
    case 940:
      return c + -38;
    case 962:
      return c + -31;
    case 972:
      return c + -64;
    case 976:
      return c + -62;
    case 977:
      return c + -57;
    case 981:
      return c + -47;
    case 982:
      return c + -54;
    case 1008:
      return c + -86;
    case 1009:
      return c + -80;
    case 7835:
      return c + -59;
    case 8017:
    case 8019:
    case 8021:
    case 8023:
      return c + 8;
    case 8115:
    case 8131:
    case 8179:
      return c + 9;
    case 8126:
      return c + -7205;
    case 8165:
      return c + 7;
    default:
      return c;
  }
}

///////////////////////////////////////////////
niExportFunc(tBool) StrIsSpace(tI32 c)
{
  return ((c == 32) || (c ==  '\t') || (c == '\r') ||
          (c == '\n') || (c == '\f') || (c == '\v') ||
          (c == 0x1680) || ((c >= 0x2000) && (c <= 0x200A)) ||
          (c == 0x2028) || (c == 0x202f) || (c == 0x3000));
}

///////////////////////////////////////////////
niExportFunc(tBool) StrIsDigit(tI32 c) {
  return ((c >= '0') && (c <= '9'));
}
niExportFunc(tBool) StrIsXDigit(tI32 c)
{
  return ((c >= '0') && (c <= '9')) ||
      ((c >= 'A') && (c <= 'F')) ||
      ((c >= 'a') && (c <= 'f'));
}

///////////////////////////////////////////////
niExportFunc(tBool) StrIsControl(tI32 c) {
  return (c >= 0x00 && c <= 0x1F) || (c == 0x7F);
}

///////////////////////////////////////////////
niExportFunc(tBool) StrIsLetter(tI32 c) {
  return (
      // Uppercase
      (c >= 0x0041 && c <= 0x005A)    // upper case latin (A-Z)
      || (c >= 0x0400 && c <= 0x042F) // upper case cyrillic
      || (c >= 0x0391 && c <= 0x03A9) // upper case greek (ALPHA-OMEGA)
      // Lowercase
      || (c >= 0x0061 && c <= 0x007A) // lower case latin (a-z)
      || (c >= 0x0430 && c <= 0x044F) // lower case cyrillic
      || (c >= 0x03B1 && c <= 0x03C9) // lower case greek (alpha-omega)
      // Title cases
      || (c == 0x01C5) // LATIN CAPITAL LETTER D WITH SMALL LETTER Z WITH CARON
      || (c == 0x1FFC) // GREEK CAPITAL LETTER OMEGA WITH PROSGEGRAMMENI
      // Modifiers
      || (c >= 0x02B0 && c <= 0x02C1) // MODIFIER LETTER SMALL H - MODIFIER LETTER REVERSED GLOTTAL STOP
      || (c >= 0x1D2C && c <= 0x1D61) // MODIFIER LETTER CAPITAL A - MODIFIER LETTER SMALL CHI
      // Other letters
      || (c >= 0x3041 && c <= 0x3094) // hiragana
      || (c >= 0x30A1 && c <= 0x30FA) // katakana
      || (c >= 0x05D0 && c <= 0x05EA) // hebrew (Alef - Tav)
      || (c >= 0x0621 && c <= 0x063A) // arabic (Hamza - Ghain)
      || (c >= 0x4E00 && c <= 0x9FC3) // cjk
          );
}

///////////////////////////////////////////////
niExportFunc(tBool) StrIsLetterDigit(tI32 c) {
  return StrIsLetter(c) || StrIsDigit(c);
}

///////////////////////////////////////////////
niExportFunc(tBool) StrIsAlpha(tI32 c)
{
  return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'));
}

///////////////////////////////////////////////
niExportFunc(tBool) StrIsAlNum(tI32 c)
{
  return StrIsDigit(c) || StrIsAlpha(c);
}

///////////////////////////////////////////////
niExportFunc(tI32) StrSize(const  achar* _s)
{
  if (!_s) return 0;
  const achar* s = _s;
  const achar* orig = s;
  const achar* last;
  do {
    last = s;
  } while (STRLIB_GETXC(&s)   != 0);
  return (tI32)((tIntPtr)last - (tIntPtr)orig);
}

///////////////////////////////////////////////
niExportFunc(tI32) StrSizeZ(const achar* _s)
{
  if (!_s) return 0;
  const achar* s = _s;
  const achar* orig = s;
  niAssert(s);
  do {
  } while (STRLIB_GETXC(&s)   != 0);
  return tI32((tIntPtr)s - (tIntPtr)orig);
}

///////////////////////////////////////////////
niExportFunc(achar*) StrZCpy(achar* dest, tI32 size, const achar* src)
{
  if (!dest || size < 1) return NULL;
  if (!src) return NULL;

  tI32 pos = 0;
  tI32 c;
  niAssert(dest);
  niAssert(src);
  niAssert(size > 0);

  size -= STRLIB_UCWIDTH(0);
  niAssert(size >= 0);

  while ((c = STRLIB_GETXC(&src)) != 0)   {
    size -= STRLIB_UCWIDTH(c);
    if (size < 0)
      break;

    pos += STRLIB_SETC(dest+pos, c);
  }

  STRLIB_SETC(dest+pos,   0);

  return dest;
}

///////////////////////////////////////////////
niExportFunc(achar*) StrZCat(achar* dest, tI32    size, const achar* src)
{
  if (!dest || size < 1) return NULL;
  if (!src) return NULL;

  tI32 pos;
  tI32 c;
  pos = StrSize(dest);
  size -= pos + STRLIB_UCWIDTH(0);
  niAssert(size >= 0);
  while ((c = STRLIB_GETXC(&src)) != 0)   {
    size -= STRLIB_UCWIDTH(c);
    if (size < 0)
      break;

    pos += STRLIB_SETC(dest+pos, c);
  }

  STRLIB_SETC(dest+pos,   0);
  return dest;
}

///////////////////////////////////////////////
niExportFunc(tI32) StrLen(const achar* s)
{
  if (!s) return 0;
  tI32 c = 0;
  while (STRLIB_GETXC(&s))
    c++;
  return c;
}

///////////////////////////////////////////////
niExportFunc(tI32) StrZLength(const achar* s, tU32 anSize)
{
  if (!s) return 0;
  tI32 c = 0;
  const achar* const b = s;
  while (STRLIB_GETXC(&s) && ((s-b) < (tI32)anSize)) {
    c++;
  }
  return c;
}

///////////////////////////////////////////////
niExportFunc(tI32) StrCmp(const achar* s1, const achar* s2)
{
  if (s1 == s2) return 0;
  if (!s1 || !s2) return -1;
  tI32 c1, c2;
  for (;;) {
    c1 = STRLIB_GETXC(&s1);
    c2 = STRLIB_GETXC(&s2);
    if (c1 != c2)
      return c1 - c2;
    if (!c1)
      return 0;
  }
}

///////////////////////////////////////////////
niExportFunc(tI32) StrNCmp(const  achar* s1, const  achar* s2, tSize n)
{
  if (s1 == s2) return 0;
  if (!s1 || !s2) return -1;

  tI32 c1, c2;
  if (n <= 0)
    return 0;

  for (;;) {
    c1 = STRLIB_GETXC(&s1);
    c2 = STRLIB_GETXC(&s2);

    if (c1 != c2)
      return c1 - c2;

    if ((!c1) || (--n <= 0))
      return 0;
  }
}

///////////////////////////////////////////////
niExportFunc(tI32) StrICmp(const  achar* s1, const  achar* s2)
{
  if (s1 == s2) return 0;
  if (!s1 || !s2) return -1;

  tI32 c1, c2;
  for (;;) {
    c1 = StrToLower(STRLIB_GETXC(&s1));
    c2 = StrToLower(STRLIB_GETXC(&s2));

    if (c1 != c2)
      return c1 - c2;

    if (!c1)
      return 0;
  }
}

///////////////////////////////////////////////
niExportFunc(tI32) StrNICmp(const achar* s1, const achar* s2, tSize   n)
{
  if (s1 == s2) return 0;
  if (!s1 || !s2) return -1;

  tI32 c1, c2;
  if (n <= 0)
    return 0;

  for (;;) {
    c1 = StrToLower(STRLIB_GETXC(&s1));
    c2 = StrToLower(STRLIB_GETXC(&s2));

    if (c1 != c2)
      return c1 - c2;

    if ((!c1) || (--n <= 0))
      return 0;
  }
}

///////////////////////////////////////////////
niExportFunc(tI32) StrZCmp(const achar* s1, const achar* s2, tSize anSize)
{
  if (s1 == s2) return 0;
  if (!s1 || !s2) return -1;

  tI32 c1, c2;
  const achar* s2base = s2;

  for (;;) {
    c1 = STRLIB_GETXC(&s1);
    c2 = STRLIB_GETXC(&s2);
    if (c1 != c2)
      return c1 - c2;
    if (!c1)
      return 0;
    if ((tU32)(s2-s2base) >= anSize)
      return c1 - c2;
  }
}

///////////////////////////////////////////////
niExportFunc(tI32) StrZICmp(const achar* s1, const achar* s2, tSize anSize)
{
  if (s1 == s2) return 0;
  if (!s1 || !s2) return -1;

  tI32 c1, c2;
  const achar* s2base = s2;

  for (;;) {
    c1 = StrToLower(STRLIB_GETXC(&s1));
    c2 = StrToLower(STRLIB_GETXC(&s2));
    if (c1 != c2)
      return c1 - c2;
    if (!c1)
      return 0;
    if ((tU32)(s2-s2base) >= anSize)
      return c1 - c2;
  }
}

///////////////////////////////////////////////
niExportFunc(achar*) StrZNCpy(achar* dest, tI32 size, const achar* src,  tSize n)
{
  if (!dest || size < 1) return NULL;
  if (!src) return NULL;

  tI32 pos = 0;
  tSize len = 0;
  tI32 c;
  niAssert(dest);
  niAssert(src);
  niAssert(size > 0);

  size -= STRLIB_UCWIDTH(0);
  niAssert(size >= 0);

  // copy at most n-1 characters
  while (((c = STRLIB_GETXC(&src)) != 0) && (len < (n-1))) {
    size -= STRLIB_UCWIDTH(c);
    if (size<0)
      break;

    pos += STRLIB_SETC(dest+pos, c);
    len++;
  }

  // add the null character
  STRLIB_SETC(dest+pos,   0);

  return dest;
}

///////////////////////////////////////////////
niExportFunc(achar*) StrZNCat(achar* dest,    tI32 size, const achar* src,  tSize n)
{
  if (!dest || size < 1) return NULL;
  if (!src) return NULL;

  tI32 pos;
  tSize len = 0;
  tI32 c;
  niAssert(dest);
  niAssert(src);
  niAssert(size >= 0);

  pos = StrSize(dest);
  size -= pos + STRLIB_UCWIDTH(0);

  while (((c = STRLIB_GETXC(&src)) != 0) &&   (len < n)) {
    size -= STRLIB_UCWIDTH(c);
    if (size<0)
      break;

    pos += STRLIB_SETC(dest+pos, c);
    len++;
  }

  STRLIB_SETC(dest+pos,   0);

  return dest;
}

///////////////////////////////////////////////
niExportFunc(achar*) StrLwr(achar* s)
{
  if (!s) return NULL;

  tI32 pos = 0;
  tI32 c, lc;
  while ((c = STRLIB_GETC(s+pos)) != 0)   {
    lc = StrToLower(c);

    if (lc != c)
      StrSetAt( s+pos, 0, lc);

    pos += STRLIB_UWIDTH(s+pos);
  }

  return s;
}

///////////////////////////////////////////////
niExportFunc(achar*) StrUpr(achar* s)
{
  if (!s) return NULL;

  tI32 pos = 0;
  tI32 c, uc;
  while ((c = STRLIB_GETC(s+pos)) != 0)   {
    uc = StrToUpper(c);

    if (uc != c)
      StrSetAt( s+pos, 0, uc);

    pos += STRLIB_UWIDTH(s+pos);
  }

  return s;
}

///////////////////////////////////////////////
niExportFunc(const achar*) StrChr(const achar* s, tI32 c)
{
  if (!s) return NULL;

  tI32 d;
  while ((d = STRLIB_GETC(s)) != 0)   {
    if (c == d)
      return (achar* )s;

    s += STRLIB_UWIDTH(s);
  }

  if (!c)
    return s;

  return NULL;
}

///////////////////////////////////////////////
niExportFunc(const achar*) StrRChr(const achar* s, tI32 c)
{
  if (!s) return NULL;

  const achar* last_match = NULL;
  tI32 c1, pos = 0;
  for (c1 = STRLIB_GETC(s); c1; c1 = STRLIB_GETC(s+pos)) {
    if (c1 == c)
      last_match = s + pos;

    pos += STRLIB_UCWIDTH(c1);
  }

  return last_match;
}

///////////////////////////////////////////////
niExportFunc(const achar*) StrStr(const achar* s1, const achar* s2)
{
  if (!s1 || !s2) return NULL;

  tI32 len = StrLen(s2);
  while (STRLIB_GETC(s1)) {
    if (StrNCmp(s1, s2, len) == 0)
      return (achar* )s1;

    s1 += STRLIB_UWIDTH(s1);
  }

  return NULL;
}

///////////////////////////////////////////////
niExportFunc(const achar*) StrRStr(const achar* aaszStr, const achar* aaszSubStr)
{
  // StrSize below takes care of NULL ptr checks
  const achar* plast;
  tSize strLen, substrLen;
  strLen = StrSize(aaszStr);
  if (strLen < 1) return NULL;
  substrLen = StrSize(aaszSubStr);
  if (substrLen < 1) return NULL;
  plast = &aaszStr[strLen-1];
  while (plast >= aaszStr) {
    if (*plast == *aaszSubStr) {
      if (substrLen == 1 || StrZCmp(plast,aaszSubStr,substrLen) == 0)
        return plast;
    }
    --plast;
  }
  return NULL;
}

///////////////////////////////////////////////
niExportFunc(const achar*) StrPBrk(const achar* s, const   achar* set)
{
  if (!s || !set) return NULL;

  const achar* setp;
  tI32 c, d;
  while ((c = STRLIB_GETC(s)) != 0)   {
    setp = set;

    while ((d = STRLIB_GETXC(&setp)) != 0) {
      if (c == d)
        return (achar* )s;
    }

    s += STRLIB_UWIDTH(s);
  }

  return NULL;
}

///////////////////////////////////////////////
niExportFunc(achar*) StrTok(achar* s, const achar* set, achar** last)
{
  if (!set || !last)
    return NULL;

  achar* prev_str;
  achar* tok;
  const achar* setp;
  tI32 c, sc;
  if (!s) {
    s = *last;
    if (!s)
      return NULL;
  }

skip_leading_delimiters:

  prev_str = s;
  c = STRLIB_GETX(&s);

  setp = set;

  while ((sc = STRLIB_GETXC(&setp))   != 0) {
    if (c == sc)
      goto skip_leading_delimiters;
  }

  if (!c) {
    *last = NULL;
    return NULL;
  }

  tok = prev_str;

  for (;;) {
    prev_str = s;
    c = STRLIB_GETX(&s);

    setp = set;

    do {
      sc = STRLIB_GETXC(&setp);
      if (sc == c) {
        if (!c) {
          *last = NULL;
          return tok;
        }
        else {
          s += StrSetAt( prev_str, 0, 0);
          *last = s;
          return tok;
        }
      }
    } while (sc);
  }
}

niExportFunc(achar*) StrSep(achar** appString, const achar *aaszDelim)
{
  achar *s;
  const achar *spanp;
  int c, sc;
  achar *tok;

  niAssert(appString != NULL);
  niAssert(aaszDelim != NULL);

  s = *appString;
  if (s == NULL) {
    return NULL;
  }
  for (tok = s;;) {
    c = *s++;
    spanp = aaszDelim;
    do {
      if ((sc = *spanp++) == c) {
        if (c == 0)
          s = NULL;
        else
          s[-1] = 0;
        *appString = s;
        return (tok);
      }
    } while (sc != 0);
  }
}

niExportFunc(achar*) StrSepQuoted(achar** appString, const achar *aaszDelim, const tU32 aQuote)
{
  achar *s;
  const achar *spanp;
  int c = 0, sc, pc = 0;
  achar *tok;

  niAssert(appString != NULL);
  niAssert(aaszDelim != NULL);

  s = *appString;
  if (s == NULL) {
    return NULL;
  }

  // niDebugFmt(("... StrSepQuoted: '%s'", s));

  bool inQuote = false;
  for (tok = s;;) {
    pc = c;
    c = *s;
    if (c == 0) {
      *appString = nullptr;
      return (tok);
    }
    else {
      ++s;
    }

    if (inQuote) {
      if (c == aQuote) {
        if (*s == aQuote) {
          c = *s++;
        }
        else if (pc == '\\') {
          // just skip
        }
        else {
          inQuote = false;
          // niDebugFmt(("... StrSepQuoted END INQUOTE: '%s', c: '%c', pc: '%c', nc: '%c'", s, c, pc, (int)*s));
        }
      }
      else if (c == 0) {
        s[-1] = 0;
        *appString = s;
        // niDebugFmt(("... StrSepQuoted RET: '%s'", tok));
        return (tok);
      }
    } else if (c == aQuote) {
      inQuote = true;
    } else {
      spanp = aaszDelim;
      do {
        if ((sc = *spanp++) == c) {
          s[-1] = 0;
          *appString = s;
          // niDebugFmt(("... StrSepQuoted RET: '%s'", tok));
          return (tok);
        }
      } while (sc != 0);
    }
  }
}

//--------------------------------------------------------------------------------------------
//
//  Unicode-aware strtol/ul/d functions
//
//--------------------------------------------------------------------------------------------

static const char kDigitCharactersMirrored[19] = {
  '9','8','7','6','5','4','3','2','1','0','1','2','3','4','5','6','7','8','9'
};
static const char* kDigitCharacters = kDigitCharactersMirrored + 9;

niExportFunc(char*) StringUIntToStr(char* apBuffer, ni::tSize anBufferSize, tU64 i)
{
  char *p = apBuffer + anBufferSize - 1;
  *p = 0;

  do {
    tU64 lsd = i % 10;
    i = i / 10;
    --p;
    *p = kDigitCharacters[lsd];
  } while(i != 0);

  niAssert(!(p < apBuffer));
  return p;
}

niExportFunc(char*) StringIntToStr(char* apBuffer, tSize anBufferSize, tI64 i)
{
  char* p = apBuffer + anBufferSize - 1;
  *p = 0;
  if (i < 0) {
    do {
      tI64 lsd = i % 10;
      i = i / 10;
      --p;
      *p = kDigitCharacters[lsd];
    } while(i != 0);
    *(--p) = '-';
  }
  else {
    do {
      tI64 lsd = i % 10;
      i = i / 10;
      --p;
      *p = kDigitCharacters[lsd];
    } while(i != 0);
  }

  niAssert(!(p < apBuffer));
  return p;
}

#define FL_UNSIGNED   1
#define FL_NEG        2
#define FL_OVERFLOW   4
#define FL_READDIGIT  8

///////////////////////////////////////////////
template <typename TS, typename TU>
static TU internal_strtoxl(const char *nptr, const char **endptr, int ibase, int flags) {
  static const TU kUMaxVal = ni::TypeMax<TU>();
  //     static const TU kUMinVal = ni::TypeMin<TU>();
  static const TS kSMaxVal = ni::TypeMax<TS>();
  static const TS kSMinVal = ni::TypeMin<TS>();
  if (!nptr)
    return (flags & FL_UNSIGNED) ? kUMaxVal : kSMaxVal;

  const char *p;
  char c;
  TU number;
  TU digval;
  TU maxval;

  p = nptr;
  number = 0;

  c = *p++;
  while (StrIsSpace(c))
    c = *p++;

  if (c == '-')  {
    flags |= FL_NEG;
    c = *p++;
  }
  else if (c == '+')
    c = *p++;

  if (ibase < 0 || ibase == 1 || ibase > 36)  {
    if (endptr) *endptr = nptr;
    return 0L;
  }
  else if (ibase == 0) {
    if (c != '0')
      ibase = 10;
    else if (*p == 'x' || *p == 'X')
      ibase = 16;
    else
      ibase = 8;
  }

  if (ibase == 16) {
    if (c == '0' && (*p == 'x' || *p == 'X')) {
      ++p;
      c = *p++;
    }
  }

  maxval = kUMaxVal / ibase;
  for (;;)  {
    if (StrIsDigit(c)) {
      digval = c - '0';
    }
    else if (StrIsAlpha(c)) {
      // handle letters, mainly for hexadecimal...
      // however the whole alpha can be used to go up to a base 36
      digval = StrToUpper(c) - 'A' + 10;
    }
    else
      break;

    if (digval >= (TU)ibase)
      break;

    flags |= FL_READDIGIT;

    if (number < maxval || (number == maxval && (digval <= kUMaxVal % ibase)))
      number = number * ibase + digval;
    else
      flags |= FL_OVERFLOW;

    c = *p++;
  }

  --p;

  if (!(flags & FL_READDIGIT)) {
    if (endptr) p = nptr;
    number = 0L;
  }
  else if ((flags & FL_OVERFLOW)
           || (!(flags & FL_UNSIGNED)
               && (((flags & FL_NEG) && (number > (TU)-kSMinVal))
                   || (!(flags & FL_NEG) && (number > (TU)kSMaxVal)))))
  {
    if (flags & FL_UNSIGNED)
      number = kUMaxVal;
    else if (flags & FL_NEG)
      number = (TU)(-kSMinVal);
    else
      number = kSMaxVal;
  }

  if (endptr != NULL) *endptr = p;

  if (flags & FL_NEG) number = (TU)(-(TS)number);

  return number;
}

///////////////////////////////////////////////
niExportFunc(tI64) StrToL(const achar* s, const achar** endp, tI32 base) {
  return (tI64)internal_strtoxl<tI64,tU64>(s, endp, base, 0);
}
niExportFunc(tI64) StrAToL(const achar* s) {
  return StrToL(s,NULL,10);
}

///////////////////////////////////////////////
niExportFunc(tU64) StrToUL(const achar* s, const achar** endp, tI32 base) {
  return internal_strtoxl<tI64,tU64>(s, endp, base, FL_UNSIGNED);
}
niExportFunc(tU64) StrAToUL(const achar* s) {
  return StrToUL(s,NULL,10);
}

#if niMinFeatures(20)

///////////////////////////////////////////////
niExportFunc(tF64) StrToD(const achar* s, const achar** endp)
{
  tInt charCount = eInvalidHandle;
  tF64 r = ni::StringToDouble(s,0,&charCount);
  if (endp) *endp = s + charCount;
  return r;
}

///////////////////////////////////////////////
niExportFunc(tF64) StrAToF(const achar* s)
{
  return StrToD(s,NULL);
}

#else

///////////////////////////////////////////////
template <typename TF>
static TF internal_strtod(const char *str, const char **endptr)
{
  //     static const TF kFMinVal = ni::TypeMin<TF>();
  static const TF kFMaxVal = ni::TypeMax<TF>();
  static const TF kFMinExp = ni::TypeMinExp<TF>();
  static const TF kFMaxExp = ni::TypeMaxExp<TF>();
  if (!str) return kFMaxVal;

  TF number;
  int exponent;
  int negative;
  char *p = (char *) str;
  TF p10;
  int n;
  int num_digits;
  int num_decimals;

  // Skip leading whitespace
  while (StrIsSpace(*p)) p++;

  // Handle optional sign
  negative = 0;
  switch (*p) {
    case '-': negative = 1; // Fall through to increment position
    case '+': p++;
  }

  number = 0.;
  exponent = 0;
  num_digits = 0;
  num_decimals = 0;

  // Process string of digits
  while (StrIsDigit(*p)) {
    number = number * 10. + (*p - '0');
    p++;
    num_digits++;
  }

  // Process decimal part
  if (*p == '.') {
    p++;
    while (StrIsDigit(*p)) {
      number = number * 10. + (*p - '0');
      p++;
      num_digits++;
      num_decimals++;
    }
    exponent -= num_decimals;
  }

  if (num_digits == 0) {
    return 0.0;
  }

  // Correct for sign
  if (negative) number = -number;

  // Process an exponent string
  if (*p == 'e' || *p == 'E') {
    // Handle optional sign
    negative = 0;
    switch (*++p) {
      case '-': negative = 1;   // Fall through to increment pos
      case '+': p++;
    }

    // Process string of digits
    n = 0;
    while (StrIsDigit(*p)) {
      n = n * 10 + (*p - '0');
      p++;
    }

    if (negative)
      exponent -= n;
    else
      exponent += n;
  }

  if (exponent < kFMinExp || exponent > kFMaxExp) {
    return kFMaxVal;
  }

  // Scale the result
  p10 = 10.;
  n = exponent;
  if (n < 0) n = -n;
  while (n) {
    if (n & 1) {
      if (exponent < 0)
        number /= p10;
      else
        number *= p10;
    }
    n >>= 1;
    p10 *= p10;
  }

  if (endptr) *endptr = p;
  return number;
}

niExportFunc(tF64) StrToD(const achar* s, const achar** endp)
{
  return internal_strtod<tF64>(s,endp);
}

///////////////////////////////////////////////
niExportFunc(tF64) StrAToF(const achar* s)
{
  return StrToD(s,NULL);
}

#endif

#undef FL_UNSIGNED
#undef FL_NEG
#undef FL_OVERFLOW
#undef FL_READDIGIT

//--------------------------------------------------------------------------------------------
//
//  String slicing functions
//
//--------------------------------------------------------------------------------------------

///////////////////////////////////////////////
niExportFunc(achar*) StrZSubstr(achar* apOut, const achar* aStr, tU32 anStrSz, tI32 anStart, tU32 size) {
  if (!apOut || !aStr) return NULL;
  if (!anStrSz) anStrSz = StrSize(aStr);
  anStart = StrMapIndex(anStrSz,anStart);
  if ((tU32)(anStart) >= anStrSz) {
    return StrZero(apOut);
  }
  if ((tU32)(anStart+size) >= anStrSz) {
    size = anStrSz-anStart;
  }
  return StrZCpy(apOut,size,aStr+anStart);
}

///////////////////////////////////////////////
// get a string between [start,end[ (end index not included)
niExportFunc(achar*) StrZSlice(achar* apOut, const achar* aStr, tU32 anStrSz, tI32 start, tI32 end)
{
  if (!apOut || !aStr) return NULL;
  if (!anStrSz) anStrSz = StrSize(aStr);
  start = StrMapIndex(anStrSz,start);
  end = StrMapIndex(anStrSz,end);
  if (end < start) {
    return StrZero(apOut);
  }
  return StrZSubstr(apOut,aStr,anStrSz,start,end-start);
}

///////////////////////////////////////////////
niExportFunc(achar*) StrZMid(achar* apOut, const achar* aStr, tU32 anStrSz, tU32 anFirst, tU32 size) {
  return StrZSubstr(apOut,aStr,anStrSz,anFirst,size);
}

///////////////////////////////////////////////
niExportFunc(achar*) StrZLeft(achar* apOut, const achar* aStr, tU32 anStrSz, tU32 anSize) {
  return StrZMid(apOut,aStr,anStrSz,0,anSize);
}

///////////////////////////////////////////////
niExportFunc(achar*) StrZRight(achar* apOut, const achar* aStr, tU32 anStrSz, tU32 anSize) {
  if (!apOut || !aStr) return NULL;
  if (!anStrSz) anStrSz = StrSize(aStr);
  const achar* b = aStr+anStrSz;
  const achar* e = aStr;
  anSize = (anSize < (tU32)((b-e)+1)) ? anSize : (tU32)((b-e)+1);
  if (anSize) {
    achar* d = apOut+(anSize-1);
    apOut[anSize] = 0;
    while (anSize--) {
      *d = *b;
      --d;
      --b;
    }
  }
  return apOut;
}

///////////////////////////////////////////////
static tI32 _StrFwdFind(tBool abICmp, const achar* aStr, tU32 anStrSz, const achar* aToFind, tU32 anToFindSz, tU32 aStartOffset) {
  const achar* pBuf = aStr;
  if (!pBuf || !aToFind)
    return -1;

  if (!anStrSz)
    anStrSz = StrSize(aStr);
  if (!anToFindSz)
    anToFindSz = StrSize(aToFind);
  if (anToFindSz > anStrSz || !anToFindSz)
    return -1;

  if (aStartOffset > anStrSz)
    return -1;

  pBuf += aStartOffset;
  {
    const achar* p = pBuf;
    const achar* e = pBuf+anStrSz;
    while (*p) {
      if (p+anToFindSz > e)
        break;
      if (abICmp ?
          StrZICmp(p,aToFind,anToFindSz) == 0 :
          StrZCmp(p,aToFind,anToFindSz) == 0)
        return (p - pBuf)+aStartOffset;
      STRLIB_GETX(&p);
    }
  }
  return -1;
}
static tI32 _StrRevFind(tBool abICmp, const achar* aStr, tU32 anStrSz, const achar* aToFind, tU32 anToFindSz, tI32 aStart)
{
  const achar* pBuf = aStr;
  if (!pBuf || !aToFind)
    return -1;

  if (!anStrSz)
    anStrSz = StrSize(aStr);
  if (!anToFindSz)
    anToFindSz = StrSize(aToFind);
  if (anToFindSz > anStrSz || !anToFindSz)
    return -1;

  if (aStart > (tI32)anStrSz)
    aStart = (tI32)anStrSz;
  aStart -= anToFindSz;
  const achar* pEnd = aStr+anStrSz;

  while (aStart >= 0) {
    const char* startP = pBuf+aStart;
    if (utf8::validate_next(startP, pEnd) == eUTFError_OK) {
      if (abICmp ?
          StrZICmp(pBuf+aStart, aToFind, anToFindSz) == 0 :
          StrZCmp(pBuf+aStart, aToFind, anToFindSz) == 0)
        return aStart;
    }
    --aStart;
  }

  return -1;
}

niExportFunc(tI32) StrZFindEx(tU32 aFlags, const achar* aStr, tU32 anStrSz, const achar* aToFind, tU32 anToFindSz, tI32 aStart) {
  return
      (aFlags&eStrFindFlags_Reversed) ?
      _StrRevFind((aFlags&eStrFindFlags_ICmp),aStr,anStrSz,aToFind,anToFindSz,
                  (aStart==eInvalidHandle)?0x7fffffff:aStart) :
      _StrFwdFind((aFlags&eStrFindFlags_ICmp),aStr,anStrSz,aToFind,anToFindSz,
                  (aStart==eInvalidHandle)?0:aStart);
}

///////////////////////////////////////////////
niExportFunc(tI32) StrZRFindChar(const achar* aStr, tU32 anStrSz, const tU32 aChar, tI32 anStart)
{
  achar tmp[8]; achar* p = tmp;
  const tU32 sz = _SetC(p,aChar); p += sz;
  _SetC(p,0);
  return StrZRFind(aStr,anStrSz,tmp,sz,anStart);
}

///////////////////////////////////////////////
niExportFunc(tI32) StrZFindChar(const achar* aStr, tU32 anStrSz, const tU32 aChar, tI32 anStart)
{
  achar tmp[8]; achar* p = tmp;
  const tU32 sz = _SetC(p,aChar); p += sz;
  _SetC(p,0);
  return StrZFind(aStr,anStrSz,tmp,sz,anStart);
}

///////////////////////////////////////////////
niExportFunc(tI32) StrZAfterPos(tU32 aFlags, const achar* aStr, tU32 anStrSz, const achar* aToFind, tU32 anToFindSz)
{
  if (!anStrSz) anStrSz = StrSize(aStr);
  if (!anToFindSz) anToFindSz = StrSize(aToFind);
  tI32 pos = StrZFindEx(aFlags,aStr,anStrSz,aToFind,anToFindSz,eInvalidHandle);
  if (pos == -1) {
    return -1;
  }
  pos += anToFindSz;
  if ((tU32)pos > anStrSz-1) {
    return -1;
  }
  return pos;
}
niExportFunc(achar*) StrZAfterEx(const tU32 aFlags, achar* apOut, const achar* aStr, tU32 anStrSz, const achar* aToFind, tU32 anToFindSz)
{
  if (!anStrSz) anStrSz = StrSize(aStr);
  tI32 pos = StrZAfterPos(aFlags,aStr,anStrSz,aToFind,anToFindSz);
  if (pos == -1) return StrZero(apOut);
  return StrZRight(apOut,aStr,anStrSz,anStrSz-pos);
}

///////////////////////////////////////////////
niExportFunc(tI32) StrZBeforePos(const tU32 aFlags, const achar* aStr, tU32 anStrSz, const achar* aToFind, tU32 anToFindSz) {
  if (!anStrSz) anStrSz = StrSize(aStr);
  if (!anToFindSz) anToFindSz = StrSize(aToFind);
  tI32 pos = StrZFindEx(aFlags,aStr,anStrSz,aToFind,anToFindSz,eInvalidHandle);
  if (pos == 0 || pos == -1) {
    return -1;
  }
  return pos;
}
niExportFunc(achar*) StrZBeforeEx(const tU32 aFlags, achar* apOut, const achar* aStr, tU32 anStrSz, const achar* aToFind, tU32 anToFindSz) {
  tI32 pos = StrZBeforePos(aFlags,aStr,anStrSz,aToFind,anToFindSz);
  if (pos == 0 || pos == -1) {
    return StrZero(apOut);
  }
  return StrZLeft(apOut,aStr,anStrSz,pos);
}

///////////////////////////////////////////////
niExportFunc(achar*) StrPutPathSep(achar *filename) {
  if (!filename) return NULL;
  tSize i = StrSize(filename);
  if ((i <= 0) || StrIsPathSep(filename[i-1]))
    return filename;
  filename[i++] = '/';
  filename[i] = 0;
  return filename;
}

niExportFunc(achar*) StrMakeStdPath(achar* path) {
  if (!path) return NULL;
  cchar* p = path;
  if (p[0] == '\\' && p[1] == '\\') {
    // windows samba path, keep as-is
    p += 2;
  }
  if (StrIsAlpha(p[0]) && StrStartsWithDriveSep(p+1)) {
    // Our standard path sets the drive letter in lowercase
    p[0] = StrToLower(p[0]);
  }
  while (*p) {
    tU32 c = _GetC(p);
    if (StrIsPathSep(c)) {
      _SetC(p,'/');
    }
    _GetX(&p);
  }
  return path;
}

niExportFunc(achar*) StrFixPath(achar* dest, const achar* path, int size) {
  if (!dest || !path)
    return NULL;
  dest[0] = '\0';
  StrZCat(dest, size-1, path);
  if (!dest[0]) {
    StrZCat(dest, size-1, "./");
    return dest;
  }
  else {
    StrPutPathSep(dest);
    return StrMakeStdPath(dest);
  }
}

///////////////////////////////////////////////
niExportFunc(tBool) StrIsAbsolutePath(const achar* path) {
  if (!path || !*path)
    return eFalse;

  // Path starts with path separator?
  if (StrIsPathSep(*path))
    return eTrue;

  // Drive separator
  return StrStartsWithDriveSep(path+1);
}

///////////////////////////////////////////////
niExportFunc(tBool) StrIsInt(const achar* cszStr) {
  if (!cszStr) return NULL;
  const achar* p = cszStr;
  while (*p) {
    if (*p == '.')
      return niFalse;
    ++p;
  }
  return niTrue;
}

///////////////////////////////////////////////
niExportFunc(tBool) StrStartsWith(const achar* aaszA, const achar* aaszB) {
  tU32 bSize = StrSize(aaszB);
  // if (!bSize) return eFalse;
  return StrZCmp(aaszA,aaszB,bSize) == 0;
}

niExportFunc(tBool) StrStartsWithI(const achar* aaszA, const achar* aaszB) {
  tU32 bSize = StrSize(aaszB);
  // if (!bSize) return eFalse;
  return StrZICmp(aaszA,aaszB,bSize) == 0;
}

niExportFunc(tBool) StrEndsWith(const achar* aaszA, const achar* aaszB) {
  tSize aSize = StrSize(aaszA);
  tSize bSize = StrSize(aaszB);
  if (bSize > aSize) return niFalse;
  const achar* ap = aaszA+(aSize-bSize);
  // if (utf8::validate_next(ap, aaszA+aSize) != eUTFError_OK) {
  // return eFalse;
  // }
  return StrCmp(ap,aaszB) == 0;
}

niExportFunc(tBool) StrEndsWithI(const achar* aaszA, const achar* aaszB) {
  tSize aSize = StrSize(aaszA);
  tSize bSize = StrSize(aaszB);
  if (bSize > aSize) return niFalse;
  const achar* ap = aaszA+(aSize-bSize);
  // if (utf8::validate_next(ap, aaszA+aSize) != eUTFError_OK) {
  // return eFalse;
  // }
  return StrICmp(ap,aaszB) == 0;
}

///////////////////////////////////////////////
template <typename T>
static inline tSize _CPCount(const T* aaszA) {
  if (!aaszA)
    return 0;
  tSize size = 0;
  while (*aaszA) {
    ++size;
    ++aaszA;
  }
  return size;
}
niExportFunc(tSize) StrCCPCount(const cchar* acszA) {
  return _CPCount(acszA);
}
niExportFunc(tSize) StrGCPCount(const gchar* agszA) {
  return _CPCount(agszA);
}
niExportFunc(tSize) StrXCPCount(const xchar* axszA) {
  return _CPCount(axszA);
}

///////////////////////////////////////////////
niExportFunc(tI32) StrMapIndex(const tSize anLen, tI32 anIndex) {
  if (anIndex < 0)
    return ((anLen+anIndex) > 0 ? (anLen+anIndex) : 0);
  else
    return anIndex;
}

///////////////////////////////////////////////
niExportFunc(tI32) StrGetCommandPath(achar* dest, tU32 destMaxSize, const achar* src, tU32* pLen) {
  if (!dest || !src)
    return 0;
  achar* d = dest;
  tU32 len = 0;
  if (src[0] == '"') {
    const achar* p = src+1;
    while (*p && (d-dest < (tI32)destMaxSize)) {
      const tU32 c = _GetX(&p);
      if (c == '"') {
        ++len;
        break;
      }
      d += _SetC(d,c);
      ++len;
    }
  }
  else {
    const achar* p = src;
    while (*p && (d-dest < (tI32)destMaxSize)) {
      const tU32 c = _GetX(&p);
      if (StrIsSpace(c)) {
        ++len;
        break;
      }
      d += _SetC(d,c);
      ++len;
    }
  }
  d += _SetC(d,0);
  if (pLen) *pLen = len;
  return (d-dest);
}

///////////////////////////////////////////////
static achar* _StrValueToHexa(achar* d, tU64 val, tU32 anPrecision, tBool abUpperCase) {
  char tmp[25];  // for 64-bit integers
  tI32 i = 0;
  char* table;
  if (abUpperCase)
    table = _HexDigitUpr;
  else
    table = _HexDigitLwr;
  do {
    tmp[i++] = table[(val & 15)];
    val >>= 4;
  } while (val && i < (niCountOf(tmp)-1));
  tmp[i] = 0;
  if (anPrecision) {
    for (tI32 len = i; len < (tI32)anPrecision; ++len) {
      *d++ = '0';
    }
  }
  while (i > 0) {
    *d++ = tmp[--i];
  }
  *d++ = 0;
  return d;
}

///////////////////////////////////////////////
niExportFunc(void) StrValueToHexa(achar* d, tU64 value) {
  if (!d) return;
  _StrValueToHexa(d,value,0,eTrue);
}
niExportFunc(void) StrBytesToHexa(achar* d, tU8* bytes, tU32 byteCount) {
  if (!d) return;
  niLoop(i,byteCount) {
    d = _StrValueToHexa(d,*bytes,2,eTrue);
  }
}

#if niMinFeatures(15)
//--------------------------------------------------------------------------------------------
//
//  Unicode character properties
//
//--------------------------------------------------------------------------------------------
extern "C" int _pcre_ucp_findprop(const unsigned int c, int *type_ptr, int *script_ptr);

niExportFunc(tU32) StrGetUCPProps(tU32 cp, tU32* apCharType, tU32* apScript) {
  int charType = 0, script = 0;
  int r = _pcre_ucp_findprop(cp, &charType, &script);
  if (apCharType) *apCharType = (tU32)charType;
  if (apScript) *apScript = (tU32)script;
  return (eUCPCategory)r;
}
#endif

//--------------------------------------------------------------------------------------------
//
//  Xml entities escape
//
//--------------------------------------------------------------------------------------------

struct sXmlSpecialChar {
  const achar theChar;
  const achar* theStr;
  tSize len;
};
static sXmlSpecialChar _kXmlSpecialChars[] = {
  {'&', _A("amp;"),4},
  {'<', _A("lt;"),3},
  {'>', _A("gt;"),3},
  {'"', _A("quot;"),5},
  {'\'',_A("apos;"),5}
};

static void _XmlReplaceSpecialCharacters(cString& o, const achar* b, const achar* e)
{
  static const achar* const _kaszXmlIndent = _A("\t");
  // static const achar* const _kaszXmlIndent = _A(" ");

  const tSize len = (e-b);
  if (len == 0)
    return;

  tSize left = len;
  o.reserve(len);
  const achar* p = b;
  while (*p && p < e) {
    if (*p == '&' && left >= 3) {
      if (*(p+1) == '#' && *(p+2) == 'x') {
        p += 3;
        const achar* hexEnd;
        const tU32 ch = (tU32)ni::StrToUL(p,&hexEnd,16);
        p = hexEnd;
        o.appendChar(ch);
        niAssert(*hexEnd == ';' && "Invalid input");
        ++p; // skip the ';'
      }
      else {
        int specialChar = -1;
        niLoop(i,niCountOf(_kXmlSpecialChars)) {
          // skip if the special char is longer than what can still be read...
          if (left < _kXmlSpecialChars[i].len)
            continue;
          // p+1, skip the &
          if (StrNCmp(_kXmlSpecialChars[i].theStr,p+1,_kXmlSpecialChars[i].len) == 0) {
            specialChar = i;
            break;
          }
        }
        if (specialChar != -1) {
          o.push_back(_kXmlSpecialChars[specialChar].theChar);
          p += _kXmlSpecialChars[specialChar].len;
          ++p; // skip the '&'
        }
        else {
          o.appendChar(ni::utf8::raw_next(p));
        }
      }
    }
    else {
      o.appendChar(ni::utf8::raw_next(p));
    }
  }
}

niExportFuncCPP(cString&) StringEncodeXml(cString& aXmlString, const achar* aaszString) {
  const achar* p = aaszString;
  for (;;) {
    tU32 c = ni::utf8::raw_next(p);
    if (!c) break;
    switch (c) {
      case '<':
        aXmlString << "&lt;";
        break;
      case '>':
        aXmlString << "&gt;";
        break;
      case '&':
        aXmlString << "&amp;";
        break;
      case '"':
        aXmlString << "&quot;";
        break;
      case '\'':
        aXmlString << "&apos;";
        break;
      case '\r':
      case '\n':
        aXmlString << niFmt(_A("&#x%X;"),c);
        break;
      default:
        aXmlString.appendChar(c);
        break;
    }
  }
  return aXmlString;
}

niExportFuncCPP(cString&) StringDecodeXml(cString& aDecodedString, const achar* aaszXmlString) {
  _XmlReplaceSpecialCharacters(aDecodedString,
                               aaszXmlString,
                               aaszXmlString+StrSize(aaszXmlString));
  return aDecodedString;
}

/* Portable character check (remember EBCDIC). Do not use isalnum() because
   its behavior is altered by the current locale.
   See http://tools.ietf.org/html/rfc3986#section-2.3
*/
static bool _IsUnreserved(tU8 inC)
{
  switch (inC) {
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
    case 'a': case 'b': case 'c': case 'd': case 'e':
    case 'f': case 'g': case 'h': case 'i': case 'j':
    case 'k': case 'l': case 'm': case 'n': case 'o':
    case 'p': case 'q': case 'r': case 's': case 't':
    case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
    case 'A': case 'B': case 'C': case 'D': case 'E':
    case 'F': case 'G': case 'H': case 'I': case 'J':
    case 'K': case 'L': case 'M': case 'N': case 'O':
    case 'P': case 'Q': case 'R': case 'S': case 'T':
    case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
    case '-': case '.': case '_': case '~':
      return true;
    default:
      break;
  }
  return false;
}

// Converts a hex character to its integer value
static inline tU8 _FromHex(tU8 ch) {
  return StrIsDigit(ch) ? ch - '0' : StrToLower(ch) - 'a' + 10;
}
// Converts an integer value to its hex character
static inline tU8 _ToHex(tU8 code) {
  return _HexDigitLwr[code & 15];
}

// Returns a url-encoded version of str, buf should be able to hold: strlen(str) * 3 + 1 bytes.
niExportFuncCPP(cString&) StringEncodeUrl(cString& o, const char *str, tI32 strLen) {
  if (strLen <= 0) {
    strLen = ni::StrSize(str);
  }
  o.reserve(o.size() + (strLen*3 + 1));
  const char* pstr = str;
  char* pbuf = (char*)o.c_str() + o.size();
  while (*pstr) {
    if (_IsUnreserved(*pstr)) {
      *pbuf++ = *pstr;
    }
    else {
      *pbuf++ = '%';
      *pbuf++ = _ToHex(*pstr >> 4);
      *pbuf++ = _ToHex(*pstr & 15);
    }
    pstr++;
  }
  *pbuf = '\0';
  o.resize((tIntPtr)pbuf-(tIntPtr)o.c_str());
  return o;
}

// Returns a url-decoded version of str, buf should be able to hold: strlen(str) + 1 bytes.
niExportFuncCPP(cString&) StringDecodeUrl(cString& o, const char *str, tI32 strLen) {
  if (strLen <= 0) {
    strLen = ni::StrSize(str);
  }
  o.reserve(o.size() + strLen + 1);
  const char* pstr = str;
  char* pbuf = (char*)o.c_str();
  while (*pstr) {
    if (*pstr == '%' && StrIsXDigit(pstr[1]) && StrIsXDigit(pstr[2])) {
      *pbuf++ = _FromHex(pstr[1]) << 4 | _FromHex(pstr[2]);
      pstr += 2;
    }
    else {
      *pbuf++ = *pstr;
    }
    pstr++;
  }
  *pbuf = '\0';
  o.resize((tIntPtr)pbuf-(tIntPtr)o.c_str());
  return o;
}

//--------------------------------------------------------------------------------------------
//
//  Safe "printf" like string function
//
//--------------------------------------------------------------------------------------------

///////////////////////////////////////////////
enum eStringCatSPrintFlag {
  eStringCatSPrintFlag_LEFT_JUSTIFY             = 1,
  eStringCatSPrintFlag_FORCE_PLUS_SIGN          = 2,
  eStringCatSPrintFlag_FORCE_SPACE              = 4,
  eStringCatSPrintFlag_ALTERNATE_CONVERSION     = 8,
  eStringCatSPrintFlag_PAD_ZERO                 = 16,
  eStringCatSPrintFlag_SHORT_INT                = 32,
  eStringCatSPrintFlag_LONG_INT                 = 64,
  eStringCatSPrintFlag_LONG_DOUBLE              = 128,
  eStringCatSPrintFlag_LONG_LONG                = 256,
  eStringCatSPrintFlag_NEG_IS_NA                = 512,
  eStringCatSPrintFlag_ForceDWORD = 0xFFFFFFFF
};

//! String
struct sStringCatFormat
{
  struct sArgArray {
    sArgArray(const Var** apArgs, tU32 anNumArgs) : mppArgs(apArgs), mnNumArgs(apArgs?anNumArgs:0) {
      mnCurArg = 0;
      Fetch();
    }

    tI32 GetI32() {
      VarConvertType(mcurArg,eType_I32);
      tI32 r = mcurArg.IsNull() ? 0 : mcurArg.GetI32();
      Fetch();
      return r;
    }
    tI64 GetInt() {
      VarConvertType(mcurArg,eType_I64);
      tI64 r = mcurArg.IsNull() ? 0 : mcurArg.GetI64();
      Fetch();
      return r;
    }
    tU64 GetUInt() {
      VarConvertType(mcurArg,eType_U64);
      tU64 r = mcurArg.IsNull() ? 0 : mcurArg.GetU64();
      Fetch();
      return r;
    }
    tF64 GetFloat() {
      VarConvertType(mcurArg,eType_F64);
      tF64 r = mcurArg.IsNull() ? 0 : mcurArg.GetF64();
      Fetch();
      return r;
    }
    const cString& GetLocalizedString() {
      tHStringPtr hspStr;
      if (VarIsString(mcurArg)) {
        hspStr = VarGetHString(mcurArg);
      }
      else {
        VarConvertType(mcurArg,eType_String);
        hspStr = _H(mcurArg.IsNull() ? _A("") : mcurArg.GetString());
      }
      mstrRet = niLStr(hspStr);
      Fetch();
      return mstrRet;
    }
    const tBool IsString() const {
      return VarIsString(mcurArg);
    }
    const cString& GetString() {
      if (VarIsString(mcurArg)) {
        mstrRet = VarGetString(mcurArg).Chars();
      }
      else {
        VarConvertType(mcurArg,eType_String);
        mstrRet = mcurArg.IsNull() ? _A("") : mcurArg.GetString().Chars();
      }
      Fetch();
      return mstrRet;
    }

    void Fetch() {
      if (mnCurArg >= mnNumArgs) {
        mcurArg = _HC(stringlib_FormatNoArg);
      }
      else {
        mcurArg = *mppArgs[mnCurArg];
      }
      ++mnCurArg;
    }

   private:
    const Var** mppArgs;
    const tU32  mnNumArgs;
    tU32        mnCurArg;
    Var         mcurArg;
    cString     mstrRet;
  };

  // information about the current format conversion mode
  struct sInfo {
    tI32 flags;
    tI32 field_width;
    tI32 precision;
  };

  // decoded string argument type
  struct sFormatArg {
    cString data;
  };
  typedef astl::list<sFormatArg> tFormatArgLst;

  // Helper for formatting (!) a character.
  static inline tI32 sprint_char(sFormatArg *string_arg, sInfo *info, tI32 val) {
    if (val < 32) {
      string_arg->data << niFmt("\\%d",val);
    }
    else {
      string_arg->data.appendChar(val);
    }
    return 1;
  }

  // Worker function for formatting integers.
  static inline tI32 sprint_i(sFormatArg *string_arg, tU64 val, tI32 precision) {
    char tmp[24];  // for 64-bit integers
    tI32 i = 0;
    do {
      tmp[i++] = (char)(val % 10);
      val /= 10;
    } while (val);
    for (tI32 len = i; len < precision; len++) {
      string_arg->data.appendChar('0');
    }
    while (i > 0) {
      string_arg->data.appendChar(tmp[--i] + '0');
    }
    return string_arg->data.length();
  }

  // Helper to add a plus sign or space in front of a number.
  static void sprint_plus_sign(sFormatArg *string_arg, sInfo *info) {
    if (info->flags & eStringCatSPrintFlag_FORCE_PLUS_SIGN) {
      string_arg->data.appendChar('+');
    }
    else if (info->flags & eStringCatSPrintFlag_FORCE_SPACE) {
      string_arg->data.appendChar('_');
    }
  }

  // Helper for formatting a boolean.
  static tI32 sprint_bool(sFormatArg *string_arg, sInfo *info, sArgArray& args, const char* aTrue, const char* aFalse) {
    tI32 val = -1;
    if (args.IsString()) {
      const cString& str = args.GetString();
      if (str.IEq("true") || str.IEq("yes") || str.IEq("on")) {
        val = 1;
      }
      else if (str.IEq("false") || str.IEq("no") || str.IEq("off")) {
        val = 0;
      }
      else {
        val = str.Long();
      }
    }
    else {
      val = args.GetInt();
    }

    if ((info->flags & eStringCatSPrintFlag_NEG_IS_NA) && (val < 0)) {
      string_arg->data += "N/A";
      return 3;
    }
    const tI32 startLen = string_arg->data.size();
    string_arg->data += (val) ? aTrue : aFalse;
    return (string_arg->data.size()-startLen);
  }

  // Helper for formatting a signed integer.
  static tI32 sprint_int(sFormatArg *string_arg, sInfo *info, tI64 val) {
    if (val < 0) {
      if (info->flags & eStringCatSPrintFlag_NEG_IS_NA) {
        string_arg->data += "N/A";
        return 3;
      }
      else {
        val = -val;
        string_arg->data.appendChar('-');
      }
    }
    else {
      sprint_plus_sign(string_arg,info);
    }
    return sprint_i(string_arg, val, info->precision);
  }

  // Helper for formatting an unsigned integer.
  static tI32 sprint_unsigned(sFormatArg *string_arg, sInfo *info, tU64 val) {
    sprint_plus_sign(string_arg, info);
    return sprint_i(string_arg, val, info->precision);
  }

  // Helper for formatting a hex integer.
  static tI32 sprint_hex(sFormatArg *string_arg, sInfo *info, tI32 caps, tU64 val) {
    char tmp[24];  // for 64-bit integers
    tI32 i = 0;
    // 24 characters max for a 64-bit integer
    sprint_plus_sign(string_arg, info);
    if (info->flags & eStringCatSPrintFlag_ALTERNATE_CONVERSION) {
      string_arg->data += "0x";
    }

    char* table;
    if (caps)
      table = _HexDigitUpr;
    else
      table = _HexDigitLwr;
    do {
      tmp[i++] = table[(val & 15)];
      val >>= 4;
    } while (val && i < niCountOf(tmp));
    tmp[i] = 0;

    for (tI32 len = i; len < info->precision; ++len) {
      string_arg->data.appendChar('0');
    }

    while (i > 0) {
      string_arg->data.appendChar(tmp[--i]);
    }
    return string_arg->data.length();
  }
  // Helper for formatting an octal integer.
  static tI32 sprint_octal(sFormatArg *string_arg, sInfo *info, tU64 val) {
    char tmp[24];  // for 64-bit integers
    tI32 i = 0;
    sprint_plus_sign(string_arg, info);
    if (info->flags & eStringCatSPrintFlag_ALTERNATE_CONVERSION) {
      string_arg->data.appendChar('0');
    }
    do {
      tmp[i++] = (char)(val & 7);
      val >>= 3;
    } while (val);
    for (tI32 len=i; len<info->precision; len++) {
      string_arg->data.appendChar('0');
    }
    while (i > 0) {
      string_arg->data.appendChar(tmp[--i] + '0');
    }
    return string_arg->data.length();
  }
  // Helper for formatting a float (piggyback on the libc implementation).
  static tI32 sprint_float(sFormatArg *string_arg, sInfo *info, tF64 val, tI32 conversion) {
    if ((info->flags & eStringCatSPrintFlag_NEG_IS_NA) && (val < 0)) {
      string_arg->data += "N/A";
      return 3;
    }

    char format[256], tmp[256];
    tI32 len = 0;
    format[len++] = '%';
    if (info->flags & eStringCatSPrintFlag_LEFT_JUSTIFY)
      format[len++] = '-';
    if (info->flags & eStringCatSPrintFlag_FORCE_PLUS_SIGN)
      format[len++] = '+';
    if (info->flags & eStringCatSPrintFlag_FORCE_SPACE)
      format[len++] = ' ';
    if (info->flags & eStringCatSPrintFlag_ALTERNATE_CONVERSION)
      format[len++] = '#';
    if (info->flags & eStringCatSPrintFlag_PAD_ZERO)
      format[len++] = '0';
    if (info->field_width > 0)
      len += sprintf(format+len, "%d", (int)info->field_width);
    if (info->precision >= 0)
      len += sprintf(format+len, ".%d", (int)info->precision);
    format[len++] = (char)conversion;
    format[len] = 0;
    sprintf(tmp, format, val);
    string_arg->data += tmp;
    info->field_width = 0;
    return string_arg->data.length();
  }

  // Helper for formatting a string.
  static tI32 sprint_string(sFormatArg *string_arg, sInfo *info, const cString& s) {
    string_arg->data = s;
    return string_arg->data.length();
  }

  // Worker function for decoding the format string (with those pretty '%' characters)
  static tI32 decode_format_string(achar* buf, tFormatArgLst& aStringArgs,
                                   const achar* apFormat,
                                   sArgArray& args,
                                   iExpressionContext* apExpressionContext)
  {
    niAssert(niStringIsOK(apFormat));
    const achar* format = apFormat;
    niAssert(!aStringArgs.empty());
    sFormatArg* string_arg = &aStringArgs.back();

    tI32 c = 0;
    tI32 len = 0, slen = 0;
    tBool done;
    while ((c = STRLIB_GETXC(&format)) !=   0) {

      if (c == '%') {
        c = STRLIB_GETC(format);
        if (c == '%') {
          // percent sign escape
          format += STRLIB_UWIDTH(format);
          buf += STRLIB_SETC(buf, '%');
          buf += STRLIB_SETC(buf, '%');
          len++;
        }
        else {
          // format specifier
#define NEXT_C() {                              \
            format += STRLIB_UWIDTH(format);    \
            c = STRLIB_GETC(format);            \
          }

          // set default conversion flags
          sInfo info;
          info.flags = 0;
          info.field_width = 0;
          info.precision = -1;

          if (c == '(') {
            // expression format
            cString expr;
            expr.reserve(256);

            // skip starting (
            int paren = 1;
            NEXT_C();

            // check if 'print' marker is present
            tBool bPrintExpr = eFalse;
            if (c == '=') {
              bPrintExpr = eTrue;
              NEXT_C();
            }

            do {
              if (c == '(') {
                ++paren;
              }
              else if (c == ')') {
                --paren;
                if (paren <= 0) {
                  NEXT_C();
                  break;
                }
              }
              expr.appendChar(c);
              NEXT_C();
            } while (c);

            if (bPrintExpr) {
              expr.Normalize();
              string_arg->data += expr;
              string_arg->data += " = ";
            }
            if (apExpressionContext) {
              Ptr<iExpressionVariable> var = apExpressionContext->Eval(expr.Chars());
              if (!var.IsOK()) {
                string_arg->data += "#EXPR-ERR#";
              }
              else {
                string_arg->data += var->GetString();
              }
            }
            else {
              string_arg->data += "#EXPR-NOCONTEXT#";
            }
            slen = string_arg->data.length();
          }
          else {
            // check for conversion flags
            done = eFalse;
            do {
              switch (c) {
                case '-':
                  info.flags |= eStringCatSPrintFlag_LEFT_JUSTIFY;
                  NEXT_C();
                  break;
                case '+':
                  info.flags |= eStringCatSPrintFlag_FORCE_PLUS_SIGN;
                  NEXT_C();
                  break;
                case ' ':
                  info.flags |= eStringCatSPrintFlag_FORCE_SPACE;
                  NEXT_C();
                  break;
                case '#':
                  info.flags |= eStringCatSPrintFlag_ALTERNATE_CONVERSION;
                  NEXT_C();
                  break;
                case '0':
                  info.flags |= eStringCatSPrintFlag_PAD_ZERO;
                  NEXT_C();
                  break;
                case '$':
                  info.flags |= eStringCatSPrintFlag_NEG_IS_NA;
                  NEXT_C();
                  break;
                default:
                  done = eTrue;
                  break;
              }
            } while (!done);

            // check for a field width specifier
            if (c == '*') {
              NEXT_C();
              info.field_width = args.GetI32();
              if (info.field_width < 0) {
                info.flags |= eStringCatSPrintFlag_LEFT_JUSTIFY;
                info.field_width = -info.field_width;
              }
            }
            else if ((c >= '0') && (c <= '9')) {
              info.field_width = 0;
              do {
                info.field_width *= 10;
                info.field_width += c - '0';
                NEXT_C();
              } while ((c >= '0') && (c <= '9'));
            }

            // check for a precision specifier
            if (c == '.')
              NEXT_C();

            if (c == '*') {
              NEXT_C();
              info.precision = args.GetI32();
              if (info.precision < 0)
                info.precision = 0;
            }
            else if ((c >= '0') && (c <= '9')) {
              info.precision = 0;
              do {
                info.precision *= 10;
                info.precision += c - '0';
                NEXT_C();
              } while ((c >= '0') && (c <= '9'));
            }

            // check for size qualifiers
            done = eFalse;
            do {
              switch (c) {
                case 'h':
                  info.flags |=   eStringCatSPrintFlag_SHORT_INT;
                  NEXT_C();
                  break;
                case 'l':
                  if (info.flags & eStringCatSPrintFlag_LONG_INT)
                    info.flags |=   eStringCatSPrintFlag_LONG_LONG;
                  else
                    info.flags |=   eStringCatSPrintFlag_LONG_INT;
                  NEXT_C();
                  break;
                case 'L':
                  info.flags |= (eStringCatSPrintFlag_LONG_DOUBLE | eStringCatSPrintFlag_LONG_LONG);
                  NEXT_C();
                  break;
                default:
                  done = eTrue;
                  break;
              }

            } while (!done);

            // format the data
            switch (c) {
              case 'c':
                // character
                slen = sprint_char(string_arg, &info, args.GetI32());
                NEXT_C();
                break;
              case 'd':
              case 'i':
                // signed integer
                slen = sprint_int(string_arg, &info, args.GetInt());
                NEXT_C();
                break;
              case 'D':
                // signed long integer
                slen = sprint_int(string_arg, &info, args.GetInt());
                NEXT_C();
                break;
              case 'e':
              case 'E':
              case 'f':
              case 'g':
              case 'G':
                // float
                slen = sprint_float(string_arg, &info, args.GetFloat(), c);
                NEXT_C();
                break;
              case 'n':
                // store current string position, ignored...
                slen = -1;
                NEXT_C();
                break;
              case 'o':
                // unsigned octal integer
                slen = sprint_octal(string_arg, &info, args.GetUInt());
                NEXT_C();
                break;
              case 'p':
                // pointer
                info.flags |= eStringCatSPrintFlag_PAD_ZERO;
#ifdef ni64
                info.field_width = 16;
#else
                info.field_width = 8;
#endif
                slen = sprint_hex(string_arg, &info, eTrue, args.GetUInt());
                NEXT_C();
                break;
              case 's':
                // string
                slen = sprint_string(string_arg, &info, args.GetString());
                NEXT_C();
                break;
              case 'S':
                // localized string
                slen = sprint_string(string_arg, &info, args.GetLocalizedString());
                NEXT_C();
                break;
              case 'u':
                // unsigned integer
                slen = sprint_unsigned(string_arg, &info, args.GetUInt());
                NEXT_C();
                break;
              case 'U':
                // unsigned long integer
                slen = sprint_unsigned(string_arg, &info, args.GetUInt());
                NEXT_C();
                break;
              case 'x':
              case 'X':
                // unsigned hex integer
                slen = sprint_hex(string_arg, &info, (c == 'X'), args.GetUInt());
                NEXT_C();
                break;
              case 'y':
                slen = sprint_bool(string_arg, &info, args, "yes", "no");
                NEXT_C();
                break;
              case 'Y':
                slen = sprint_bool(string_arg, &info, args, "YES", "NO");
                NEXT_C();
                break;
              case 'z':
                slen = sprint_bool(string_arg, &info, args, "on", "off");
                NEXT_C();
                break;
              case 'Z':
                slen = sprint_bool(string_arg, &info, args, "ON", "OFF");
                NEXT_C();
                break;
              default:
                // weird shit...
                slen    = -1;
                break;
            }
          }

          if (slen >= 0) {
            if (slen < info.field_width) {
              if (info.flags & eStringCatSPrintFlag_LEFT_JUSTIFY) {
                // left align the result
                while (slen < info.field_width) {
                  string_arg->data.appendChar(' ');
                  slen++;
                }
              }
              else {
                // right align the result
                tI32 shift = info.field_width - slen;
                if (shift > 0) {
                  achar cShiftFiller = ' ';
                  if (info.flags & eStringCatSPrintFlag_PAD_ZERO) {
                    cShiftFiller = '0';
                  }
                  cString strShift;
                  strShift.resize(shift);
                  achar* strShiftWriteBuffer = strShift.data();
                  niLoop(i,shift) {
                    strShiftWriteBuffer[i] = cShiftFiller;
                  }
                  string_arg->data.insert((tI32)0,strShift);
                  slen += shift;
                }
              }
            }
            buf += STRLIB_SETC(buf, '%');
            buf += STRLIB_SETC(buf, 's');
            len += slen;
            string_arg = &astl::push_back(aStringArgs);
          }
        }
      }
      else {
        // normal character
        buf += STRLIB_SETC(buf, c);
        len++;
      }
    }
    STRLIB_SETC(buf, 0);
    return len;
  }

  ///////////////////////////////////////////////
  static cString& CatFormat(cString& o, const achar* aaszFormat, const Var** apArgs, tU32 anNumArgs, iExpressionContext* apExpressionContext)
  {
    if (!niStringIsOK(aaszFormat))
      return o;

    if (!apArgs || anNumArgs == 0) {
      // no args concat directly, no formatting
      o << aaszFormat;
      return o;
    }

    tI32 c;

    // decoding can only lower the length of the format string
    tU32 fmtSizeZ = StrSizeZ(aaszFormat);
    astl::vector<achar> dfData; dfData.resize(fmtSizeZ * sizeof(char));
    achar* decoded_format = dfData.data();

    o.reserve(fmtSizeZ);

    // allocate first item
    tFormatArgLst lstStringArgs;
    astl::push_back(lstStringArgs);

    // 1st pass: decode
    sArgArray args(apArgs,anNumArgs);
    /*tI32 len = */ decode_format_string(decoded_format, lstStringArgs, aaszFormat, args, apExpressionContext);

    // 2nd pass: concatenate
    tFormatArgLst::const_iterator iter_arg = lstStringArgs.begin();
    while ((c = STRLIB_GETX(&decoded_format))   != 0) {
      if (c == '%') {
        if ((c = STRLIB_GETX(&decoded_format)) ==   '%') {
          // percent sign escape
          o.appendChar('%');
        }
        else if (c == 's') {
          // string argument
          o += iter_arg->data;
          ++iter_arg;
        }
      }
      else {
        // normal character
        o.appendChar(c);
      }
    }
    return o;
  }

#undef NEXT_C
};

niExportFuncCPP(cString&) StringCatFormatEx(cString& o, iExpressionContext* apExprCtx, const achar* aaszFormat, const Var** apArgs, tU32 anNumArgs) {
  if (!niStringIsOK(aaszFormat))
    return o;
  if (!apExprCtx) {
    apExprCtx = ni::GetLang()->GetExpressionContext();
  }
  return sStringCatFormat::CatFormat(o,aaszFormat,apArgs,anNumArgs,apExprCtx);
}

#undef STRLIB_GETC
#undef STRLIB_GETX
#undef STRLIB_GETXC
#undef STRLIB_SETC
#undef STRLIB_UWIDTH
#undef STRLIB_UCWIDTH

niExportFuncCPP(cString&) StringToPropertyName(cString& strName, const achar* aszName)
{
  strName = AZEROSTR;
  if (!niStringIsOK(aszName))
    return strName;
  const achar* p = aszName;
  if (StrStartsWith(p,"Set") || StrStartsWith(p,"Get"))
    p += 3;
  while (*p) {
    const tU32 ch = StrGetNextX(&p);
    const tU32 next = StrGetNext(p);
    strName.appendChar(StrToLower(ch));
    if (next && ni::StrIsUpper(next) && !ni::StrIsUpper(ch))
      strName.appendChar('_');
  }
  return strName;
}

niExportFuncCPP(cString&) StringToPropertyMethodName(cString& strName, const achar* aszName)
{
  strName = AZEROSTR;
  if (!niStringIsOK(aszName))
    return strName;
  const achar* p = aszName;
  tBool bPrevUnderscoreOrDigit = eTrue;
  while (*p) {
    const tU32 ch = StrGetNext(p);
    if (bPrevUnderscoreOrDigit) {
      strName.appendChar(StrToUpper(ch));
      bPrevUnderscoreOrDigit = StrIsDigit(ch);
    }
    else if (ch == '_') {
      bPrevUnderscoreOrDigit = eTrue;
    }
    else {
      strName.appendChar(ch);
      bPrevUnderscoreOrDigit = StrIsDigit(ch);
    }
    StrGetNextX(&p);
  }
  return strName;
}

niExportFuncCPP(cString&) StringToPropertyName2(cString& strName, const achar* aszName)
{
  strName = AZEROSTR;
  if (!niStringIsOK(aszName))
    return strName;
  const achar* p = aszName;
  p += 3;
  while (*p) {
    const tU32 ch = StrGetNextX(&p);
    strName.appendChar(StrToLower(ch));
    break;
  }
  while (*p) {
    const tU32 ch = StrGetNextX(&p);
    strName.appendChar(ch);
  }
  return strName;
}

niExportFuncCPP(cString&) StringToPropertyMethodName2(cString& strName, const achar* aszName)
{
  strName = AZEROSTR;
  if (!niStringIsOK(aszName))
    return strName;
  const achar* p = aszName;
  while (*p) {
    const tU32 ch = StrGetNextX(&p);
    strName.appendChar(StrToUpper(ch));
    break;
  }
  while (*p) {
    const tU32 ch = StrGetNextX(&p);
    strName.appendChar(ch);
  }
  return strName;
}


niExportFunc(ni::tI32) StrCharItFindFirstOf(ni::StrCharIt& aIt, const ni::tU32* aToFind, const ni::tU32 aNumToFind, ni::tU32* aCharFound) {
  while (!aIt.is_end()) {
    const ni::tU32 c = aIt.next();
    niLoop(i,aNumToFind) {
      if (aToFind[i] == c) {
        if (aCharFound) {
          *aCharFound = c;
        }
        return aIt.pos();
      }
    }
  }
  return -1;
}

niExportFunc(ni::tI32) StrCharItRFindFirstOf(ni::StrCharIt& aIt, const ni::tU32* aToFind, const ni::tU32 aNumToFind, ni::tU32* aCharFound) {
  while (!aIt.is_start()) {
    const ni::tU32 c = aIt.prior();
    niLoop(i,aNumToFind) {
      if (aToFind[i] == c) {
        if (aCharFound) {
          *aCharFound = c;
        }
        return aIt.pos();
      }
    }
  }
  return -1;
}

niExportFunc(tI32) StrFindProtocol(const achar* aURL) {
  if (!niStringIsOK(aURL))
    return -1;
  const achar* p = aURL;
  while (*p) {
    const achar a = *(p+1);
    if (!a)
      break;
    const achar b = *(p+2);
    if (!b)
      break;
    if (*p == ':' && a == '/' && b == '/') {
      return (p-aURL);
    }
    else if ((a != ':' && b != ':') &&
             (*p == '/' || a == '/' || b == '/' ||
              *p == '\\' || a == '\\' || b == '\\'))
    {
      // any '/' or '\' character will break indicates a regular path (no procotol defined)
      return -1;
    }
    ++p;
  }
  return -1;
}

niExportFunc(const achar*) GetTypeString(achar* aStr, tType aType)
{
  *aStr = 0;
  if (niFlagIs(aType,eTypeFlags_Constant))
    StrCat(aStr,_A("const "));

  tBool isScriptType = eTrue;
  if (niFlagIs(aType,ni::eTypeFlags_STRING)) {
    StrCat(aStr,"STRING");
  }
  else if (niFlagIs(aType,ni::eTypeFlags_TABLE)) {
    StrCat(aStr,"TABLE");
  }
  else if (niFlagIs(aType,ni::eTypeFlags_ARRAY)) {
    StrCat(aStr,"ARRAY");
  }
  else if (niFlagIs(aType,ni::eTypeFlags_USERDATA)) {
    StrCat(aStr,"USERDATA");
  }
  else if (niFlagIs(aType,ni::eTypeFlags_CLOSURE)) {
    StrCat(aStr,"CLOSURE");
  }
  else if (niFlagIs(aType,ni::eTypeFlags_NATIVECLOSURE)) {
    StrCat(aStr,"NATIVECLOSURE");
  }
  else if (niFlagIs(aType,ni::eTypeFlags_FUNCPROTO)) {
    StrCat(aStr,"FUNCPROTO");
  }
  else {
    isScriptType = eFalse;
  }

  if (!isScriptType || niType(aType) != eType_IUnknown) {
    if (isScriptType) {
      StrCat(aStr,"|");
    }
    switch (niType(aType)) {
      case eType_Null:    StrCat(aStr,_A("Null")); break;
      case eType_I8:      StrCat(aStr,_A("I8")); break;
      case eType_U8:      StrCat(aStr,_A("U8")); break;
      case eType_I16:     StrCat(aStr,_A("I16")); break;
      case eType_U16:     StrCat(aStr,_A("U16")); break;
      case eType_I32:     StrCat(aStr,_A("I32")); break;
      case eType_U32:     StrCat(aStr,_A("U32")); break;
      case eType_F32:     StrCat(aStr,_A("F32")); break;
      case eType_AChar:   StrCat(aStr,_A("AChar")); break;
      case eType_I64:     StrCat(aStr,_A("I64")); break;
      case eType_U64:     StrCat(aStr,_A("U64")); break;
      case eType_F64:     StrCat(aStr,_A("F64")); break;
      case eType_Vec2f:   StrCat(aStr,_A("Vec2f")); break;
      case eType_Vec2i:   StrCat(aStr,_A("Vec2i")); break;
      case eType_Vec3f:   StrCat(aStr,_A("Vec3f")); break;
      case eType_Vec3i:   StrCat(aStr,_A("Vec3i")); break;
      case eType_String:  StrCat(aStr,_A("String")); break;
      case eType_Vec4f:   StrCat(aStr,_A("Vec4f")); break;
      case eType_Vec4i:   StrCat(aStr,_A("Vec4i")); break;
      case eType_UUID:    StrCat(aStr,_A("UUID")); break;
      case eType_Matrixf: StrCat(aStr,_A("Matrixf")); break;
      case eType_Variant: StrCat(aStr,_A("Variant")); break;
      case eType_IUnknown: {
        StrCat(aStr,_A("IUnknown"));
        break;
      }
      default: {
        achar buf[4] = {0};               // niType is 8bits, 3 chars max + the end zero
        StrCat(aStr,"#");
        StrCat(aStr,StringUIntToStr(buf,niCountOf(buf),niType(aType)));
        StrCat(aStr,"#");
        break;
      }
    }
  }

  if (niFlagIs(aType,eTypeFlags_Pointer))
    StrCat(aStr,_A("*"));

  return aStr;
}

niExportFuncCPP(cString) StringURLGetProtocol(const achar* aURL) {
  const tI32 ppos = StrFindProtocol(aURL);
  if (ppos > 0) {
    cString ret;
    ret.resize(ppos);
    achar* retWriteBuffer = ret.data();
    niLoop(i,ppos) {
      retWriteBuffer[i] = aURL[i];
    }
    return ret;
  }
  return AZEROSTR;
}

niExportFuncCPP(cString) StringURLGetPath(const achar* aURL) {
  const tI32 ppos = StrFindProtocol(aURL);
  if (ppos > 0)
    return aURL+ppos+3;
  return aURL;
}

niExportFuncCPP(tU32) StringSplit(const cString& aToSplit, const achar* aaszSeparators, astl::vector<cString>* apOut)
{
  tU32 token_num = 0;
  if (!niIsStringOK(aaszSeparators)) {
    if (!aToSplit.empty()) {
      ++token_num;
      if (apOut) {
        apOut->push_back(aToSplit.Chars());
      }
    }
  }
  else if (!aToSplit.empty()) {
    cString buffer = aToSplit;
    achar* pBuffer = buffer.data();

    achar* tokLast;
    achar* token = StrTok(pBuffer,aaszSeparators,&tokLast);
    if (token) {
      ++token_num;
      if (apOut) {
        apOut->push_back(token);
      }
    }
    while(token) {
      token = StrTok(NULL,aaszSeparators,&tokLast);
      if (token) {
        ++token_num;
        if (apOut) {
          apOut->push_back(token);
        }
      }
    }
  }

  return token_num;
}

niExportFuncCPP(void) StringSplitNameFlags(const cString& aToSplit, cString* pstrName, astl::vector<cString>* plstFlags)
{
  cString strName;
  const achar* p = aToSplit.Chars();
  while (*p) {
    const tU32 c = StrGetNext(p);
    if (c == '[')
      break;
    strName.appendChar(c);
    StrGetNextX(&p);
  }
  if (pstrName) {
    strName.Trim();
    *pstrName = strName;
  }
  if (!plstFlags || !*p)
    return;
  while (1) {
    if (StrGetNext(p) == '[') {
      StrGetNextX(&p);
      cString str;
      while (*p) {
        const tU32 c = StrGetNext(p);
        if (c == ']')
          break;
        else if (c == ',') {
          str.Trim();
          plstFlags->push_back(str);
          str.Clear();
        }
        else {
          str.appendChar(c);
        }
        StrGetNextX(&p);
      }
      if (str.IsNotEmpty()) {
        str.Trim();
        plstFlags->push_back(str);
      }
    }
    if (!*p)
      break;
    StrGetNextX(&p);
  }
}

niExportFuncCPP(tU32) StringSplitSep(const cString& aToSplit, const achar* aaszSeparators, astl::vector<cString>* apOut)
{
  tU32 token_num = 0;
  if (!niIsStringOK(aaszSeparators)) {
    if (!aToSplit.empty()) {
      ++token_num;
      if (apOut) {
        apOut->push_back(aToSplit.Chars());
      }
    }
  }
  else if (!aToSplit.empty()) {
    cString buffer = aToSplit;
    achar* pBuffer = buffer.data();
    achar* token = StrSep(&pBuffer,aaszSeparators);
    if (token) {
      ++token_num;
      if (apOut) {
        apOut->push_back(token);
      }
    }
    while(token) {
      token = StrSep(&pBuffer,aaszSeparators);
      if (token) {
        ++token_num;
        if (apOut) {
          apOut->push_back(token);
        }
      }
    }
  }
  return token_num;
}

niExportFuncCPP(tU32) StringSplitSepQuoted(const cString& aToSplit,
                                           const achar* aaszSeparators,
                                           const tU32 aQuote,
                                           astl::vector<cString>* apOut)
{
  tU32 token_num = 0;
  if (!niIsStringOK(aaszSeparators)) {
    if (!aToSplit.empty()) {
      ++token_num;
      if (apOut) {
        apOut->push_back(aToSplit.Chars());
      }
    }
  }
  else if (!aToSplit.empty()) {
    cString buffer = aToSplit;
    achar* pBuffer = buffer.data();
    achar* token = StrSepQuoted(&pBuffer,aaszSeparators,aQuote);
    if (token) {
      ++token_num;
      if (apOut) {
        apOut->push_back(token);
      }
    }
    while(token) {
      token = StrSepQuoted(&pBuffer,aaszSeparators,aQuote);
      if (token) {
        ++token_num;
        if (apOut) {
          apOut->push_back(token);
        }
      }
    }
  }
  return token_num;
}

niExportFuncCPP(cString) StringVecJoin(const astl::vector<cString>& aVec, const achar* aaszJoin) {
  cString r;
  niLoop(i,aVec.size()) {
    r += aVec[i];
    if (niStringIsOK(aaszJoin) && i != (aVec.size()-1)) {
      r += aaszJoin;
    }
  }
  return r;
}

niExportFunc(const achar*) StrHasText(const achar* aText) {
  if (!niStringIsOK(aText))
    return NULL;

  StrCharIt it(aText);
  while (!it.is_end()) {
    const tU32 c = it.next();
    if (!StrIsEmptyChar(c))
      return it.current();
  }

  return NULL;
}

niExportFuncCPP(tU32) StringSplitScript(
  astl::vector<cString>* apStatements,
  const achar* aScript,
  const achar* aSeparator,
  const achar* aCommentPrefix,
  const achar* aCommentBlockStart, const achar* aCommentBlockEnd,
  tBool abNormalizeEmptyChars)
{
  tU32 nNumStatementsAdded = 0;
  cString sb;
  const tSize sepLen = StrLen(aSeparator);
  const tSize commentBlockEndSz = StrSize(aCommentBlockEnd);
  const tSize commentBlockEndLen = StrLen(aCommentBlockEnd);
  tBool inSingleQuote = eFalse;
  tBool inDoubleQuote = eFalse;
  tBool inEscape = eFalse;
  for (StrCharIt it(aScript); !it.is_end(); it.next()) {
    const tU32 c = it.peek_next();
    if (inEscape) {
      inEscape = eFalse;
      sb.appendChar(c);
      continue;
    }
    // MySQL style escapes
    if (c == '\\') {
      inEscape = true;
      sb.appendChar(c);
      continue;
    }
    if (!inDoubleQuote && (c == '\'')) {
      inSingleQuote = !inSingleQuote;
    }
    else if (!inSingleQuote && (c == '"')) {
      inDoubleQuote = !inDoubleQuote;
    }
    if (!inSingleQuote && !inDoubleQuote) {
        // We've reached the end of the current statement
      if (StrStartsWith(it.current(), aSeparator)) {
        if (StrHasText(sb.Chars())) {
          if (apStatements)
            apStatements->push_back(sb);
          sb.Clear();
          ++nNumStatementsAdded;
        }
        it.advance(sepLen-1);
        continue;
      }
      else if (StrStartsWith(it.current(), aCommentPrefix)) {
        // Skip over any content from the start of the comment to the EOL
        const tI32 indexOfNextNewline = StrZFindChar(it.current(), 0, '\n');
        if (indexOfNextNewline > 0) {
          it.to_pos(it.pos() + indexOfNextNewline);
          continue;
        }
        else {
          // If there's no EOL, we must be at the end of the script, so stop here.
          break;
        }
      }
      else if (StrStartsWith(it.current(), aCommentBlockStart)) {
        // Skip over any block comments
        const tI32 indexOfCommentEnd = StrZFind(it.current(), 0, aCommentBlockEnd, commentBlockEndSz);
        if (indexOfCommentEnd > 0) {
          it.to_pos(it.pos() + indexOfCommentEnd);
          it.advance(commentBlockEndLen-1);
          continue;
        }
        else {
          niError(niFmt("Missing block comment end delimiter: %s", aCommentBlockEnd));
          return nNumStatementsAdded;
        }
      }
      else if (abNormalizeEmptyChars && StrIsEmptyChar(c)) {
        // Avoid multiple adjacent empty characters
        if (!sb.IsEmpty() && (sb.back() != ' ')) {
          sb.appendChar(' ');
        }
        continue;
      }
    }
    sb.appendChar(c);
  }
  if (StrHasText(sb.Chars())) {
    if (apStatements)
      apStatements->push_back(sb);
    ++nNumStatementsAdded;
  }
  return nNumStatementsAdded;
}

niExportFunc(tBool) StringEncodeCsvShouldQuote(const achar *aaszString, const achar aDelim, const achar aQuote) {
  const achar* p = aaszString;
  for (;;) {
    const tU32 c = ni::utf8::raw_next(p);
    if (!c)
      return eFalse;
    if (c == aQuote ||
        c == aDelim ||
        c == '\n' ||
        c == '\r')
    {
      return eTrue;
    }
  }
}

// Format a string to be RFC 4180-compliant
niExportFuncCPP(cString) StringEncodeCsvQuote(const char *aaszString, const char aQuote) {
  // Sequence used for escaping quote characters that appear in text
  const char double_quote[3] = { aQuote, aQuote, 0 };

  // Start initial quote escape sequence
  cString o;
  o.appendChar(aQuote);

  const achar* p = aaszString;
  for (;;) {
    const tU32 c = ni::utf8::raw_next(p);
    if (!c)
      break;
    if (c == aQuote) {
      o.append(double_quote);
    }
    else {
      o.appendChar(c);
    }
  }

  // Finish off quote escape
  o.appendChar(aQuote);
  return o;
}

niExportFuncCPP(cString&) StringAppendCsvValue(
  cString& o, const achar aDelim, const achar aQuote,
  const achar* aValue, const tU32 i)
{
  if (i != 0) {
    o.appendChar(aDelim);
  }
  if (StringEncodeCsvShouldQuote(aValue,aQuote,aDelim)) {
    o << StringEncodeCsvQuote(aValue,aQuote);
  }
  else {
    o << aValue;
  }
  return o;
}

__forceinline tBool _StringDecodeCsvShouldUnquote(const char *str, const tU32 aQuote) {
  return niStringIsOK(str) && (ni::utf8::raw_peek(str) == aQuote);
}

// A valid quoted field starts with a quote, trailing spaces are not allowed.
niExportFunc(tBool) StringDecodeCsvShouldUnquote(const char *str, const tU32 aQuote) {
  return _StringDecodeCsvShouldUnquote(str,aQuote);
}

niExportFuncCPP(cString&) StringDecodeCsvUnquote(
  cString& o, const tU32 aQuote, const char *str, tI32 strLen)
{
  // The input and output can't be the same buffer.
  niAssert(!niStringIsOK(str) || (o.Chars() != str));
  if (!_StringDecodeCsvShouldUnquote(str,aQuote)) {
    // doesn't start with a quote, so its not quoted and we just append as-is
    o.appendEx(str,strLen);
  }
  else {
    tBool isQuotedQuote = eFalse;
    StrCharIt it(str,strLen);
    it.next(); // skip the first quote
    while (!it.is_end()) {
      const tU32 c = it.next();
      if (isQuotedQuote) {
        isQuotedQuote = eFalse;
        if (c == aQuote) {
          // "" -> "
          o.push_back('"');
        }
        else {
          o.appendChar(c);
        }
      }
      else if (c == aQuote) {
        isQuotedQuote = eTrue;
      }
      else {
        o.appendChar(c);
      }
    }
  }
  return o;
}

niExportFuncCPP(tU32) StringSplitCsvFields(const cString& aToSplit,
                                           const achar* aaszSeparators,
                                           const tU32 aQuote,
                                           astl::vector<cString>* apOut)
{
  tU32 token_num = 0;
  if (!niIsStringOK(aaszSeparators)) {
    if (!aToSplit.empty()) {
      ++token_num;
      if (apOut) {
        StringDecodeCsvUnquote(apOut->emplace_back(), aQuote, aToSplit);
      }
    }
  }
  else if (!aToSplit.empty()) {
    cString buffer = aToSplit;
    achar* pBuffer = buffer.data();
    achar* token = StrSepQuoted(&pBuffer,aaszSeparators,aQuote);
    if (token) {
      ++token_num;
      if (apOut) {
        StringDecodeCsvUnquote(apOut->emplace_back(), aQuote, token, 0);
      }
    }
    while(token) {
      token = StrSepQuoted(&pBuffer,aaszSeparators,aQuote);
      if (token) {
        ++token_num;
        if (apOut) {
          StringDecodeCsvUnquote(apOut->emplace_back(), aQuote, token, 0);
        }
      }
    }
  }
  return token_num;
}

}
