#include "stdafx.h"
#ifdef niCPP_Lambda

using namespace ni;

struct FCURLGetString {
  QPtr<iCURL> _curl;
  FCURLGetString() {
    _curl = ni::New_niCURL_CURL(niVarNull,niVarNull);
  }
};

TEST_FIXTURE(FCURLGetString,APICoinLore) {
  cString content = _curl->URLGetString("https://api.coinlore.com/api/ticker/?id=90");
  CHECK(content.StartsWith("[{\"id\":\"90\""));
  niDebugFmt(("... content: %s", content));
}
#endif
