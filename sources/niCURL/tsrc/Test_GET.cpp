#include "stdafx.h"
#include "Test_Utils.h"
#include <niLang/Utils/TimerSleep.h>

using namespace ni;

struct FCURLGet {
  QPtr<iCURL> _curl;
  FCURLGet() {
    _curl = ni::New_niCURL_CURL(niVarNull, niVarNull);
  }
};

///////////////////////////////////////////////
TEST_FIXTURE(FCURLGet, HttpAuthBasic) {
  Ptr<iFile> recvData = ni::CreateFileDynamicMemory(0, "");
  Ptr<iFile> recvHeader = ni::CreateFileDynamicMemory(0, "");
  QPtr<iFuture> futureValue;
  tBool bCompleted = eFalse;

  _curl->SetHttpAuth(ni::eCURLHttpAuth_Basic);
  _curl->SetUserName("admin");
  _curl->SetUserPass("admin12345");

  Ptr<iRunnable> runnable = _curl->URLGet(
    ni::MessageHandler([=, &bCompleted, &futureValue](
                         tU32 anMsg, const Var& A, const Var& B) {
      futureValue = B;
      switch (anMsg) {
        case eCURLMessage_Started: {
          niDebugFmt(("... Started"));
          break;
        }
        case eCURLMessage_ReceivingHeader: {
          niDebugFmt(("... Receiving Header"));
          break;
        }
        case eCURLMessage_ReceivingData: {
          niDebugFmt(("... Receiving Data"));
          break;
        }
        case eCURLMessage_Progress: {
          niDebugFmt(("... Progress %s", A));
          break;
        }
        case eCURLMessage_Completed: {
          bCompleted = eTrue;
          niDebugFmt(("... Completed"));
          break;
        }
        case eCURLMessage_ResponseCode: {
          niDebugFmt(("... ResponseCode: %d", A));
          break;
        }
        case eCURLMessage_Failed: {
          niDebugFmt(("... Failed: %s", A));
          break;
        }
        default: {
          niError(niFmt("Unexpected message '%s'.", MessageID_ToString(anMsg)));
          break;
        }
      }
    }),
    _GetHTTPSTestCasesUrl("Test_niCURL_basic_auth.php").c_str(),
    recvData,
    recvHeader);
  CHECK_RETURN_IF_FAILED(runnable.IsOK());
  runnable->Run();

  recvHeader->SeekSet(0);
  niDebugFmt(
    ("I/Page Header: Received %d bytes\n%s",
     recvHeader->GetSize(),
     recvHeader->ReadString()));

  recvData->SeekSet(0);
  cString result = recvData->ReadString();
  niDebugFmt(
    ("I/Page Data: Received %d bytes\n%s", recvData->GetSize(), result));

  CHECK_EQUAL(eTrue, bCompleted);
  CHECK_EQUAL(eTrue, futureValue->GetIsDone());
  CHECK_EQUAL(eTrue, futureValue->Wait(0));
  CHECK_EQUAL(eTrue, futureValue->GetValue().GetBoolValue());

  CHECK_EQUAL(cString("login_ok"), result);
}

///////////////////////////////////////////////
TEST_FIXTURE(FCURLGet, HttpAuthBasicIncorrectPwd) {

  Ptr<iFile> recvData = ni::CreateFileDynamicMemory(0, "");
  Ptr<iFile> recvHeader = ni::CreateFileDynamicMemory(0, "");
  QPtr<iFuture> futureValue;
  tBool bCompleted = eFalse;

  _curl->SetHttpAuth(ni::eCURLHttpAuth_Basic);
  _curl->SetUserName(NULL);
  _curl->SetUserPass(NULL);

  Ptr<iRunnable> runnable = _curl->URLGet(
    ni::MessageHandler([=, &bCompleted, &futureValue](
                         tU32 anMsg, const Var& A, const Var& B) {
      futureValue = B;
      switch (anMsg) {
        case eCURLMessage_Started: {
          niDebugFmt(("... Started"));
          break;
        }
        case eCURLMessage_ReceivingHeader: {
          niDebugFmt(("... Receiving Header"));
          break;
        }
        case eCURLMessage_ReceivingData: {
          niDebugFmt(("... Receiving Data"));
          break;
        }
        case eCURLMessage_Progress: {
          niDebugFmt(("... Progress %s", A));
          break;
        }
        case eCURLMessage_Completed: {
          bCompleted = eTrue;
          niDebugFmt(("... Completed"));
          break;
        }
        case eCURLMessage_ResponseCode: {
          niDebugFmt(("... ResponseCode: %d", A));
          break;
        }
        case eCURLMessage_Failed: {
          niDebugFmt(("... Failed: %s", A));
          break;
        }
        default: {
          niError(niFmt("Unexpected message '%s'.", MessageID_ToString(anMsg)));
          break;
        }
      }
    }),
    _GetHTTPSTestCasesUrl("Test_niCURL_basic_auth.php").c_str(),
    recvData,
    recvHeader);
  CHECK_RETURN_IF_FAILED(runnable.IsOK());
  runnable->Run();

  recvHeader->SeekSet(0);
  niDebugFmt(
    ("I/Page Header: Received %d bytes\n%s",
     recvHeader->GetSize(),
     recvHeader->ReadString()));

  recvData->SeekSet(0);
  cString result = recvData->ReadString();
  niDebugFmt(
    ("I/Page Data: Received %d bytes\n%s", recvData->GetSize(), result));

  CHECK_EQUAL(eTrue, bCompleted);
  CHECK_EQUAL(eTrue, futureValue->GetIsDone());
  CHECK_EQUAL(eTrue, futureValue->Wait(0));
  CHECK_EQUAL(eTrue, futureValue->GetValue().GetBoolValue());

  CHECK_EQUAL(cString("login_failed"), result);
}

///////////////////////////////////////////////
TEST_FIXTURE(FCURLGet, GetPage) {
  Ptr<iFile> recvData = ni::CreateFileDynamicMemory(0, "");
  Ptr<iFile> recvHeader = ni::CreateFileDynamicMemory(0, "");
  QPtr<iFuture> futureValue;
  tBool bCompleted = eFalse;
  Ptr<iRunnable> runnable = _curl->URLGet(
    ni::MessageHandler([=, &bCompleted, &futureValue](
                         tU32 anMsg, const Var& A, const Var& B) {
      futureValue = B;
      switch (anMsg) {
        case eCURLMessage_Started: {
          niDebugFmt(("... Started"));
          break;
        }
        case eCURLMessage_ReceivingHeader: {
          niDebugFmt(("... Receiving Header"));
          break;
        }
        case eCURLMessage_ReceivingData: {
          niDebugFmt(("... Receiving Data"));
          break;
        }
        case eCURLMessage_Progress: {
          niDebugFmt(("... Progress %s", A));
          break;
        }
        case eCURLMessage_Completed: {
          bCompleted = eTrue;
          niDebugFmt(("... Completed"));
          break;
        }
        case eCURLMessage_ResponseCode: {
          niDebugFmt(("... ResponseCode: %d", A));
          break;
        }
        case eCURLMessage_Failed: {
          niDebugFmt(("... Failed: %s", A));
          break;
        }
        default: {
          niError(niFmt("Unexpected message '%s'.", MessageID_ToString(anMsg)));
          break;
        }
      }
    }),
    _GetHTTPSTestCasesUrl("index.php").c_str(),
    recvData,
    recvHeader);
  CHECK_RETURN_IF_FAILED(runnable.IsOK());
  runnable->Run();

  recvHeader->SeekSet(0);
  niDebugFmt(
    ("I/Page Header: Received %d bytes\n%s",
     recvHeader->GetSize(),
     recvHeader->ReadString()));

  recvData->SeekSet(0);
  ni::cString content = recvData->ReadString();
  niDebugFmt(
    ("I/Page Data: Received %d bytes\n%s", recvData->GetSize(), content));

  CHECK_EQUAL(eTrue, bCompleted);
  CHECK_EQUAL(eTrue, futureValue->GetIsDone());
  CHECK_EQUAL(eTrue, futureValue->Wait(0));
  CHECK_EQUAL(eTrue, futureValue->GetValue().GetBoolValue());
  CHECK_EQUAL(eTrue, content.contains("<h1>ByteCollider</h1>"));
  CHECK_EQUAL(eTrue, content.contains("<h2>Test Cases</h2>"));
}

///////////////////////////////////////////////
TEST_FIXTURE(FCURLGet, GetPageAsync) {

  Ptr<iMessageQueue> mq =
    ni::GetOrCreateMessageQueue(ni::ThreadGetCurrentThreadID());
  Ptr<iFile> recvData = ni::CreateFileDynamicMemory(0, "");
  Ptr<iFile> recvHeader = ni::CreateFileDynamicMemory(0, "");
  QPtr<iFuture> futureValue;
  tBool bCompleted = eFalse;
  Ptr<iRunnable> runnable = _curl->URLGet(
    ni::MessageHandler(
      [=, &bCompleted, &futureValue](tU32 anMsg, const Var& A, const Var& B) {
        futureValue = B;
        switch (anMsg) {
          case eCURLMessage_Started: {
            niDebugFmt(("... Started"));
            break;
          }
          case eCURLMessage_ReceivingHeader: {
            niDebugFmt(("... Receiving Header"));
            break;
          }
          case eCURLMessage_ReceivingData: {
            niDebugFmt(("... Receiving Data"));
            break;
          }
          case eCURLMessage_Progress: {
            niDebugFmt(("... Progress %s", A));
            break;
          }
          case eCURLMessage_Completed: {
            bCompleted = eTrue;
            niDebugFmt(("... Completed"));
            break;
          }
          case eCURLMessage_ResponseCode: {
            niDebugFmt(("... ResponseCode: %d", A));
            break;
          }
          case eCURLMessage_Failed: {
            niDebugFmt(("... Failed: %s", A));
            break;
          }
          default: {
            niWarning(
              niFmt("Unexpected message '%s'.", MessageID_ToString(anMsg)));
            break;
          }
        }
      }),
    _GetHTTPSTestCasesUrl("index.php").c_str(),
    recvData,
    recvHeader);
  CHECK_RETURN_IF_FAILED(runnable.IsOK());

  Ptr<iFuture> thread = ni::GetConcurrent()->ThreadRun(runnable);

  // Wait for the request to complete...
  thread->Wait(eInvalidHandle);

  // Dispatch the messages...
  while (mq->PollAndDispatch()) {
  }

  recvHeader->SeekSet(0);
  niDebugFmt(
    ("I/Async Page Header: Received %d bytes\n%s",
     recvHeader->GetSize(),
     recvHeader->ReadString()));

  recvData->SeekSet(0);
  ni::cString content = recvData->ReadString();
  niDebugFmt(
    ("I/Page Data: Received %d bytes\n%s", recvData->GetSize(), content));

  CHECK_EQUAL(eTrue, bCompleted);
  CHECK_EQUAL(eTrue, futureValue->GetIsDone());
  CHECK_EQUAL(eTrue, futureValue->Wait(0));
  CHECK_EQUAL(eTrue, futureValue->GetValue().GetBoolValue());

  CHECK_EQUAL(eTrue, content.contains("<h1>ByteCollider</h1>"));
  CHECK_EQUAL(eTrue, content.contains("<h2>Test Cases</h2>"));
}

///////////////////////////////////////////////
TEST_FIXTURE(FCURLGet, GetFileLocal) {

  niDebugFmt(("... A: %x", ni::ThreadGetCurrentThreadID()));
  Ptr<iMessageQueue> mq =
    ni::GetOrCreateMessageQueue(ni::ThreadGetCurrentThreadID());
  niDebugFmt(("... B"));
  Ptr<iFile> recvData = ni::CreateFileDynamicMemory(0, "");
  niDebugFmt(("... C"));
  Ptr<iFile> recvHeader = ni::CreateFileDynamicMemory(0, "");
  QPtr<iFuture> futureValue;
  tBool bCompleted = eFalse;

  niDebugFmt(("... D"));
  cPath testPath = ni::GetToolkitDir("niLang", "data");
  testPath.RemoveDirectoryBack();
  niDebugFmt(("... E"));
  testPath.AddDirectoryBack("data/niUI");
  niDebugFmt(("... F"));
  testPath.SetFile("loading.tga");
  testPath.SetProtocol("file");
  niDebugFmt(("... TESTPATH: %s", testPath.GetPath()));

  Ptr<iRunnable> runnable = _curl->URLGet(
    ni::MessageHandler(
      [=, &bCompleted, &futureValue](tU32 anMsg, const Var& A, const Var& B) {
        futureValue = B;
        switch (anMsg) {
          case eCURLMessage_Started: {
            niDebugFmt(("... Started"));
            break;
          }
          case eCURLMessage_ReceivingHeader: {
            niDebugFmt(("... Receiving Header"));
            break;
          }
          case eCURLMessage_ReceivingData: {
            niDebugFmt(("... Receiving Data"));
            break;
          }
          case eCURLMessage_Progress: {
            niDebugFmt(("... Progress %s", A));
            break;
          }
          case eCURLMessage_Completed: {
            bCompleted = eTrue;
            niDebugFmt(("... Completed"));
            break;
          }
          case eCURLMessage_ResponseCode: {
            niDebugFmt(("... ResponseCode: %d", A));
            break;
          }
          case eCURLMessage_Failed: {
            niDebugFmt(("... Failed: %s", A));
            break;
          }
          default: {
            niWarning(
              niFmt("Unexpected message '%s'.", MessageID_ToString(anMsg)));
            break;
          }
        }
      }),
    testPath.GetPath().Chars(),
    recvData,
    recvHeader);
  CHECK_RETURN_IF_FAILED(runnable.IsOK());

  Ptr<iFuture> thread = ni::GetConcurrent()->ThreadRun(runnable);

  // Wait for the request to complete...
  thread->Wait(eInvalidHandle);

  // Dispatch the messages...
  while (mq->PollAndDispatch()) {
  }

  recvHeader->SeekSet(0);
  niDebugFmt(
    ("I/Async File Header: Received %d bytes\n%s",
     recvHeader->GetSize(),
     recvHeader->ReadString()));

  recvData->SeekSet(0);
  niDebugFmt(("I/Async File Data: Received %d bytes", recvData->GetSize()));

  CHECK_EQUAL(eTrue, bCompleted);
  CHECK_EQUAL(eTrue, futureValue->GetIsDone());
  CHECK_EQUAL(eTrue, futureValue->Wait(0));
  CHECK_EQUAL(eTrue, futureValue->GetValue().GetBoolValue());
}

///////////////////////////////////////////////
TEST_FIXTURE(FCURLGet, GetFileHTTPS) {

  Ptr<iMessageQueue> mq =
    ni::GetOrCreateMessageQueue(ni::ThreadGetCurrentThreadID());
  Ptr<iFile> recvData = ni::CreateFileDynamicMemory(0, "");
  Ptr<iFile> recvHeader = ni::CreateFileDynamicMemory(0, "");
  QPtr<iFuture> futureValue;
  tBool bCompleted = eFalse;

  cString testURL = _GetHTTPSTestCasesUrl("AnimMixer_Basic.vpk");
  niDebugFmt(("... TESTPATH: %s", testURL));

  Ptr<iRunnable> runnable = _curl->URLGet(
    ni::MessageHandler(
      [=, &bCompleted, &futureValue](tU32 anMsg, const Var& A, const Var& B) {
        futureValue = B;
        switch (anMsg) {
          case eCURLMessage_Started: {
            niDebugFmt(("... Started"));
            break;
          }
          case eCURLMessage_ReceivingHeader: {
            niDebugFmt(("... Receiving Header"));
            break;
          }
          case eCURLMessage_ReceivingData: {
            niDebugFmt(("... Receiving Data"));
            break;
          }
          case eCURLMessage_Progress: {
            niDebugFmt(("... Progress %s", A));
            break;
          }
          case eCURLMessage_Completed: {
            bCompleted = eTrue;
            niDebugFmt(("... Completed"));
            break;
          }
          case eCURLMessage_ResponseCode: {
            niDebugFmt(("... ResponseCode: %d", A));
            break;
          }
          case eCURLMessage_Failed: {
            niDebugFmt(("... Failed: %s", A));
            break;
          }
          default: {
            niWarning(
              niFmt("Unexpected message '%s'.", MessageID_ToString(anMsg)));
            break;
          }
        }
      }),
    testURL.Chars(),
    recvData,
    recvHeader);
  CHECK_RETURN_IF_FAILED(runnable.IsOK());

  Ptr<iFuture> thread = ni::GetConcurrent()->ThreadRun(runnable);

  // Wait for the request to complete...
  thread->Wait(eInvalidHandle);

  // Dispatch the messages...
  while (mq->PollAndDispatch()) {
    ni::SleepMs(100);
  }

  recvHeader->SeekSet(0);
  niDebugFmt(
    ("I/Async File Header: Received %d bytes\n%s",
     recvHeader->GetSize(),
     recvHeader->ReadString()));

  recvData->SeekSet(0);
  niDebugFmt(("I/Async File Data: Received %d bytes", recvData->GetSize()));
  CHECK_EQUAL(730848, recvData->GetSize());

  CHECK_EQUAL(eTrue, bCompleted);
  CHECK_EQUAL(eTrue, futureValue->GetIsDone());
  CHECK_EQUAL(eTrue, futureValue->Wait(0));
  CHECK_EQUAL(eTrue, futureValue->GetValue().GetBoolValue());
}

///////////////////////////////////////////////
TEST_FIXTURE(FCURLGet, ChunkedTransfer) {

  //_curl->SetTransferMode(eCURLTransferMode_Streaming);
  //CHECK_EQUAL(eCURLTransferMode_Streaming, _curl->GetTransferMode());

  Ptr<iMessageQueue> mq =
    ni::GetOrCreateMessageQueue(ni::ThreadGetCurrentThreadID());
  Ptr<iFile> recvData = ni::CreateFileDynamicMemory(0, "");
  Ptr<iFile> recvHeader = ni::CreateFileDynamicMemory(0, "");

  QPtr<iFuture> futureValue;
  tBool bCompleted = eFalse;
  tU32 numChunks = 0;
  tU32 lastDataSize = 0;

  NN<tStringCVec> requestHeaders{tStringCVec::Create()};
  requestHeaders->push_back("Accept: text/event-stream");

  Ptr<iRunnable> runnable = _curl->URLGet(
    ni::MessageHandler(
      [=, &bCompleted, &futureValue, &numChunks, &lastDataSize](
        tU32 anMsg, const Var& A, const Var& B) {
        futureValue = B;
        switch (anMsg) {
          case eCURLMessage_Started: {
            niDebugFmt(("... Started"));
            break;
          }
          case eCURLMessage_ReceivingHeader: {
            niDebugFmt(("... Receiving Header"));
            break;
          }
          case eCURLMessage_ReceivingData: {
            niDebugFmt(("... Receiving Data"));
            break;
          }
          case eCURLMessage_Progress: {
            // Get the latest chunk data
            tU32 currentSize = recvData->GetSize();
            if (currentSize > lastDataSize) {
              recvData->SeekSet(lastDataSize);

              niLet chunkSize = currentSize - lastDataSize;
              cString chunkData;
              chunkData.resize(chunkSize);
              recvData->ReadRaw(chunkData.data(), chunkSize);

              niDebugFmt(
                ("... Received chunk #%d: %d bytes - %s",
                 ++numChunks,
                 currentSize - lastDataSize,
                 chunkData));

              lastDataSize = currentSize;
            }
            break;
          }
          case eCURLMessage_Completed: {
            bCompleted = eTrue;
            niDebugFmt(("... Completed"));
            break;
          }
          case eCURLMessage_ResponseCode: {
            niDebugFmt(("... ResponseCode: %d", A));
            break;
          }
          case eCURLMessage_Failed: {
            niDebugFmt(("... Failed: %s", A));
            break;
          }
          default: {
            niWarning(
              niFmt("Unexpected message '%s'.", MessageID_ToString(anMsg)));
            break;
          }
        }
      }),
    _GetHTTPSTestCasesUrl("Test_niCURL_Chunked.php?case=basic").c_str(),
    //_GetLocalTestCasesUrl("Test_niCURL_Chunked.php?case=basic").c_str(),
    recvData,
    recvHeader,
    requestHeaders);

  CHECK_RETURN_IF_FAILED(runnable.IsOK());
  runnable->Run();

  // Dispatch the messages until complete
  while (!bCompleted) {
    if (!mq->PollAndDispatch()) {
      ni::SleepMs(100);
    }
  }

  recvHeader->SeekSet(0);
  cString headers = recvHeader->ReadString();
  niDebugFmt(("... Headers: %d bytes\n%s", recvHeader->GetSize(), headers));

  recvData->SeekSet(0);
  cString data = recvData->ReadString();
  niDebugFmt(("... Total data: %d bytes\n%s", recvData->GetSize(), data));

  CHECK_EQUAL(eTrue, bCompleted);
  CHECK_EQUAL(eTrue, futureValue->GetIsDone());
  CHECK_EQUAL(eTrue, futureValue->Wait(0));
  CHECK_EQUAL(eTrue, futureValue->GetValue().GetBoolValue());

  CHECK(headers.icontains("Content-Type: text/event-stream"));
  CHECK(data.icontains(
    "data: {\"type\":\"message_delta\",\"delta\":{\"text\":\"Hello\"}"));
  CHECK(data.icontains("data: [DONE]"));

  niDebugFmt(("... numChunks: %d", numChunks));
  CHECK_EQUAL(4, numChunks);
}
