#include "stdafx.h"
#include <niLang/STL/optional.h>
#include <niLang/STL/memory.h>

using namespace ni;

struct ASTL_optional {};

static astl::optional<cString> doSucceed() {
  return _ASTR("OK");
}

static astl::optional<cString> doFail() {
  return ASTL_NULLOPT;
}

TEST_FIXTURE(ASTL_optional,base) {
  {
    auto r = doSucceed();
    CHECK_EQUAL(true,!!r);
    CHECK_EQUAL(_ASTR("OK"), r.value());

    int hasValue = -1;
    r.and_then([&](auto&& v) {
      hasValue = 1;
      return r;
    });
    CHECK_EQUAL(1, hasValue);
  }

  {
    auto r = doFail();
    CHECK_EQUAL(true,!r);

    int hasValue = -1;
    r.and_then([&](auto&& v) {
      hasValue = 1;
      return r;
    });
    CHECK_EQUAL(-1, hasValue);
  }

  {
    auto r = astl::make_optional(_ASTR("Foo"));
    CHECK_EQUAL(true,!!r);
    CHECK_EQUAL(_ASTR("Foo"), r.value());

    int hasValue = -1;
    r.and_then([&](auto&& v) {
      hasValue = 1;
      return r;
    });
    CHECK_EQUAL(1, hasValue);
  }
}
