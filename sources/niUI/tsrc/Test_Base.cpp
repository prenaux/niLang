#include "stdafx.h"

using namespace ni;

struct FBase {
  FBase() {
    niDebugFmt(("FBase::FBase"));
  }
  ~FBase() {
    niDebugFmt(("FBase::~FBase"));
  }
};

TEST_FIXTURE(FBase, Base) {
  TEST_STEPS(5);

  static int count = 0;
  niDebugFmt(("FBase-Base: %d %s", count++, UnitTest::runFixtureName));
}

struct Base2 : public ni::cWidgetSinkImpl<> {

  TEST_CONSTRUCTOR(Base2) {
    niDebugFmt(("Base2::Base2"));
  }
  ~Base2() {
    niDebugFmt(("Base2::~Base2"));
  }

};
TEST_FIXTURE_WIDGET(FBase,Base2);

struct Base3 : public ni::cWidgetSinkImpl<> {
  TEST_CONSTRUCTOR(Base3) {
    niDebugFmt(("Base3::Base3"));
    // TEST_STEPS(100000);
  }

  ~Base3() {
    niDebugFmt(("Base3::~Base3"));
  }

  virtual tI32 __stdcall AddRef() niImpl {
    return tWidgetBaseImpl::AddRef();
  }
  virtual tI32 __stdcall Release() niImpl {
    return tWidgetBaseImpl::Release();
  }

  tBool __stdcall OnSinkAttached() niImpl {
    niLog(Info, "Base3::OnSinkAttached");
    return eTrue;
  };

  tBool __stdcall OnDestroy() niImpl {
    niLog(Info, "Base3::OnDestroy");
    return eTrue;
  }

  tBool __stdcall OnSinkDetached() niImpl {
    niLog(Info, "Base3::OnSinkDetached");
    return eTrue;
  }

  tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niImpl {
    apCanvas->BlitFill(apCanvas->GetViewport().ToFloat(), 0xFFFF0000);
    return eFalse;
  }
};
TEST_WIDGET(Base3);
