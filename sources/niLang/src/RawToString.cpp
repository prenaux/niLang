#include "API/niLang/Types.h"
#include "API/niLang/IFile.h"
#include "API/niLang/StringDef.h"
#include "mbedtls/base64.h"

using namespace ni;

//--------------------------------------------------------------------------------------------
//
//  Base32
//
//--------------------------------------------------------------------------------------------

#define BASE32_ENCODED_LENGTH(inlen) ((((inlen) + 4) / 5) * 8)
#define BASE32_DECODED_LENGTH(inlen) (5 * ((inlen) / 8) + 5)

#define BASE32_CROCKFORD

#ifdef BASE32_CROCKFORD
static const char b32str_lwr[33] = "0123456789abcdefghjkmnpqrstvwxyz";
static const char b32str_upr[33] = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";

#define B32(_)                                                          \
  (((_) == '0' || (_) == 'O' || (_) == 'o') ? 0                         \
   : ((_) == '1' || (_) == 'I' || (_) == 'i' || (_) == 'L' || (_) == 'l') ? 1 \
   : (_) == '2' ? 2                                                     \
   : (_) == '3' ? 3                                                     \
   : (_) == '4' ? 4                                                     \
   : (_) == '5' ? 5                                                     \
   : (_) == '6' ? 6                                                     \
   : (_) == '7' ? 7                                                     \
   : (_) == '8' ? 8                                                     \
   : (_) == '9' ? 9                                                     \
   : ((_) == 'A' || (_) == 'a') ? 10                                    \
   : ((_) == 'B' || (_) == 'b') ? 11                                    \
   : ((_) == 'C' || (_) == 'c') ? 12                                    \
   : ((_) == 'D' || (_) == 'd') ? 13                                    \
   : ((_) == 'E' || (_) == 'e') ? 14                                    \
   : ((_) == 'F' || (_) == 'f') ? 15                                    \
   : ((_) == 'G' || (_) == 'g') ? 16                                    \
   : ((_) == 'H' || (_) == 'h') ? 17                                    \
   : ((_) == 'J' || (_) == 'j') ? 18                                    \
   : ((_) == 'K' || (_) == 'k') ? 19                                    \
   : ((_) == 'M' || (_) == 'm') ? 20                                    \
   : ((_) == 'N' || (_) == 'n') ? 21                                    \
   : ((_) == 'P' || (_) == 'p') ? 22                                    \
   : ((_) == 'Q' || (_) == 'q') ? 23                                    \
   : ((_) == 'R' || (_) == 'r') ? 24                                    \
   : ((_) == 'S' || (_) == 's') ? 25                                    \
   : ((_) == 'T' || (_) == 't') ? 26                                    \
   : ((_) == 'V' || (_) == 'v') ? 27                                    \
   : ((_) == 'W' || (_) == 'w') ? 28                                    \
   : ((_) == 'X' || (_) == 'x') ? 29                                    \
   : ((_) == 'Y' || (_) == 'y') ? 30                                    \
   : ((_) == 'Z' || (_) == 'z') ? 31                                    \
   : -1)

#else
static const char b32str[33] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567"; // standard?
#define B32(_)                                  \
  ((_) == 'A' ? 0                               \
   : (_) == 'B' ? 1                             \
   : (_) == 'C' ? 2                             \
   : (_) == 'D' ? 3                             \
   : (_) == 'E' ? 4                             \
   : (_) == 'F' ? 5                             \
   : (_) == 'G' ? 6                             \
   : (_) == 'H' ? 7                             \
   : (_) == 'I' ? 8                             \
   : (_) == 'J' ? 9                             \
   : (_) == 'K' ? 10                            \
   : (_) == 'L' ? 11                            \
   : (_) == 'M' ? 12                            \
   : (_) == 'N' ? 13                            \
   : (_) == 'O' ? 14                            \
   : (_) == 'P' ? 15                            \
   : (_) == 'Q' ? 16                            \
   : (_) == 'R' ? 17                            \
   : (_) == 'S' ? 18                            \
   : (_) == 'T' ? 19                            \
   : (_) == 'U' ? 20                            \
   : (_) == 'V' ? 21                            \
   : (_) == 'W' ? 22                            \
   : (_) == 'X' ? 23                            \
   : (_) == 'Y' ? 24                            \
   : (_) == 'Z' ? 25                            \
   : (_) == '2' ? 26                            \
   : (_) == '3' ? 27                            \
   : (_) == '4' ? 28                            \
   : (_) == '5' ? 29                            \
   : (_) == '6' ? 30                            \
   : (_) == '7' ? 31                            \
   : -1)
#endif

void base32_encode(const char *in, size_t inlen, char *out, size_t* outbufSize, const tBool abUpperCase = eFalse)
{
#ifdef BASE32_CROCKFORD
  const char* b32str = abUpperCase ? b32str_upr : b32str_lwr;
#endif
  size_t outlen = *outbufSize;
  while (inlen && outlen) {
    *out++ = b32str[(tU8(in[0]) >> 3) & 0x1f];
    if (!--outlen)
      break;
    *out++ = b32str[((tU8(in[0]) << 2) + ((--inlen) ? (tU8(in[1]) >> 6) : 0)) & 0x1f];
    if (!--outlen || !inlen)
      break;
    *out++ = b32str[(tU8(in[1]) >> 1) & 0x1f];
    if (!--outlen)
      break;
    *out++ = b32str[((tU8(in[1]) << 4) + ((--inlen) ? tU8(in[2]) >> 4 : 0)) & 0x1f];
    if (!--outlen || !inlen)
      break;
    *out++ = b32str[((tU8(in[2]) << 1) + ((--inlen) ? tU8(in[3]) >> 7 : 0)) & 0x1f];
    if (!--outlen || !inlen)
      break;
    *out++ = b32str[(tU8(in[3]) >> 2) & 0x1f];
    if (!--outlen)
      break;
    *out++ = b32str[((tU8(in[3]) << 3) + ((--inlen) ? tU8(in[4]) >> 5 : 0)) & 0x1f];
    if (!--outlen || !inlen)
      break;
    *out++ = b32str[tU8(in[4]) & 0x1f];
    if (!--outlen)
      break;
    --inlen;
    in += 5;
  }
  if (outlen)
    *out = '\0';
  *outbufSize -= outlen;
}

static const signed char b32[0x100] = {
  B32 (0), B32 (1), B32 (2), B32 (3),
  B32 (4), B32 (5), B32 (6), B32 (7),
  B32 (8), B32 (9), B32 (10), B32 (11),
  B32 (12), B32 (13), B32 (14), B32 (15),
  B32 (16), B32 (17), B32 (18), B32 (19),
  B32 (20), B32 (21), B32 (22), B32 (23),
  B32 (24), B32 (25), B32 (26), B32 (27),
  B32 (28), B32 (29), B32 (30), B32 (31),
  B32 (32), B32 (33), B32 (34), B32 (35),
  B32 (36), B32 (37), B32 (38), B32 (39),
  B32 (40), B32 (41), B32 (42), B32 (43),
  B32 (44), B32 (45), B32 (46), B32 (47),
  B32 (48), B32 (49), B32 (50), B32 (51),
  B32 (52), B32 (53), B32 (54), B32 (55),
  B32 (56), B32 (57), B32 (58), B32 (59),
  B32 (60), B32 (61), B32 (62), B32 (63),
  B32 (32), B32 (65), B32 (66), B32 (67),
  B32 (68), B32 (69), B32 (70), B32 (71),
  B32 (72), B32 (73), B32 (74), B32 (75),
  B32 (76), B32 (77), B32 (78), B32 (79),
  B32 (80), B32 (81), B32 (82), B32 (83),
  B32 (84), B32 (85), B32 (86), B32 (87),
  B32 (88), B32 (89), B32 (90), B32 (91),
  B32 (92), B32 (93), B32 (94), B32 (95),
  B32 (96), B32 (97), B32 (98), B32 (99),
  B32 (100), B32 (101), B32 (102), B32 (103),
  B32 (104), B32 (105), B32 (106), B32 (107),
  B32 (108), B32 (109), B32 (110), B32 (111),
  B32 (112), B32 (113), B32 (114), B32 (115),
  B32 (116), B32 (117), B32 (118), B32 (119),
  B32 (120), B32 (121), B32 (122), B32 (123),
  B32 (124), B32 (125), B32 (126), B32 (127),
  B32 (128), B32 (129), B32 (130), B32 (131),
  B32 (132), B32 (133), B32 (134), B32 (135),
  B32 (136), B32 (137), B32 (138), B32 (139),
  B32 (140), B32 (141), B32 (142), B32 (143),
  B32 (144), B32 (145), B32 (146), B32 (147),
  B32 (148), B32 (149), B32 (150), B32 (151),
  B32 (152), B32 (153), B32 (154), B32 (155),
  B32 (156), B32 (157), B32 (158), B32 (159),
  B32 (160), B32 (161), B32 (162), B32 (163),
  B32 (132), B32 (165), B32 (166), B32 (167),
  B32 (168), B32 (169), B32 (170), B32 (171),
  B32 (172), B32 (173), B32 (174), B32 (175),
  B32 (176), B32 (177), B32 (178), B32 (179),
  B32 (180), B32 (181), B32 (182), B32 (183),
  B32 (184), B32 (185), B32 (186), B32 (187),
  B32 (188), B32 (189), B32 (190), B32 (191),
  B32 (192), B32 (193), B32 (194), B32 (195),
  B32 (196), B32 (197), B32 (198), B32 (199),
  B32 (200), B32 (201), B32 (202), B32 (203),
  B32 (204), B32 (205), B32 (206), B32 (207),
  B32 (208), B32 (209), B32 (210), B32 (211),
  B32 (212), B32 (213), B32 (214), B32 (215),
  B32 (216), B32 (217), B32 (218), B32 (219),
  B32 (220), B32 (221), B32 (222), B32 (223),
  B32 (224), B32 (225), B32 (226), B32 (227),
  B32 (228), B32 (229), B32 (230), B32 (231),
  B32 (232), B32 (233), B32 (234), B32 (235),
  B32 (236), B32 (237), B32 (238), B32 (239),
  B32 (240), B32 (241), B32 (242), B32 (243),
  B32 (244), B32 (245), B32 (246), B32 (247),
  B32 (248), B32 (249), B32 (250), B32 (251),
  B32 (252), B32 (253), B32 (254), B32 (255)
};

#define IS_BASE32_CH(CH) (0 <= b32[(tU8)(CH)])

static bool decode_8(char const *in, char **outp, tInt *outleft)
{
#define return_false                            \
  do {                                          \
    *outp = out;                                \
    return false;                               \
  } while (false)

  char *out = *outp;
  if (!IS_BASE32_CH(in[0]) || !IS_BASE32_CH(in[1]))
    return false;

  if (*outleft)
  {
    *out++ = ((b32[tU8(in[0])] << 3)
              | (b32[tU8(in[1])] >> 2));
    --*outleft;
  }

  if (in[2] == '=')
  {
    if (in[3] != '=' || in[4] != '=' || in[5] != '='
        || in[6] != '=' || in[7] != '=')
      return_false;
  }
  else
  {
    if (!IS_BASE32_CH(in[2]) || !IS_BASE32_CH(in[3]))
      return_false;

    if (*outleft)
    {
      *out++ = ((b32[tU8(in[1])] << 6)
                | (b32[tU8(in[2])] << 1)
                | (b32[tU8(in[3])] >> 4));
      --*outleft;
    }

    if (in[4] == '=')
    {
      if (in[5] != '=' || in[6] != '=' || in[7] != '=')
        return_false;
    }
    else
    {
      if (!IS_BASE32_CH(in[4]))
        return_false;

      if (*outleft)
      {
        *out++ = ((b32[tU8(in[3])] << 4)
                  | (b32[tU8(in[4])] >> 1));
        --*outleft;
      }

      if (in[5] == '=')
      {
        if (in[6] != '=' || in[7] != '=')
          return_false;
      }
      else
      {
        if (!IS_BASE32_CH(in[5]) || !IS_BASE32_CH(in[6]))
          return_false;

        if (*outleft)
        {
          *out++ = ((b32[tU8(in[4])] << 7)
                    | (b32[tU8(in[5])] << 2)
                    | (b32[tU8(in[6])] >> 3));
          --*outleft;
        }

        if (in[7] != '=')
        {
          if (!IS_BASE32_CH(in[7]))
            return_false;

          if (*outleft)
          {
            *out++ = ((b32[tU8(in[6])] << 5)
                      | (b32[tU8(in[7])]));
            --*outleft;
          }
        }
      }
    }
  }

  *outp = out;
  return true;

#undef return_false
}

bool base32_decode(const char *in, tInt inlen, char *out, tInt *outlen)
{
  char buf[8];
  tInt outleft = *outlen;

  const char* s = in;
  while (inlen >= 0) {
    int i = 0, readBytes = 0;
    for ( ; (i < 8) && (inlen >= 0); --inlen) {
      const tU8 ch = *s++;
      if (ch == '\n' || ch == '-' || ch == '_' || ch == '=') {
        // skip ignored characters
        continue;
      }
      else {
        buf[i++] = ch;
        ++readBytes;
      }
    }
    for ( ; i < 8; ++i) {
      buf[i] = '=';
    }

    if (readBytes > 0) {
      if (!decode_8(buf, &out, &outleft))
        break;
    }
  }

  *outlen -= outleft;
  return inlen == 0;
}

namespace ni {
niExportFunc(tInt) Base32EncodeOutputSize(tInt slen) {
  return BASE32_ENCODED_LENGTH(slen) + 1;
}
niExportFunc(tInt) Base32Encode(tPtr dst, tInt dlen, const tPtr src, tInt slen, const tBool abUpperCase) {
  size_t olen = dlen;
  base32_encode((const char*)src, slen, (char*)dst, &olen, abUpperCase);
  return (tInt)olen;
}
niExportFunc(tInt) Base32DecodeOutputSize(const char* src, tInt slen) {
  if (slen <= 0) {
    slen = StrLen(src);
  }
  return BASE32_DECODED_LENGTH(slen);
}
niExportFunc(tInt) Base32Decode(tPtr dst, tInt dlen, const char* src, tInt slen) {
  if (slen <= 0) {
    slen = StrLen(src);
  }
  base32_decode(src, slen, (char*)dst, &dlen);
  dst[dlen] = 0;
  return dlen;
}
niExportFuncCPP(cString) Base32EncodeToString(const tPtr src, tInt slen, const tBool abUpperCase) {
  niAssert(src != NULL);
  cString r;
  const tInt sz = Base32EncodeOutputSize(slen);
  if (sz <= 0) {
    return r;
  }
  r.resize(sz);
  tInt effectiveLen = Base32Encode((tPtr)r.data(), sz, src, slen, abUpperCase);
  // the string's size need to be set to the effective string len, unused chars are set to 0
  r.resize(effectiveLen);
  return r;
}
}

//--------------------------------------------------------------------------------------------
//
//  Base64
//
//--------------------------------------------------------------------------------------------
namespace ni {
niExportFunc(tInt) Base64EncodeOutputSize(tInt slen) {
  size_t olen = 0;
  mbedtls_base64_encode(NULL, 0, &olen, NULL, slen);
  return (tInt)olen;
}
niExportFunc(tInt) Base64Encode(tPtr dst, tInt dlen, const tPtr src, tInt slen) {
  niAssert(dst != NULL);
  niAssert(src != NULL);
  size_t olen = 0;
  if (mbedtls_base64_encode(dst, dlen, &olen, src, slen) != 0) {
    return -1;
  }
  return (tInt)olen;
}

niExportFunc(tInt) Base64DecodeOutputSize(const char* src, tInt slen) {
  niAssert(src != NULL);
  if (slen <= 0) {
    slen = StrLen(src);
  }
  size_t olen = 0;
  mbedtls_base64_decode(NULL, 0, &olen, (tPtr)src, slen);
  return (tInt)olen;
}
niExportFunc(tInt) Base64Decode(tPtr dst, tInt dlen, const char* src, tInt slen) {
  niAssert(dst != NULL);
  niAssert(src != NULL);
  if (slen <= 0) {
    slen = StrLen(src);
  }
  size_t olen = 0;
  if (mbedtls_base64_decode(dst, dlen, &olen, (tPtr)src, slen) != 0) {
    return -1;
  }
  return (tInt)olen;
}

niExportFuncCPP(cString) Base64EncodeToString(const tPtr src, tInt slen) {
  niAssert(src != NULL);
  cString r;
  const tInt sz = Base64EncodeOutputSize(slen);
  if (sz <= 0) {
    return r;
  }
  r.resize(sz);
  tInt effectiveLen = Base64Encode((tPtr)r.data(), sz, src, slen);
  // the string's size need to be set to the effective string len, unused chars are set to 0
  r.resize(effectiveLen);
  return r;
}
}

//--------------------------------------------------------------------------------------------
//
//  Hexadecimal
//
//--------------------------------------------------------------------------------------------
namespace ni {
niExportFunc(tInt) HexEncodeOutputSize(tInt slen) {
  return (slen*2)+1;
}
niExportFunc(tInt) HexEncode(tPtr dst, tInt dlen, const tPtr src, tInt slen, const tBool abUpperCase)
{
  if (dlen < (slen*2)+1) {
    return -1; // buffer too small
  }
  niAssert(dst != NULL);
  niAssert(src != NULL);

  const char* const hex = abUpperCase ? "0123456789ABCDEF" : "0123456789abcdef";
  tPtr s = src;
  char* d = (char*)dst;
  niLoop(i,slen) {
    *(d++) = hex[(*s>>4) & 0xF];
    *(d++) = hex[ *s     & 0xF];
    ++s;
  }
  *d = 0;

  return (slen*2);
}

__forceinline tU8 _HexCharToByte(char input)
{
  if (input >= '0' && input <= '9')
    return input - '0';
  if (input >= 'A' && input <= 'F')
    return input - 'A' + 10;
  if (input >= 'a' && input <= 'f')
    return input - 'a' + 10;
  return 0;
}
niExportFunc(tInt) HexDecodeOutputSize(const char* src, tInt slen) {
  niAssert(src != NULL);
  if (slen <= 0) {
    slen = StrLen(src);
  }
  return slen/2;
}
niExportFunc(tInt) HexDecode(tPtr dst, tInt dlen, const char* src, tInt slen) {
  if (slen <= 0) {
    slen = StrLen(src);
  }
  if (dlen < (slen/2)) {
    return -1; // buffer too small
  }
  niAssert(dst != NULL);
  niAssert(src != NULL);

  const char* s = src;
  tPtr d = dst;
  niLoop(i,slen/2) {
    *(d++) = (_HexCharToByte(*s)<<4) + _HexCharToByte(*(s+1));
    s += 2;
  }
  return slen/2;
}

niExportFuncCPP(cString) HexEncodeToString(const tPtr src, tInt slen, const tBool abUpperCase) {
  niAssert(src != NULL);
  cString r;
  const tInt sz = HexEncodeOutputSize(slen);
  if (sz <= 0) {
    return r;
  }
  r.resize(sz);
  tInt effectiveLen = HexEncode((tPtr)r.data(), sz, src, slen, abUpperCase);
  // the string's size need to be set to the effective string len, unused chars are set to 0
  r.resize(effectiveLen);
  return r;
}
}
