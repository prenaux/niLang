#include "stdafx.h"
#include "../src/API/niLang/Utils/Buffer.h"

using namespace ni;

namespace {

struct FProperties {
};

TEST_FIXTURE(FProperties,BinDir) {
  cString binDir = ni::GetLang()->GetProperty("ni.dirs.bin");
  CHECK(binDir.EndsWith(niLOA_Bin "/"));
}

}
