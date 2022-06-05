#include "stdafx.h"

using namespace ni;

struct FFileMemory {
};

TEST_FIXTURE(FFileMemory,EmptyString) {
  QPtr<iFile> fp = ni::CreateFileDynamicMemory(0,"--string--");
  fp->BeginTextFileWrite(eTextEncodingFormat_Native,eTrue);
  fp->WriteString("");
  fp->SeekSet(0);
  fp->BeginTextFileRead(eTrue);
  CHECK_EQUAL(eFalse, fp->GetPartialRead());
  CHECK_EQUAL(_ASTR(""), fp->ReadStringLine());
  CHECK_EQUAL(eTrue, fp->GetPartialRead());
}
