#include "stdafx.h"
#include "../src/API/niLang/Utils/UnknownImpl.h"
#include "../src/API/niLang/ILang.h"
#include "../src/API/niLang/Utils/ConcurrentImpl.h"
#include "../src/API/niLang/IExpression.h"
#include "../src/API/niLang_ModuleDef.h"

using namespace ni;

struct FExpression {
};

TEST_FIXTURE(FExpression,BitwiseOr) {
  tU32 v = (tU32)ni::GetLang()->Eval("1|4|8|16")->GetFloat();
  niDebugFmt((_A("V: %d"),v));
  CHECK_EQUAL(1|4|8|16,v);
}

TEST_FIXTURE(FExpression,String1) {
  Ptr<iExpressionContext> ptrCtx = ni::GetLang()->CreateExpressionContext();
  CHECK(ptrCtx.IsOK());

  Ptr<iExpression> expr = ptrCtx->CreateExpression("\"hello foo\" + ' bar yey'");
  CHECK_RETURN_IF_FAILED(expr.IsOK());

  cString v = expr->GetEvalResult()->GetString();
  niDebugFmt((_A("V: %s"),v));
  CHECK_EQUAL(_ASTR("hello foo bar yey"),v);
}

TEST_FIXTURE(FExpression,String2) {
  Ptr<iExpressionContext> ptrCtx = ni::GetLang()->CreateExpressionContext();
  CHECK(ptrCtx.IsOK());

  Ptr<iExpression> expr = ptrCtx->CreateExpression("'hello' + 123");
  CHECK_RETURN_IF_FAILED(expr.IsOK());

  cString v = expr->GetEvalResult()->GetString();
  niDebugFmt((_A("V: %s"),v));
  CHECK_EQUAL(_ASTR("hello123"),v);
}

TEST_FIXTURE(FExpression,String3) {
  Ptr<iExpressionContext> ptrCtx = ni::GetLang()->CreateExpressionContext();
  CHECK(ptrCtx.IsOK());

  Ptr<iExpression> expr = ptrCtx->CreateExpression("123.456 + 'hello'");
  CHECK_RETURN_IF_FAILED(expr.IsOK());

  cString v = expr->GetEvalResult()->GetString();
  niDebugFmt((_A("V: %s"),v));
  CHECK_EQUAL(_ASTR("123.456hello"),v);
}

TEST_FIXTURE(FExpression,String4) {
  Ptr<iExpressionContext> ptrCtx = ni::GetLang()->CreateExpressionContext();
  CHECK(ptrCtx.IsOK());

  Ptr<iExpression> expr = ptrCtx->CreateExpression("Vec3(1,2,3) + 'hello'");
  CHECK_RETURN_IF_FAILED(expr.IsOK());

  cString v = expr->GetEvalResult()->GetString();
  niDebugFmt((_A("V: %s"),v));
  CHECK_EQUAL(_ASTR("Vec3(1,2,3)hello"),v);
}


TEST_FIXTURE(FExpression,String5) {
  Ptr<iExpressionContext> ptrCtx = ni::GetLang()->CreateExpressionContext();
  CHECK(ptrCtx.IsOK());

  Ptr<iExpression> expr = ptrCtx->CreateExpression("\"hello foo\" + ` bar yey 123 + 456 `yep` ` ? time");
  CHECK_RETURN_IF_FAILED(expr.IsOK());

  cString v = expr->GetEvalResult()->GetString();
  niDebugFmt((_A("V: %s"),v));
  CHECK_EQUAL(_ASTR("hello foo bar yey 123 + 456 `yep` ` ? time"),v);
}

TEST_FIXTURE(FExpression,StringError) {
  Ptr<iExpressionContext> ptrCtx = ni::GetLang()->CreateExpressionContext();
  CHECK(ptrCtx.IsOK());

  Ptr<iExpression> expr = ptrCtx->CreateExpression("5 * 'hello'");
  CHECK(!expr.IsOK());
}

TEST_FIXTURE(FExpression,StringConvVec3) {
  Ptr<iExpressionContext> ptrCtx = ni::GetLang()->CreateExpressionContext();
  CHECK(ptrCtx.IsOK());

  Ptr<iExpression> expr = ptrCtx->CreateExpression("ToVec3('Vec3(1,2,3)')");
  CHECK_RETURN_IF_FAILED(expr.IsOK());

  sVec3f v = expr->GetEvalResult()->GetVec3();
  niDebugFmt((_A("V: %s"),v));
  CHECK_EQUAL(Vec3f(1,2,3),v);
}

TEST_FIXTURE(FExpression,StringConvFloat) {
  Ptr<iExpressionContext> ptrCtx = ni::GetLang()->CreateExpressionContext();
  CHECK(ptrCtx.IsOK());

  {
    Ptr<iExpression> expr = ptrCtx->CreateExpression("ToFloat(20)");
    CHECK_RETURN_IF_FAILED(expr.IsOK());

    tF64 v = expr->GetEvalResult()->GetFloat();
    niDebugFmt((_A("V: %s"),v));
    CHECK_EQUAL(20.0,v);
  }

  {
    Ptr<iExpression> expr = ptrCtx->CreateExpression("ToFloat('20')");
    CHECK_RETURN_IF_FAILED(expr.IsOK());

    tF64 v = expr->GetEvalResult()->GetFloat();
    niDebugFmt((_A("V: %s"),v));
    CHECK_EQUAL(20.0,v);
  }
}

TEST_FIXTURE(FExpression,StringFormat_Err) {
  CHECK_EQUAL(_ASTR(""),ni::GetLang()->Eval("format('foo%.2f',")->GetString());
  CHECK_EQUAL(_ASTR(""),ni::GetLang()->Eval("format('foo%.2f',)")->GetString());
}

TEST_FIXTURE(FExpression,StringFormat) {
  CHECK_EQUAL(_ASTR("str:"),ni::GetLang()->Eval("'str:'+format()")->GetString());
  CHECK_EQUAL(_ASTR("str:foo123.78"),ni::GetLang()->Eval("'str:'+format('foo%.2f',123.781)")->GetString());
  CHECK_EQUAL(_ASTR("str:foo%.2f"),ni::GetLang()->Eval("'str:'+format('foo%.2f')")->GetString());
  CHECK_EQUAL(_ASTR("str:foo123bar"),ni::GetLang()->Eval("'str:'+format('foo%d%d%dbar',1,2,3)")->GetString());

  CHECK_EQUAL(_ASTR("str:N/A"),ni::GetLang()->Eval("'str:'+format('%$d',-123)")->GetString());
  CHECK_EQUAL(_ASTR("str:N/A"),ni::GetLang()->Eval("'str:'+format('%$D',-14325)")->GetString());
  CHECK_EQUAL(_ASTR("str:N/A"),ni::GetLang()->Eval("'str:'+format('%$f',-1.123)")->GetString());
  CHECK_EQUAL(_ASTR("str:N/A"),ni::GetLang()->Eval("'str:'+format('%$g',-1.123)")->GetString());

  CHECK_EQUAL(_ASTR("str:yes"),ni::GetLang()->Eval("'str:'+format('%y','true')")->GetString());
  CHECK_EQUAL(_ASTR("str:on"),ni::GetLang()->Eval("'str:'+format('%z','yes')")->GetString());

  CHECK_EQUAL(_ASTR("str:yes"),ni::GetLang()->Eval("'str:'+format('%y',-1)")->GetString());
  CHECK_EQUAL(_ASTR("str:no"),ni::GetLang()->Eval("'str:'+format('%y',0)")->GetString());
  CHECK_EQUAL(_ASTR("str:yes"),ni::GetLang()->Eval("'str:'+format('%y',1)")->GetString());
  CHECK_EQUAL(_ASTR("str:on"),ni::GetLang()->Eval("'str:'+format('%z',-1)")->GetString());
  CHECK_EQUAL(_ASTR("str:off"),ni::GetLang()->Eval("'str:'+format('%z',0)")->GetString());
  CHECK_EQUAL(_ASTR("str:on"),ni::GetLang()->Eval("'str:'+format('%z',1)")->GetString());

  CHECK_EQUAL(_ASTR("str:N/A"),ni::GetLang()->Eval("'str:'+format('%$y',-1)")->GetString());
  CHECK_EQUAL(_ASTR("str:no"),ni::GetLang()->Eval("'str:'+format('%$y',0)")->GetString());
  CHECK_EQUAL(_ASTR("str:yes"),ni::GetLang()->Eval("'str:'+format('%$y',1)")->GetString());
  CHECK_EQUAL(_ASTR("str:N/A"),ni::GetLang()->Eval("'str:'+format('%$z',-1)")->GetString());
  CHECK_EQUAL(_ASTR("str:off"),ni::GetLang()->Eval("'str:'+format('%$z',0)")->GetString());
  CHECK_EQUAL(_ASTR("str:on"),ni::GetLang()->Eval("'str:'+format('%$z',1)")->GetString());

  CHECK_EQUAL(_ASTR("str:foo=bar"),ni::GetLang()->Eval("'str:'+format('%s=%s','foo','bar')")->GetString());
}

struct MapURLResolver : public cIUnknownImpl<iExpressionURLResolver> {
  typedef astl::map<cString,tU32> tMap;
  tMap map;
  virtual Var __stdcall ResolveURL(const achar* aURL) {
    cString addr = aURL;
    addr = addr.After("://");
    niDebugFmt(("RESOLVE: %s",addr));
    tMap::const_iterator it = map.find(addr);
    if (it == map.end())
      return NULL;
    return it->second;
  }
};

TEST_FIXTURE(FExpression,Protocol) {
  Ptr<iExpressionContext> ptrCtx = ni::GetLang()->CreateExpressionContext();
  CHECK(ptrCtx.IsOK());

  Ptr<MapURLResolver> resolver = niNew MapURLResolver();
  resolver->map["one"] = 1;
  resolver->map["two"] = 2;
  resolver->map["three"] = 3;
  resolver->map["alpha"] = 16;
  resolver->map["123"] = 123;

  ptrCtx->RegisterURLResolver("value",resolver);

  {
    const achar* exprStr = "value://one";
    Ptr<iExpression> expr = ptrCtx->CreateExpression(exprStr);
    CHECK_RETURN_IF_FAILED(expr.IsOK());
    {
      tU32 v = (tU32)expr->GetEvalResult()->GetFloat();
      niDebugFmt((_A("%s: %s"),exprStr,v));
      CHECK_EQUAL(1,v);
    }
  }

  {
    const achar* exprStr = "value://123";
    Ptr<iExpression> expr = ptrCtx->CreateExpression(exprStr);
    CHECK_RETURN_IF_FAILED(expr.IsOK());
    {
      tU32 v = (tU32)expr->GetEvalResult()->GetFloat();
      niDebugFmt((_A("%s: %s"),exprStr,v));
      CHECK_EQUAL(123,v);
    }
  }

  {
    //
    // The expression below can also be written: "(value://three)+(value://three)".
    //
    // "value://three+value://three" will not work, it will look for
    // 'three+value://three'. URL tokens end only at spaces or closing
    // parenthesis.
    //
    const achar* exprStr = "value://three + value://three";
    Ptr<iExpression> expr = ptrCtx->CreateExpression(exprStr);
    CHECK_RETURN_IF_FAILED(expr.IsOK());
    {
      tU32 v = (tU32)expr->GetEvalResult()->GetFloat();
      niDebugFmt((_A("%s: %s"),exprStr,v));
      CHECK_EQUAL(6,v);
    }
  }

  {
    Ptr<iExpression> expr = ptrCtx->CreateExpression("value://three + value://three");
    CHECK_RETURN_IF_FAILED(expr.IsOK());
    {
      tU32 v = (tU32)expr->GetEvalResult()->GetFloat();
      niDebugFmt((_A("V: %s"),v));
      CHECK_EQUAL(6,v);
    }
  }

  {
    Ptr<iExpression> expr = ptrCtx->CreateExpression("sqrt(value://alpha) + sqrt(value://alpha)");
    CHECK_RETURN_IF_FAILED(expr.IsOK());
    {
      tU32 v = (tU32)expr->GetEvalResult()->GetFloat();
      niDebugFmt((_A("V: %s"),v));
      CHECK_EQUAL(8,v);
    }
    resolver->map["alpha"] = 64;
    {
      tU32 v = (tU32)expr->Eval()->GetFloat();
      niDebugFmt((_A("V: %s"),v));
      CHECK_EQUAL(16,v);
    }
  }
}

TEST_FIXTURE(FExpression,RGB) {
  TEST_TIMEREPORT();

  Ptr<iExpressionContext> ptrCtx = ni::GetLang()->CreateExpressionContext();
  CHECK(ptrCtx.IsOK());

  Ptr<iExpressionVariable> result = ptrCtx->Eval(_A("rgb(205, 92, 92)"));
  CHECK_EQUAL(eExpressionVariableType_Vec3, result->GetType());
  CHECK_EQUAL(eTrue, niFlagIs(result->GetFlags(),eExpressionVariableFlags_Color));

  sVec3f v;
  v = result->GetVec3();
  CHECK_CLOSE(Vec3<tF32>(205,92,92),v,sVec3f::Epsilon());

  v = ptrCtx->Eval(_A("rgb(80.39215686274509803921568627451%,36.078431372549019607843137254902%,36.078431372549019607843137254902%)"))->GetVec3();
  CHECK_CLOSE(Vec3<tF32>(0.8039215686274509803921568627451f,0.36078431372549019607843137254902f,0.36078431372549019607843137254902f),v,sVec3f::Epsilon());

  v = ptrCtx->Eval(_A("rgb(0.5f, 0.3f, 0.2f)"))->GetVec3();
  CHECK_CLOSE(Vec3<tF32>(0.5f,0.3f,0.2f),v,sVec3f::Epsilon());
}

TEST_FIXTURE(FExpression,Eval) {

  TEST_TIMEREPORT();
  iExpressionContext* ctx = ni::GetLang()->CreateExpressionContext();
  ctx->SetDefaultEnumDef(niEnumDef(eType));
  ctx->SetGlobalEnumSearch(false);
  for (int i = 0; i < 1000; ++i) {
    Ptr<iExpressionVariable> v = ctx->Eval("Vec3f");
    int val = (int)(((tU32)v->GetFloat()));
    CHECK_EQUAL(eType_Vec3f, val);
    if (eType_Vec3f != val) break;
  }
}

TEST_FIXTURE(FExpression,Eval2) {
  TEST_TIMEREPORT();

  Ptr<iExpressionContext> ctx = ni::GetLang()->CreateExpressionContext();
  ctx->SetDefaultEnumDef(niEnumDef(eType));
  ctx->SetGlobalEnumSearch(false);
  Ptr<iExpression> expr = ctx->CreateExpression("eType.Vec3f");
  for (int i = 0; i < 1000; ++i) {
    Ptr<iExpressionVariable> v = expr->Eval();
    int val = (int)(((tU32)v->GetFloat()));
    CHECK_EQUAL(eType_Vec3f, val);
    if (eType_Vec3f != val) break;
  }
}

TEST_FIXTURE(FExpression,Eval3) {
  TEST_TIMEREPORT();

  Ptr<iExpressionContext> ctx = ni::GetLang()->CreateExpressionContext();
  for (int i = 0; i < 1000; ++i) {
    Ptr<iExpressionVariable> v = ctx->Eval("(1+3+5)/2");
    CHECK_EQUAL(4.5, v->GetFloat());
    if (4.5 != v->GetFloat()) break;
  }
}

TEST_FIXTURE(FExpression,Eval4) {
  TEST_TIMEREPORT();

  Ptr<iExpressionContext> ctx = ni::GetLang()->CreateExpressionContext();
  Ptr<iExpression> expr = ctx->CreateExpression("(1+3+5)/2");
  for (int i = 0; i < 1000; ++i) {
    Ptr<iExpressionVariable> v = expr->Eval();
    CHECK_EQUAL(4.5, v->GetFloat());
    if (4.5 != v->GetFloat()) break;
  }
}

TEST_FIXTURE(FExpression,Time) {
  TEST_TIMEREPORT();

  Ptr<iExpressionContext> ptrCtx = ni::GetLang()->CreateExpressionContext();
  CHECK(ptrCtx.IsOK());

  Ptr<iExpression> expr = ptrCtx->CreateExpression("time");
  CHECK_RETURN_IF_FAILED(expr.IsOK());

  tF64 time1, time2;
  {
    time1 = expr->Eval()->GetFloat();
    CHECK_CLOSE(time1, ni::TimerInSeconds(), 0.001);
  }

  // do something to make sure time1/2 have different values
  for (int i = 0; i < 1000; ++i) {
    expr->Eval();
  }


  {
    time2 = expr->Eval()->GetFloat();
    CHECK_CLOSE(time2, ni::TimerInSeconds(), 0.001);
  }

  niDebugFmt((_A("T1: %s, T2: %s"),time1,time2));
  CHECK(time1 != time2);
}

TEST_FIXTURE(FExpression,VecFloatMul) {
  Ptr<iExpressionContext> ptrCtx = ni::GetLang()->CreateExpressionContext();
  CHECK(ptrCtx.IsOK());

  {
    Ptr<iExpression> expr = ptrCtx->CreateExpression("Vec3(1,2,3) * 3");
    CHECK_RETURN_IF_FAILED(expr.IsOK());

    sVec3f v = expr->GetEvalResult()->GetVec3();
    niDebugFmt((_A("V: %s"),v));
    CHECK_EQUAL(Vec3f(3,6,9),v);
  }

  {
    const tF64 time = ni::TimerInSeconds();
    Ptr<iExpression> expr = ptrCtx->CreateExpression("Vec3(1,2,3) * time");
    CHECK_RETURN_IF_FAILED(expr.IsOK());

    sVec3f r = Vec3f(1 * time, 2 * time, 3 * time);
    sVec3f v = expr->GetEvalResult()->GetVec3();
    niDebugFmt((_A("V: %s, r: %s"),v,r));
    CHECK(VecEqual(r,v,0.1f));
  }
}

TEST_FIXTURE(FExpression,Enums) {

  {
    Ptr<iExpressionContext> ptrCtx = ni::GetLang()->CreateExpressionContext();
    ptrCtx->SetGlobalEnumSearch(eTrue);
    tU32 v;
    v = ptrCtx->Eval("eType.U32")->GetFloat();
    niDebugFmt((_A("V: %d"),v));
    CHECK_EQUAL(0,v);

    // register here, to test that SetGlobalEnumSearch is correct...
    // this isnt the best because its global and we cant unregister
    // in the context of the test cases it should be ok...
    // if its already registered the test above will fail...
    ni::GetLang()->RegisterModuleDef(GetModuleDef_niLang());

    v = ptrCtx->Eval("eType.U32")->GetFloat();
    niDebugFmt((_A("V: %d"),v));
    CHECK_EQUAL(eType_U32,v);
  }

  {
    tU32 v = ni::GetLang()->StringToEnum("U32",niEnumExpr(eType));
    niDebugFmt((_A("V: %d"),v));
    CHECK_EQUAL(eType_U32,v);
  }
}

TEST_FIXTURE(FExpression,Flags) {
  // Register to make sure it has been registered...
  ni::GetLang()->RegisterModuleDef(GetModuleDef_niLang());

  niLoop(i,1000) {
    tU32 v = (tU32)ni::GetLang()->Eval("eType.IUnknown|eTypeFlags.Pointer")->GetFloat();
    // niDebugFmt((_A("V: %d"),v));
    CHECK_EQUAL(eType_IUnknownPtr,v);
    if (eType_IUnknownPtr != v) break;
  }

  {
    tU32 v = ni::GetLang()->StringToEnum("Constant|Pointer",niFlagsExpr(eTypeFlags));
    niDebugFmt((_A("V: %d"),v));
    CHECK_EQUAL(eTypeFlags_Constant|eTypeFlags_Pointer,v);
  }
}

TEST_FIXTURE(FExpression,Space) {
  tF64 v = ni::GetLang()->Eval(" ")->GetFloat();
  niDebugFmt((_A("V: %g"),v));
  CHECK_EQUAL(0,v);
}

TEST_FIXTURE(FExpression,DigestHex) {
  cString md5 = ni::GetLang()->Eval(
      "DigestHex('C:/Documents and Settings/Owner/My Documents/abcddefghiàéèçù€µ°你好αβ/Gaya/test你好.txt','md5')")->GetString();
  TEST_DEBUGFMT("md5: %s", md5);
  CHECK_EQUAL(_ASTR("1a6b48c841b77528c637d428ee3b7b80"), md5);

  cString sha1 = ni::GetLang()->Eval(
      "DigestHex('C:/Documents and Settings/Owner/My Documents/abcddefghiàéèçù€µ°你好αβ/Gaya/test你好.txt','sha1')")->GetString();
  TEST_DEBUGFMT("sha1: %s", sha1);
  CHECK_EQUAL(_ASTR("ab3b8d91ca977cbc8ff1e617427dc9e0f63b782b"), sha1);
}

TEST_FIXTURE(FExpression,KDFCrypt) {
  cString r1 = ni::GetLang()->Eval(
      "KDFCrypt('12345', '$2a$08$VXQoSyZF288.82vVBGuEN.')")->GetString();
  TEST_DEBUGFMT("r1: %s", r1);
  CHECK_EQUAL(_ASTR("$2a$08$VXQoSyZF288.82vVBGuEN.1NasswyF6K7sDZxyH/xGuLP4BEg26yK"), r1);

  cString s1 = ni::GetLang()->Eval(
      "KDFGenSaltBlowfish(4)")->GetString();
  TEST_DEBUGFMT("s1: %s", s1);
  CHECK(s1.StartsWith("$2a$04$"));

  cString s2 = ni::GetLang()->Eval(
      "KDFGenSaltBlowfish(8)")->GetString();
  TEST_DEBUGFMT("s2: %s", s2);
  CHECK(s2.StartsWith("$2a$08$"));

  cString s3 = ni::GetLang()->Eval(
      "KDFGenSaltBlowfish(8)")->GetString();
  TEST_DEBUGFMT("s3: %s", s3);
  CHECK(s3.StartsWith("$2a$08$"));

  CHECK_NOT_EQUAL(s2,s3);
}

TEST_FIXTURE(FExpression,StrLen) {
  tInt r1 = (tInt)ni::GetLang()->Eval("StrLen('$2a$08$VXQoSyZF288.82vVBGuEN.')")->GetFloat();
  TEST_DEBUGFMT("r1: %s", r1);
  CHECK_EQUAL(29, r1);
}

TEST_FIXTURE(FExpression,StrSlice) {
  // 7 to last character
  cString r0 = ni::GetLang()->Eval("StrSlice('$2a$08$VXQoSyZF288.82vVBGuEN.',7,0)")->GetString();
  TEST_DEBUGFMT("r0: %s", r0);
  CHECK_EQUAL(_ASTR("VXQoSyZF288.82vVBGuEN."), r0);

  // trim the last character
  cString r1 = ni::GetLang()->Eval("StrSlice('$2a$08$VXQoSyZF288.82vVBGuEN.',7,-1)")->GetString();
  TEST_DEBUGFMT("r1: %s", r1);
  CHECK_EQUAL(_ASTR("VXQoSyZF288.82vVBGuEN"), r1);

  // trim the last 3 characters
  cString r2 = ni::GetLang()->Eval("StrSlice('$2a$08$VXQoSyZF288.82vVBGuEN.',7,-3)")->GetString();
  TEST_DEBUGFMT("r2: %s", r2);
  CHECK_EQUAL(_ASTR("VXQoSyZF288.82vVBGu"), r2);

  cString r3 = ni::GetLang()->Eval("StrSlice('$2a$08$VXQoSyZF288.82vVBGuEN.',0)")->GetString();
  TEST_DEBUGFMT("r3: %s", r3);
  CHECK_EQUAL(_ASTR("$2a$08$VXQoSyZF288.82vVBGuEN."), r3);
}

TEST_FIXTURE(FExpression,StrMid) {
  cString r1 = ni::GetLang()->Eval("StrMid('$2a$08$VXQoSyZF288.82vVBGuEN.',7,5)")->GetString();
  TEST_DEBUGFMT("r1: %s", r1);
  CHECK_EQUAL(_ASTR("VXQoS"), r1);
}

TEST_FIXTURE(FExpression,StrLeft) {
  cString r1 = ni::GetLang()->Eval("StrLeft('$2a$08$VXQoSyZF288.82vVBGuEN.',7)")->GetString();
  TEST_DEBUGFMT("r1: %s", r1);
  CHECK_EQUAL(_ASTR("$2a$08$"), r1);
}

TEST_FIXTURE(FExpression,StrRight) {
  cString r1 = ni::GetLang()->Eval("StrRight('$2a$08$VXQoSyZF288.82vVBGuEN.',4)")->GetString();
  TEST_DEBUGFMT("r1: %s", r1);
  CHECK_EQUAL(_ASTR("uEN."), r1);
}

TEST_FIXTURE(FExpression,StrBeforeAfter) {
  cString r1 = ni::GetLang()->Eval(
      "StrBefore('$2a$08$VXQoSyZF288.82vVBGuEN.','.')")->GetString();
  TEST_DEBUGFMT("r1: %s", r1);
  CHECK_EQUAL(_ASTR("$2a$08$VXQoSyZF288"), r1);

  cString r2 = ni::GetLang()->Eval(
      "StrRBefore('$2a$08$VXQoSyZF288.82vVBGuEN.','.')")->GetString();
  TEST_DEBUGFMT("r2: %s", r2);
  CHECK_EQUAL(_ASTR("$2a$08$VXQoSyZF288.82vVBGuEN"), r2);

  cString r3 = ni::GetLang()->Eval(
      "StrBefore(StrRAfter('$2a$08$VXQoSyZF288.82vVBGuEN.','$'),'.')")->GetString();
  TEST_DEBUGFMT("r3: %s", r3);
  CHECK_EQUAL(_ASTR("VXQoSyZF288"), r3);

  cString r4 = ni::GetLang()->Eval(
      "StrAfter('$2a$08$VXQoSyZF288.82vVBGuEN.','.')")->GetString();
  TEST_DEBUGFMT("r4: %s", r4);
  CHECK_EQUAL(_ASTR("82vVBGuEN."), r4);
}

TEST_FIXTURE(FExpression,StrFind) {
  tInt r1 = (tInt)ni::GetLang()->Eval(
      "StrFind('$2a$08$VXQoSyZF288.82vVBGuEN.','$08')")->GetFloat();
  TEST_DEBUGFMT("r1: %s", r1);
  CHECK_EQUAL(3, r1);

  tInt r2 = (tInt)ni::GetLang()->Eval(
      "StrFind('$2a$08$VXQoSyZF288.82vVBGuEN.','$')")->GetFloat();
  TEST_DEBUGFMT("r2: %s", r2);
  CHECK_EQUAL(0, r2);

  tInt r3 = (tInt)ni::GetLang()->Eval(
      "StrRFind('$2a$08$VXQoSyZF288.82vVBGuEN.','$')")->GetFloat();
  TEST_DEBUGFMT("r3: %s", r3);
  CHECK_EQUAL(6, r3);

  tInt r4 = (tInt)ni::GetLang()->Eval(
      "StrRFind('$2a$08$VXQoSyZF288.82vVBGuEN.','marcel')")->GetFloat();
  TEST_DEBUGFMT("r4: %s", r4);
  CHECK_EQUAL(-1, r4);
}

TEST_FIXTURE(FExpression,StrToLowerUpper) {
  cString r1 = ni::GetLang()->Eval(
      "StrToLower('$2a$08$VXQoSyZF288.82vVBGuEN.')")->GetString();
  TEST_DEBUGFMT("r1: %s", r1);
  CHECK_EQUAL(_ASTR("$2a$08$vxqosyzf288.82vvbguen."), r1);

  cString r2 = ni::GetLang()->Eval(
      "StrToUpper('$2a$08$VXQoSyZF288.82vVBGuEN.')")->GetString();
  TEST_DEBUGFMT("r2: %s", r2);
  CHECK_EQUAL(_ASTR("$2A$08$VXQOSYZF288.82VVBGUEN."), r2);
}

TEST_FIXTURE(FExpression,StrTrimNormalize) {
  cString r1 = ni::GetLang()->Eval(
      "StrTrim('  Hello   World    ')")->GetString();
  TEST_DEBUGFMT("r1: %s", r1);
  CHECK_EQUAL(_ASTR("Hello   World"), r1);

  cString r2 = ni::GetLang()->Eval(
      "StrNormalize('  Hello   World    ')")->GetString();
  TEST_DEBUGFMT("r2: %s", r2);
  CHECK_EQUAL(_ASTR("Hello World"), r2);
}

TEST_FIXTURE(FExpression,LogicalAnd) {
  tU32 v;
  v = (tU32)ni::GetLang()->Eval("0 && 0")->GetFloat();
  niDebugFmt((_A("V: 0 && 0 = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("1 && 0")->GetFloat();
  niDebugFmt((_A("V: 1 && 0 = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("1 && 1")->GetFloat();
  niDebugFmt((_A("V: 1 && 1 = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("(1/3) && 25")->GetFloat();
  niDebugFmt((_A("V: (1/3) && 25 = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("25 && (1/3)")->GetFloat();
  niDebugFmt((_A("V: 25 && (1/3) = %d"),v));
  CHECK_EQUAL(1,v);
}

TEST_FIXTURE(FExpression,LogicalOr) {
  tU32 v;
  v = (tU32)ni::GetLang()->Eval("0 || 0")->GetFloat();
  niDebugFmt((_A("V: 0 || 0 = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("1 || 0")->GetFloat();
  niDebugFmt((_A("V: 1 || 0 = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("1 || 1")->GetFloat();
  niDebugFmt((_A("V: 1 || 1 = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("(1/3) || 25")->GetFloat();
  niDebugFmt((_A("V: (1/3) || 25 = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("25 || (1/3)")->GetFloat();
  niDebugFmt((_A("V: 25 || (1/3) = %d"),v));
  CHECK_EQUAL(1,v);
}

TEST_FIXTURE(FExpression,LogicalNot) {
  tU32 v;
  v = (tU32)ni::GetLang()->Eval("!0")->GetFloat();
  niDebugFmt((_A("V: !0 = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("!1")->GetFloat();
  niDebugFmt((_A("V: !1 = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("!(1/3)")->GetFloat();
  niDebugFmt((_A("V: !(1/3) = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("!25")->GetFloat();
  niDebugFmt((_A("V: !25 = %d"),v));
  CHECK_EQUAL(0,v);

  v = (tU32)ni::GetLang()->Eval("!!0")->GetFloat();
  niDebugFmt((_A("V: !!0 = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("!!1")->GetFloat();
  niDebugFmt((_A("V: !!1 = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("!!(1/3)")->GetFloat();
  niDebugFmt((_A("V: !!(1/3) = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("!!25")->GetFloat();
  niDebugFmt((_A("V: !!25 = %d"),v));
  CHECK_EQUAL(1,v);
}

TEST_FIXTURE(FExpression,CmpEq) {
  tU32 v;
  v = (tU32)ni::GetLang()->Eval("0 == 0")->GetFloat();
  niDebugFmt((_A("V: 0 == 0 = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("1 == 0")->GetFloat();
  niDebugFmt((_A("V: 1 == 0 = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("1 == 1")->GetFloat();
  niDebugFmt((_A("V: 1 == 1 = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("(1/3) == 25")->GetFloat();
  niDebugFmt((_A("V: (1/3) == 25 = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("25 == (1/3)")->GetFloat();
  niDebugFmt((_A("V: 25 == (1/3) = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("(1/3) == (1/3)")->GetFloat();
  niDebugFmt((_A("V: (1/3) == (1/3) = %d"),v));
  CHECK_EQUAL(1,v);
}

TEST_FIXTURE(FExpression,CmpNotEq) {
  tU32 v;
  v = (tU32)ni::GetLang()->Eval("0 != 0")->GetFloat();
  niDebugFmt((_A("V: 0 != 0 = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("1 != 0")->GetFloat();
  niDebugFmt((_A("V: 1 != 0 = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("1 != 1")->GetFloat();
  niDebugFmt((_A("V: 1 != 1 = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("(1/3) != 25")->GetFloat();
  niDebugFmt((_A("V: (1/3) != 25 = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("25 != (1/3)")->GetFloat();
  niDebugFmt((_A("V: 25 != (1/3) = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("(1/3) != (1/3)")->GetFloat();
  niDebugFmt((_A("V: (1/3) != (1/3) = %d"),v));
  CHECK_EQUAL(0,v);
}

TEST_FIXTURE(FExpression,CmpLt) {
  tU32 v;
  v = (tU32)ni::GetLang()->Eval("0 < 0")->GetFloat();
  niDebugFmt((_A("V: 0 < 0 = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("1 < 0")->GetFloat();
  niDebugFmt((_A("V: 1 < 0 = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("1 < 1")->GetFloat();
  niDebugFmt((_A("V: 1 < 1 = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("(1/3) < 25")->GetFloat();
  niDebugFmt((_A("V: (1/3) < 25 = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("25 < (1/3)")->GetFloat();
  niDebugFmt((_A("V: 25 < (1/3) = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("(1/3) < (1/3)")->GetFloat();
  niDebugFmt((_A("V: (1/3) < (1/3) = %d"),v));
  CHECK_EQUAL(0,v);
}

TEST_FIXTURE(FExpression,CmpLte) {
  tU32 v;
  v = (tU32)ni::GetLang()->Eval("0 <= 0")->GetFloat();
  niDebugFmt((_A("V: 0 <= 0 = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("1 <= 0")->GetFloat();
  niDebugFmt((_A("V: 1 <= 0 = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("1 <= 1")->GetFloat();
  niDebugFmt((_A("V: 1 <= 1 = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("(1/3) <= 25")->GetFloat();
  niDebugFmt((_A("V: (1/3) <= 25 = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("25 <= (1/3)")->GetFloat();
  niDebugFmt((_A("V: 25 <= (1/3) = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("(1/3) <= (1/3)")->GetFloat();
  niDebugFmt((_A("V: (1/3) <= (1/3) = %d"),v));
  CHECK_EQUAL(1,v);
}

TEST_FIXTURE(FExpression,CmpGt) {
  tU32 v;
  v = (tU32)ni::GetLang()->Eval("0 > 0")->GetFloat();
  niDebugFmt((_A("V: 0 > 0 = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("1 > 0")->GetFloat();
  niDebugFmt((_A("V: 1 > 0 = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("1 > 1")->GetFloat();
  niDebugFmt((_A("V: 1 > 1 = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("(1/3) > 25")->GetFloat();
  niDebugFmt((_A("V: (1/3) > 25 = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("25 > (1/3)")->GetFloat();
  niDebugFmt((_A("V: 25 > (1/3) = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("(1/3) > (1/3)")->GetFloat();
  niDebugFmt((_A("V: (1/3) > (1/3) = %d"),v));
  CHECK_EQUAL(0,v);
}

TEST_FIXTURE(FExpression,CmpGte) {
  tU32 v;
  v = (tU32)ni::GetLang()->Eval("0 >= 0")->GetFloat();
  niDebugFmt((_A("V: 0 >= 0 = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("1 >= 0")->GetFloat();
  niDebugFmt((_A("V: 1 >= 0 = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("1 >= 1")->GetFloat();
  niDebugFmt((_A("V: 1 >= 1 = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("(1/3) >= 25")->GetFloat();
  niDebugFmt((_A("V: (1/3) >= 25 = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("25 >= (1/3)")->GetFloat();
  niDebugFmt((_A("V: 25 >= (1/3) = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("(1/3) >= (1/3)")->GetFloat();
  niDebugFmt((_A("V: (1/3) >= (1/3) = %d"),v));
  CHECK_EQUAL(1,v);
}

TEST_FIXTURE(FExpression,LogicalStr) {
  tU32 v;
  v = (tU32)ni::GetLang()->Eval("!!'yes'")->GetFloat();
  niDebugFmt((_A("V: !!'yes' = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("!!'on'")->GetFloat();
  niDebugFmt((_A("V: !!'on' = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("!!'true'")->GetFloat();
  niDebugFmt((_A("V: !!'true' = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("!!'foo'")->GetFloat();
  niDebugFmt((_A("V: !!'foo' = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("!!'false'")->GetFloat();
  niDebugFmt((_A("V: !!'false' = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("!!'off'")->GetFloat();
  niDebugFmt((_A("V: !!'off' = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("!!'no'")->GetFloat();
  niDebugFmt((_A("V: !!'no' = %d"),v));
  CHECK_EQUAL(0,v);

  v = (tU32)ni::GetLang()->Eval("!'yes'")->GetFloat();
  niDebugFmt((_A("V: !'yes' = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("!'on'")->GetFloat();
  niDebugFmt((_A("V: !'on' = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("!'true'")->GetFloat();
  niDebugFmt((_A("V: !'true' = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("!'foo'")->GetFloat();
  niDebugFmt((_A("V: !'foo' = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("!'false'")->GetFloat();
  niDebugFmt((_A("V: !'false' = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("!'off'")->GetFloat();
  niDebugFmt((_A("V: !'off' = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("!'no'")->GetFloat();
  niDebugFmt((_A("V: !'no' = %d"),v));
  CHECK_EQUAL(1,v);
}

TEST_FIXTURE(FExpression,CmpStr) {
  tU32 v;
  v = (tU32)ni::GetLang()->Eval("'foo' == 'foo'")->GetFloat();
  niDebugFmt((_A("V: 'foo' == 'foo' = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("'foo' != 'foo'")->GetFloat();
  niDebugFmt((_A("V: 'foo' != 'foo' = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("'foo' < 'foo'")->GetFloat();
  niDebugFmt((_A("V: 'foo' < 'foo' = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("'foo' <= 'foo'")->GetFloat();
  niDebugFmt((_A("V: 'foo' <= 'foo' = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("'foo' > 'foo'")->GetFloat();
  niDebugFmt((_A("V: 'foo' > 'foo' = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("'foo' >= 'foo'")->GetFloat();
  niDebugFmt((_A("V: 'foo' >= 'foo' = %d"),v));
  CHECK_EQUAL(1,v);

  v = (tU32)ni::GetLang()->Eval("'foo' == 'bar'")->GetFloat();
  niDebugFmt((_A("V: 'foo' == 'bar' = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("'foo' != 'bar'")->GetFloat();
  niDebugFmt((_A("V: 'foo' != 'bar' = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("'foo' < 'bar'")->GetFloat();
  niDebugFmt((_A("V: 'foo' < 'bar' = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("'foo' <= 'bar'")->GetFloat();
  niDebugFmt((_A("V: 'foo' <= 'bar' = %d"),v));
  CHECK_EQUAL(0,v);
  v = (tU32)ni::GetLang()->Eval("'foo' > 'bar'")->GetFloat();
  niDebugFmt((_A("V: 'foo' > 'bar' = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("'foo' >= 'bar'")->GetFloat();
  niDebugFmt((_A("V: 'foo' >= 'bar' = %d"),v));
  CHECK_EQUAL(1,v);
}

TEST_FIXTURE(FExpression,IsType) {
  tU32 v;

  v = (tU32)ni::GetLang()->Eval("IsFloat(123)")->GetFloat();
  niDebugFmt((_A("V: IsFloat(123) = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("IsFloat('foo')")->GetFloat();
  niDebugFmt((_A("V: IsFloat('foo') = %d"),v));
  CHECK_EQUAL(0,v);

  v = (tU32)ni::GetLang()->Eval("IsVec2(Vec2(1,2))")->GetFloat();
  niDebugFmt((_A("V: IsVec2(Vec2(1,2)) = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("IsVec2(123)")->GetFloat();
  niDebugFmt((_A("V: IsVec2(123) = %d"),v));
  CHECK_EQUAL(0,v);

  v = (tU32)ni::GetLang()->Eval("IsVec3(Vec3(1,2,3))")->GetFloat();
  niDebugFmt((_A("V: IsVec3(Vec3(1,2,3)) = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("IsVec3(123)")->GetFloat();
  niDebugFmt((_A("V: IsVec3(123) = %d"),v));
  CHECK_EQUAL(0,v);

  v = (tU32)ni::GetLang()->Eval("IsVec4(Vec4(1,2,3,4))")->GetFloat();
  niDebugFmt((_A("V: IsVec4(Vec4(1,2,3,4)) = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("IsVec4(123)")->GetFloat();
  niDebugFmt((_A("V: IsVec4(123) = %d"),v));
  CHECK_EQUAL(0,v);

  v = (tU32)ni::GetLang()->Eval("IsMatrix(MatrixIdentity())")->GetFloat();
  niDebugFmt((_A("V: IsMatrix(MatrixIdentity()) = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("IsMatrix(123)")->GetFloat();
  niDebugFmt((_A("V: IsMatrix(123) = %d"),v));
  CHECK_EQUAL(0,v);

  v = (tU32)ni::GetLang()->Eval("IsString('foo')")->GetFloat();
  niDebugFmt((_A("V: IsString('foo') = %d"),v));
  CHECK_EQUAL(1,v);
  v = (tU32)ni::GetLang()->Eval("isstring(123)")->GetFloat();
  niDebugFmt((_A("V: isstring(123) = %d"),v));
  CHECK_EQUAL(0,v);
}

TEST_FIXTURE(FExpression,Iota) {
  Ptr<iExpressionContext> ctx = ni::GetLang()->CreateExpressionContext();
  {
    const tF64 v = ctx->Eval("iota")->GetFloat();
    niDebugFmt(("iota = %g", v));
    CHECK_EQUAL(v,0);
  }
  {
    const tF64 v = ctx->Eval("iota")->GetFloat();
    niDebugFmt(("iota = %g", v));
    CHECK_EQUAL(v,1);
  }
  {
    const tF64 v = ctx->Eval("iota")->GetFloat();
    niDebugFmt(("iota = %g", v));
    CHECK_EQUAL(v,2);
  }
}

TEST_FIXTURE(FExpression,Get) {
  const tF64 pi = ni::GetLang()->Eval("pi")->GetFloat();
  niDebugFmt(("pi = %g", pi));
  CHECK_EQUAL(pi, niPi);
  {
    const tF64 v = ni::GetLang()->Eval("Get('pi')")->GetFloat();
    niDebugFmt(("Get('pi') = %g", v));
    CHECK_EQUAL(v,pi);
  }
}

TEST_FIXTURE(FExpression,Set) {
  {
    const tF64 v = ni::GetLang()->Eval("Set('x',123,Get('x'))")->GetFloat();
    niDebugFmt(("Set('x',123,Get('x')) = %g", v));
    CHECK_EQUAL(v,123);
  }
  {
    // this should fail
    const tF64 v = ni::GetLang()->Eval("Get('x')")->GetFloat();
    niDebugFmt(("Get('x') = %g", v));
    CHECK_EQUAL(v,0);
  }
  {
    const tF64 v = ni::GetLang()->Eval("Set('x',123,'y',2000,Get('x')+Get('y'))")->GetFloat();
    niDebugFmt(("Set('x',123,'y',2000,Get('x')+Get('y')) = %g", v));
    CHECK_EQUAL(v,2123);
  }
  {
    const tF64 v = ni::GetLang()->Eval("Set('x',456,'y',3000,Get('x')+Get('y'))")->GetFloat();
    niDebugFmt(("Set('x',456,'y',3000,Get('x')+Get('y')) = %g", v));
    CHECK_EQUAL(v,3456);
  }
}

TEST_FIXTURE(FExpression,SetIota) {
  Ptr<iExpressionContext> ctx = ni::GetLang()->CreateExpressionContext();
  Ptr<iExpression> expr = ctx->CreateExpression("Set('x',iota,Get('x')*2)");
  tF64 v;

  v = expr->Eval()->GetFloat();
  niDebugFmt(("Set('x',iota,Get('x')*2) = %g", v));
  CHECK_EQUAL(v,2);

  v = expr->Eval()->GetFloat();
  niDebugFmt(("Set('x',iota,Get('x')*2) = %g", v));
  CHECK_EQUAL(v,4);

  v = expr->Eval()->GetFloat();
  niDebugFmt(("Set('x',iota,Get('x')*2) = %g", v));
  CHECK_EQUAL(v,6);
}

TEST_FIXTURE(FExpression,Percent) {
  tU32 v = (tU32)ni::GetLang()->Eval("10 * 50%")->GetFloat();
  niDebugFmt((_A("V: %d"),v));
  CHECK_EQUAL(5,v);
}

TEST_FIXTURE(FExpression,If) {
  Ptr<iExpressionContext> ctx = ni::GetLang()->CreateExpressionContext();

  {
    const achar* exprStr = "If(Mod(iota,2),'odd','even'))";
    Ptr<iExpression> expr = ctx->CreateExpression(exprStr);
    {
      const tF64 v = ctx->Eval("iota")->GetFloat();
      niDebugFmt(("iota = %g", v));
      CHECK_EQUAL(v,1);
    }
    {
      cString v = expr->Eval()->GetString();
      niDebugFmt(("%s = %s", exprStr, v));
      CHECK_EQUAL(v,_ASTR("even"));
    }
    {
      cString v = expr->Eval()->GetString();
      niDebugFmt(("%s = %s", exprStr, v));
      CHECK_EQUAL(v,_ASTR("odd"));
    }
    {
      cString v = expr->Eval()->GetString();
      niDebugFmt(("%s = %s", exprStr, v));
      CHECK_EQUAL(v,_ASTR("even"));
    }
  }
}

TEST_FIXTURE(FExpression,IfSet) {
  Ptr<iExpressionContext> ctx = ni::GetLang()->CreateExpressionContext();

  {
    const achar* exprStr = "Set('i', iota, If(Get('i') > 4, '>4', Get('i') == 4, 'is4', Get('i') == 3, 'is3', '<=2'))";
    Ptr<iExpression> expr = ctx->CreateExpression(exprStr);
    {
      const tF64 v = ctx->Eval("iota")->GetFloat();
      niDebugFmt(("iota = %g", v));
      CHECK_EQUAL(v,1);
    }
    {
      cString v = expr->Eval()->GetString();
      niDebugFmt(("%s = %s", exprStr, v));
      CHECK_EQUAL(v,_ASTR("<=2"));
    }
    {
      cString v = expr->Eval()->GetString();
      niDebugFmt(("%s = %s", exprStr, v));
      CHECK_EQUAL(v,_ASTR("is3"));
    }
    {
      cString v = expr->Eval()->GetString();
      niDebugFmt(("%s = %s", exprStr, v));
      CHECK_EQUAL(v,_ASTR("is4"));
    }
    {
      cString v = expr->Eval()->GetString();
      niDebugFmt(("%s = %s", exprStr, v));
      CHECK_EQUAL(v,_ASTR(">4"));
    }
  }
}

TEST_FIXTURE(FExpression,EvalExpr) {
  Ptr<iExpressionContext> ctx = ni::GetLang()->CreateExpressionContext();
  {
    {
      const achar* expr = "Eval('1+2')";
      cString v = ni::GetLang()->Eval(expr)->GetString();
      niDebugFmt(("%s = %s", expr, v));
      CHECK_EQUAL(v,"3");
    }

    const achar* exprStr =  "Eval(Format('%s %s','1 + 2 + 3','+ 5'))";
    Ptr<iExpression> expr = ctx->CreateExpression(exprStr);
    {
      cString v = expr->Eval()->GetString();
      niDebugFmt(("%s = %s", exprStr, v));
      CHECK_EQUAL(v,"11");
    }
  }
}

TEST_FIXTURE(FExpression,StrAfter) {
  const cString v = ni::GetLang()->Eval("StrAfter('Container-02C-G1-FHD01','-')")->GetString();
  CHECK_EQUAL(_ASTR("02C-G1-FHD01"),v);
}

TEST_FIXTURE(FExpression,Quat) {
  Ptr<iExpressionContext> ptrCtx = ni::GetLang()->CreateExpressionContext();
  CHECK(ptrCtx.IsOK());

  Ptr<iExpression> expr = ptrCtx->CreateExpression("QuatMul(QuatIdentity(),QuatIdentity())");
  CHECK_RETURN_IF_FAILED(expr.IsOK());

  Ptr<iExpressionVariable> result = expr->GetEvalResult();
  CHECK_RETURN_IF_FAILED(result.IsOK());

  sVec4f v = expr->GetEvalResult()->GetVec4();
  niDebugFmt((_A("V: %s"),v));

  CHECK_EQUAL(eExpressionVariableType_Vec4, result->GetType());
  CHECK_EQUAL(eTrue, niFlagIs(result->GetFlags(),eExpressionVariableFlags_Quat));
}

TEST_FIXTURE(FExpression,CrashInvalidExpr_TruncIf) {
  Ptr<iExpressionContext> ptrCtx = ni::GetLang()->CreateExpressionContext();
  CHECK(ptrCtx.IsOK());

  Ptr<iExpression> expr = ptrCtx->CreateExpression("If(");
  CHECK_EQUAL(0,(tIntPtr)expr.ptr());
}
