#include "stdafx.h"
#include "Test_Utils.h"

using namespace ni;

struct FCURLGetString {
  QPtr<iCURL> _curl;
  FCURLGetString() {
    _curl = ni::New_niCURL_CURL(niVarNull,niVarNull);
  }
};

TEST_FIXTURE(FCURLGetString,SomeJson) {
  cString content = _curl->URLGetString(
    _GetHTTPSTestCasesUrl("Test_niCURL_FetchGetJson.php?param=test_json_value").c_str());
  CHECK(content.StartsWith(R"""({"name":"Test_niCURL_FetchGetJson",)"""));
  niDebugFmt(("... content: %s", content));
}
