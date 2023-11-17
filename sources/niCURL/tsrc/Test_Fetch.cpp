#include "stdafx.h"
#include <niCURL_ModuleDef.h>
#include "Test_Utils.h"

#ifdef niJSCC
#include <emscripten.h>
#endif

namespace {

using namespace ni;

struct FCURLFetch {
  QPtr<iCURL> _curl;
  FCURLFetch() {
#ifdef niJSCC
    // We remove any JSCC extension to have a clean test.
    // TODO: This shouldn't be a global module thing ideally.
    emscripten_run_script(R"""(function() {
      if ("niCURL" in Module) {
        delete Module.niCURL;
      }
    }();)""");
#endif
    _curl = ni::New_niCURL_CURL(niVarNull,niVarNull);
  }
};

struct MyFetchSink : public cIUnknownImpl<iFetchSink> {
  cString result = "";

  virtual void __stdcall OnFetchSink_Success(iFetchRequest* apFetch) {
    niDebugFmt(("... OnFetchSink_Success: status: %d, got %d bytes",
                apFetch->GetStatus(),
                apFetch->GetReceivedData()->GetSize()));
    result = "success";
  }
  virtual void __stdcall OnFetchSink_Error(iFetchRequest* apFetch) {
    niDebugFmt(("... OnFetchSink_Error: status: %d, got %d bytes",
                apFetch->GetStatus(),
                apFetch->GetReceivedData()->GetSize()));
    result = "error";
  }
  virtual void __stdcall OnFetchSink_Progress(iFetchRequest* apFetch) {
    niDebugFmt(("... OnFetchSink_Progress: status: %d, got %d bytes",
                apFetch->GetStatus(),
                apFetch->GetReceivedData()->GetSize()));
  }
  virtual void __stdcall OnFetchSink_ReadyStateChange(iFetchRequest* apFetch) {
    niDebugFmt(("... OnFetchSink_ReadyStateChange: %s",
                niEnumToChars(eFetchReadyState,apFetch->GetReadyState())));
  }
};

TEST_FIXTURE(FCURLFetch,Get) {
  Ptr<iMessageQueue> mq = ni::GetOrCreateMessageQueue(ni::ThreadGetCurrentThreadID());

  Nonnull<tStringCVec> requestHeaders { tStringCVec::Create() };
  requestHeaders->push_back("X-Ni-Header: HdrNarf");

  Nonnull<MyFetchSink> sink = ni::MakeNonnull<MyFetchSink>();
  Nonnull<iFetchRequest> request = _curl->FetchGet(
    _GetHTTPSTestCasesUrl("Test_niCURL_FetchGet.php?param=test_value").c_str(),
    sink,
    requestHeaders).non_null();

  UnitTest::TestLoop(TEST_PARAMS_CALL,
    ni::Runnable([mq,request]() {
      mq->PollAndDispatch();
      if (request->GetReadyState() == eFetchReadyState_Done) {
        return eFalse;
      }
      return eTrue;
    }),
    ni::Runnable([request,TEST_PARAMS_LAMBDA]() {
      cString headers = request->GetReceivedHeaders()->ReadString();
      niDebugFmt(("... headers: %d bytes, %s",
                  request->GetReceivedHeaders()->GetSize(),
                  headers));

      cString data = request->GetReceivedData()->ReadString();
      niDebugFmt(("... data: %d bytes, %s",
                  request->GetReceivedData()->GetSize(),
                  data));

      CHECK(headers.icontains("Access-Control-Allow-Origin: *"));
      CHECK_EQUAL(_ASTR("Test_niCURL_FetchGet:HdrNarf:test_value"),data);
      CHECK_EQUAL(eFalse, request->GetHasFailed());
      return eTrue;
    }));
}

TEST_FIXTURE(FCURLFetch,Post) {
  Ptr<iMessageQueue> mq = ni::GetOrCreateMessageQueue(ni::ThreadGetCurrentThreadID());

  Nonnull<tStringCVec> requestHeaders { tStringCVec::Create() };
  requestHeaders->push_back("X-Ni-Header: HdrNarf");

  Nonnull<iFile> postData {ni::CreateFileDynamicMemory(128,"postdata")};
  postData->WriteString(R"""({"param":"posted_value", "key2":"value2"})""");
  postData->SeekSet(0);

  Nonnull<MyFetchSink> sink = ni::MakeNonnull<MyFetchSink>();
  Nonnull<iFetchRequest> request = _curl->FetchPost(
    _GetHTTPSTestCasesUrl("Test_niCURL_FetchPost.php").c_str(),
    postData,
    sink,
    requestHeaders).non_null();

  UnitTest::TestLoop(TEST_PARAMS_CALL,
    ni::Runnable([mq,request]() {
      mq->PollAndDispatch();
      if (request->GetReadyState() == eFetchReadyState_Done) {
        return eFalse;
      }
      return eTrue;
    }),
    ni::Runnable([request,TEST_PARAMS_LAMBDA]() {
      cString headers = request->GetReceivedHeaders()->ReadString();
      niDebugFmt(("... headers: %d bytes, %s",
                  request->GetReceivedHeaders()->GetSize(),
                  headers));

      cString data = request->GetReceivedData()->ReadString();
      niDebugFmt(("... data: %d bytes, %s",
                  request->GetReceivedData()->GetSize(),
                  data));

      CHECK(headers.icontains("Access-Control-Allow-Origin: *"));
      CHECK_EQUAL(_ASTR("Test_niCURL_FetchPost:HdrNarf:posted_value"),data);
      CHECK_EQUAL(eFalse, request->GetHasFailed());
      return eTrue;
    }));
}

TEST_FIXTURE(FCURLFetch,GetJson) {
  Ptr<iMessageQueue> mq = ni::GetOrCreateMessageQueue(ni::ThreadGetCurrentThreadID());

  Nonnull<tStringCVec> requestHeaders { tStringCVec::Create() };
  requestHeaders->push_back("X-Ni-Header: HdrNarf_GetJson");

  Nonnull<MyFetchSink> sink = ni::MakeNonnull<MyFetchSink>();
  Nonnull<iFetchRequest> request = _curl->FetchGet(
    _GetHTTPSTestCasesUrl("Test_niCURL_FetchGetJson.php?param=test_json_value").c_str(),
    sink,
    requestHeaders).non_null();

  UnitTest::TestLoop(TEST_PARAMS_CALL,
    ni::Runnable([mq,request]() {
      mq->PollAndDispatch();
      if (request->GetReadyState() == eFetchReadyState_Done) {
        return eFalse;
      }
      return eTrue;
    }),
    ni::Runnable([request,TEST_PARAMS_LAMBDA,sink]() {
      cString headers = request->GetReceivedHeaders()->ReadString();
      niDebugFmt(("... headers: %d bytes, %s",
                  request->GetReceivedHeaders()->GetSize(),
                  headers));

      cString data = request->GetReceivedData()->ReadString();
      niDebugFmt(("... data: %d bytes, %s",
                  request->GetReceivedData()->GetSize(),
                  data));

      CHECK(headers.icontains("Access-Control-Allow-Origin: *"));
      CHECK(headers.icontains("Content-Type: application/json"));
      CHECK_EQUAL(eFalse, request->GetHasFailed());
      CHECK_EQUAL(sink->result, "success");

      Ptr<iDataTable> dt = ni::CreateDataTable("Response");
      CHECK(ni::JsonParseStringToDataTable(data,dt));
      CHECK_EQUAL(_ASTR("Test_niCURL_FetchGetJson"), dt->GetString("name"));
      CHECK_EQUAL(_ASTR("test_json_value"), dt->GetString("param"));
      CHECK_EQUAL(_ASTR("HdrNarf_GetJson"), dt->GetString("received_x_ni_header"));

      return eFalse;
    }));
}

}
