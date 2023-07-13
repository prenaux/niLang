#include "stdafx.h"
#include <niAppLibConsole.h>
#include <niCURL.h>
#include <niLang/Utils/DataTableUtils.h>

using namespace ni;

struct FThriftDynCall {
};

TEST_FIXTURE(FThriftDynCall, LoadSchema) {
  Nonnull<iDataTable> dtShared = niCheckNN(dtShared, ni::LoadDataTable("Test_niThrift://shared.thrift.xml"), ;);
  CHECK_EQUAL(_ASTR("shared"), VarGetString(dtShared->GetVarFromPath("/document/@name", niVarNull)));

  Nonnull<iDataTable> dtTutorial = niCheckNN(dtTutorial, ni::LoadDataTable("Test_niThrift://tutorial.thrift.xml"), ;);
  CHECK_EQUAL(_ASTR("tutorial"), VarGetString(dtTutorial->GetVarFromPath("/document/@name", niVarNull)));
}
