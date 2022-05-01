#include "stdafx.h"
#include "../src/API/niLang/STL/scope_guard.h"
#include "../src/API/niLang/Utils/DataTableUtils.h"
#include "./data/json_crash.html.hxx"

using namespace ni;

static const cString _dtJson = R"""(
  {"string":"bar","number":123,"isnull":null}
)""";

struct FJson {
};

TEST_FIXTURE(FJson,Basic) {
  Ptr<iDataTable> dt = ni::CreateDataTable("dt");
  CHECK_EQUAL(eTrue, JsonParseStringToDataTable(_dtJson.Chars(), dt));
  CHECK_EQUAL(_ASTR("bar"), dt->GetString("string"));
  CHECK_EQUAL(123, dt->GetInt("number"));
  CHECK_EQUAL(0, dt->GetInt("isnull"));
}

TEST_FIXTURE(FJson,JsonCrash) {
  Ptr<iDataTable> dt = ni::CreateDataTable("dt");
  Ptr<iFile> fp = niFileOpenBin2H(json_crash_html);
  CHECK_EQUAL(eFalse, JsonParseFileToDataTable(fp, dt));
}
