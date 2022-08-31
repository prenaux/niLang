#include "stdafx.h"
#include "../src/API/niLang/StringDef.h"
#include "../src/API/niLang/Utils/CollectionImpl.h"
#include "../src/API/niLang/STL/utils.h"
#include "../src/API/niLang/STL/string.h"
#include "../src/API/niLang/Utils/Buffer.h"
#include "../src/API/niLang/StringLib.h"
#include "../src/API/niLang/StringLibIt.h"
#include "../src/API/niLang/Utils/StrBreakIt.h"
#include "../src/API/niLang_ModuleDef.h"
#ifndef niEmbedded // no STL in general on embedded devices
#include <string>
#endif

#include "Test_String.h"

// 22 lines, 294 words
const char* _MultiLineText =
    "If you can keep your head when all about you Are losing theirs and blaming it on you,\n"
    "If you can trust yourself when all men doubt you But make allowance for their doubting too,\n"
    "If you can wait and not be tired by waiting, Or being lied about, don't deal in lies,\n"
    "Or being hated, don't give way to hating, And yet don't look too good, nor talk too wise:\n"
    "\n"
    "If you can dream - and not make dreams your master,\n"
    "If you can think - and not make thoughts your aim;\n"
    "If you can meet with Triumph and Disaster And treat those two impostors just the same;\n"
    "If you can bear to hear the truth you've spoken Twisted by knaves to make a trap for fools,\n"
    "Or watch the things you gave your life to, broken, And stoop and build'em up with worn-out tools:\n"
    "\r\n"
    "If you can make one heap of all your winnings And risk it all on one turn of pitch-and-toss, And lose, and start again at your beginnings And never breath a word about your loss;\n"
    "If you can force your heart and nerve and sine To serve your turn long after they are gone, And so hold on when there is nothing in you Except the Will which says to them: \"Hold on!\"\n"
    "\n"
    "If you can talk with crowds and keep your virtue,\n"
    "Or walk with kings'nor lose the common touch,\n"
    "If neither foes nor loving friends can hurt you;\n"
    "If all men count with you, but none too much,\n"
    "If you can fill the unforgiving minute With sixty seconds' worth of distance run, Yours is the Earth and everything that's in it, And - which is more - you'll be a Man, my son!\n"
    "\r\n"
    "-- Rudyard Kipling\n"
    "\n";

static const char* _MultiLineText_Line0 = "If you can keep your head when all about you Are losing theirs and blaming it on you,";
static const char* _MultiLineText_Line4 = "";
static const char* _MultiLineText_Line7 = "If you can meet with Triumph and Disaster And treat those two impostors just the same;";
static const char* _MultiLineText_Line20 = "-- Rudyard Kipling";

static const char* _MultiLineText2 = "<f = Tahoma>< s = 32 >Coucou voici gjp AVT PJ</s></f> gjp un $1,234.56 <s=24><c=RGBA(1,1,0,1)>super</c></s> <b>Mot <i>Tagge</i></b>!\n\nOn se demande d'ailleurs <bc=RGB(0,0,0)>ce</bc> qu'il va se passer a la ligne suivante avec les changements de tailles de fonts qu'on a <b>pu</b> faire.";
static const char* _MultiLineText2_Line0 = "<f = Tahoma>< s = 32 >Coucou voici gjp AVT PJ</s></f> gjp un $1,234.56 <s=24><c=RGBA(1,1,0,1)>super</c></s> <b>Mot <i>Tagge</i></b>!";

extern "C" {
#include "Test_String_Data_PathFull.h"
#include "Test_String_Data_PathFullBS.h"
#include "Test_String_Data_PathDirSlash.h"
#include "Test_String_Data_PathDirNoSlash.h"
#include "Test_String_Data_PathFile.h"
#include "Test_String_Data_PathEl1.h"
#include "Test_String_Data_PathEl2.h"
#include "Test_String_Data_PathEl3.h"
#include "Test_String_Data_PathEl4.h"
#include "Test_String_Data_PathEl5.h"
#include "Test_String_Data_PathEl6.h"
#include "Test_String_Data_PathEl7.h"
#include "Test_String_Data_Bug1.h"
#include "Test_String_Data_Bug1Slash.h"
#include "Test_String_Data_Bug1GB.h"
#include "Test_String_Data_Bug1U16.h"
#include "Test_String_Data_Bug2.h"
}

extern "C" const ni::achar* GetTestDataStringName(ni::tU32 aString) {
  switch (aString) {
    case eTestDataString_PathFull: return "PathFull";
    case eTestDataString_PathFullBS_notzeroterminated: return "PathFullBS";
    case eTestDataString_PathDirSlash: return "PathDirSlash";
    case eTestDataString_PathDirNoSlash: return "PathDirNoSlash";
    case eTestDataString_PathFile: return "PathFile";
    case eTestDataString_PathEl1: return "PathEl1";
    case eTestDataString_PathEl2: return "PathEl2";
    case eTestDataString_PathEl3: return "PathEl3";
    case eTestDataString_PathEl4: return "PathEl4";
    case eTestDataString_PathEl5: return "PathEl5";
    case eTestDataString_PathEl6: return "PathEl6";
    case eTestDataString_PathEl7: return "PathEl7";
    case eTestDataString_Bug1: return "Bug1";
    case eTestDataString_Bug1Slash: return "Bug1Slash";
    case eTestDataString_Bug1GB_notzeroterminated: return "Bug1GB";
    case eTestDataString_Bug1U16: return "Bug1U16";
    case eTestDataString_Bug2_notzeroterminated: return "Bug2";
  }
  niAssert(0 && "Unreachable");
  return NULL;
}
extern "C" const ni::achar* GetTestDataString(ni::tU32 aString) {
  switch (aString) {
    case eTestDataString_PathFull:
      return (const ni::achar*)_kaszTestString_PathFull;
    case eTestDataString_PathFullBS_notzeroterminated:
      return (const ni::achar*)_kaszTestString_PathFullBS_notzeroterminated;
    case eTestDataString_PathDirSlash:
      return (const ni::achar*)_kaszTestString_PathDirSlash;
    case eTestDataString_PathDirNoSlash:
      return (const ni::achar*)_kaszTestString_PathDirNoSlash;
    case eTestDataString_PathFile:
      return (const ni::achar*)_kaszTestString_PathFile;
    case eTestDataString_PathEl1:
      return (const ni::achar*)_kaszTestString_PathEl1;
    case eTestDataString_PathEl2:
      return (const ni::achar*)_kaszTestString_PathEl2;
    case eTestDataString_PathEl3:
      return (const ni::achar*)_kaszTestString_PathEl3;
    case eTestDataString_PathEl4:
      return (const ni::achar*)_kaszTestString_PathEl4;
    case eTestDataString_PathEl5:
      return (const ni::achar*)_kaszTestString_PathEl5;
    case eTestDataString_PathEl6:
      return (const ni::achar*)_kaszTestString_PathEl6;
    case eTestDataString_PathEl7:
      return (const ni::achar*)_kaszTestString_PathEl7;
    case eTestDataString_Bug1:
      return (const ni::achar*)_kaszTestString_Bug1;
    case eTestDataString_Bug1Slash:
      return (const ni::achar*)_kaszTestString_Bug1Slash;
    case eTestDataString_Bug1GB_notzeroterminated:
      return (const ni::achar*)_kaszTestString_Bug1GB_notzeroterminated;
    case eTestDataString_Bug1U16:
      return (const ni::achar*)_kaszTestString_Bug1U16;
    case eTestDataString_Bug2_notzeroterminated:
      return (const ni::achar*)_kaszTestString_Bug2_notzeroterminated;
  }
  niAssert(0 && "Unreachable");
  return NULL;
}
extern "C" const ni::tU32 GetTestDataStringSize(ni::tU32 aString) {
  switch (aString) {
    case eTestDataString_PathFull:
      return _kaszTestString_PathFull_size;
    case eTestDataString_PathFullBS_notzeroterminated:
      return _kaszTestString_PathFullBS_size;
    case eTestDataString_PathDirSlash:
      return _kaszTestString_PathDirSlash_size;
    case eTestDataString_PathDirNoSlash:
      return _kaszTestString_PathDirNoSlash_size;
    case eTestDataString_PathFile:
      return _kaszTestString_PathFile_size;
    case eTestDataString_PathEl1:
      return _kaszTestString_PathEl1_size;
    case eTestDataString_PathEl2:
      return _kaszTestString_PathEl2_size;
    case eTestDataString_PathEl3:
      return _kaszTestString_PathEl3_size;
    case eTestDataString_PathEl4:
      return _kaszTestString_PathEl4_size;
    case eTestDataString_PathEl5:
      return _kaszTestString_PathEl5_size;
    case eTestDataString_PathEl6:
      return _kaszTestString_PathEl6_size;
    case eTestDataString_PathEl7:
      return _kaszTestString_PathEl7_size;
    case eTestDataString_Bug1:
      return _kaszTestString_Bug1_size;
    case eTestDataString_Bug1Slash:
      return _kaszTestString_Bug1Slash_size;
    case eTestDataString_Bug1GB_notzeroterminated:
      return _kaszTestString_Bug1GB_size;
    case eTestDataString_Bug1U16:
      return _kaszTestString_Bug1U16_size;
    case eTestDataString_Bug2_notzeroterminated:
      return _kaszTestString_Bug2_size;
  }
  niAssert(0 && "Unreachable");
  return 0;
}

const ni::tU32 knStrCatSpeedLoops = 10;

// abcddefghiàéèçù€µ°你好αβcba, in UTF8, first 10 chars ascii, chinese char at 18, total 25 chars, 41 bytes
static const ni::tU8 _TestString[] = {
  0x61, 0x62, 0x63, 0x64, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0xc3, 0xa0, 0xc3, 0xa9, 0xc3, 0xa8,
  0xc3, 0xa7, 0xc3, 0xb9, 0xe2, 0x82, 0xac, 0xc2, 0xb5, 0xc2, 0xb0, 0xe4, 0xbd, 0xa0, 0xe5, 0xa5,
  0xbd, 0xce, 0xb1, 0xce, 0xb2, 0x63, 0x62, 0x61, 0x00
};
const ni::tU32 _knTestStringNumChars = 25;
const ni::tU32 _knTestStringSize = sizeof(_TestString);
niCAssert(_knTestStringSize == 41);

const ni::cchar* __cdecl GetTestStringUTF8() {
  return (ni::cchar*)_TestString;
}
const ni::cchar* __cdecl GetTestStringProperty2UTF8() {
  return (ni::cchar*)_TestString;
}
const ni::gchar* __cdecl GetTestStringUTF16() {
  static ni::BufferUTF16 _strUTF16(GetTestStringUTF8());
  // static const ni::cGString _strUTF16(GetTestStringUTF8());
  return _strUTF16.Chars();
}
const ni::xchar* __cdecl GetTestStringUTF32() {
  static ni::BufferUTF32 _strUTF32(GetTestStringUTF8());
  // static const ni::cXString _strUTF32(GetTestStringUTF8());
  return _strUTF32.Chars();
}

static ni::cString _GetUTFPropertyMethod(ni::tBool abSet) {
  ni::cString utfSet = (abSet?_ASTR("Set"):_ASTR("Get"))+GetTestStringUTF8();
  ni::achar* utfSetWriteBuffer = utfSet.data();
  // set the first letter to upper case
  utfSetWriteBuffer[3] = (ni::achar)ni::StrToUpper(utfSet[3]);
  return utfSet;
}

namespace {

// 10 chars
static const ni::achar  _kaszTestString[] = _A("abcddefghi");
static const ni::tU32   _knTestStringLen = 10UL;

const ni::achar* _kaszTestData = _A("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");

struct niCore_String {
  niCore_String() {
  }
  ~niCore_String() {
  }
};

struct niCore_HString {
  niCore_HString() {
  }
  ~niCore_HString() {
  }
};

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,StrFormatNULL) {
  ni::cString str;
  str.Format(_A("ABC [%s]\n"),(void*)NULL);
}


///////////////////////////////////////////////
struct sFuncRetString {
  ni::cString _str;
  void Set(ni::cString v) { _str = v; }
  ni::cString Get() const { return _str; }
  ni::cString GetX(ni::cString v) { return v; }
};
TEST_FIXTURE(niCore_String,FuncRetString) {
  const ni::cString ref = _kaszTestString;
  const ni::cString test = _kaszTestString;
  CHECK_EQUAL(ref,test);
  sFuncRetString r;
  r.Set(test);
  CHECK_EQUAL(ref,r.Get());
  CHECK_EQUAL(ref,r.GetX(test));
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,IndexAndAt) {
  const ni::cString test = _kaszTestString;
  CHECK_EQUAL('a',test[0]);
  CHECK_EQUAL('a',test.at(0));
  CHECK_EQUAL('b',test[1]);
  CHECK_EQUAL('b',test.at(1));
  CHECK_EQUAL('e',test[5]);
  CHECK_EQUAL('e',test.at(5));
  CHECK_EQUAL('i',test[9]);
  CHECK_EQUAL('i',test.at(9));
  CHECK_EQUAL('i',test[test.length()-1]);
  CHECK_EQUAL('i',test.at(test.length()-1));
  CHECK_EQUAL(0,test[test.length()]);
  CHECK_EQUAL(0,test.at(test.length()));
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,SetBeginEnd) {
  const ni::cString ref = _kaszTestString;
  CHECK_EQUAL(ref,ni::cString(GetTestStringUTF8(),GetTestStringUTF8()+_knTestStringLen));
  CHECK_EQUAL(ref,ni::cString(GetTestStringUTF16(),GetTestStringUTF16()+_knTestStringLen));
  CHECK_EQUAL(ref,ni::cString(GetTestStringUTF32(),GetTestStringUTF32()+_knTestStringLen));
  CHECK_EQUAL(ref.slice(0,-3),ni::cString(GetTestStringUTF8(),GetTestStringUTF8()+(_knTestStringLen-3)));
  CHECK_EQUAL(ref.slice(0,-3),ni::cString(GetTestStringUTF16(),GetTestStringUTF16()+(_knTestStringLen-3)));
  CHECK_EQUAL(ref.slice(0,-3),ni::cString(GetTestStringUTF32(),GetTestStringUTF32()+(_knTestStringLen-3)));

  ni::cString test;
  test.Set(GetTestStringUTF8(),GetTestStringUTF8()+_knTestStringLen);
  CHECK_EQUAL(ref,test);
  test.Set(GetTestStringUTF16(),GetTestStringUTF16()+_knTestStringLen);
  CHECK_EQUAL(ref,test);
  test.Set(GetTestStringUTF32(),GetTestStringUTF32()+_knTestStringLen);
  CHECK_EQUAL(ref,test);
  test.Set(GetTestStringUTF8(),GetTestStringUTF8()+(_knTestStringLen-3));
  CHECK_EQUAL(ref.slice(0,-3),test);
  test.Set(GetTestStringUTF16(),GetTestStringUTF16()+(_knTestStringLen-3));
  CHECK_EQUAL(ref.slice(0,-3),test);
  test.Set(GetTestStringUTF32(),GetTestStringUTF32()+(_knTestStringLen-3));
  CHECK_EQUAL(ref.slice(0,-3),test);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,Empty) {
  ni::cString emptyA, emptyB;
  CHECK_EQUAL((ni::tSize)0,emptyA.length());
  CHECK_EQUAL((ni::tSize)0,emptyB.length());
  CHECK_EQUAL(emptyA.c_str(),emptyB.c_str()); // make sure both point to the same static empty string

  emptyA = _kaszTestString;
  emptyA.erase();
  CHECK_EQUAL((ni::tSize)0,emptyA.length());
  CHECK_EQUAL((ni::tSize)0,emptyB.length());
  CHECK_EQUAL(emptyA.c_str(),emptyB.c_str()); // make sure both point to the same static empty string

  ni::cString strEmpty = ni::StringEmpty<ni::achar>();
  CHECK_EQUAL((ni::tSize)0,strEmpty.length());
  CHECK(strEmpty.empty());
  CHECK_EQUAL(0,strEmpty[0]);
  CHECK_EQUAL(0,strEmpty.at(0));
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,Substr) {
  ni::cString testA(_kaszTestString);
  CHECK_EQUAL(ni::cString(_A("ghi")),testA.substr(7));
  CHECK_EQUAL(ni::cString(_A("ghi")),testA.substr(7,3));
  CHECK_EQUAL(ni::cString(_A("abcddefghi")),testA.substr(0));
  CHECK_EQUAL(ni::cString(_A("abcd")),testA.substr(0,4));
  CHECK_EQUAL(ni::cString(_A("dd")),testA.substr(3,2));
  CHECK_EQUAL(ni::cString(_A("ghi")),testA.substr(-3));
  CHECK_EQUAL(ni::cString(_A("abcddefghi")),testA.substr(-10000));
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,Slice) {
  ni::cString testA(_kaszTestString);
  CHECK_EQUAL(ni::cString(_A("ghi")),testA.slice(7));
  CHECK_EQUAL(ni::cString(_A("ghi")),testA.slice(7,10));
  CHECK_EQUAL(ni::cString(_A("abcddefghi")),testA.slice(0));
  CHECK_EQUAL(ni::cString(_A("abcd")),testA.slice(0,4));
  CHECK_EQUAL(ni::cString(_A("dd")),testA.slice(3,5));
  CHECK_EQUAL(ni::cString(_A("ghi")),testA.slice(-3));
  CHECK_EQUAL(ni::cString(_A("abcddefghi")),testA.slice(-10000));
  CHECK_EQUAL(ni::cString(_A("abcddef")),testA.slice(0,-3));
  CHECK_EQUAL(ni::cString(_A("abcddefgh")),testA.slice(0,-1));
  CHECK_EQUAL(ni::cString(_A("")),testA.slice(0,-10000));
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,Split_TestPath1) {
  const ni::cString strTestPath = GetTestDataString(eTestDataString_PathFull);
  ni::Ptr<ni::tStringCVec> vToks = ni::tStringCVec::Create();
  CHECK_EQUAL((ni::tSize)7,StringSplit(strTestPath,_A("/\\"),vToks.ptr()));
  CHECK_EQUAL((ni::tSize)7,vToks->size());
  for (ni::tU32 i = eTestDataString_PathEl1; i <= eTestDataString_PathEl7; ++i) {
    CHECK_EQUAL(_ASTR(GetTestDataString(i)),vToks->at(i-eTestDataString_PathEl1));
  }
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,Split_TestPath2) {
  const ni::cString strTestPath(GetTestDataString(eTestDataString_PathFullBS_notzeroterminated),
                                GetTestDataStringSize(eTestDataString_PathFullBS_notzeroterminated));
  ni::Ptr<ni::tStringCVec> vToks = ni::tStringCVec::Create();
  CHECK_EQUAL((ni::tSize)7,StringSplit(strTestPath,_A("/\\"),vToks.ptr()));
  CHECK_EQUAL((ni::tSize)7,vToks->size());
  for (ni::tU32 i = eTestDataString_PathEl1; i <= eTestDataString_PathEl7; ++i) {
    CHECK_EQUAL(_ASTR(GetTestDataString(i)),vToks->at(i-eTestDataString_PathEl1));
  }
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,Split_TestPath3) {
  const ni::cString strTestPath = _A("this/is/a/test\\path\\with/a/");
  ni::Ptr<ni::tStringCVec> vToks = ni::tStringCVec::Create();
  CHECK_EQUAL((ni::tSize)7,StringSplit(strTestPath,_A("/\\"),vToks.ptr()));
  CHECK_EQUAL((ni::tSize)7,vToks->size());
  CHECK_EQUAL(_ASTR("this"),vToks->at(0));
  CHECK_EQUAL(_ASTR("is"),vToks->at(1));
  CHECK_EQUAL(_ASTR("a"),vToks->at(2));
  CHECK_EQUAL(_ASTR("test"),vToks->at(3));
  CHECK_EQUAL(_ASTR("path"),vToks->at(4));
  CHECK_EQUAL(_ASTR("with"),vToks->at(5));
  CHECK_EQUAL(_ASTR("a"),vToks->at(6));
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,Split_TestPath4) {
  const ni::cString strTestPath = _A("");
  ni::Ptr<ni::tStringCVec> vToks = ni::tStringCVec::Create();
  CHECK_EQUAL((ni::tSize)0,StringSplit(strTestPath,_A("/\\"),vToks.ptr()));
  CHECK_EQUAL((ni::tSize)0,vToks->size());
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,Split_TestPath5) {
  const ni::cString strTestPath = _A("/");
  ni::Ptr<ni::tStringCVec> vToks = ni::tStringCVec::Create();
  CHECK_EQUAL((ni::tSize)0,StringSplit(strTestPath,_A("/\\"),vToks.ptr()));
  CHECK_EQUAL((ni::tSize)0,vToks->size());
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,Split_TestPathNULL1) {
  const ni::cString strTestPath = _A("/");
  ni::Ptr<ni::tStringCVec> vToks = ni::tStringCVec::Create();
  CHECK_EQUAL((ni::tSize)1,StringSplit(strTestPath,(const ni::achar*)NULL,vToks.ptr()));
  CHECK_EQUAL((ni::tSize)1,vToks->size());
  CHECK_EQUAL(_ASTR("/"),vToks->at(0));
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,Split_TestPathNULL2) {
  const ni::cString strTestPath = _A("");
  ni::Ptr<ni::tStringCVec> vToks = ni::tStringCVec::Create();
  CHECK_EQUAL((ni::tSize)0,StringSplit(strTestPath,(const ni::achar*)NULL,vToks.ptr()));
  CHECK_EQUAL((ni::tSize)0,vToks->size());
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,Split_TestPathNULL4) {
  const ni::cString strTestPath = _A("/");
  CHECK_EQUAL((ni::tSize)1,StringSplit(strTestPath,(const ni::achar*)NULL,(astl::vector<ni::cString>*)NULL));
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,Split_TestPathNULL5) {
  const ni::cString strTestPath = _A("");
  CHECK_EQUAL((ni::tSize)0,StringSplit(strTestPath,(const ni::achar*)NULL,(astl::vector<ni::cString>*)NULL));
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,Split_TestPathNULL6) {
  const ni::cString strTestPath = _A("this/is/a/test\\path\\with/a/file.txt");
  CHECK_EQUAL((ni::tSize)8,StringSplit(strTestPath,_A("/\\"),(astl::vector<ni::cString>*)NULL));
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,Erase) {
  ni::cString str = _A("abc");
  str.erase();
  CHECK_EQUAL((ni::tSize)0,str.length());
  str += _A("abc");
  str.erase(1);
  CHECK_EQUAL(ni::cString(_A("a")),str);
  str = _A("abc");
  str.eraseCharAt(0);
  CHECK_EQUAL(ni::cString(_A("bc")),str);
  str = _A("abc");
  str.eraseCharAt(1);
  CHECK_EQUAL(ni::cString(_A("ac")),str);
  str = _A("abc");
  str.eraseCharAt(2);
  CHECK_EQUAL(ni::cString(_A("ab")),str);
  str = _A("abc");
  str.eraseCharAt(-1);
  CHECK_EQUAL(ni::cString(_A("ab")),str);
  str = _A("abc");
  str.eraseCharAt(-2);
  CHECK_EQUAL(ni::cString(_A("ac")),str);
  str = _A("abc");
  str.eraseCharAt(-3);
  CHECK_EQUAL(ni::cString(_A("bc")),str);
}

///////////////////////////////////////////////
TEST(niCore_StrLib_Compare) {
  CHECK(ni::StrCmp("loadme","loadme_id") < 0);
  CHECK(ni::StrCmp("loadme","loadme_id") <= 0);
  CHECK(ni::StrCmp("loadme_id","loadme") > 0);
  CHECK(ni::StrCmp("loadme_id","loadme") >= 0);
  CHECK(ni::StrCmp("loadme","loadme") == 0);
  CHECK(ni::StrCmp("loadme","loadme1") != 0);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,Map_Bug1) {
  typedef ni::cString tString;
  astl::map<tString,tString> map;
  astl::map<tString,tString>::iterator it;
  astl::upsert(map,_ASTR("loadme"),_ASTR("1"));
  it = map.find(_A("loadme"));
  CHECK(it != map.end());
  it = map.find(_A("loadme_id"));
  CHECK(it == map.end()); // was a bug here, find the entry...
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,Compare) {
  ni::cString testA(_kaszTestString);
  ni::cString testB(_kaszTestString);
  ni::cString testC(_kaszTestString); testC.ToUpper();
  ni::cString testD(_A("xyzw"));
  ni::cString defaultConst;
  ni::cString empty = ni::StringEmpty<ni::achar>();

  CHECK_EQUAL(defaultConst,empty);

  CHECK_EQUAL(false,testA == empty);
  CHECK_EQUAL(true,testA != empty);
  CHECK_EQUAL(true,testA >= empty);
  CHECK_EQUAL(false,testA <= empty);
  CHECK_EQUAL(true,testA > empty);
  CHECK_EQUAL(false,testA < empty);
  CHECK_EQUAL(niFalse,testA.IEq(empty));
  CHECK_EQUAL(niFalse,testA.Eq(empty));

  CHECK_EQUAL(true,testA == testB);
  CHECK_EQUAL(false,testA != testB);
  CHECK_EQUAL(true,testA >= testB);
  CHECK_EQUAL(true,testA <= testB);
  CHECK_EQUAL(false,testA > testB);
  CHECK_EQUAL(false,testA < testB);
  CHECK_EQUAL(niTrue,testA.IEq(testB));
  CHECK_EQUAL(niTrue,testA.Eq(testB));

  CHECK_EQUAL(false,testA == testC);
  CHECK_EQUAL(true,testA != testC);
  CHECK_EQUAL(true,testA >= testC);
  CHECK_EQUAL(false,testA <= testC);
  CHECK_EQUAL(true,testA > testC);
  CHECK_EQUAL(false,testA < testC);
  CHECK_EQUAL(niTrue,testA.IEq(testC));
  CHECK_EQUAL(niFalse,testA.Eq(testC));
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,Find) {
  ni::cString test(_kaszTestString);
  CHECK_EQUAL(-1,test.find(_A("abcddefghiiii")));
  CHECK_EQUAL(-1,test.find(_A("xyz")));
  CHECK_EQUAL(0,test.find(_A("abc")));
  CHECK_EQUAL(-1,test.find(_A("ABC")));
  CHECK_EQUAL(1,test.find(_A("bcd")));
  CHECK_EQUAL(3,test.find(_A("dd")));
  CHECK_EQUAL(-1,test.find(_A("DD")));
  CHECK_EQUAL(3,test.find(_A("d")));
  CHECK_EQUAL(9,test.find(_A("i")));
}
TEST_FIXTURE(niCore_String,FindUTF8) {
  ni::cString test(GetTestDataString(eTestDataString_PathFull));
  CHECK_EQUAL(-1,test.find(_A("abcddefghiiii")));
  CHECK_EQUAL(-1,test.find(_A("xyz")));
  CHECK_EQUAL(0,test.find(GetTestDataString(eTestDataString_PathEl1)));
  CHECK_EQUAL(3,test.find(GetTestDataString(eTestDataString_PathEl2)));
  ni::tI32 pos = test.find(GetTestDataString(eTestDataString_PathFile));
  CHECK_EQUAL(_ASTR(test.substr(pos)),_ASTR(GetTestDataString(eTestDataString_PathFile)));
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,RFind) {
  ni::cString test(_kaszTestString);
  CHECK_EQUAL(-1,test.rfind(_A("abcddefghiiii")));
  CHECK_EQUAL(-1,test.rfind(_A("xyz")));
  CHECK_EQUAL(0,test.rfind(_A("abc")));
  CHECK_EQUAL(-1,test.rfind(_A("ABC")));
  CHECK_EQUAL(1,test.rfind(_A("bcd")));
  CHECK_EQUAL(3,test.rfind(_A("dd")));
  CHECK_EQUAL(-1,test.rfind(_A("DD")));
  CHECK_EQUAL(4,test.rfind(_A("d")));
  CHECK_EQUAL(9,test.rfind(_A("i")));
}
TEST_FIXTURE(niCore_String,RFindUTF8) {
  ni::cString test(GetTestDataString(eTestDataString_PathFull));
  CHECK_EQUAL(-1,test.rfind(_A("abcddefghiiii")));
  CHECK_EQUAL(-1,test.rfind(_A("xyz")));
  CHECK_EQUAL(0,test.rfind(GetTestDataString(eTestDataString_PathEl1)));
  CHECK_EQUAL(3,test.rfind(GetTestDataString(eTestDataString_PathEl2)));
  ni::tI32 pos = test.rfind(GetTestDataString(eTestDataString_PathFile));
  CHECK_EQUAL(_ASTR(test.substr(pos)),_ASTR(GetTestDataString(eTestDataString_PathFile)));
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,IFind) {
  ni::cString test(_kaszTestString);
  CHECK_EQUAL(-1,test.ifind(_A("abcddefghiiii")));
  CHECK_EQUAL(-1,test.ifind(_A("xyz")));
  CHECK_EQUAL(0,test.ifind(_A("abc")));
  CHECK_EQUAL(0,test.ifind(_A("ABC")));
  CHECK_EQUAL(1,test.ifind(_A("bcd")));
  CHECK_EQUAL(3,test.ifind(_A("dd")));
  CHECK_EQUAL(3,test.ifind(_A("DD")));
  CHECK_EQUAL(3,test.ifind(_A("d")));
  CHECK_EQUAL(9,test.ifind(_A("i")));
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,IRFind) {
  ni::cString test(_kaszTestString);
  CHECK_EQUAL(-1,test.irfind(_A("abcddefghiiii")));
  CHECK_EQUAL(-1,test.irfind(_A("xyz")));
  CHECK_EQUAL(0,test.irfind(_A("abc")));
  CHECK_EQUAL(0,test.irfind(_A("ABC")));
  CHECK_EQUAL(1,test.irfind(_A("bcd")));
  CHECK_EQUAL(3,test.irfind(_A("dd")));
  CHECK_EQUAL(3,test.irfind(_A("DD")));
  CHECK_EQUAL(4,test.irfind(_A("d")));
  CHECK_EQUAL(9,test.irfind(_A("i")));
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,PushBack) {
  ni::cString test(_kaszTestString);
  test.push_back('X');
  CHECK_EQUAL(ni::cString(_A("abcddefghiX")),test);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,Append) {
  ni::cString test(_kaszTestString);
  test.append(_A("XYZ"));
  CHECK_EQUAL(ni::cString(_A("abcddefghiXYZ")),test);
  test.append(ni::cString(_A("ABC")));
  CHECK_EQUAL(ni::cString(_A("abcddefghiXYZABC")),test);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,AppendWChar) {
  ni::cString test(_kaszTestString);
  test.append(L"XYZ");
  CHECK_EQUAL(ni::cString(_A("abcddefghiXYZ")),test);
  test.append(ni::cString(L"ABC"));
  CHECK_EQUAL(ni::cString(_A("abcddefghiXYZABC")),test);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,InsertPos) {
  ni::cString test(_kaszTestString);
  test.insert(4,_A("X"));
  CHECK_EQUAL(ni::cString(_A("abcdXdefghi")),test);
  test.insert(9,_A("x"));
  CHECK_EQUAL(ni::cString(_A("abcdXdefgxhi")),test);
  test.insert(12,_A("ABC"));
  CHECK_EQUAL(ni::cString(_A("abcdXdefgxhiABC")),test);
  test.insert((ni::tI32)0,_A("WWW"));
  CHECK_EQUAL(ni::cString(_A("WWWabcdXdefgxhiABC")),test);
  test.insert(0xFFFF,_A("123"));
  CHECK_EQUAL(ni::cString(_A("WWWabcdXdefgxhiABC123")),test);
  test.insert(3,_A('1'));
  CHECK_EQUAL(ni::cString(_A("WWW1abcdXdefgxhiABC123")),test);
  test.insert(-3,_A("xyz"));
  CHECK_EQUAL(ni::cString(_A("WWW1abcdXdefgxhiABCxyz123")),test);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,InsertChar) {
  ni::cString test(_kaszTestString);
  test.insert(4,'X');
  CHECK_EQUAL(ni::cString(_A("abcdXdefghi")),test);
  test.insert(9,'x');
  CHECK_EQUAL(ni::cString(_A("abcdXdefgxhi")),test);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,InsertIt) {
  ni::cString test(_kaszTestString);
  test.insert(test.begin()+4,_A("X"));
  CHECK_EQUAL(ni::cString(_A("abcdXdefghi")),test);
  test.insert(test.begin()+9,_A("x"));
  CHECK_EQUAL(ni::cString(_A("abcdXdefgxhi")),test);
  test.insert(test.begin()+12,_A("ABC"));
  CHECK_EQUAL(ni::cString(_A("abcdXdefgxhiABC")),test);
  test.insert(test.begin()+0,_A("WWW"));
  CHECK_EQUAL(ni::cString(_A("WWWabcdXdefgxhiABC")),test);
  test.insert(test.begin()+0xFFFF,_A("123"));
  CHECK_EQUAL(ni::cString(_A("WWWabcdXdefgxhiABC123")),test);
  test.insert(test.begin()+3,_A('1'));
  CHECK_EQUAL(ni::cString(_A("WWW1abcdXdefgxhiABC123")),test);
  test.insert(test.end()-3,_A("xyz"));
  CHECK_EQUAL(ni::cString(_A("WWW1abcdXdefgxhiABCxyz123")),test);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,Mid24) {
  ni::cString test(_kaszTestString);
  CHECK_EQUAL(ni::cString(_A("cdde")),test.Mid(2,4));
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,Mid2InvalidHandle) {
  ni::cString test(_kaszTestString);
  CHECK_EQUAL(ni::cString(_A("cddefghi")),test.Mid(2,ni::eInvalidHandle));
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,MidInvalidHandle) {
  ni::cString test(_kaszTestString);
  CHECK_EQUAL(ni::cString(_A("")),test.Mid(ni::eInvalidHandle,ni::eInvalidHandle));
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,Left3) {
  ni::cString test(_kaszTestString);
  CHECK_EQUAL(ni::cString(_A("abc")),test.Left(3));
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,LeftInvalidHandle) {
  ni::cString test(_kaszTestString);
  CHECK_EQUAL(ni::cString(_kaszTestString),test.Left(ni::eInvalidHandle));
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,Right3) {
  ni::cString test(_kaszTestString);
  CHECK_EQUAL(ni::cString(_A("ghi")),test.Right(3));
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,RightInvalidHandle) {
  ni::cString test(_kaszTestString);
  CHECK_EQUAL(ni::cString(_kaszTestString),test.Right(ni::eInvalidHandle));
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,After) {
  ni::cString test(_kaszTestString);
  CHECK_EQUAL(ni::cString(_A("defghi")),test.After(_A("cd")));
  CHECK_EQUAL(ni::cString(_A("i")),test.After(_A("gh")));
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,RAfter) {
  ni::cString test(_kaszTestString);
  CHECK_EQUAL(ni::cString(_A("efghi")),test.RAfter(_A("d")));
  CHECK_EQUAL(ni::cString(_A("i")),test.RAfter(_A("h")));
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,Before) {
  ni::cString test(_kaszTestString);
  CHECK_EQUAL(ni::cString(_A("ab")),test.Before(_A("cd")));
  CHECK_EQUAL(ni::cString(_A("abcddefgh")),test.Before(_A("i")));
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,RBefore) {
  ni::cString test(_kaszTestString);
  CHECK_EQUAL(ni::cString(_A("abcd")),test.RBefore(_A("d")));
  CHECK_EQUAL(ni::cString(_A("abcddefgh")),test.RBefore(_A("i")));
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,Split) {
  astl::vector<ni::cString> toks;
  ni::cString splitMe = _A("item1,item2;item3,  item4| item5");
  CHECK_EQUAL((ni::tSize)5,StringSplit(splitMe,_A(",;| "),&toks));
  CHECK_EQUAL((ni::tSize)5,toks.size());
  CHECK_EQUAL(ni::cString(_A("item1")),toks[0]);
  CHECK_EQUAL(ni::cString(_A("item2")),toks[1]);
  CHECK_EQUAL(ni::cString(_A("item3")),toks[2]);
  CHECK_EQUAL(ni::cString(_A("item4")),toks[3]);
  CHECK_EQUAL(ni::cString(_A("item5")),toks[4]);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,NI_Str_CatSpeed)
{
  TEST_TIMEREPORT();

  ni::cString str1 = "0123456789";
  ni::cString str2 = "abcdefghij";
  ni::cString concat;

  for( int i = 0; i < knStrCatSpeedLoops; i ++ )
    concat += str1 + str2;
}

///////////////////////////////////////////////
// Test the native implementation for reference
#ifndef niEmbedded // no STL in general on embedded devices
TEST_FIXTURE(niCore_String,STD_Str_CatSpeed)
{
  TEST_TIMEREPORT();

  std::string str1 = "0123456789";
  std::string str2 = "abcdefghij";
  std::string concat;

  for( int i = 0; i < knStrCatSpeedLoops; i ++ )
    concat += str1 + str2;
}
#endif

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,LShiftOp) {
  {
    ni::cString str;
    str << (ni::tI8)-123;
    CHECK_EQUAL(_ASTR("-123"),str);
  }
  {
    ni::cString str;
    str << (ni::tU8)123;
    CHECK_EQUAL(_ASTR("123"),str);
  }
  {
    ni::cString str;
    str << (ni::tI16)-32245;
    CHECK_EQUAL(_ASTR("-32245"),str);
  }
  {
    ni::cString str;
    str << (ni::tU16)65535;
    CHECK_EQUAL(_ASTR("65535"),str);
  }
  {
    ni::cString str;
    str << (ni::tI32)-2012345678;
    CHECK_EQUAL(_ASTR("-2012345678"),str);
  }
  {
    ni::cString str;
    str << (ni::tU32)4123456789UL;
    CHECK_EQUAL(_ASTR("4123456789"),str);
  }
  {
    ni::cString str;
    str << (ni::tI64)-123456789123456789LL;
    CHECK_EQUAL(_ASTR("-123456789123456789"),str);
  }
  {
    ni::cString str;
    str << (ni::tU64)123456789123456789ULL;
    CHECK_EQUAL(_ASTR("123456789123456789"),str);
  }
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,AppendSelf) {
  ni::cString str = "abc";
  CHECK_EQUAL(3,str.length());
  str.append(str);
  CHECK_EQUAL(6,str.length());
  str.append(str);
  CHECK_EQUAL(12,str.length());
  CHECK_EQUAL(_ASTR("abcabcabcabc"),str);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,CatSelf) {
  ni::cString str = "abc";
  niLoop(i,3) {
    str += str;
  }
  CHECK_EQUAL(_ASTR("abcabcabcabcabcabcabcabc"),str);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,CatSelfLShift) {
  ni::cString str = "abc";
  niLoop(i,3) {
    str << str;
  }
  CHECK_EQUAL(_ASTR("abcabcabcabcabcabcabcabc"),str);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,Compact) {
  ni::cString str = "abc";
  niLoop(i,3) {
    str << str;
  }
  CHECK_EQUAL(_ASTR("abcabcabcabcabcabcabcabc"),str);
  CHECK(str.capacity() > str.length());
  str.compact();
  CHECK_EQUAL(str.length()+1,str.capacity());
  CHECK_EQUAL(_ASTR("abcabcabcabcabcabcabcabc"),str);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,CopyConstructor) {
  ni::cString a = "abc";
  ni::cString b = L"abc";
  ni::cString e = _ASTR("abc");
  CHECK_EQUAL(_ASTR("abc"),a);
  CHECK_EQUAL(_ASTR("abc"),b);
  CHECK_EQUAL(_ASTR("abc"),e);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,StrFormatConvert) {
  niDebugFmt((_A("CSZ: %s"),GetTestStringUTF8()));
  niDebugFmt((_A("USZ: %s"),niToAChars(GetTestStringUTF16())));
  niDebugFmt((_A("XSZ: %s"),niToAChars(GetTestStringUTF32())));
  niDebugFmt((_A("ASZ: %s"),ASZ_TEST_STRING));
  ni::cString f = GetTestStringUTF16();
  // ni::cUString u = _USTR(GetTestStringUTF16());
  ni::cString v = _ASTR(GetTestStringUTF32());
  ni::cString vf = _ASTR(GetTestStringUTF32()).Chars();

  ni::BufferUTF8 utf8(ASZ_TEST_STRING);
  ni::BufferUTF8 nat(utf8.Chars());
  // ni::cString z = nat.Chars();
  niDebugFmt((_A("NATIVE: %s"),nat.Chars()));
  niDebugFmt((_A("UTF8: %s"),utf8.Chars()));
  niDebugFmt((_A("VF: %s"),vf));
  niDebugFmt((_A("V: %s"),v));
  // niDebugFmt((_A("U: %s"),u.Chars()));
  // niDebugFmt((_A("W: %s"),w.Chars()));
  // niDebugFmt((_A("WF: %s"),wf.Chars()));
  // CHECK_EQUAL(22,z.length());
  // CHECK_EQUAL(22,u.length());
  // CHECK_EQUAL(u,z);
  // CHECK_EQUAL(u,v);
  // CHECK_EQUAL(u,w);
  // CHECK_EQUAL(v,w);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,Short) {
  CHECK_EQUAL(_ASTR("a"),_ASTR("a"));
  CHECK_EQUAL(_ASTR(L"a"),_ASTR(L"a"));
  CHECK_EQUAL(_ASTR("0"),_ASTR("0"));
  CHECK_EQUAL(_ASTR(L"0"),_ASTR(L"0"));
  CHECK_EQUAL(1,_ASTR("a").length());
  CHECK_EQUAL(1,_ASTR(L"a").length());
  CHECK_EQUAL(1,_ASTR("0").length());
  CHECK_EQUAL(1,_ASTR(L"0").length());
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,CharIt) {
  CHECK_EQUAL(_nHao,_ASTR(GetTestStringUTF8()).charIt(19).peek_next());
  CHECK_EQUAL(6,_ASTR(GetTestStringUTF8()).charIt(19).length());
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,FindChar) {
  const ni::cString str = GetTestStringUTF8();
  CHECK_EQUAL(1,str.find('b'));
  CHECK_EQUAL(38,str.rfind('b'));
  CHECK_EQUAL(30,str.find(_nHao));
  CHECK_EQUAL(30,str.rfind(_nHao));
  CHECK_EQUAL(33,str.find(_nAlpha));
  CHECK_EQUAL(33,str.rfind(_nAlpha));
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,PropertyName) {
  ni::cString tmp;
  const ni::cString utfSet = _GetUTFPropertyMethod(ni::eTrue);
  niDebugFmt(("UTFSET: %s",utfSet));
  const ni::cString utfGet = _GetUTFPropertyMethod(ni::eFalse);
  niDebugFmt(("UTFGET: %s",utfGet));
  CHECK_EQUAL(_ASTR("name"),StringToPropertyName(tmp,"SetName"));
  CHECK_EQUAL(_ASTR("name"),StringToPropertyName(tmp,"GetName"));
  CHECK_EQUAL(_ASTR("context3_d"),StringToPropertyName(tmp,"SetContext3D"));
  CHECK_EQUAL(_ASTR("context3_d"),StringToPropertyName(tmp,"GetContext3D"));
  CHECK_EQUAL(_ASTR("context55_d"),StringToPropertyName(tmp,"SetContext55D"));
  CHECK_EQUAL(_ASTR("context55_d"),StringToPropertyName(tmp,"GetContext55D"));
  CHECK_EQUAL(_ASTR("fill_color4"),StringToPropertyName(tmp,"SetFillColor4"));
  CHECK_EQUAL(_ASTR("fill_color4"),StringToPropertyName(tmp,"GetFillColor4"));
  CHECK_EQUAL(_ASTR("color4_from_name"),StringToPropertyName(tmp,"GetColor4FromName"));
  CHECK_EQUAL(_ASTR("color4_from_name"),StringToPropertyName(tmp,"Color4FromName"));
  CHECK_EQUAL(_ASTR("vec3"),StringToPropertyName(tmp,"SetVec3"));
  CHECK_EQUAL(_ASTR("vec3"),StringToPropertyName(tmp,"GetVec3"));
  CHECK_EQUAL(_ASTR("my_cool_property"),StringToPropertyName(tmp,"SetMyCoolProperty"));
  CHECK_EQUAL(_ASTR("my_cool_property"),StringToPropertyName(tmp,"GetMyCoolProperty"));
  CHECK_EQUAL(_ASTR(GetTestStringUTF8()),StringToPropertyName(tmp,utfSet.Chars()));
  CHECK_EQUAL(_ASTR(GetTestStringUTF8()),StringToPropertyName(tmp,utfGet.Chars()));
  CHECK_EQUAL(_ASTR("Name"),StringToPropertyMethodName(tmp,"name"));
  CHECK_EQUAL(_ASTR("Context3D"),StringToPropertyMethodName(tmp,"context3d"));
  CHECK_EQUAL(_ASTR("Context55D"),StringToPropertyMethodName(tmp,"context55d"));
  CHECK_EQUAL(_ASTR("MyCoolProperty"),StringToPropertyMethodName(tmp,"my_cool_property"));
  CHECK_EQUAL(utfSet,_ASTR("Set")+StringToPropertyMethodName(tmp,GetTestStringUTF8()));
  CHECK_EQUAL(utfGet,_ASTR("Get")+StringToPropertyMethodName(tmp,GetTestStringUTF8()));
  CHECK_EQUAL(_ASTR("loaded_module_from_id"),StringToPropertyName(tmp,"GetLoadedModuleFromID"));
}

TEST_FIXTURE(niCore_String,PropertyName2) {
  ni::cString tmp;
  const ni::cString utfSet = _GetUTFPropertyMethod(ni::eTrue);
  niDebugFmt(("UTFSET: %s",utfSet));
  const ni::cString utfGet = _GetUTFPropertyMethod(ni::eFalse);
  niDebugFmt(("UTFGET: %s",utfGet));
  CHECK_EQUAL(_ASTR("name"),StringToPropertyName2(tmp,"SetName"));
  CHECK_EQUAL(_ASTR("name"),StringToPropertyName2(tmp,"GetName"));
  CHECK_EQUAL(_ASTR("context3D"),StringToPropertyName2(tmp,"SetContext3D"));
  CHECK_EQUAL(_ASTR("context3D"),StringToPropertyName2(tmp,"GetContext3D"));
  CHECK_EQUAL(_ASTR("context55D"),StringToPropertyName2(tmp,"SetContext55D"));
  CHECK_EQUAL(_ASTR("context55D"),StringToPropertyName2(tmp,"GetContext55D"));
  CHECK_EQUAL(_ASTR("myCoolProperty"),StringToPropertyName2(tmp,"SetMyCoolProperty"));
  CHECK_EQUAL(_ASTR("myCoolProperty"),StringToPropertyName2(tmp,"GetMyCoolProperty"));
  CHECK_EQUAL(_ASTR(GetTestStringProperty2UTF8()),StringToPropertyName2(tmp,utfSet.Chars()));
  CHECK_EQUAL(_ASTR(GetTestStringProperty2UTF8()),StringToPropertyName2(tmp,utfGet.Chars()));
  CHECK_EQUAL(_ASTR("Name"),StringToPropertyMethodName2(tmp,"name"));
  CHECK_EQUAL(_ASTR("Context3D"),StringToPropertyMethodName2(tmp,"context3D"));
  CHECK_EQUAL(_ASTR("Context55D"),StringToPropertyMethodName2(tmp,"context55D"));
  CHECK_EQUAL(_ASTR("MyCoolProperty"),StringToPropertyMethodName2(tmp,"myCoolProperty"));
  CHECK_EQUAL(utfSet,_ASTR("Set")+StringToPropertyMethodName2(tmp,GetTestStringProperty2UTF8()));
  CHECK_EQUAL(utfGet,_ASTR("Get")+StringToPropertyMethodName2(tmp,GetTestStringProperty2UTF8()));
  CHECK_EQUAL(_ASTR("loadedModuleFromID"),StringToPropertyName2(tmp,"GetLoadedModuleFromID"));
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,StartsWith) {
  CHECK(ni::StrStartsWith("};","};"));
  CHECK(!ni::StrStartsWith("};",""));
  CHECK(ni::StrEndsWith("};",""));
  CHECK(ni::StrStartsWithI("};","};"));
  CHECK(!ni::StrStartsWithI("};",""));
  CHECK(ni::StrEndsWithI("};",""));
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,EqEmpty) {
  ni::tHStringPtr hspEmpty = _H("");
  CHECK(ni::StrCmp("","") == 0);
  CHECK(ni::StrCmp(NULL,NULL) == 0);
  CHECK(ni::StrEq("",""));
  CHECK(ni::StrEq(NULL,NULL));
  CHECK(_H("") == _H(""));
  CHECK(hspEmpty == _H(""));
  CHECK(ni::StrCmp(niHStr(hspEmpty),"") == 0);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,PrintTestData) {
  niLoop(i,eTestDataString_Last) {
    // copy in a cString since some of the test strings are not zero terminated
    const ni::cString v(GetTestDataString((eTestDataString)i), GetTestDataStringSize((eTestDataString)i));
    niDebugFmt(("TESTDATA[%d,%s]: (%d) '%s'",
                i,
                GetTestDataStringName((eTestDataString)i),
                GetTestDataStringSize((eTestDataString)i),
                v));
  }
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,EndsWith2) {
  ni::cString v(GetTestDataString(eTestDataString_Bug2_notzeroterminated), GetTestDataStringSize(eTestDataString_Bug2_notzeroterminated));
  CHECK(!ni::StrEndsWith(v.Chars(),":/"));
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,BreakLineIt) {
  ni::StrBreakLineIt lineIt(_MultiLineText);
  ni::tU32 numLines = 0;
  while (!lineIt.is_end()) {
    switch (numLines) {
      case 0:
        CHECK_EQUAL(_ASTR(_MultiLineText_Line0),_ASTR(lineIt.current()));
        break;
      case 4:
        CHECK_EQUAL(0,lineIt.current().length());
        CHECK_EQUAL(_ASTR(_MultiLineText_Line4),_ASTR(lineIt.current()));
        break;
      case 7:
        CHECK_EQUAL(_ASTR(_MultiLineText_Line7),_ASTR(lineIt.current()));
        break;
      case 20:
        CHECK_EQUAL(_ASTR(_MultiLineText_Line20),_ASTR(lineIt.current()));
        break;
      case 21:
        CHECK_EQUAL(0,lineIt.current().length());
        CHECK_EQUAL(_ASTR(_MultiLineText_Line4),_ASTR(lineIt.current()));
        break;
    }
    lineIt.next();
    ++numLines;
  }
  CHECK_EQUAL(22,numLines);

  lineIt.to_start();
  lineIt.advance(7);
  CHECK_EQUAL(_ASTR(_MultiLineText_Line7),_ASTR(lineIt.current()));

  lineIt.advance(50);
  CHECK(lineIt.is_end());
  CHECK(lineIt.current().length() == 0);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,BreakLineIt2) {
  ni::StrBreakLineIt lineIt(_MultiLineText2);
  ni::tU32 numLines = 0;
  while (!lineIt.is_end()) {
    switch (numLines) {
      case 0:
        CHECK_EQUAL(_ASTR(_MultiLineText2_Line0),_ASTR(lineIt.current()));
        break;
      case 1:
        CHECK_EQUAL(0,lineIt.current().length());
        break;
      case 2:
        CHECK(lineIt.current().length() > 0);
        break;
    }
    lineIt.next();
    ++numLines;
  }
  CHECK_EQUAL(3,numLines);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,BreakLineIt3) {
  ni::StrBreakLineIt lineIt(_MultiLineText_Line0);
  ni::tU32 numLines = 0;
  while (!lineIt.is_end()) {
    const ni::StrCharIt& theLine = lineIt.current();
    CHECK_EQUAL(_ASTR(_MultiLineText_Line0),_ASTR(theLine));
    lineIt.next();
    ++numLines;
  }
  CHECK_EQUAL(1,numLines);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,BreakWordIt) {
  ni::StrBreakWordIt wordIt(_MultiLineText);
  ni::tU32 numWords = 0;
  while (!wordIt.is_end()) {
    wordIt.next();
    ++numWords;
  }
  CHECK_EQUAL(289,numWords);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,BreakWordIt2) {
  ni::StrBreakWordIt wordIt("hello!");
  ni::tU32 numWords = 0;
  while (!wordIt.is_end()) {
    const ni::StrCharIt& theWord = wordIt.current();
    CHECK_EQUAL(_ASTR("hello"),_ASTR(theWord));
    wordIt.next();
    ++numWords;
  }
  CHECK_EQUAL(1,numWords);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,BreakSpaceIt) {
  ni::StrBreakSpaceIt wordIt(_MultiLineText);
  ni::tU32 numSpaces = 0;
  while (!wordIt.is_end()) {
    // const ni::StrCharIt& theSpace = wordIt.current();
    // niDebugFmt(("W[%d]: '%s' (sep: '%s')",numSpaces,_ASTR(theSpace),_ASTR(wordIt.breaker().lastSeparator)));
    wordIt.next();
    ++numSpaces;
  }
  CHECK_EQUAL(294,numSpaces);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,BreakSpaceIt2) {
  ni::StrBreakSpaceIt wordIt("hello!");
  ni::tU32 numWords = 0;
  while (!wordIt.is_end()) {
    const ni::StrCharIt& theWord = wordIt.current();
    CHECK_EQUAL(_ASTR("hello!"),_ASTR(theWord));
    wordIt.next();
    ++numWords;
  }
  CHECK_EQUAL(1,numWords);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,BreakChrIt) {
  ni::StrBreakChrIt breakIt(_MultiLineText,",;");
  ni::tU32 numBreaks = 0;
  while (!breakIt.is_end()) {
    // const ni::StrCharIt& theBreak = breakIt.current();
    // niDebugFmt(("W[%d]: '%s' (sep: '%s')",numBreaks,_ASTR(theBreak),_ASTR(breakIt.breaker().lastSeparator)));
    breakIt.next();
    ++numBreaks;
  }
  CHECK_EQUAL(27,numBreaks);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,Normalize) {
  ni::cString str1("  Hello   World ");
  ni::cString str2("  Hello  World ");
  ni::cString str3("   Hello   World   ");
  ni::cString str4(" Hello World ");
  ni::cString str5("Hello World");
  ni::cString str6("  Hello   World    ");
  CHECK_EQUAL(_ASTR("Hello World"),_ASTR(str1.Normalize()));
  CHECK_EQUAL(_ASTR("Hello World"),_ASTR(str2.Normalize()));
  CHECK_EQUAL(_ASTR("Hello World"),_ASTR(str3.Normalize()));
  CHECK_EQUAL(_ASTR("Hello World"),_ASTR(str4.Normalize()));
  CHECK_EQUAL(_ASTR("Hello World"),_ASTR(str5.Normalize()));
  CHECK_EQUAL(_ASTR("Hello World"),_ASTR(str6.Normalize()));
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_String,StrGetUCPProps) {
  using namespace ni;

  for (int i = 32; i < 127; ++i) {
    ni::tU32 charType = 0, script = 0;
    ni::tU32 cat = ni::StrGetUCPProps(i, &charType, &script);
    niDebugFmt(("CHAR[%3d,%02X]: %c, category: %s (%d), charType: %s (%d), script: %s (%d)",
                i, i, i,
                niEnumToChars(eUCPCategory, cat), cat,
                niEnumToChars(eUCPCharType, charType), charType,
                niEnumToChars(eUCPScript, script), script));
  }
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_HString,Basics) {
  using namespace ni;

  tHStringPtr hspA = _H("A");
  tHStringPtr hspA1 = _H("A");
  tHStringPtr hspB = _H("B");
  tHStringPtr hspB1 = _H("B");
  CHECK_EQUAL((tI64)hspA.ptr(),(tI64)hspA1.ptr());
  CHECK_EQUAL(_ASTR("A"),_ASTR(niHStr(hspA)));
  CHECK_EQUAL((tI64)hspB.ptr(),(tI64)hspB1.ptr());
  CHECK_EQUAL(_ASTR("B"),_ASTR(niHStr(hspB)));
}

///////////////////////////////////////////////
// Test the compiler `#ifdef niTypeIntIsOtherType`
TEST_FIXTURE(niCore_String,IntIsOtherType) {
  ni::cString r;
  r << (int)-123;
  r << ",";
  r << (unsigned)789;
  r << ",";
  r << (ni::tInt)-123;
  r << ",";
  r << (ni::tUInt)789;
  r << ",";
  r << -123;
  r << ",";
  r << 789;
  CHECK_EQUAL(_ASTR("-123,789,-123,789,-123,789"), r);

  CHECK_EQUAL(_ASTR("-123"), r.Set((int)-123));
  CHECK_EQUAL(_ASTR("789"), r.Set((unsigned)789));
  CHECK_EQUAL(_ASTR("-123"), r.Set((ni::tInt)-123));
  CHECK_EQUAL(_ASTR("789"), r.Set((ni::tUInt)789));
  CHECK_EQUAL(_ASTR("-123"), r.Set(-123));
  CHECK_EQUAL(_ASTR("789"), r.Set(789));
}

///////////////////////////////////////////////
// Test the compiler `#ifdef niTypeIntPtrIsOtherType`
TEST_FIXTURE(niCore_String,IntPtrIsOtherType) {
  ni::cString r;
  r << (ni::tIntPtr)-321;
  r << ",";
  r << (ni::tUIntPtr)987;
  r << ",";
  r << (intptr_t)-321;
  r << ",";
  r << (uintptr_t)987;
  CHECK_EQUAL(_ASTR("-321,987,-321,987"), r);

  CHECK_EQUAL(_ASTR("-321"), r.Set((ni::tIntPtr)-321));
  CHECK_EQUAL(_ASTR("987"), r.Set((ni::tUIntPtr)987));
  CHECK_EQUAL(_ASTR("-321"), r.Set((intptr_t)-321));
  CHECK_EQUAL(_ASTR("987"), r.Set((uintptr_t)987));
}

} // end of anonymous namespace
