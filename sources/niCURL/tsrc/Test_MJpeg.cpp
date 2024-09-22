#include "stdafx.h"

#if !defined niJSCC
#define TEST_MJPEG
#include <niLang/Utils/TimerSleep.h>
#include <niUI.h>
#endif

#if defined niCPP_Lambda && defined TEST_MJPEG
using namespace ni;

// Found at https://vivre-motion.com/pages/ip-camera#public_stream
static const char* _kDefaultMjpegURL =
    "http://pendelcam.kip.uni-heidelberg.de/mjpg/video.mjpg?fps=10"
    // "http://admin:abc123@192.168.0.201:9001/video/mjpg.cgi"
    // "http://172.16.46.236:55000/image/01-CAM-139.mjpg"
    // "http://cam-mckeldin-eastview.umd.edu/axis-cgi/mjpg/video.cgi?resolution=160x120&fps=10"
    // "http://195.196.36.242/mjpg/video.mjpg?resolution=160x120"
;
static const tU32 _kMaxNumFrames = 3;
static const tU32 _kMaxNumParts = _kMaxNumFrames*10;

struct FCURLMJpeg {
  QPtr<iGraphics> _graphics;
  QPtr<iCURL> _curl;
  FCURLMJpeg() {
    _graphics = ni::New_niUI_Graphics(niVarNull,niVarNull);
    _curl = ni::New_niCURL_CURL(niVarNull,niVarNull);
  }
};

tBool __stdcall ReadMjpegPart(iGraphics* apGraphics, iFile* apJpeg, tU32 anPartId) {
  static cString _basePath = UnitTest::GetTestOutputFilePath("test_curl_mjpeg");
  cString path;
  tBool bRet = eFalse;
  tOffset headerSize = -1;
  {
    tPtr p = apJpeg->GetBase();
    // find the jpeg header
    niLoop(i,ni::Min(apJpeg->GetSize()-2,0x1FFF)) {
      if (*p == 0xFF && (*(p+1) == 0xD9 || *(p+1) == 0xD8)) {
        headerSize = p-apJpeg->GetBase();
        break;
      }
      ++p;
    }
  }
  if (headerSize < 0) {
    niDebugFmt(("... Couldn't find the jpeg header."));
  }
  else {
    apJpeg->SeekSet(headerSize);

    path = niFmt("%s_%03d.jpg", _basePath, anPartId);
    QPtr<iBitmap2D> bmp = apGraphics->LoadBitmap(apJpeg);
    if (bmp.IsOK()) {
      apGraphics->SaveBitmap(path.Chars(),bmp,25);
      bRet = eTrue;
      niDebugFmt(("... Saved jpeg stream."));
    }
    else {
      niDebugFmt(("... Couldn't load jpeg from stream."));
    }
  }

  path = niFmt("%s_%03d_dump.jpg", _basePath, anPartId);
  Ptr<iFile> fpDump = ni::GetRootFS()->FileOpen(path.Chars(),eFileOpenMode_Write);
  if (fpDump.IsOK()) {
    apJpeg->SeekSet(0);
    fpDump->WriteFile(apJpeg->GetFileBase());
  }

  return bRet;
}

TEST_FIXTURE(FCURLMJpeg,IPCam) {
  AUTO_WARNING_MODE(); // We dont control the test mjpeg stream...

  const cString url = ni::GetProperty("url_mjpeg",_kDefaultMjpegURL);

  Ptr<iMessageQueue> mq = ni::GetOrCreateMessageQueue(ni::ThreadGetCurrentThreadID());
  QPtr<iFuture> futureValue;
  int numPartReceived = 0;
  int numJpegDecoded = 0;

  Ptr<iRunnable> runnable = _curl->URLGetMultiPart(
      ni::MessageHandler([=,this,&futureValue,&numPartReceived,&numJpegDecoded] (tU32 anMsg, const Var& A, const Var& B) {
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
            case eCURLMessage_ReceivedPart: {
              niDebugFmt(("... Received Part: %d.",numPartReceived));
              ++numPartReceived;
              if (ReadMjpegPart(_graphics,VarQueryInterface<iFile>(A),numJpegDecoded)) {
                ++numJpegDecoded;
              }
              if (numJpegDecoded >= _kMaxNumFrames || numPartReceived >= _kMaxNumParts) {
                futureValue->Cancel();
              }
              break;
            }
            case eCURLMessage_Progress: {
              niDebugFmt(("... Progress %s", A));
              break;
            }
            case eCURLMessage_Completed: {
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
              niWarning(niFmt("Unexpected message '%s'.", MessageID_ToString(anMsg)));
              break;
            }
          }
        }),
      url.Chars(),
      ".jpg");

  Ptr<iFuture> thread = ni::GetConcurrent()->ThreadRun(runnable);

  while (!thread->Wait(100)) {
    mq->PollAndDispatch();
  }

  CHECK_EQUAL(_kMaxNumFrames,numJpegDecoded);
  CHECK(numPartReceived < _kMaxNumParts);
  if (futureValue.IsOK()) {
    CHECK_EQUAL(eTrue,futureValue->GetIsDone());
    CHECK_EQUAL(eTrue,futureValue->Wait(0));
    CHECK_EQUAL(eTrue,futureValue->GetValue().GetBoolValue());
  }
}
#endif
