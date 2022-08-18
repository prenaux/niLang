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

///////////////////////////////////////////////
// Test the compiler `#ifdef niTypeIntIsOtherType`
TEST_FIXTURE(FVar,IntIsOtherType) {
  {
    Var v((int)-123);
    Var u = (int)-123;
    CHECK_EQUAL(v,u);
    CHECK_EQUAL((int)-123,(int)v.GetIntValue());
  }
  {
    Var v((unsigned)789);
    Var u = (unsigned)789;
    CHECK_EQUAL(v,u);
    CHECK_EQUAL((unsigned)789,(unsigned)v.GetIntValue());
  }
  {
    Var v((ni::tInt)-123);
    Var u = (ni::tInt)-123;
    CHECK_EQUAL(v,u);
    CHECK_EQUAL((ni::tInt)-123,(ni::tInt)v.GetIntValue());
  }
  {
    Var v((ni::tUInt)789);
    Var u = (ni::tUInt)789;
    CHECK_EQUAL(v,u);
    CHECK_EQUAL((ni::tUInt)789,(ni::tUInt)v.GetIntValue());
  }
  {
    Var v(-123);
    Var u = -123;
    CHECK_EQUAL(v,u);
    CHECK_EQUAL(-123,v.GetIntValue());
  }
  {
    Var v(789);
    Var u = 789;
    CHECK_EQUAL(v,u);
    CHECK_EQUAL(789,v.GetIntValue());
  }
}

///////////////////////////////////////////////
// Test the compiler `#ifdef niTypeIntPtrIsOtherType`
TEST_FIXTURE(FVar,IntPtrIsOtherType) {
  {
    Var v((ni::tIntPtr)-321);
    Var u = (ni::tIntPtr)-321;
    CHECK_EQUAL(v,u);
    CHECK_EQUAL((ni::tIntPtr)-321,(ni::tIntPtr)v.GetIntValue());
  }
  {
    Var v((ni::tUIntPtr)987);
    Var u = (ni::tUIntPtr)987;
    CHECK_EQUAL(v,u);
    CHECK_EQUAL((ni::tUIntPtr)987,(ni::tUIntPtr)v.GetIntValue());
  }
  {
    Var v((intptr_t)-321);
    Var u = (intptr_t)-321;
    CHECK_EQUAL(v,u);
    CHECK_EQUAL((intptr_t)-321,(intptr_t)v.GetIntValue());
  }
  {
    Var v((uintptr_t)987);
    Var u = (uintptr_t)987;
    CHECK_EQUAL(v,u);
    CHECK_EQUAL((uintptr_t)987,(uintptr_t)v.GetIntValue());
  }
}
