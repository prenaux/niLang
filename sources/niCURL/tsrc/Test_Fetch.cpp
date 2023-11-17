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
    #ifdef niJSCC
      // we remove any JSCC extension to have a clean test
      emscripten_run_script(R"""({
      if ("niCURL" in Module) {
        delete Module.niCURL;
      }
      })""");

      _curl = ni::New_niCURL_CURL(niVarNull,niVarNull);
    #else
      _curl = ni::New_niCURL_CURL(niVarNull,niVarNull);
    #endif
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
        shouldOverrideFetch: function(url) {
          if (url.includes("https://api.coinlore.com/api/ticker/")) {
            return true;
          }
          return false;
        },
        handleFetchOverride: async function (aRequestUrl, onSuccess, onError, onProgress) {
          console.log("Module.niCURL: handleFetchOverride: " + aRequestUrl);
          await new Promise(r => setTimeout(r, 5000));
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
    ni::Runnable([request,TEST_PARAMS_LAMBDA,sink]() {
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
      CHECK(data.StartsWith("[{\"id\":\"90\""));
      CHECK(headers.icontains("Content-Type: application/json"));
      CHECK_EQUAL(eFalse, request->GetHasFailed());
      CHECK_EQUAL(sink->result, "success");
      return eFalse;
    }));
}

// NOTE: All JSCC exclusive tests go here
#ifdef niJSCC
// if the override fetch exists but it is not implement
TEST_FIXTURE(FCURLFetch,JSCC_OverrideFetchNotImplemented) {
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
      CHECK(headers.icontains("Content-Type: application/json"));
      Ptr<iDataTable> dataDT = ni::CreateDataTable();
      const tBool validJson = JsonParseFileToDataTable(request->GetReceivedData(), dataDT);
      CHECK(validJson);
      CHECK_EQUAL(eFalse, request->GetHasFailed());
      return eTrue;
    }));
}

TEST_FIXTURE(FCURLFetch,JSCC_NoOverrideFetchExists) {
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

TEST_FIXTURE(FCURLFetch,JSCC_SpamRequest) {
  emscripten_run_script(R"""({
    niExtensions = {
      niCURL: {
        shouldOverrideFetch: function(url) {
          if (url.includes("https://api.coinlore.com/api/ticker/")) {
            return true;
          }
          return false;
        },
        handleFetchOverride: async function (aRequestUrl, onSuccess, onError, onProgress) {
          console.log("Module.niCURL: handleFetchOverride: " + aRequestUrl);
          await new Promise(r => setTimeout(r, 10000));
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
          onSuccess(result); // calling this twice should not cause an error
        }
      }
    }

    Object.assign(Module, niExtensions);
  })""");
  // We need to recreate niCURL because niCURL looks if there is a fetch override in
  // the constructor (which makes sense generally but not for testing)
  _curl = ni::New_niCURL_CURL(niVarNull,niVarNull);
  Ptr<iMessageQueue> mq = ni::GetOrCreateMessageQueue(ni::ThreadGetCurrentThreadID());

  Nonnull<MyFetchSink> sink = ni::MakeNonnull<MyFetchSink>();
  Nonnull<iFetchRequest> request = _curl->FetchGet(
    "https://api.coinlore.com/api/ticker/?id=90",
    sink,
    NULL).non_null();

  QPtr<iCURL> curl = _curl;
  UnitTest::TestLoop(TEST_PARAMS_CALL,
    ni::Runnable([mq,request,sink,curl,TEST_PARAMS_LAMBDA]() {
      mq->PollAndDispatch();
      if (request->GetReadyState() == eFetchReadyState_Done) {
        return eFalse;
      }
      // try to make the same request multiple times (it should always return the
      // same request until that request is done)
      Nonnull<iFetchRequest> theRequest = curl->FetchGet(
      "https://api.coinlore.com/api/ticker/?id=90",
      sink,
      NULL).non_null();
      CHECK_EQUAL(theRequest, request);
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

      CHECK(data.StartsWith("[{\"id\":\"90\""));
      CHECK_EQUAL(eFalse, request->GetHasFailed());
      CHECK_EQUAL(sink->result, "success");
      return eFalse;
    }));
}

TEST_FIXTURE(FCURLFetch,JSCC_OverrideFetchSkip) {
  emscripten_run_script(R"""({
    niExtensions = {
      niCURL: {
        shouldOverrideFetch: function(url) {
          return false;
        },
        handleFetchOverride: async function (aRequestUrl, onSuccess, onError, onProgress) {
          // THIS WILL NEVER RUN ANYWAYS
          await new Promise(r => setTimeout(r, 10000));
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
    ni::Runnable([request,TEST_PARAMS_LAMBDA,sink]() {
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
      CHECK_EQUAL(sink->result, "success");
      return eTrue;
    }));
}

TEST_FIXTURE(FCURLFetch,JSCC_OverrideFetchError) {
  emscripten_run_script(R"""({
    niExtensions = {
      niCURL: {
        shouldOverrideFetch: function(url) {
          if (url.includes("https://api.coinlore.com/api/ticker/")) {
            return true;
          }
          return false;
        },
        handleFetchOverride: async function (aRequestUrl, onSuccess, onError, onProgress) {
          console.log("Module.niCURL: handleFetchOverride: " + aRequestUrl);
          await new Promise(r => setTimeout(r, 5000));
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
  Nonnull<MyFetchSink> sink = ni::MakeNonnull<MyFetchSink>();
  Nonnull<iFetchRequest> request = _curl->FetchGet(
    "https://api.coinlore.com/api/ticker/?id=90",
    sink,
    requestHeaders).non_null();
  QPtr<iCURL> curl = _curl;
  UnitTest::TestLoop(TEST_PARAMS_CALL,
    ni::Runnable([mq,request,sink,curl,TEST_PARAMS_LAMBDA]() {
      mq->PollAndDispatch();
      if (request->GetReadyState() == eFetchReadyState_Done) {
        return eFalse;
      }
      // we also test spamming, if should return the same request always.
      // I would like to override request with the result but I am in const territory here...
      Nonnull<iFetchRequest> theRequest = curl->FetchGet(
      "https://api.coinlore.com/api/ticker/?id=90",
      sink,
      NULL).non_null();
      CHECK_EQUAL(theRequest, request);
      return eTrue;
    }),
    ni::Runnable([request,TEST_PARAMS_LAMBDA,sink]() {
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
      // cString xml = ni::DataTableToXML(dataDT);
      // niDebugFmt(("XML: %s", xml));
      CHECK_EQUAL(request->GetStatus(), 500);
      CHECK_EQUAL(dataDT->GetString("payload"), "Couldn't fetch the URL");
      CHECK_EQUAL(sink->result, "error");
      CHECK_EQUAL(eFalse, request->GetHasFailed());
      return eTrue;
    }));
}

TEST_FIXTURE(FCURLFetch,JSCC_RequestNoAwaits) {
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

  Nonnull<MyFetchSink> sink = ni::MakeNonnull<MyFetchSink>();
  Nonnull<iFetchRequest> request = _curl->FetchGet(
    "https://api.coinlore.com/api/ticker/?id=90",
    sink,
    NULL).non_null();

  QPtr<iCURL> curl = _curl;
  UnitTest::TestLoop(TEST_PARAMS_CALL,
    ni::Runnable([mq,request,sink,curl,TEST_PARAMS_LAMBDA]() {
      mq->PollAndDispatch();
      if (request->GetReadyState() == eFetchReadyState_Done) {
        return eFalse;
      }
      // try to make the same request multiple times (it should always return the
      // same request until that request is done)
      Nonnull<iFetchRequest> theRequest = curl->FetchGet(
      "https://api.coinlore.com/api/ticker/?id=90",
      sink,
      NULL).non_null();
      CHECK_EQUAL(theRequest, request);
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
      CHECK(data.StartsWith("[{\"id\":\"90\""));
      CHECK(headers.icontains("Content-Type: application/json"));
      CHECK_EQUAL(eFalse, request->GetHasFailed());
      CHECK_EQUAL(sink->result, "success");
      return eFalse;
    }));
}
#endif

}
