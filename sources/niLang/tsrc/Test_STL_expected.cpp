#include "stdafx.h"
#include <niLang/STL/expected.h>
#include <niLang/STL/unique_ptr.h>

using namespace ni;

struct ASTL_expected {};

struct sError {
  cString _msg;
  astl::shared_ptr<sError> _prev;
};

typedef astl::expected<cString,sError> tExpectedString;

astl::expected<cString,sError> doSucceed() {
  return "OK";
}

astl::expected<cString,sError> doFail() {
  return astl::make_unexpected(sError { "Failed" });
}

TEST_FIXTURE(ASTL_expected,base) {
  sError err = {"an error 1"};
  sError err2 = {"an error 2", astl::make_shared<sError>(err)};

  {
    auto r = doSucceed();
    CHECK_EQUAL(true,!!r);
    CHECK_EQUAL(_ASTR("OK"), *r);
  }
  {
    auto r = doFail();
    CHECK_EQUAL(true,!r);
    CHECK_EQUAL(_ASTR("Failed"), r.error()._msg);
  }
}
