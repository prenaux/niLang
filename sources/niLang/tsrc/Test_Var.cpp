#include "stdafx.h"
#include "../src/API/niLang/Var.h"

struct FVar {
};

using namespace ni;

TEST_FIXTURE(FVar,ConvertString) {
  tHStringPtr hstr = _H("Hello");
  Var varHStr = hstr;
  Var varStr = varHStr;
  VarConvertType(varStr,eType_String);
  CHECK_EQUAL(eType_IUnknownPtr,varHStr.GetType());
  CHECK_EQUAL(_ASTR("Hello"),VarGetString(varHStr));
  CHECK_EQUAL(eType_String,varStr.GetType());
  CHECK_EQUAL(_ASTR("Hello"),VarGetString(varStr));
  varStr = niVarNull;
  varHStr = niVarNull;
  hstr = NULL;
}

tI32 _AllocDiff() {
  sVec4i ms;
  ni_mem_get_stats(&ms);
  return ms.x-ms.y;
}

TEST_FIXTURE(FVar,Var_cString) {
  const tU32 diffA = _AllocDiff();
  {
    cString strA = "a";
    // cString strB = "b";
    // Var varA = strA.Chars();
    // Var varB = strB.Chars();
    // ni_mem_enable_trace_alloc(eTrue);
    Var varA = strA;
    // ni_mem_enable_trace_alloc(eFalse);
    // Var varB = strB;
    CHECK_EQUAL(_ASTR("a"), VarGetString(varA));
  }
  {
    Var varA(_ASTR("abc"));
    varA = _ASTR("def");
    CHECK_EQUAL(_ASTR("def"), VarGetString(varA));
  }
  {
    Var varA(_ASTR("abc"));
    Var varB(_ASTR("xyz"));
    varA = varB;
    CHECK_EQUAL(_ASTR("xyz"), VarGetString(varA));
  }
  const tU32 diffB = _AllocDiff();
  CHECK_EQUAL(diffA, diffB);
  niDebugFmt(("... Diff A: %s B: %s", diffA, diffB));
}

TEST_FIXTURE(FVar,Var_Matrixf) {
  const tU32 diffA = _AllocDiff();
  {
    sMatrixf mtx;
    Var varA = mtx;
  }
  const tU32 diffB = _AllocDiff();
  CHECK_EQUAL(diffA, diffB);
  niDebugFmt(("... Diff A: %s B: %s", diffA, diffB));
}

TEST_FIXTURE(FVar,StrToFloat) {
  Var varStrToFloat = "20";
  VarConvertType(varStrToFloat,eType_F64);
  CHECK_EQUAL(20,varStrToFloat.GetFloatValue());
}

TEST_FIXTURE(FVar,HStrToFloat) {
  Var varHStrToFloat = _H("20");
  VarConvertType(varHStrToFloat,eType_F64);
  CHECK_EQUAL(20,varHStrToFloat.GetFloatValue());
}
