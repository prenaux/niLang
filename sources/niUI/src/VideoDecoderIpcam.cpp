// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"
#include "VideoUtils.h"

#if NI_VIDEO_USE_IPCAM
#include <niCURL.h>
#include <niLang/Utils/TimerSleep.h>
#include <niLang/Utils/ConcurrentImpl.h>

#include "VideoDecoderTexture.h"

#define IPCAM_ASYNC_INIT
#define IPCAM_PIXEL_FORMAT "R8G8B8A8"

#define IPCAM_TRACE(aFmt) //niDebugFmt(aFmt)

/*
  Example .ipcam file:
    <ipcam url=""http://admin:abc123@192.168.0.201:9001/video/mjpg.cgi" />

  Properties:
    width = 640
    height = 480
    fps = 10
    status_square_size = 6
    connection_timeout = 3
    request_timeout = 0

  Status square colors:
    Created = white (1,1,1)
    Stopped = red (1,0,0)
    Connecting = yellow (1,1,0)
    Reconnecting = orange (1,0.5,0.5)
    Decoding = green (0,1,0)
    Unknown = pink (1,0,1)
*/
struct cVideoDecoderIpcam : public ni::cIUnknownImpl<ni::iVideoDecoder,ni::eIUnknownImplFlags_Default>
{
  niBeginClass(cVideoDecoderIpcam);

  enum eIpcamStatus {
    eIpcamStatus_Created = 0,
    eIpcamStatus_Stopped,
    eIpcamStatus_Reconnecting,
    eIpcamStatus_Connecting,
    eIpcamStatus_Decoding
  };

  ///////////////////////////////////////////////
  cVideoDecoderIpcam(iGraphics* apGraphics, iHString* ahspName, iFile* apFile, tVideoDecoderFlags aFlags)
      : mFlags(aFlags)
      , mptrGraphics(apGraphics)
      , mbUpdateOnBind(eTrue)
  {
    niCheckIsOK(apFile,;);
    mpTargetTexture = NULL;

    ni::Ptr<ni::iDataTable> dt = ni::GetLang()->CreateDataTable("ipcam");
    if (!ni::GetLang()->SerializeDataTable("xml", ni::eSerializeMode_Read, dt, apFile)) {
      niError("Can't read ipcam datatable.");
      return;
    }

    mptrCURL = (iCURL*)niCreateInstance(niCURL,CURL,niVarNull,niVarNull);
    if (!mptrCURL.IsOK()) {
      niError("Can't create the CURL instance.");
      return;
    }

    mnWidth = dt->GetIntDefault("width",640);
    mnHeight = dt->GetIntDefault("height",480);
    mnFPS = dt->GetIntDefault("fps",10);
    mnStatusSquareSize = dt->GetIntDefault("status_square_size",6);

    mptrCURL->SetConnectionTimeoutInSecs(
        dt->GetIntDefault("connection_timeout",3));
    mptrCURL->SetRequestTimeoutInSecs(
        dt->GetIntDefault("request_timeout",0));

    mptrTargetBitmap = mptrGraphics->CreateBitmap2D(mnWidth,mnHeight, IPCAM_PIXEL_FORMAT);
    if (!mptrTargetBitmap.IsOK()) {
      niError(niFmt("Can't create target bitmap for ipcam '%s'.",
                    apFile->GetSourcePath()));
      return;
    }
    mptrTargetBitmap->Clear();
    mbTargetBitmapDirty = eTrue;

    if (niFlagIs(mFlags,eVideoDecoderFlags_TargetTexture)) {
      Ptr<iTexture> ptrTex = mptrGraphics->CreateTexture(
          NULL,
          eBitmapType_2D,
          mptrTargetBitmap->GetPixelFormat()->GetFormat(),
          0,
          mnWidth, mnHeight, 0,
          eTextureFlags_Dynamic|eTextureFlags_Overlay);
      if (!ptrTex.IsOK()) {
        niError(niFmt("Can't create target texture for ipcam  ipcam '%s'.",
                      apFile->GetSourcePath()));
        return;
      }

      mpTargetTexture = niNew sVideoDecoderTexture(apGraphics,ahspName,this,ptrTex);
    }

    mstrURL = dt->GetStringDefault("url","");
    if (mstrURL.empty()) {
      niError("No source URL specified.");
      return;
    }

    niLog(Info,niFmt("Initialized ipcam: %s, width: %d, height: %d.",
                     mstrURL, mnWidth, mnHeight));
  }

  ///////////////////////////////////////////////
  ~cVideoDecoderIpcam() {
    IPCAM_TRACE(("Destroying ipcam %p: %s, width: %d, height: %d.",
                 (tIntPtr)this, mstrURL, mnWidth, mnHeight));
    niSafeDelete(mpTargetTexture);
    Invalidate();
  }

  ///////////////////////////////////////////////
  void __stdcall Invalidate() {
    _StopDecoding();
  }

  ///////////////////////////////////////////////
  ni::tBool __stdcall IsOK() const {
    niClassIsOK(cVideoDecoderIpcam);
    return mstrURL.IsNotEmpty();
  }

  ///////////////////////////////////////////////
  virtual const achar* __stdcall GetVideoDecoderName() const {
    return _A("Ipcam");
  }

  ///////////////////////////////////////////////
  virtual tF64 __stdcall GetVideoFps() const {
    return ni::FDiv(1.0,(tF64)mnFPS);
  }

  ///////////////////////////////////////////////
  virtual tF64 __stdcall GetLength() const {
    return 0;
  }
  virtual void __stdcall SetTime(tF64 afTime) {
  }
  virtual tF64 __stdcall GetTime() const {
    return 0;
  }
  virtual void __stdcall SetPause(tBool abPause) {
    mbPaused = abPause;
    if (mbPaused) {
      _StopDecoding();
    }
  }
  virtual tBool __stdcall GetPause() const {
    return mbPaused;
  }
  virtual void __stdcall SetNumLoops(tU32 anNumLoops) {
  }
  virtual tU32 __stdcall GetNumLoops() const {
    return 0;
  }
  virtual void __stdcall SetSpeed(tF32 afSpeed) {
  }
  virtual tF32 __stdcall GetSpeed() const {
    return 1;
  }

  ///////////////////////////////////////////////
  virtual tVideoDecoderFlags __stdcall GetFlags() const {
    return mFlags;
  }
  virtual iTexture* __stdcall GetTargetTexture() {
    return mpTargetTexture;
  }
  virtual iBitmap2D* __stdcall GetTargetBitmap() {
    return mptrTargetBitmap;
  }

  ///////////////////////////////////////////////
  virtual tU32 __stdcall GetNumSoundTracks() const {
    return 0;
  }
  virtual iUnknown* __stdcall GetSoundTrackData(tU32 anNumTrack) {
    return NULL;
  }

  ///////////////////////////////////////////////
  tBool mbUpdateOnBind;
  virtual void __stdcall SetUpdateOnBind(tBool abUpdateOnBind) niImpl {
    mbUpdateOnBind = abUpdateOnBind;
  }
  virtual tBool __stdcall GetUpdateOnBind() const {
    return mbUpdateOnBind;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall Update(tBool abUpdateTarget, tF32 afFrameTime) {
    if (mbTargetBitmapDirty &&
        niFlagIs(mFlags,eVideoDecoderFlags_TargetTexture))
    {
      _DrawStatusSquare(mptrTargetBitmap);
      mptrGraphics->BlitBitmapToTexture(
          mptrTargetBitmap,mpTargetTexture->mptrTexture,0);
      mfTargetLastUpdateTime = ni::TimerInSeconds();
      mbTargetBitmapDirty = eFalse;
    }

    if (!mbPaused && afFrameTime > 0.0f) {
      if (mptrFuture.IsOK() && mptrFuture->GetIsDone()) {
        _StopDecoding();
      }
      else if (mnStatus == eIpcamStatus_Created) {
        _StartDecoding(eIpcamStatus_Connecting);
      }
      else if (mnStatus == eIpcamStatus_Stopped) {
        WeakPtr<cVideoDecoderIpcam> _this = this;
        _UpdateStatus(eIpcamStatus_Reconnecting);
        niExec(IO,_this) {
          QPtr<cVideoDecoderIpcam> videoDecoder = _this;
          if (videoDecoder.IsOK()) {
            ni::SleepMs(videoDecoder->mptrCURL->GetConnectionTimeoutInSecs() * 1000);
            niLog(Info,niFmt("Trying to reconnect to ipcam '%s'.", videoDecoder->mstrURL));
            videoDecoder->_StartDecoding(eIpcamStatus_Connecting);
          }
          return eTrue;
        };
      }
      else if ((mnStatus == eIpcamStatus_Decoding) &&
               (ni::TimerInSeconds()-mfTargetLastUpdateTime) > (tF64)(mptrCURL->GetConnectionTimeoutInSecs()))
      {
        // Timed out...
        _StopDecoding();
      }
    }

    return eTrue;
  }

  ///////////////////////////////////////////////
  tBool _IsDecoding() {
    return mptrFuture.IsOK() && !mptrFuture->GetIsDone();
  }

  void _StopDecoding() {
    if (mptrFuture.IsOK()) {
      mptrFuture->Cancel();
      mptrFuture = NULL;
    }
    _UpdateStatus(eIpcamStatus_Stopped);
  }

  void _StartDecoding(eIpcamStatus aConnStatus) {
    _StopDecoding();
    _UpdateStatus(aConnStatus);

    WeakPtr<cVideoDecoderIpcam> _this = this;
    Ptr<iRunnable> runnable = mptrCURL->URLGetMultiPart(
      ni::MessageHandler([_this] (tU32 anMsg, const Var& A, const Var& B) {
        QPtr<cVideoDecoderIpcam> videoDecoder = _this;
        if (!videoDecoder.IsOK())
          return;

        videoDecoder->mptrFuture = B;
        switch (anMsg) {
        case eCURLMessage_Started: {
          IPCAM_TRACE(("... Started"));
          break;
        }
        case eCURLMessage_ReceivingHeader: {
          IPCAM_TRACE(("... Receiving Header"));
          break;
        }
        case eCURLMessage_ReceivingData: {
          IPCAM_TRACE(("... Receiving Data"));
          break;
        }
        case eCURLMessage_ReceivedPart: {
          videoDecoder->_UpdateStatus(eIpcamStatus_Decoding);
          IPCAM_TRACE(("... Received Part: %d.",videoDecoder->mnNumPartDecoded));
          ++videoDecoder->mnNumPartDecoded;
          QPtr<iBitmap2D> bmpDecoded = videoDecoder->_ReadMjpegPart(
            VarQueryInterface<iFile>(A),videoDecoder->mnNumJpegDecoded);
          if (bmpDecoded.IsOK()) {
            ++videoDecoder->mnNumJpegDecoded;
            if (videoDecoder->mnNumJpegDecoded == 1) {
              niLog(Info,
                    niFmt("First mjpeg frame decoded with size %dx%d from '%s'.",
                          bmpDecoded->GetWidth(),
                          bmpDecoded->GetHeight(),
                          videoDecoder->mstrURL));
            }
            niExec(Main,videoDecoder,bmpDecoded) {
              videoDecoder->mptrTargetBitmap = bmpDecoded;
              videoDecoder->mbTargetBitmapDirty = eTrue;
              return ni::eTrue;
            };
          }

          break;
        }
        case eCURLMessage_Progress: {
          IPCAM_TRACE(("... Progress %s", A));
          break;
        }
        case eCURLMessage_ResponseCode: {
          IPCAM_TRACE(("... ResponseCode: %d", A));
          break;
        }
        case eCURLMessage_Failed: {
          niLog(Info, niFmt("Ipcam '%s' Failed: %s", videoDecoder->mstrURL, A));
          videoDecoder->_UpdateStatus(eIpcamStatus_Stopped);
          break;
        }
        case eCURLMessage_Completed: {
          niLog(Info, niFmt("Ipcam '%s' Completed: %s", videoDecoder->mstrURL, A));
          videoDecoder->_UpdateStatus(eIpcamStatus_Stopped);
          break;
        }
        default: {
          niWarning(niFmt("Unexpected message '%s'.", MessageID_ToString(anMsg)));
          break;
        }
        }
      },eInvalidHandle),
      this->mstrURL.Chars(),
      ".jpg");
    ni::GetConcurrent()->ThreadRun(runnable);
  }

  QPtr<iBitmap2D> __stdcall _ReadMjpegPart(iFile* apJpeg, tU32 anPartId) {
    cString path;
    tI32 headerSize = -1;
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
      return NULL;
    }

    IPCAM_TRACE(("Jpeg header at %d.", headerSize));
    apJpeg->SeekSet(headerSize);
    return mptrGraphics->LoadBitmap(apJpeg);
  }

  void _DrawStatusSquare(iBitmap2D* apBmp) {
    if (mnStatusSquareSize <= 0)
      return;

    sColor4f color;
    switch (mnStatus) {
      case eIpcamStatus_Created: {
        color = Vec4f(1,1,1,1);
        break;
      }
      case eIpcamStatus_Stopped: {
        color = Vec4f(1,0,0,1);
        break;
      }
      case eIpcamStatus_Connecting: {
        color = Vec4f(1,1,0,1);
        break;
      }
      case eIpcamStatus_Reconnecting: {
        color = Vec4f(1,0.5,0.5,1);
        break;
      }
      case eIpcamStatus_Decoding: {
        color = Vec4f(0,1,0,1);
        break;
      }
      default: {
        color = Vec4f(1,0,1,1);
        break;
      }
    }
    apBmp->ClearRectf(
        Recti(mptrTargetBitmap->GetWidth()-mnStatusSquareSize-4,
              mptrTargetBitmap->GetHeight()-mnStatusSquareSize-4,
              mnStatusSquareSize+4, mnStatusSquareSize+4),
        sColor4f::Zero());
    apBmp->ClearRectf(
        Recti(mptrTargetBitmap->GetWidth()-mnStatusSquareSize-2,
              mptrTargetBitmap->GetHeight()-mnStatusSquareSize-2,
              mnStatusSquareSize, mnStatusSquareSize),
        color);
  }

  void _UpdateStatus(eIpcamStatus aStatus) {
    mnStatus = aStatus;
    mbTargetBitmapDirty = eTrue;
  }

  Ptr<iCURL>               mptrCURL;
  cString                  mstrURL;
  tU32                     mnWidth;
  tU32                     mnHeight;
  tU32                     mnFPS;
  Ptr<iGraphics>           mptrGraphics;
  const tVideoDecoderFlags mFlags;
  sVideoDecoderTexture*    mpTargetTexture;
  Ptr<iBitmap2D>           mptrTargetBitmap;
  tBool                    mbTargetBitmapDirty;
  tF64                     mfTargetLastUpdateTime;
  tBool                    mbPaused;
  QPtr<iFuture>            mptrFuture;
  tU32                     mnNumPartDecoded;
  tU32                     mnNumJpegDecoded;
  tI32                     mnStatusSquareSize;
  tU32                     mnStatus;

  niEndClass(cVideoDecoderIpcam);
};

iVideoDecoder* CreateVideoDecoder_Ipcam(iGraphics* apGraphics, iHString* ahspName, iFile* apFile, tVideoDecoderFlags aFlags) {
  return niNew cVideoDecoderIpcam(apGraphics,ahspName,apFile,aFlags);
}
#endif
