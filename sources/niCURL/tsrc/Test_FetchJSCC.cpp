#include "stdafx.h"
#include <niCURL_ModuleDef.h>

#ifdef niJSCC

#include <emscripten.h>
#include "Test_Utils.h"

namespace {

using namespace ni;

struct FCURLFetchJSCC {
  QPtr<iCURL> _curl;
  FCURLFetchJSCC() {
    // We remove any JSCC extension to have a clean test.
    // TODO: This shouldn't be a global module thing ideally.
    emscripten_run_script(R"""({
      if ("niCURL" in Module) {
          delete Module.niCURL;
      }
    })""");
    _curl = ni::New_niCURL_CURL(niVarNull,niVarNull);
  }
};

struct FCURLFetchJSCCSink : public cIUnknownImpl<iFetchSink> {
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

TEST_FIXTURE(FCURLFetchJSCC,OverrideFetchNotImplemented) {
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

  Ptr<iMessageQueue> mq = ni::GetOrCreateMessageQueue(ni::ThreadGetCurrentThreadID());

  Nonnull<tStringCVec> requestHeaders { tStringCVec::Create() };
  requestHeaders->push_back("X-Ni-Header: HdrNarf_GetJson");

  Nonnull<FCURLFetchJSCCSink> sink = ni::MakeNonnull<FCURLFetchJSCCSink>();
  Nonnull<iFetchRequest> request = _curl->FetchGet(
    _GetHTTPSTestCasesUrl("Test_niCURL_FetchGetJson.php?param=test_json_value").c_str(),
    sink,
    requestHeaders).non_null();

  UnitTest::TestLoop(TEST_PARAMS_CALL,
    ni::Runnable([mq,request,TEST_PARAMS_LAMBDA]() {
      niUnused(testResults_);
      mq->PollAndDispatch();
      if (request->GetReadyState() == eFetchReadyState_Done) {
        niDebugFmt(("... %s: request->GetReadyState() == Done", m_testName));
        return eFalse;
      }
      return eTrue;
    }),
    ni::Runnable([request,TEST_PARAMS_LAMBDA]() {
      cString headers = request->GetReceivedHeaders()->ReadString();
      niDebugFmt(("... headers: %d bytes, %s",
                  request->GetReceivedHeaders()->GetSize(),
                  headers));

      cString content = request->GetReceivedData()->ReadString();
      niDebugFmt(("... content: %d bytes, %s", content.size(), content));

      CHECK(headers.icontains("Content-Type: application/json"));
      Ptr<iDataTable> dataDT = ni::CreateDataTable();
      const tBool validJson = JsonParseStringToDataTable(content, dataDT);
      CHECK(validJson);
      CHECK_EQUAL(eFalse, request->GetHasFailed());
      CHECK_EQUAL(_ASTR("Test_niCURL_FetchGetJson"), dataDT->GetString("name"));
      CHECK_EQUAL(_ASTR("HdrNarf_GetJson"), dataDT->GetString("received_x_ni_header"));
      return eTrue;
    }));
}

TEST_FIXTURE(FCURLFetchJSCC,NoOverrideFetchExists) {
  Ptr<iMessageQueue> mq = ni::GetOrCreateMessageQueue(ni::ThreadGetCurrentThreadID());

  Nonnull<tStringCVec> requestHeaders { tStringCVec::Create() };
  requestHeaders->push_back("Accept: application/json");
  requestHeaders->push_back("X-Ni-Header: HdrNarf_GetJson");

  Nonnull<FCURLFetchJSCCSink> sink = ni::MakeNonnull<FCURLFetchJSCCSink>();
  Nonnull<iFetchRequest> request = _curl->FetchGet(
    _GetHTTPSTestCasesUrl("Test_niCURL_FetchGetJson.php?param=test_json_value").c_str(),
    sink,
    requestHeaders).non_null();

  UnitTest::TestLoop(TEST_PARAMS_CALL,
    ni::Runnable([mq,request,TEST_PARAMS_LAMBDA]() {
      niUnused(testResults_);
      mq->PollAndDispatch();
      if (request->GetReadyState() == eFetchReadyState_Done) {
        niDebugFmt(("... %s: request->GetReadyState() == Done", m_testName));
        return eFalse;
      }
      return eTrue;
    }),
    ni::Runnable([request,sink,TEST_PARAMS_LAMBDA]() {
      cString headers = request->GetReceivedHeaders()->ReadString();
      niDebugFmt(("... headers: %d bytes, %s",
                  request->GetReceivedHeaders()->GetSize(),
                  headers));

      cString content = request->GetReceivedData()->ReadString();
      niDebugFmt(("... content: %d bytes, %s", content.size(), content));

      Ptr<iDataTable> dataDT = ni::CreateDataTable("");
      const tBool validJson = JsonParseStringToDataTable(content, dataDT);
      CHECK(validJson);

      CHECK_EQUAL(eFalse, request->GetHasFailed());
      CHECK_EQUAL(_ASTR("Test_niCURL_FetchGetJson"), dataDT->GetString("name"));
      CHECK_EQUAL(_ASTR("HdrNarf_GetJson"), dataDT->GetString("received_x_ni_header"));
      CHECK_EQUAL(sink->result, "success");
      return eTrue;
    }));
}

TEST_FIXTURE(FCURLFetchJSCC,OverrideFetchSuccess) {
  emscripten_run_script(R"""({
    niExtensions = {
      niCURL: {
        shouldOverrideFetch: function(url) {
          console.log("OverrideFetchError: Module.niCURL: shouldOverrideFetch: " + url);
          if (url.includes("/test_cases/Test_niCURL_FetchGetJson.php")) {
            console.log("OverrideFetchError: Module.niCURL: shouldOverrideFetch: yes");
            return true;
          }
          else {
            console.log("OverrideFetchError: Module.niCURL: shouldOverrideFetch: no");
            return false;
          }
        },
        handleFetchOverride: async function (aRequestUrl, onSuccess, onError, onProgress) {
          console.log("Module.niCURL: handleFetchOverride: " + aRequestUrl);
          await new Promise(r => setTimeout(r, 1000));
          console.log("Module.niCURL: handleFetchOverride: result");
          var result = `{
            "status": "OK",
            "url": "http://example.com",
            "headers": {
              "Content-Type": "application/json",
              "Accept": "application/json"
            },
            "payload": {
              "name": "Fetch Override"
            }
          }`
          console.log("Module.niCURL: handleFetchOverride: onSuccess");
          onSuccess(result);
        }
      }
    }

    Object.assign(Module, niExtensions);
  })""");

  Nonnull<tStringCVec> requestHeaders { tStringCVec::Create() };
  requestHeaders->push_back("X-Ni-Header: HdrNarf_GetJson_WithOverride");

  // We need to recreate niCURL because niCURL looks if there is a fetch override in
  // the constructor (which makes sense generally but not for testing)
  _curl = ni::New_niCURL_CURL(niVarNull,niVarNull);
  Ptr<iMessageQueue> mq = ni::GetOrCreateMessageQueue(ni::ThreadGetCurrentThreadID());

  Nonnull<FCURLFetchJSCCSink> sink = ni::MakeNonnull<FCURLFetchJSCCSink>();
  Nonnull<iFetchRequest> request = _curl->FetchGet(
    _GetHTTPSTestCasesUrl("Test_niCURL_FetchGetJson.php?param=test_json_value").c_str(),
    sink,
    requestHeaders).non_null();

  QPtr<iCURL> curl = _curl;
  UnitTest::TestLoop(TEST_PARAMS_CALL,
    ni::Runnable([mq,request,TEST_PARAMS_LAMBDA]() {
      niUnused(testResults_);
      mq->PollAndDispatch();
      if (request->GetReadyState() == eFetchReadyState_Done) {
        niDebugFmt(("... %s: request->GetReadyState() == Done", m_testName));
        return eFalse;
      }
      return eTrue;
      }),
    ni::Runnable([request,sink,TEST_PARAMS_LAMBDA]() {
      cString headers = request->GetReceivedHeaders()->ReadString();
      niDebugFmt(("... headers: %d bytes, %s",
                  request->GetReceivedHeaders()->GetSize(),
                  headers));

      cString content = request->GetReceivedData()->ReadString();
      niDebugFmt(("... content: %d bytes, %s", content.size(), content));

      Ptr<iDataTable> dataDT = ni::CreateDataTable("");
      const tBool validJson = JsonParseStringToDataTable(content, dataDT);
      CHECK(validJson);

      CHECK_EQUAL(eFalse, request->GetHasFailed());
      CHECK_EQUAL(_ASTR("Fetch Override"), dataDT->GetString("name"));
      // TODO: Header should pass through, we'll likely need them for auth later on
      // CHECK_EQUAL(_ASTR("HdrNarf_GetJson_WithOverride"), dataDT->GetString("received_x_ni_header"));
      CHECK_EQUAL(sink->result, "success");
      return eFalse;
    }));
}

TEST_FIXTURE(FCURLFetchJSCC,OverrideFetchSkip) {
  emscripten_run_script(R"""({
    niExtensions = {
      niCURL: {
        shouldOverrideFetch: function(url) {
          return false;
        },
        handleFetchOverride: async function (aRequestUrl, onSuccess, onError, onProgress) {
          // THIS WILL NEVER RUN ANYWAYS
          await new Promise(r => setTimeout(r, 1000));
          return false;
        }
      }
    }

    Object.assign(Module, niExtensions);
  })""");

  // We need to recreate niCURL because niCURL looks if there is a fetch override in
  // the constructor (which makes sense generally but not for testing)
  _curl = ni::New_niCURL_CURL(niVarNull,niVarNull);

  Ptr<iMessageQueue> mq = ni::GetOrCreateMessageQueue(ni::ThreadGetCurrentThreadID());

  Nonnull<tStringCVec> requestHeaders { tStringCVec::Create() };
  requestHeaders->push_back("Accept: application/json");
  Nonnull<FCURLFetchJSCCSink> sink = ni::MakeNonnull<FCURLFetchJSCCSink>();
  Nonnull<iFetchRequest> request = _curl->FetchGet(
    _GetHTTPSTestCasesUrl("Test_niCURL_FetchGetJson.php?param=test_json_value").c_str(),
    sink,
    requestHeaders).non_null();

  UnitTest::TestLoop(TEST_PARAMS_CALL,
    ni::Runnable([mq,request,TEST_PARAMS_LAMBDA]() {
      niUnused(testResults_);
      mq->PollAndDispatch();
      if (request->GetReadyState() == eFetchReadyState_Done) {
        niDebugFmt(("... %s: request->GetReadyState() == Done", m_testName));
        return eFalse;
      }
      return eTrue;
    }),
    ni::Runnable([request,TEST_PARAMS_LAMBDA,sink]() {
      cString headers = request->GetReceivedHeaders()->ReadString();
      niDebugFmt(("... headers: %d bytes, %s",
                  request->GetReceivedHeaders()->GetSize(),
                  headers));

      cString content = request->GetReceivedData()->ReadString();
      niDebugFmt(("... content: %d bytes, %s", content.size(), content));

      Ptr<iDataTable> dataDT = ni::CreateDataTable("");
      const tBool validJson = JsonParseStringToDataTable(content, dataDT);
      CHECK(validJson);

      CHECK_EQUAL(eFalse, request->GetHasFailed());
      CHECK_EQUAL(_ASTR("Test_niCURL_FetchGetJson"), dataDT->GetString("name"));
      CHECK_EQUAL(sink->result, "success");
      return eTrue;
    }));
}

TEST_FIXTURE(FCURLFetchJSCC,OverrideFetchError) {
  emscripten_run_script(R"""({
    niExtensions = {
      niCURL: {
        shouldOverrideFetch: function(url) {
          console.log("OverrideFetchError: Module.niCURL: shouldOverrideFetch: " + url);
          if (url.includes("/test_cases/Test_niCURL_FetchGetJson.php")) {
            console.log("OverrideFetchError: Module.niCURL: shouldOverrideFetch: yes");
            return true;
          }
          else {
            console.log("OverrideFetchError: Module.niCURL: shouldOverrideFetch: no");
            return false;
          }
        },
        handleFetchOverride: async function (aRequestUrl, onSuccess, onError, onProgress) {
          console.log("Module.niCURL: handleFetchOverride: " + aRequestUrl);
          await new Promise(r => setTimeout(r, 1000));
          onError(`
              {
                "url": "http://example.com",
                "payload": "Couldn't fetch the URL"
              }`);
          onError("HOLA");
        }
      }
    }

    Object.assign(Module, niExtensions);
  })""");

  // We need to recreate niCURL because niCURL looks if there is a fetch override in
  // the constructor (which makes sense generally but not for testing)
  _curl = ni::New_niCURL_CURL(niVarNull,niVarNull);

  Ptr<iMessageQueue> mq = ni::GetOrCreateMessageQueue(ni::ThreadGetCurrentThreadID());

  Nonnull<tStringCVec> requestHeaders { tStringCVec::Create() };
  requestHeaders->push_back("Accept: application/json");
  Nonnull<FCURLFetchJSCCSink> sink = ni::MakeNonnull<FCURLFetchJSCCSink>();
  Nonnull<iFetchRequest> request = _curl->FetchGet(
    _GetHTTPSTestCasesUrl("Test_niCURL_FetchGetJson.php?param=test_json_value").c_str(),
    sink,
    requestHeaders).non_null();
  QPtr<iCURL> curl = _curl;
  UnitTest::TestLoop(TEST_PARAMS_CALL,
    ni::Runnable([mq,request,sink,curl,TEST_PARAMS_LAMBDA]() {
      mq->PollAndDispatch();
      if (request->GetReadyState() == eFetchReadyState_Done) {
        niDebugFmt(("... %s: request->GetReadyState() == Done", m_testName));
        return eFalse;
      }
      // we also test spamming, if should return the same request always.
      // I would like to override request with the result but I am in const territory here...
      Nonnull<iFetchRequest> theRequest = curl->FetchGet(
      _GetHTTPSTestCasesUrl("Test_niCURL_FetchGetJson.php?param=test_json_value").c_str(),
      sink,
      NULL).non_null();
      CHECK_EQUAL(theRequest, request);
      return eTrue;
    }),
    ni::Runnable([request,sink,TEST_PARAMS_LAMBDA]() {
      cString headers = request->GetReceivedHeaders()->ReadString();
      niDebugFmt(("... headers: %d bytes, %s",
                  request->GetReceivedHeaders()->GetSize(),
                  headers));

      cString content = request->GetReceivedData()->ReadString();
      niDebugFmt(("... content: %d bytes, %s", content.size(), content));

      Ptr<iDataTable> dataDT = ni::CreateDataTable("");
      const tBool validJson = JsonParseStringToDataTable(content, dataDT);
      CHECK(validJson);

      CHECK_EQUAL(eFalse, request->GetHasFailed());

      CHECK_EQUAL(request->GetStatus(), 500);
      CHECK_EQUAL(dataDT->GetString("payload"), "Couldn't fetch the URL");
      CHECK_EQUAL(sink->result, "error");
      return eTrue;
    }));
}

TEST_FIXTURE(FCURLFetchJSCC,RequestNoAwaits) {
  emscripten_run_script(R"""({
    niExtensions = {
      niCURL: {
        shouldOverrideFetch: function(url) {
            return true;
        },
        handleFetchOverride: async function (aRequestUrl, onSuccess, onError, onProgress) {
          console.log("Module.niCURL: handleFetchOverride: " + aRequestUrl);
          var result = `
              {
                "status": "OK",
                "url": "http://example.com",
                "headers": {
                  "Content-Type": "application/json",
                  "Accept": "application/json"
                },
                "payload": [{"id":"90","symbol":"BTC","name":"Bitcoin","nameid":"bitcoin","rank":1,"price_usd":"23864.25","percent_change_24h":"-2.25","percent_change_1h":"0.17","percent_change_7d":"-4.28","market_cap_usd":"460296162080.63","volume24":"28850217961.42","volume24_native":"1208930.23","csupply":"19288102.00","price_btc":"1.00","tsupply":"19288102","msupply":"21000000"}]
              }`
          onSuccess(result);
        }
      }
    }

    Object.assign(Module, niExtensions);
  })""");
  // We need to recreate niCURL because niCURL looks if there is a fetch override in
  // the constructor (which makes sense generally but not for testing)
  _curl = ni::New_niCURL_CURL(niVarNull,niVarNull);
  Ptr<iMessageQueue> mq = ni::GetOrCreateMessageQueue(ni::ThreadGetCurrentThreadID());

  Nonnull<FCURLFetchJSCCSink> sink = ni::MakeNonnull<FCURLFetchJSCCSink>();
  Nonnull<iFetchRequest> request = _curl->FetchGet(
    _GetHTTPSTestCasesUrl("Test_niCURL_FetchGetJson.php?param=test_json_value").c_str(),
    sink,
    NULL).non_null();

  QPtr<iCURL> curl = _curl;
  UnitTest::TestLoop(TEST_PARAMS_CALL,
    ni::Runnable([mq,request,sink,curl,TEST_PARAMS_LAMBDA]() {
      mq->PollAndDispatch();
      if (request->GetReadyState() == eFetchReadyState_Done) {
        niDebugFmt(("... %s: request->GetReadyState() == Done", m_testName));
        return eFalse;
      }
      // try to make the same request multiple times (it should always return the
      // same request until that request is done)
      Nonnull<iFetchRequest> theRequest = curl->FetchGet(
      _GetHTTPSTestCasesUrl("Test_niCURL_FetchGetJson.php?param=test_json_value").c_str(),
      sink,
      NULL).non_null();
      CHECK_EQUAL(theRequest, request);
      return eTrue;
      }),
    ni::Runnable([request,sink,TEST_PARAMS_LAMBDA]() {
      cString headers = request->GetReceivedHeaders()->ReadString();
      niDebugFmt(("... headers: %d bytes, %s",
                  request->GetReceivedHeaders()->GetSize(),
                  headers));

      cString content = request->GetReceivedData()->ReadString();
      niDebugFmt(("... content: %d bytes, %s", content.size(), content));

      Ptr<iDataTable> dataDT = ni::CreateDataTable("");
      const tBool validJson = JsonParseStringToDataTable(content, dataDT);
      CHECK(validJson);

      CHECK_EQUAL(eFalse, request->GetHasFailed());
      CHECK_EQUAL(_ASTR("Test_niCURL_FetchGetJson"), dataDT->GetString("name"));
      CHECK_EQUAL(sink->result, "success");
      return eFalse;
    }));
}

}
#endif // #ifdef niJSCC
