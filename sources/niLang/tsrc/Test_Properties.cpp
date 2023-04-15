#include "stdafx.h"
#include "../src/API/niLang/Utils/Buffer.h"

using namespace ni;

namespace {

struct FProperties {
};

TEST_FIXTURE(FProperties,BinDir) {
  cString binDir = ni::GetLang()->GetProperty("ni.dirs.bin");
  niDebugFmt(("... ni.dirs.bin = '%s'", binDir));
  niDebugFmt(("... niLOA_Bin = '%s'", niLOA_Bin));
  CHECK(binDir.EndsWith(niLOA_Bin "/"));
}

}
