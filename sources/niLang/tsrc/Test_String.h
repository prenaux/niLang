#ifndef __TEST_STRING_H_61ECEE3A_DB03_4C7A_A6A8_1FECA5653B44__
#define __TEST_STRING_H_61ECEE3A_DB03_4C7A_A6A8_1FECA5653B44__

extern "C" const ni::cchar* __cdecl GetTestStringUTF8();
extern "C" const ni::gchar* __cdecl GetTestStringUTF16();
extern "C" const ni::xchar* __cdecl GetTestStringUTF32();
extern "C" const ni::achar* _kaszTestData;
extern "C" const ni::tU32 _knTestStringSize;
extern "C" const ni::tU32 _knTestStringNumChars;
enum eTestDataString {
  eTestDataString_PathFull = 0,
  eTestDataString_PathFullBS_notzeroterminated = 1,
  eTestDataString_PathDirSlash = 2,
  eTestDataString_PathDirNoSlash = 3,
  eTestDataString_PathFile = 4,
  eTestDataString_PathEl1 = 5,
  eTestDataString_PathEl2 = 6,
  eTestDataString_PathEl3 = 7,
  eTestDataString_PathEl4 = 8,
  eTestDataString_PathEl5 = 9,
  eTestDataString_PathEl6 = 10,
  eTestDataString_PathEl7 = 11,
  // path with unicode characters UTF8
  eTestDataString_Bug1 = 12,
  // path with unicode characters UTF8, with forward slashes
  eTestDataString_Bug1Slash = 13,
  // path with GB Chinese characters encoding
  eTestDataString_Bug1GB_notzeroterminated = 14,
  // path with UTF16 characters encoding
  eTestDataString_Bug1U16 = 15,
  eTestDataString_Bug2_notzeroterminated = 16,
  eTestDataString_Last = 17,
};
extern "C" const ni::tU32   GetTestDataStringSize(ni::tU32 aString);
extern "C" const ni::achar* GetTestDataString(ni::tU32 aString);
extern "C" const ni::achar* GetTestDataStringName(ni::tU32 aString);

static const ni::tU32 _knTestDataSize = sizeof(ni::achar)*180;
static const ni::tU32 _nUAigu = 0x000000F9;
static const ni::tU32 _nHao   = 0x0000597D;
static const ni::tU32 _nAlpha = 0x000003B1;
static const ni::tU32 _nBeta  = 0x000003B2;
static const ni::tU32 _nEndTailSize = 3; // tail is 3 bytes = cba

#ifdef niUnicode
# if niUCharSize == 4
#  define ASZ_TEST_STRING GetTestStringUTF32()
# elif niUCharSize == 2
#  define ASZ_TEST_STRING GetTestStringUTF16()
# elif niUCharSize == 1
#  define ASZ_TEST_STRING GetTestStringUTF8()
# endif
#else
# define ASZ_TEST_STRING GetTestStringUTF8()
#endif

extern "C" const char* _MultiLineText;

#endif // __TEST_STRING_H_61ECEE3A_DB03_4C7A_A6A8_1FECA5653B44__
