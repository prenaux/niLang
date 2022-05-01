#include "stdafx.h"
#include "../src/API/niLang/ILang.h"
#include "../src/API/niLang/IFile.h"

using namespace ni;

const ni::tU32 kNumTests = 10000000;

struct FQueryInterface
{
};

TEST_FIXTURE(FQueryInterface,FileMem) {
  TEST_TRACK_MEMORY_BEGIN();
  Ptr<iFile> ptrFile = ni::GetLang()->CreateFileDynamicMemory();

  TEST_TIMING_BEGIN(QueryILang) {
    niLoop(i,kNumTests) {
      iLang* lang = (iLang*)ni::GetLang()->QueryInterface(niGetInterfaceUUID(iLang));
      CHECK_RETURN_IF_FAILED(lang != NULL);
    }
  } TEST_TIMING_END();

  TEST_TIMING_BEGIN(QueryIFileMem) {
    niLoop(i,kNumTests) {
      iFileMemory* mem = (iFileMemory*)ptrFile->QueryInterface(niGetInterfaceUUID(iFileMemory));
      CHECK_RETURN_IF_FAILED(mem != NULL);
    }
  } TEST_TIMING_END();

  TEST_TIMING_BEGIN(QueryIFile) {
    niLoop(i,kNumTests) {
      iFile* fp = (iFile*)ptrFile->QueryInterface(niGetInterfaceUUID(iFile));
      CHECK_RETURN_IF_FAILED(fp != NULL);
    }
  } TEST_TIMING_END();

  TEST_TIMING_BEGIN(QueryIUnknown) {
    niLoop(i,kNumTests) {
      iUnknown* unk = ptrFile->QueryInterface(niGetInterfaceUUID(iUnknown));
      CHECK_RETURN_IF_FAILED(unk != NULL);
    }
  } TEST_TIMING_END();

  TEST_TRACK_MEMORY_END();
}
