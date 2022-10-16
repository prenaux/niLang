#include "stdafx.h"
#include <niCURL_ModuleDef.h>

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
  virtual void __stdcall OnFetchSink_ReadyStateChanged(iFetchRequest* apFetch) {
    niDebugFmt(("... OnFetchSink_ReadyStateChanged: %s",
                niEnumToChars(eFetchReadyState,apFetch->GetReadyState())));
  }
};

TEST_FIXTURE(FCURLFetch,Get) {
  Ptr<iMessageQueue> mq = ni::GetOrCreateMessageQueue(ni::ThreadGetCurrentThreadID());

  Nonnull<tStringCVec> requestHeaders { tStringCVec::Create() };
  requestHeaders->push_back("X-Ni-Header: HdrNarf");

  Nonnull<MyFetchSink> sink = ni::NewNonnull<MyFetchSink>();
  Nonnull<iFetchRequest> request = _curl->FetchGet(
    "https://www.bytecollider.com/test_cases/Test_niCURL_FetchGet.php?param=test_value",
    sink,
    requestHeaders).non_null();

  // Dispatch the messages...
  while (1) {
    mq->PollAndDispatch();
    if (request->GetReadyState() == eFetchReadyState_Done)
      break;
  }

  cString headers = request->GetReceivedHeaders()->ReadString();
  niDebugFmt(("... headers: %d bytes, %s",
              request->GetReceivedHeaders()->GetSize(),
              headers));

  cString data = request->GetReceivedData()->ReadString();
  niDebugFmt(("... data: %d bytes, %s",
              request->GetReceivedData()->GetSize(),
              data));

  CHECK_EQUAL(_ASTR("Test_niCURL_FetchGet:HdrNarf:test_value"),data);
  CHECK_EQUAL(eFalse, request->GetHasFailed());
}

TEST_FIXTURE(FCURLFetch,Post) {
  Ptr<iMessageQueue> mq = ni::GetOrCreateMessageQueue(ni::ThreadGetCurrentThreadID());

  Nonnull<tStringCVec> requestHeaders { tStringCVec::Create() };
  requestHeaders->push_back("X-Ni-Header: HdrNarf");

  Nonnull<iFile> postData {ni::CreateFileDynamicMemory(128,"postdata")};
  postData->WriteString(R"""({"param":"posted_value", "key2":"value2"})""");
  postData->SeekSet(0);

  Nonnull<MyFetchSink> sink = ni::NewNonnull<MyFetchSink>();
  Nonnull<iFetchRequest> request = _curl->FetchPost(
    "https://www.bytecollider.com/test_cases/Test_niCURL_FetchPost.php",
    postData,
    sink,
    requestHeaders).non_null();

  // Dispatch the messages...
  while (1) {
    mq->PollAndDispatch();
    if (request->GetReadyState() == eFetchReadyState_Done)
      break;
  }

  cString headers = request->GetReceivedHeaders()->ReadString();
  niDebugFmt(("... headers: %d bytes, %s",
              request->GetReceivedHeaders()->GetSize(),
              headers));

  cString data = request->GetReceivedData()->ReadString();
  niDebugFmt(("... data: %d bytes, %s",
              request->GetReceivedData()->GetSize(),
              data));

  CHECK_EQUAL(_ASTR("Test_niCURL_FetchPost:HdrNarf:posted_value"),data);
  CHECK_EQUAL(eFalse, request->GetHasFailed());
}

TEST_FIXTURE(FCURLFetch,GetJson) {
  Ptr<iMessageQueue> mq = ni::GetOrCreateMessageQueue(ni::ThreadGetCurrentThreadID());

  Nonnull<MyFetchSink> sink = ni::NewNonnull<MyFetchSink>();
  Nonnull<iFetchRequest> request = _curl->FetchGet(
    "https://api.coinlore.com/api/ticker/?id=90",
    sink,
    NULL).non_null();

  // Dispatch the messages...
  while (1) {
    mq->PollAndDispatch();
    if (request->GetReadyState() == eFetchReadyState_Done)
      break;
  }

  cString headers = request->GetReceivedHeaders()->ReadString();
  niDebugFmt(("... headers: %d bytes, %s",
              request->GetReceivedHeaders()->GetSize(),
              headers));

  cString data = request->GetReceivedData()->ReadString();
  niDebugFmt(("... data: %d bytes, %s",
              request->GetReceivedData()->GetSize(),
              data));

  CHECK(data.StartsWith("[{\"id\":\"90\""));
  CHECK(headers.contains("Content-Type: application/json"));
  CHECK_EQUAL(eFalse, request->GetHasFailed());
}

}
