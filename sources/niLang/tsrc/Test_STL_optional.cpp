#include "stdafx.h"
#include <niLang/STL/optional.h>
#include <niLang/STL/memory.h>

using namespace ni;

struct ASTL_optional {};

static astl::optional<cString> doSucceed() {
  return _ASTR("OK");
}

static astl::optional<cString> doFail() {
  return astl::nullopt;
}

TEST_FIXTURE(ASTL_optional,base) {
  {
    auto r = doSucceed();
    CHECK_EQUAL(true,!!r);
    CHECK_EQUAL(_ASTR("OK"), *r);
  }
  {
    auto r = doFail();
    CHECK_EQUAL(true,!r);
  }
  {
    auto r = astl::make_optional(_ASTR("Foo"));
    CHECK_EQUAL(true,!!r);
    CHECK_EQUAL(_ASTR("Foo"), *r);
  }
}
