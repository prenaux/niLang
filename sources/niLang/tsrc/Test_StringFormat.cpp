#include "stdafx.h"
#include <niLang/StringDef.h>
#include <niLang/Utils/CollectionImpl.h>
#include <niLang/STL/utils.h>

struct niCore_StringFormat {};
struct niCore_CatFormat {};

//--------------------------------------------------------------------------------------------
//
//  Unicode-aware sprintf() functions
//
//--------------------------------------------------------------------------------------------

///////////////////////////////////////////////
TEST_FIXTURE(niCore_StringFormat,StrNULL) {
  ni::cString str;
  str.Format(_A("%s"),(void*)NULL);
#ifndef niCore_StringBase_CatFormat
  CHECK_EQUAL(_ASTR("?NULL?"),str);
#else
  CHECK_EQUAL(_ASTR("0"),str);
#endif
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_StringFormat,Str) {
  ni::cString str;
  str.Format(_A("%s"),_A("HelloWorld"));
  CHECK_EQUAL(_ASTR("HelloWorld"),str);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_StringFormat,Ptr) {
  ni::cString str;
#ifdef ni64
  str.Format(_A("%p"),0);
  CHECK_EQUAL(_ASTR("0000000000000000"),str);
  void* p = (void*)0xABCD0000FFFF0000;
  str.Format(_A("%p"),p);
  CHECK_EQUAL(_ASTR("ABCD0000FFFF0000"),str);
#else
  str.Format(_A("%p"),0);
  CHECK_EQUAL(_ASTR("00000000"),str);
  void* p = (void*)0xABCD0000;
  str.Format(_A("%p"),p);
  CHECK_EQUAL(_ASTR("ABCD0000"),str);
#endif
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_StringFormat,I8) {
  ni::cString str;
  ni::tI8 v = 123;
  str.Format(_A("%d,%d"),v,v);
  CHECK_EQUAL(_ASTR("123,123"),str);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_StringFormat,Int) {
  ni::cString str;
  int v = 2012345678;
  str.Format(_A("%d,%d"),v,v);
  CHECK_EQUAL(_ASTR("2012345678,2012345678"),str);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_StringFormat,UInt) {
  ni::cString str;
  unsigned int v = 2012345678;
  str.Format(_A("%d,%d"),v,v);
  CHECK_EQUAL(_ASTR("2012345678,2012345678"),str);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_StringFormat,U32) {
  ni::cString str;
  ni::tU32 v = 2012345678;
  str.Format(_A("%d,%d"),v,v);
  CHECK_EQUAL(_ASTR("2012345678,2012345678"),str);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_StringFormat,U64) {
  ni::cString str;
  ni::tU64 v = 2000000000000ULL;
  str.Format(_A("%lld,%lld"),v,v);
  CHECK_EQUAL(_ASTR("2000000000000,2000000000000"),str);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_CatFormat,Empty) {
  ni::cString str;
  StringCatFormat(str,_A(""),_A("HelloWorld"));
  CHECK_EQUAL(_ASTR(""),str);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_CatFormat,Str) {
  ni::cString str;
  StringCatFormat(str,_A("%s"),_A("HelloWorld"));
  CHECK_EQUAL(_ASTR("HelloWorld"),str);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_CatFormat,CStr) {
  ni::cString str;
  ni::StringCatFormat(str,_A("%s"),_ASTR("HelloWorld"));
  CHECK_EQUAL(_ASTR("HelloWorld"),str);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_CatFormat,HStr) {
  ni::cString str;
  ni::StringCatFormat(str,_A("%s"),_H("HelloWorld"));
  CHECK_EQUAL(_ASTR("HelloWorld"),str);
}
TEST_FIXTURE(niCore_CatFormat,HStrP) {
  ni::cString str;
  ni::tHStringPtr ptrHW = _H("HelloWorld");
  ni::iHString* pHW = ptrHW;
  ni::StringCatFormat(str,_A("%s"),pHW);
  CHECK_EQUAL(_ASTR("HelloWorld"),str);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_CatFormat,Str3NULL) {
  ni::cString str;
  ni::StringCatFormat(str,_A("%3s"),(char*)NULL);
  CHECK_EQUAL(_ASTR("   "),str);
}
TEST_FIXTURE(niCore_CatFormat,StrRightInvalid) {
  ni::cString str;
  ni::StringCatFormat(str,_A("%10s"),"hello");
  CHECK_EQUAL(_ASTR("     hello"),str);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_CatFormat,Int) {
  ni::cString str;
  StringCatFormat(str,_A("%d"),(ni::tI32)-123);
  CHECK_EQUAL(_ASTR("-123"),str);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_CatFormat,Float) {
  ni::cString str;
  StringCatFormat(str,_A("%g"),123.456);
  CHECK_EQUAL(_ASTR("123.456"),str);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_CatFormat,FloatStr) {
  ni::cString str;
  StringCatFormat(str,_A("%g"),_A("123.456"));
  CHECK_EQUAL(_ASTR("123.456"),str);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_CatFormat,Vec2) {
  ni::cString str;
  ni::StringCatFormat(str,_A("%s"),ni::Vec2<ni::tI32>(1,2));
  CHECK_EQUAL(_ASTR("Vec2(1,2)"),str);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_CatFormat,I8) {
  ni::cString str;
  ni::tI8 v = 123;
  ni::StringCatFormat(str,_A("%d,%d"),v,v);
  CHECK_EQUAL(_ASTR("123,123"),str);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_CatFormat,I32) {
  ni::cString str;
  ni::tI32 v = 2012345678;
  ni::StringCatFormat(str,_A("%d,%d"),v,v);
  CHECK_EQUAL(_ASTR("2012345678,2012345678"),str);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_CatFormat,U32) {
  ni::cString str;
  ni::tU32 v = 2012345678;
  ni::StringCatFormat(str,_A("%d,%d"),v,v);
  CHECK_EQUAL(_ASTR("2012345678,2012345678"),str);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_CatFormat,U64) {
  ni::cString str;
  ni::tU64 v = 2000000000000ULL;
  ni::StringCatFormat(str,_A("%lld,%lld"),v,v);
  CHECK_EQUAL(_ASTR("2000000000000,2000000000000"),str);
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_CatFormat,A0) {
  ni::cString str;
  ni::StringCatFormat(str,_A("L:%d,%d,%d,%d,%d,%d,%d,%d"));
  // no args so there should be no formatting either
  CHECK_EQUAL(_ASTR("L:%d,%d,%d,%d,%d,%d,%d,%d"),str);
}
TEST_FIXTURE(niCore_CatFormat,A1) {
  ni::cString str;
  ni::StringCatFormat(str,_A("L:%d,%d,%d,%d,%d,%d,%d,%d"),
                      (ni::tI32)1);
  CHECK_EQUAL(_ASTR("L:1,0,0,0,0,0,0,0"),str);
}
TEST_FIXTURE(niCore_CatFormat,A2) {
  ni::cString str;
  ni::StringCatFormat(str,_A("L:%d,%d,%d,%d,%d,%d,%d,%d"),
                      (ni::tI32)1,(ni::tI32)2);
  CHECK_EQUAL(_ASTR("L:1,2,0,0,0,0,0,0"),str);
}
TEST_FIXTURE(niCore_CatFormat,A3) {
  ni::cString str;
  ni::StringCatFormat(str,_A("L:%d,%d,%d,%d,%d,%d,%d,%d"),
                      (ni::tI32)1,(ni::tI32)2,(ni::tI32)3);
  CHECK_EQUAL(_ASTR("L:1,2,3,0,0,0,0,0"),str);
}
TEST_FIXTURE(niCore_CatFormat,A4) {
  ni::cString str;
  ni::StringCatFormat(str,_A("L:%d,%d,%d,%d,%d,%d,%d,%d"),
                      (ni::tI32)1,(ni::tI32)2,(ni::tI32)3,(ni::tI32)4);
  CHECK_EQUAL(_ASTR("L:1,2,3,4,0,0,0,0"),str);
}
TEST_FIXTURE(niCore_CatFormat,A5) {
  ni::cString str;
  ni::StringCatFormat(str,_A("L:%d,%d,%d,%d,%d,%d,%d,%d"),
                      (ni::tI32)1,(ni::tI32)2,(ni::tI32)3,(ni::tI32)4,(ni::tI32)5);
  CHECK_EQUAL(_ASTR("L:1,2,3,4,5,0,0,0"),str);
}
TEST_FIXTURE(niCore_CatFormat,A6) {
  ni::cString str;
  ni::StringCatFormat(str,_A("L:%d,%d,%d,%d,%d,%d,%d,%d"),
                      (ni::tI32)1,(ni::tI32)2,(ni::tI32)3,(ni::tI32)4,
                      (ni::tI32)5,(ni::tI32)6);
  CHECK_EQUAL(_ASTR("L:1,2,3,4,5,6,0,0"),str);
}
TEST_FIXTURE(niCore_CatFormat,A7) {
  ni::cString str;
  ni::StringCatFormat(str,_A("L:%d,%d,%d,%d,%d,%d,%d,%d"),
                      (ni::tI32)1,(ni::tI32)2,(ni::tI32)3,(ni::tI32)4,
                      (ni::tI32)5,(ni::tI32)6,(ni::tI32)7);
  CHECK_EQUAL(_ASTR("L:1,2,3,4,5,6,7,0"),str);
}
TEST_FIXTURE(niCore_CatFormat,A8) {
  ni::cString str;
  ni::StringCatFormat(str,_A("L:%d,%d,%d,%d,%d,%d,%d,%d"),
                      (ni::tI32)1,(ni::tI32)2,(ni::tI32)3,(ni::tI32)4,
                      (ni::tI32)5,(ni::tI32)6,(ni::tI32)7,(ni::tI32)8);
  CHECK_EQUAL(_ASTR("L:1,2,3,4,5,6,7,8"),str);
}
TEST_FIXTURE(niCore_CatFormat,A24) {
  ni::cString str = _A("L:");
  str.CatFormat(_A("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24);
  CHECK_EQUAL(_ASTR("L:1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24"),str);
}


///////////////////////////////////////////////
TEST_FIXTURE(niCore_CatFormat,IntPtr) {
  ni::cString str;
#ifdef ni64
  niCAssert(sizeof(ni::tIntPtr) == 8);
  str.Format(_A("%p"),0);
  CHECK_EQUAL(_ASTR("0000000000000000"),str);
  ni::tIntPtr p = (ni::tIntPtr)0xABCD0000FFFF0000;
  str.Format(_A("%p"),p);
  CHECK_EQUAL(_ASTR("ABCD0000FFFF0000"),str);
#else
  niCAssert(sizeof(ni::tIntPtr) == 4);
  str.Format(_A("%p"),0);
  CHECK_EQUAL(_ASTR("00000000"),str);
  ni::tIntPtr p = (ni::tIntPtr)0xABCD0000;
  str.Format(_A("%p"),p);
  CHECK_EQUAL(_ASTR("ABCD0000"),str);
#endif
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_CatFormat,IUnknownPtr) {
  {
    ni::cString strA,strB;
    strA.CatFormat(_A("%p"),ni::GetLang());
    strB.CatFormat(_A("%p"),(ni::tIntPtr)ni::GetLang());
    CHECK_EQUAL(strB,strA);
  }
  {
    ni::cString strA,strB;
    strA.CatFormat(_A("%x"),ni::GetLang());
    strB.CatFormat(_A("%x"),(ni::tIntPtr)ni::GetLang());
    CHECK_EQUAL(strB,strA);
  }
}

///////////////////////////////////////////////
TEST_FIXTURE(niCore_CatFormat,Expr) {
  ni::cString strA, strB, strC ;
  strA.CatFormat("Narf: { %(1+3) }",1);
  niPrintln(niFmt("... Expr: strA: '%s'", strA));
  strB.CatFormat("Narf: { %(= 1+3+ ( 3 * 5 ) ) }",1);
  niPrintln(niFmt("... Expr: strB: '%s'", strB));
  strC.CatFormat("Narf: { %(Format('%.2f',1/3))%s }","->Fagiano");
  niPrintln(niFmt("... Expr: strC: '%s'", strC));
}
