#include "stdafx.h"
#include <niCC.h>
#include <niCURL_ModuleDef.h>
#include "Test_Utils.h"
#include <niLang/Utils/TimerSleep.h>

#ifdef niJSCC
#include <emscripten.h>
#endif

namespace {

using namespace ni;

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

struct sFCURLFetch_Base : public UnitTest::Test {
  NN_mut<iCURL> _curl = niDeferredInit(NN_mut<iCURL>);
  NN_mut<iMessageQueue> _mq = niDeferredInit(NN_mut<iMessageQueue>);
  NN<iFetchRequest> _request = niDeferredInit(NN<iFetchRequest>);

  virtual NN<iFetchRequest> CreateRequest(UnitTest::TestResults& testResults_) = 0;
  virtual void CheckResult(UnitTest::TestResults& testResults_, const cString& aHeaders, const cString& aData) = 0;

  sFCURLFetch_Base(const char* aName, const char* aFile, const int aLine)
      : Test(aName, aFile, aLine, "FCURLFetch") {
  }

  ~sFCURLFetch_Base() {
    // niDebugFmt(("... ~sFCURLFetch_Base %s", m_testName));
  }

  virtual NN_mut<iCURL> CreateCURL() {
#ifdef niJSCC
    // We remove any JSCC extension to have a clean test.
    // TODO: This shouldn't be a global module thing ideally.
    emscripten_run_script(R"""({
      if ("niCURL" in Module) {
        delete Module.niCURL;
      }
    })""");
#endif
    QPtr<iCURL> curl = ni::New_niCURL_CURL(niVarNull,niVarNull);
    return curl.non_null();
  }

  void BeforeRunImpl(UnitTest::TestResults& testResults_) niImpl {
    m_numSteps = ni::eInvalidHandle; // Run as an "infinite" loop
    _curl = this->CreateCURL();
    _mq = as_NN(ni::GetOrCreateMessageQueue(ni::ThreadGetCurrentThreadID()));
    _request = this->CreateRequest(testResults_);
  }

  void RunImpl(UnitTest::TestResults& testResults_) niImpl {
    //niDebugFmt(("... RunImpl"));
    if (!_mq->PollAndDispatch()) {
      ni::SleepMs(100); // Avoid busy loop
    }
    if (_request->GetReadyState() == eFetchReadyState_Done) {
      //niDebugFmt(("... GetReadyState() == eFetchReadyState_Done"));
      m_numSteps = 0; // We're done
    }
  }

  void AfterRunImpl(UnitTest::TestResults& testResults_) niImpl {
    cString headers = _request->GetReceivedHeaders()->ReadString();
    niDebugFmt(("... headers: %d bytes, %s",
                _request->GetReceivedHeaders()->GetSize(),
                headers));

    cString data = _request->GetReceivedData()->ReadString();
    niDebugFmt(("... data: %d bytes, %s",
                _request->GetReceivedData()->GetSize(),
                data));

    this->CheckResult(testResults_, headers, data);

    // Cleanup here, we dont want these to run as global C++ destructors
    _curl = NN_mut<iCURL>::tUnsafeUncheckedInitializer(nullptr);
    _mq = NN_mut<iMessageQueue>::tUnsafeUncheckedInitializer(nullptr);
    _request = NN<iFetchRequest>::tUnsafeUncheckedInitializer(nullptr);
  }
};

struct sFCURLFetch_Get : public sFCURLFetch_Base {
  sFCURLFetch_Get() : sFCURLFetch_Base("FCURLFetch-Get", __FILE__, __LINE__) {
  }

  virtual NN<iFetchRequest> CreateRequest(UnitTest::TestResults& testResults_) niImpl {
    NN_mut<tStringCVec> requestHeaders { tStringCVec::Create() };
    requestHeaders->push_back("X-Ni-Header: HdrNarf");

    Nonnull<MyFetchSink> sink = ni::MakeNonnull<MyFetchSink>();
    return _curl->FetchGet(
      _GetHTTPSTestCasesUrl("Test_niCURL_FetchGet.php?param=test_value").c_str(),
      sink,
      requestHeaders).non_null();
  }

  virtual void CheckResult(UnitTest::TestResults& testResults_, const cString& aHeaders, const cString& aData) niImpl {
#if !defined niJSCC
    CHECK(aHeaders.icontains("Access-Control-Allow-Origin: *"));
#endif
    CHECK_EQUAL(_ASTR("Test_niCURL_FetchGet:HdrNarf:test_value"),aData);
    CHECK_EQUAL(eFalse, _request->GetHasFailed());
  }
};
TEST_CLASS(sFCURLFetch_Get);

struct sFCURLFetch_Post : public sFCURLFetch_Base {
  sFCURLFetch_Post() : sFCURLFetch_Base("FCURLFetch-Post", __FILE__, __LINE__) {
  }

  virtual NN<iFetchRequest> CreateRequest(UnitTest::TestResults& testResults_) niImpl {
    Nonnull<tStringCVec> requestHeaders { tStringCVec::Create() };
    requestHeaders->push_back("X-Ni-Header: HdrNarf");

    Nonnull<iFile> postData {ni::CreateFileDynamicMemory(128,"postdata")};
    postData->WriteString(R"""({"param":"posted_value", "key2":"value2"})""");
    postData->SeekSet(0);

    Nonnull<MyFetchSink> sink = ni::MakeNonnull<MyFetchSink>();
    return _curl->FetchPost(
      _GetHTTPSTestCasesUrl("Test_niCURL_FetchPost.php").c_str(),
      postData,
      sink,
      requestHeaders).non_null();
  }

  virtual void CheckResult(UnitTest::TestResults& testResults_, const cString& aHeaders, const cString& aData) niImpl {
#if !defined niJSCC
    CHECK(aHeaders.icontains("Access-Control-Allow-Origin: *"));
#endif
    CHECK_EQUAL(_ASTR("Test_niCURL_FetchPost:HdrNarf:posted_value"),aData);
    CHECK_EQUAL(eFalse, _request->GetHasFailed());
  }
};
TEST_CLASS(sFCURLFetch_Post);

struct sFCURLFetch_GetJson : public sFCURLFetch_Base {
  NN_mut<MyFetchSink> _sink = niDeferredInit(NN_mut<MyFetchSink>);

  sFCURLFetch_GetJson() : sFCURLFetch_Base("FCURLFetch-GetJson", __FILE__, __LINE__) {
  }

  virtual NN<iFetchRequest> CreateRequest(UnitTest::TestResults& testResults_) niImpl {
    Nonnull<tStringCVec> requestHeaders { tStringCVec::Create() };
    requestHeaders->push_back("X-Ni-Header: HdrNarf_GetJson");

    _sink = ni::MakeNonnull<MyFetchSink>();
    return _curl->FetchGet(
      _GetHTTPSTestCasesUrl("Test_niCURL_FetchGetJson.php?param=test_json_value").c_str(),
      _sink,
      requestHeaders).non_null();
  }

  virtual void CheckResult(UnitTest::TestResults& testResults_, const cString& aHeaders, const cString& aData) niImpl {
#if !defined niJSCC
    CHECK(aHeaders.icontains("Access-Control-Allow-Origin: *"));
#endif
    CHECK(aHeaders.icontains("Content-Type: application/json"));
    CHECK_EQUAL(eFalse, _request->GetHasFailed());
    CHECK_EQUAL(_sink->result, "success");

    Ptr<iDataTable> dt = ni::CreateDataTable("Response");
    CHECK(ni::JsonParseStringToDataTable(aData,dt));
    CHECK_EQUAL(_ASTR("Test_niCURL_FetchGetJson"), dt->GetString("name"));
    CHECK_EQUAL(_ASTR("test_json_value"), dt->GetString("param"));
    CHECK_EQUAL(_ASTR("HdrNarf_GetJson"), dt->GetString("received_x_ni_header"));
  };
};
TEST_CLASS(sFCURLFetch_GetJson);

#ifdef niJSCC

struct sFCURLFetch_JSCC_OverrideNull : public sFCURLFetch_Base {
  NN_mut<MyFetchSink> _sink = niDeferredInit(NN_mut<MyFetchSink>);

  sFCURLFetch_JSCC_OverrideNull() : sFCURLFetch_Base("FCURLFetch-JSCC_OverrideNull", __FILE__, __LINE__) {
  }

  virtual NN_mut<iCURL> CreateCURL() niImpl {
    emscripten_run_script(R"""({
      niExtensions = {
        niCURL: {
          handleFetchOverride: null
        }
      }
      Object.assign(Module, niExtensions);
    })""");
    QPtr<iCURL> curl = ni::New_niCURL_CURL(niVarNull,niVarNull);
    return curl.non_null();
  }

  virtual NN<iFetchRequest> CreateRequest(UnitTest::TestResults& testResults_) niImpl {
    Nonnull<tStringCVec> requestHeaders { tStringCVec::Create() };
    requestHeaders->push_back("X-Ni-Header: HdrNarf_GetJson");

    _sink = ni::MakeNonnull<MyFetchSink>();
    return _curl->FetchGet(
      _GetHTTPSTestCasesUrl("Test_niCURL_FetchGetJson.php?param=test_json_value").c_str(),
      _sink,
      requestHeaders).non_null();
  }

  virtual void CheckResult(UnitTest::TestResults& testResults_, const cString& aHeaders, const cString& aData) niImpl {
    CHECK(aHeaders.icontains("Content-Type: application/json"));
    Ptr<iDataTable> dataDT = ni::CreateDataTable();
    const tBool validJson = JsonParseStringToDataTable(aData, dataDT);
    CHECK(validJson);
    CHECK_EQUAL(eFalse, _request->GetHasFailed());
    CHECK_EQUAL(_ASTR("Test_niCURL_FetchGetJson"), dataDT->GetString("name"));
    CHECK_EQUAL(_ASTR("HdrNarf_GetJson"), dataDT->GetString("received_x_ni_header"));
  };
};
TEST_CLASS(sFCURLFetch_JSCC_OverrideNull);

struct sFCURLFetch_JSCC_OverrideGetJsonWait1s : public sFCURLFetch_Base {
  NN_mut<MyFetchSink> _sink = niDeferredInit(NN_mut<MyFetchSink>);

  sFCURLFetch_JSCC_OverrideGetJsonWait1s() : sFCURLFetch_Base("FCURLFetch-JSCC_OverrideGetJsonWait1s", __FILE__, __LINE__) {
  }

  virtual NN_mut<iCURL> CreateCURL() niImpl {
    emscripten_run_script(R"""({
      niExtensions = {
        niCURL: {
          shouldOverrideFetch: function(url) {
            if (url.includes("/test_cases/Test_niCURL_FetchGetJson.php")) {
              return true;
            }
            else {
              return false;
            }
          },
          handleFetchOverride: async function (aRequestUrl, onSuccess, onError, onProgress) {
            console.log("OverrideGetJsonWait1s: handleFetchOverride: " + aRequestUrl);
            await new Promise(r => setTimeout(r, 1000));
            console.log("OverrideGetJsonWait1s: handleFetchOverride: result");
            var result = `{
              "headers": {
                "Content-Type": "application/json",
                "Accept": "application/json"
              },
              "payload": {
                "name": "Fetch Override"
              }
            }`
            console.log("OverrideGetJsonWait1s: handleFetchOverride: onSuccess");
            onSuccess(result);
            console.log("OverrideGetJsonWait1s: handleFetchOverride: done");
          }
        }
      }
      Object.assign(Module, niExtensions);
    })""");
    QPtr<iCURL> curl = ni::New_niCURL_CURL(niVarNull,niVarNull);
    return curl.non_null();
  }

  virtual NN<iFetchRequest> CreateRequest(UnitTest::TestResults& testResults_) niImpl {
    Nonnull<tStringCVec> requestHeaders { tStringCVec::Create() };
    requestHeaders->push_back("X-Ni-Header: HdrNarf_GetJson");

    _sink = ni::MakeNonnull<MyFetchSink>();
    return _curl->FetchGet(
      _GetHTTPSTestCasesUrl("Test_niCURL_FetchGetJson.php?param=test_json_value").c_str(),
      _sink,
      requestHeaders).non_null();
  }

  virtual void CheckResult(UnitTest::TestResults& testResults_, const cString& aHeaders, const cString& aData) niImpl {
    CHECK(aHeaders.icontains("Content-Type: application/json"));
    Ptr<iDataTable> dataDT = ni::CreateDataTable();
    const tBool validJson = JsonParseStringToDataTable(aData, dataDT);
    CHECK(validJson);
    CHECK_EQUAL(eFalse, _request->GetHasFailed());
    CHECK_EQUAL(_ASTR("Fetch Override"), dataDT->GetString("name"));
    CHECK_EQUAL(_sink->result, "success");
  };
};
TEST_CLASS(sFCURLFetch_JSCC_OverrideGetJsonWait1s);

struct sFCURLFetch_JSCC_OverrideGetJsonDirect : public sFCURLFetch_Base {
  NN_mut<MyFetchSink> _sink = niDeferredInit(NN_mut<MyFetchSink>);

  sFCURLFetch_JSCC_OverrideGetJsonDirect() : sFCURLFetch_Base("FCURLFetch-JSCC_OverrideGetJsonDirect", __FILE__, __LINE__) {
  }

  virtual NN_mut<iCURL> CreateCURL() niImpl {
    emscripten_run_script(R"""({
      niExtensions = {
        niCURL: {
          shouldOverrideFetch: function(url) {
            if (url.includes("/test_cases/Test_niCURL_FetchGetJson.php")) {
              return true;
            }
            else {
              return false;
            }
          },
          handleFetchOverride: async function (aRequestUrl, onSuccess, onError, onProgress) {
            console.log("OverrideGetJsonDirect: handleFetchOverride: " + aRequestUrl);
            console.log("OverrideGetJsonDirect: handleFetchOverride: direct result");
            var result = `{
              "headers": {
                "Content-Type": "application/json",
                "Accept": "application/json"
              },
              "payload": {
                "name": "Fetch Override"
              }
            }`
            console.log("OverrideGetJsonDirect: handleFetchOverride: onSuccess");
            onSuccess(result);
            console.log("OverrideGetJsonDirect: handleFetchOverride: done");
          }
        }
      }
      Object.assign(Module, niExtensions);
    })""");
    QPtr<iCURL> curl = ni::New_niCURL_CURL(niVarNull,niVarNull);
    return curl.non_null();
  }

  virtual NN<iFetchRequest> CreateRequest(UnitTest::TestResults& testResults_) niImpl {
    Nonnull<tStringCVec> requestHeaders { tStringCVec::Create() };
    requestHeaders->push_back("X-Ni-Header: HdrNarf_GetJson");

    _sink = ni::MakeNonnull<MyFetchSink>();
    return _curl->FetchGet(
      _GetHTTPSTestCasesUrl("Test_niCURL_FetchGetJson.php?param=test_json_value").c_str(),
      _sink,
      requestHeaders).non_null();
  }

  virtual void CheckResult(UnitTest::TestResults& testResults_, const cString& aHeaders, const cString& aData) niImpl {
    CHECK(aHeaders.icontains("Content-Type: application/json"));
    Ptr<iDataTable> dataDT = ni::CreateDataTable();
    const tBool validJson = JsonParseStringToDataTable(aData, dataDT);
    CHECK(validJson);
    CHECK_EQUAL(eFalse, _request->GetHasFailed());
    CHECK_EQUAL(_ASTR("Fetch Override"), dataDT->GetString("name"));
    CHECK_EQUAL(_sink->result, "success");
  };
};
TEST_CLASS(sFCURLFetch_JSCC_OverrideGetJsonDirect);

struct sFCURLFetch_JSCC_OverrideSkip : public sFCURLFetch_Base {
  NN_mut<MyFetchSink> _sink = niDeferredInit(NN_mut<MyFetchSink>);

  sFCURLFetch_JSCC_OverrideSkip() : sFCURLFetch_Base("FCURLFetch-JSCC_OverrideSkip", __FILE__, __LINE__) {
  }

  virtual NN_mut<iCURL> CreateCURL() niImpl {
    emscripten_run_script(R"""({
      niExtensions = {
        niCURL: {
          shouldOverrideFetch: function(url) {
            return false;
          },
          handleFetchOverride: async function (aRequestUrl, onSuccess, onError, onProgress) {
            throw new Error("OverrideSkip shouldn't call handleFetchOverride.");
            return false;
          }
        }
      }

      Object.assign(Module, niExtensions);
    })""");

    QPtr<iCURL> curl = ni::New_niCURL_CURL(niVarNull,niVarNull);
    return curl.non_null();
  }

  virtual NN<iFetchRequest> CreateRequest(UnitTest::TestResults& testResults_) niImpl {
    Nonnull<tStringCVec> requestHeaders { tStringCVec::Create() };
    requestHeaders->push_back("X-Ni-Header: HdrNarf_GetJson");

    _sink = ni::MakeNonnull<MyFetchSink>();
    return _curl->FetchGet(
      _GetHTTPSTestCasesUrl("Test_niCURL_FetchGetJson.php?param=test_json_value").c_str(),
      _sink,
      requestHeaders).non_null();
  }

  virtual void CheckResult(UnitTest::TestResults& testResults_, const cString& aHeaders, const cString& aData) niImpl {
    CHECK(aHeaders.icontains("Content-Type: application/json"));
    Ptr<iDataTable> dataDT = ni::CreateDataTable();
    const tBool validJson = JsonParseStringToDataTable(aData, dataDT);
    CHECK(validJson);
    CHECK_EQUAL(eFalse, _request->GetHasFailed());
    CHECK_EQUAL(_ASTR("Test_niCURL_FetchGetJson"), dataDT->GetString("name"));
    CHECK_EQUAL(_sink->result, "success");
  };
};
TEST_CLASS(sFCURLFetch_JSCC_OverrideSkip);

struct sFCURLFetch_JSCC_OverrideError : public sFCURLFetch_Base {
  NN_mut<MyFetchSink> _sink = niDeferredInit(NN_mut<MyFetchSink>);

  sFCURLFetch_JSCC_OverrideError() : sFCURLFetch_Base("FCURLFetch-JSCC_OverrideError", __FILE__, __LINE__) {
  }

  virtual NN_mut<iCURL> CreateCURL() niImpl {
    emscripten_run_script(R"""({
      niExtensions = {
        niCURL: {
          shouldOverrideFetch: function(url) {
            if (url.includes("/test_cases/Test_niCURL_FetchGetJson.php")) {
              return true;
            }
            else {
              return false;
            }
          },
          handleFetchOverride: async function (aRequestUrl, onSuccess, onError, onProgress) {
            console.log("OverrideError: handleFetchOverride: " + aRequestUrl);
            await new Promise(r => setTimeout(r, 1000));
            console.log("OverrideError: handleFetchOverride: request");
            console.log("OverrideError: handleFetchOverride: onError1");
            onError(`{
              "payload": "Couldn't fetch the URL"
            }`);
            console.log("OverrideError: handleFetchOverride: onError2");
            onError("HOLA");
            console.log("OverrideError: handleFetchOverride: done");
          }
        }
      }

      Object.assign(Module, niExtensions);
    })""");

    QPtr<iCURL> curl = ni::New_niCURL_CURL(niVarNull,niVarNull);
    return curl.non_null();
  }

  virtual NN<iFetchRequest> CreateRequest(UnitTest::TestResults& testResults_) niImpl {
    Nonnull<tStringCVec> requestHeaders { tStringCVec::Create() };
    requestHeaders->push_back("X-Ni-Header: HdrNarf_GetJson");

    _sink = ni::MakeNonnull<MyFetchSink>();
    return _curl->FetchGet(
      _GetHTTPSTestCasesUrl("Test_niCURL_FetchGetJson.php?param=test_json_value").c_str(),
      _sink,
      requestHeaders).non_null();
  }

  virtual void CheckResult(UnitTest::TestResults& testResults_, const cString& aHeaders, const cString& aData) niImpl {
    CHECK(aHeaders.icontains("Content-Type: application/json"));
    Ptr<iDataTable> dataDT = ni::CreateDataTable();
    const tBool validJson = JsonParseStringToDataTable(aData, dataDT);
    CHECK(validJson);
    CHECK_EQUAL(eFalse, _request->GetHasFailed());
    CHECK_EQUAL(_request->GetStatus(), 500);
    CHECK_EQUAL(dataDT->GetString("payload"), "Couldn't fetch the URL");
    CHECK_EQUAL(_sink->result, "error");
  };
};
TEST_CLASS(sFCURLFetch_JSCC_OverrideError);

#endif // #ifdef niJSCC

}
