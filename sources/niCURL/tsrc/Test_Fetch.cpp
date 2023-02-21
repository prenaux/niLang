#include "stdafx.h"
#include <niCURL_ModuleDef.h>

#ifdef niJSCC
#include <emscripten.h>
#endif

namespace {

using namespace ni;

struct FCURLFetch {
  QPtr<iCURL> _curl;
  FCURLFetch() {
    _curl = ni::New_niCURL_CURL(niVarNull,niVarNull);
  }
};

struct MyFetchSink : public cIUnknownImpl<iFetchSink> {
  virtual void __stdcall OnFetchSink_Success(iFetchRequest* apFetch) {
    niDebugFmt(("... OnFetchSink_Success: status: %d, got %d bytes",
                apFetch->GetStatus(),
                apFetch->GetReceivedData()->GetSize()));
  }
  virtual void __stdcall OnFetchSink_Error(iFetchRequest* apFetch) {
    niDebugFmt(("... OnFetchSink_Error: status: %d, got %d bytes",
                apFetch->GetStatus(),
                apFetch->GetReceivedData()->GetSize()));
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
TEST_FIXTURE(FCURLFetch,OverrideFetchSkip) {
#ifdef niJSCC
  emscripten_run_script(R"""({
    niExtensions = {
      niCURL: {
        handleFetchOverride: function () {
          console.log("Module.niCURL: handleFetchOverride");
          return `{ "status": "SKIP"  }`
        }
      }
    }

    Object.assign(Module, niExtensions);
  })""");
#endif

  // We need to recreate niCURL because niCURL looks if there is a fetch override in
  // the constructor (which makes sense generally but not for testing)
  _curl = ni::New_niCURL_CURL(niVarNull,niVarNull);

  Ptr<iMessageQueue> mq = ni::GetOrCreateMessageQueue(ni::ThreadGetCurrentThreadID());

  Nonnull<tStringCVec> requestHeaders { tStringCVec::Create() };
  requestHeaders->push_back("Accept: application/json");
  Nonnull<MyFetchSink> sink = ni::MakeNonnull<MyFetchSink>();
  Nonnull<iFetchRequest> request = _curl->FetchGet(
    "https://api.coinlore.com/api/ticker/?id=90",
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

      Ptr<iFile> receivedData = request->GetReceivedData();
      cString data = receivedData->ReadString();
      niDebugFmt(("... data: %d bytes, %s",
                  request->GetReceivedData()->GetSize(),
                  data));
      receivedData->SeekSet(0);
      Ptr<iDataTable> dataDT = ni::CreateDataTable("");
      const tBool validJson =
          JsonParseFileToDataTable(receivedData, dataDT);
      CHECK(validJson);
      CHECK(data.StartsWith("[{\"id\":"));
      CHECK_EQUAL(eFalse, request->GetHasFailed());
      return eTrue;
    }));
}

TEST_FIXTURE(FCURLFetch,OverrideFetchError) {
#ifdef niJSCC
  emscripten_run_script(R"""({
    niExtensions = {
      niCURL: {
        handleFetchOverride: function () {
          console.log("Module.niCURL: handleFetchOverride");
          return `
              {
                "status": "ERROR",
                "url": "http://example.com",
                "payload": "Couldn't fetch the URL"
              }`
        }
      }
    }

    Object.assign(Module, niExtensions);
  })""");
#endif

  // We need to recreate niCURL because niCURL looks if there is a fetch override in
  // the constructor (which makes sense generally but not for testing)
  _curl = ni::New_niCURL_CURL(niVarNull,niVarNull);

  Ptr<iMessageQueue> mq = ni::GetOrCreateMessageQueue(ni::ThreadGetCurrentThreadID());

  Nonnull<tStringCVec> requestHeaders { tStringCVec::Create() };
  requestHeaders->push_back("Accept: application/json");
  Nonnull<MyFetchSink> sink = ni::MakeNonnull<MyFetchSink>();
  Nonnull<iFetchRequest> request = _curl->FetchGet(
    "https://api.coinlore.com/api/ticker/?id=90",
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

      Ptr<iFile> receivedData = request->GetReceivedData();
      cString data = receivedData->ReadString();
      niDebugFmt(("... data: %d bytes, %s",
                  request->GetReceivedData()->GetSize(),
                  data));
      receivedData->SeekSet(0);
      Ptr<iDataTable> dataDT = ni::CreateDataTable("");
      const tBool validJson =
          JsonParseFileToDataTable(receivedData, dataDT);
      CHECK(validJson);
#ifdef niJSCC
      // cString xml = ni::DataTableToXML(dataDT);
      // niDebugFmt(("XML: %s", xml));
      CHECK_EQUAL(request->GetStatus(), 500);
      CHECK_EQUAL(dataDT->GetString("payload"), "Couldn't fetch the URL");
#else
      CHECK(data.StartsWith("[{\"id\":"));
#endif
      CHECK_EQUAL(eFalse, request->GetHasFailed());
      return eTrue;
    }));
}

TEST_FIXTURE(FCURLFetch,Get) {
  Ptr<iMessageQueue> mq = ni::GetOrCreateMessageQueue(ni::ThreadGetCurrentThreadID());

  Nonnull<tStringCVec> requestHeaders { tStringCVec::Create() };
  requestHeaders->push_back("X-Ni-Header: HdrNarf");

  Nonnull<MyFetchSink> sink = ni::MakeNonnull<MyFetchSink>();
  Nonnull<iFetchRequest> request = _curl->FetchGet(
    "https://www.bytecollider.com/test_cases/Test_niCURL_FetchGet.php?param=test_value",
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

#if !defined niJSCC
      CHECK(headers.icontains("Access-Control-Allow-Origin: *"));
#endif
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
    "https://www.bytecollider.com/test_cases/Test_niCURL_FetchPost.php",
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

#if !defined niJSCC
      CHECK(headers.icontains("Access-Control-Allow-Origin: *"));
#endif
      CHECK_EQUAL(_ASTR("Test_niCURL_FetchPost:HdrNarf:posted_value"),data);
      CHECK_EQUAL(eFalse, request->GetHasFailed());
      return eTrue;
    }));
}

TEST_FIXTURE(FCURLFetch,GetJson) {
#ifdef niJSCC
  emscripten_run_script(R"""({
    niExtensions = {
      niCURL: {
        handleFetchOverride: function () {
          console.log("Module.niCURL: handleFetchOverride");
          return `
              {
                "status": "OK",
                "url": "http://example.com",
                "headers": {
                  "Content-Type": "application/json",
                  "Accept": "application/json"
                },
                "payload": "[{\\"id\\":\\"90\\"}]"
              }`
        }
      }
    }

    Object.assign(Module, niExtensions);
  })""");
#endif
  // We need to recreate niCURL because niCURL looks if there is a fetch override in
  // the constructor (which makes sense generally but not for testing)
  _curl = ni::New_niCURL_CURL(niVarNull,niVarNull);
  Ptr<iMessageQueue> mq = ni::GetOrCreateMessageQueue(ni::ThreadGetCurrentThreadID());

  Nonnull<MyFetchSink> sink = ni::MakeNonnull<MyFetchSink>();
  Nonnull<iFetchRequest> request = _curl->FetchGet(
    "https://api.coinlore.com/api/ticker/?id=90",
    sink,
    NULL).non_null();

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

#if !defined niJSCC
      CHECK(headers.icontains("Access-Control-Allow-Origin: *"));
#endif
#ifdef niJSCC
      Ptr<iFile> recData = request->GetReceivedData();
      recData->SeekSet(0);
      Ptr<iDataTable> dt = ni::CreateDataTable();
      const tBool validJson = JsonParseFileToDataTable(recData, dt);
      data = dt->GetString("payload");
#endif
      CHECK(data.StartsWith("[{\"id\":\"90\""));
      CHECK(headers.icontains("Content-Type: application/json"));
      CHECK_EQUAL(eFalse, request->GetHasFailed());
      return eFalse;
    }));
}

// if the override fetch exists but it is not implement
TEST_FIXTURE(FCURLFetch,OverrideFetchNotImplemented) {
#ifdef niJSCC
  emscripten_run_script(R"""({
    niExtensions = {
      niCURL: {
        handleFetchOverride: null
      }
    }

    Object.assign(Module, niExtensions);
  })""");

  tBool hasFetchOverride = static_cast<tBool>(EM_ASM_INT({
      return Module["niCURL"].handleFetchOverride != null;
    }));
  CHECK(!hasFetchOverride);
#endif

  Ptr<iMessageQueue> mq = ni::GetOrCreateMessageQueue(ni::ThreadGetCurrentThreadID());

  Nonnull<MyFetchSink> sink = ni::MakeNonnull<MyFetchSink>();
  Nonnull<iFetchRequest> request = _curl->FetchGet(
    "https://api.coinlore.com/api/ticker/?id=90",
    sink,
    NULL).non_null();

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

      CHECK(data.StartsWith("[{\"id\":\"90\""));
#if !defined niJSCC
      CHECK(headers.icontains("Access-Control-Allow-Origin: *"));
#endif
      CHECK(headers.icontains("Content-Type: application/json"));
      Ptr<iDataTable> dataDT = ni::CreateDataTable();
      const tBool validJson = JsonParseFileToDataTable(request->GetReceivedData(), dataDT);
      CHECK(validJson);
      CHECK(data.StartsWith("[{\"id\":"));
      CHECK_EQUAL(eFalse, request->GetHasFailed());
      return eTrue;
    }));
}


TEST_FIXTURE(FCURLFetch,NoOverrideFetchExists) {
#ifdef niJSCC
  // we remove any JSCC extension to have a clean test
  emscripten_run_script(R"""({
   if ("niCURL" in Module) {
    delete Module.niCURL;
  }
  })""");

  // We need to recreate niCURL because niCURL looks if there is a fetch override in
  // the constructor (which makes sense generally but not for testing)
  _curl = ni::New_niCURL_CURL(niVarNull,niVarNull);
#endif

  Ptr<iMessageQueue> mq = ni::GetOrCreateMessageQueue(ni::ThreadGetCurrentThreadID());

  Nonnull<tStringCVec> requestHeaders { tStringCVec::Create() };
  requestHeaders->push_back("Accept: application/json");

  Nonnull<MyFetchSink> sink = ni::MakeNonnull<MyFetchSink>();
  Nonnull<iFetchRequest> request = _curl->FetchGet(
    "https://api.coinlore.com/api/ticker/?id=90",
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

      Ptr<iDataTable> dataDT = ni::CreateDataTable("");
      const tBool validJson = JsonParseFileToDataTable(request->GetReceivedData(), dataDT);
      CHECK(validJson);
      CHECK(data.StartsWith("[{\"id\":"));
      CHECK_EQUAL(eFalse, request->GetHasFailed());
      return eTrue;
    }));
}

}
