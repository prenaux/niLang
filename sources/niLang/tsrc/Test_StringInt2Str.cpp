#include "stdafx.h"
#include <niLang/StringDef.h>
#include <niLang/Utils/CollectionImpl.h>
#include <niLang/STL/utils.h>
#include <niLang/Utils/StringIntToStr.h>

struct Test_StringInt2Str {};

TEST_FIXTURE(Test_StringInt2Str,UInt) {
  ni::achar buf[128];
  const ni::achar* p;
  p = ni::StringUIntToStr(buf,niCountOf(buf),25);
  CHECK_EQUAL(_ASTR("25"),_ASTR(p));

  p = ni::StringUIntToStr(buf,niCountOf(buf),12345678);
  CHECK_EQUAL(_ASTR("12345678"),_ASTR(p));

  p = ni::StringUIntToStr(buf,niCountOf(buf),(ni::tU64)1234567899887755ULL);
  CHECK_EQUAL(_ASTR("1234567899887755"),_ASTR(p));

  //// Signed to unsigned int str, get the equivalent convert to a uint64 ////
  p = ni::StringUIntToStr(buf,niCountOf(buf),-25);
  CHECK_EQUAL(_ASTR("18446744073709551591"),_ASTR(p));

  p = ni::StringUIntToStr(buf,niCountOf(buf),-12345678);
  CHECK_EQUAL(_ASTR("18446744073697205938"),_ASTR(p));

  p = ni::StringUIntToStr(buf,niCountOf(buf),-1234567899887755LL);
  CHECK_EQUAL(_ASTR("18445509505809663861"),_ASTR(p));
}

TEST_FIXTURE(Test_StringInt2Str,Int) {
  ni::achar buf[128];
  const ni::achar* p;
  p = ni::StringIntToStr(buf,niCountOf(buf),25);
  CHECK_EQUAL(_ASTR("25"),_ASTR(p));

  p = ni::StringIntToStr(buf,niCountOf(buf),12345678);
  CHECK_EQUAL(_ASTR("12345678"),_ASTR(p));

  p = ni::StringIntToStr(buf,niCountOf(buf),(ni::tU64)1234567899887755ULL);
  CHECK_EQUAL(_ASTR("1234567899887755"),_ASTR(p));

  p = ni::StringIntToStr(buf,niCountOf(buf),-25);
  CHECK_EQUAL(_ASTR("-25"),_ASTR(p));

  p = ni::StringIntToStr(buf,niCountOf(buf),-12345678);
  CHECK_EQUAL(_ASTR("-12345678"),_ASTR(p));

  p = ni::StringIntToStr(buf,niCountOf(buf),(ni::tI64)-1234567899887755LL);
  CHECK_EQUAL(_ASTR("-1234567899887755"),_ASTR(p));
}

TEST_FIXTURE(Test_StringInt2Str,StringClassToInt) {
  CHECK_EQUAL(_ASTR("25"),ni::cString().Set((ni::tU32)25));
  CHECK_EQUAL(_ASTR("12345678"),ni::cString().Set((ni::tU32)12345678));
  CHECK_EQUAL(_ASTR("1234567899887755"),ni::cString().Set((ni::tU64)1234567899887755ULL));

  CHECK_EQUAL(_ASTR("-25"),ni::cString().Set((ni::tI32)-25));
  CHECK_EQUAL(_ASTR("-12345678"),ni::cString().Set((ni::tI32)-12345678));
  CHECK_EQUAL(_ASTR("-1234567899887755"),ni::cString().Set((ni::tI64)-1234567899887755LL));
}
