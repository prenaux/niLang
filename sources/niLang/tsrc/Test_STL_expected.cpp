#include "stdafx.h"
#include <niLang/STL/expected.h>
#include <niLang/STL/memory.h>

using namespace ni;

struct ASTL_expected {};

struct sError {
  cString _msg;
  astl::shared_ptr<sError> _prev;
};

typedef astl::expected<cString,sError> tExpectedString;

static astl::expected<cString,sError> doSucceed() {
  return "OK";
}

static astl::expected<cString,sError> doFail() {
  return astl::make_unexpected(sError { "Failed" });
}

TEST_FIXTURE(ASTL_expected,DefaultConstructor) {
  tExpectedString r;
  CHECK_EQUAL(false,!!r);
  CHECK_EQUAL(true,r.has_error());
  CHECK_EQUAL(false,r.has_value());
  CHECK_EQUAL(_ASTR("DefaultValue"), r.value_or("DefaultValue"));
  CHECK_EQUAL(_ASTR("FnDefaultValue"), r.value_or_fn([]() { return "FnDefaultValue"; }));
}

TEST_FIXTURE(ASTL_expected,Success) {
  auto r = doSucceed();
  CHECK_EQUAL(true,!!r);
  CHECK_EQUAL(false,r.has_error());
  CHECK_EQUAL(true,r.has_value());
  CHECK_EQUAL(_ASTR("OK"), r.value());
  CHECK_EQUAL(_ASTR("OK"), r.value_or("DefaultValue"));
  CHECK_EQUAL(_ASTR("OK"), r.value_or_fn([]() { return "FnDefaultValue"; }));
}

TEST_FIXTURE(ASTL_expected,Failure) {
  auto r = doFail();
  CHECK_EQUAL(true,!r);
  CHECK_EQUAL(true,r.has_error());
  CHECK_EQUAL(false,r.has_value());
  CHECK_EQUAL(_ASTR("Failed"), r.error()._msg);
  CHECK_EQUAL(_ASTR("DefaultValue"), r.value_or("DefaultValue"));
  CHECK_EQUAL(_ASTR("FnDefaultValue"), r.value_or_fn([]() { return "FnDefaultValue"; }));
}
