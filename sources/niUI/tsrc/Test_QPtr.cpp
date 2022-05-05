#include "stdafx.h"

using namespace ni;

struct FQPtr {
  FQPtr() {
  }
  ~FQPtr() {
  }
};

struct iMockWidget : public iUnknown {
  niDeclareInterfaceUUID(iMockWidget,0x6e435573,0x7668,0x474a,0xbb,0xdc,0x5e,0x1c,0x6b,0x4e,0x8e,0xef);
};
struct MockWidget : public ni::cWidgetSinkImpl<iMockWidget> {
};

struct QueryTest : public ni::cWidgetSinkImpl<> {

  TEST_CONSTRUCTOR(QueryTest) {
    TEST_STEPS(5);
  }
  ~QueryTest() {
  }

  tBool __stdcall OnSinkAttached() niImpl {
    niLog(Info, "QueryTest::OnSinkAttached");


    Ptr<iWidget> theWidget = mpWidget->GetUIContext()->CreateWidgetRaw("MockWidget",NULL);
    CHECK(theWidget.IsOK());
    CHECK_EQUAL(2, theWidget->GetNumRefs()); // one ref in Ptr<> and one as child of the RootWidget
    CHECK_EQUAL(mpWidget->GetUIContext()->GetRootWidget(), theWidget->GetParent());

    WeakPtr<iWidget> weakWidgetRef = theWidget;

    Ptr<MockWidget> mock = niNew MockWidget();
    CHECK(mock.IsOK());
    CHECK_EQUAL(1, mock->GetNumRefs());
    theWidget->AddSink(mock);
    CHECK_EQUAL(2, mock->GetNumRefs());

    QPtr<iMockWidget> qmock = weakWidgetRef;
    CHECK(qmock.IsOK());
    CHECK_EQUAL(2, theWidget->GetNumRefs()); // Refcount incorrect with incomplete refcounting in QPtr
    CHECK_EQUAL(3, mock->GetNumRefs()); // Refcount incorrect with incomplete refcounting in QPtr

    QPtr<iFile> cantquery = weakWidgetRef;
    CHECK(!cantquery.IsOK());
    CHECK_EQUAL(2, theWidget->GetNumRefs()); // Refcount incorrect with incomplete refcounting in QPtr
    CHECK_EQUAL(3, mock->GetNumRefs());

    return eFalse;
  };

  tBool __stdcall OnDestroy() niImpl {
    niLog(Info, "QueryTest::OnDestroy");
    return eTrue;
  }

  tBool __stdcall OnSinkDetached() niImpl {
    niLog(Info, "QueryTest::OnSinkDetached");
    return eTrue;
  }
};

TEST_FIXTURE_WIDGET(FQPtr, QueryTest)
