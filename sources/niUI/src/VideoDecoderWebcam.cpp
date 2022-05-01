// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"
#include "VideoUtils.h"

#if NI_VIDEO_USE_WEBCAM
#include <niLang/Utils/ConcurrentImpl.h>
#include "escapi/escapi.h"

using namespace ni;

#include "VideoDecoderTexture.h"

#define WEBCAM_ASYNC_INIT
#define WEBCAM_PIXEL_FORMAT1 "B8G8R8A8"
#define WEBCAM_PIXEL_FORMAT2 "R8G8B8A8"

static cString _getWebcamName(int aIndex) {
  char deviceName[256];
  ESCAPI_getCaptureDeviceName(aIndex,deviceName,sizeof(deviceName));
  return cString(deviceName);
}

/*
  Example .webcam file:
  <webcam device_index="0" preferred_mode="0" width="640" height="480" />

 */
class cVideoDecoderWebcam : public ni::cIUnknownImpl<ni::iVideoDecoder,ni::eIUnknownImplFlags_Default>
{
  niBeginClass(cVideoDecoderWebcam);

 public:
  ///////////////////////////////////////////////
  cVideoDecoderWebcam(iGraphics* apGraphics, iHString* ahspName, iFile* apFile, tVideoDecoderFlags aFlags)
      : mFlags(aFlags)
      , mptrFile(apFile)
      , mptrGraphics(apGraphics)
      , mbUpdateOnBind(eTrue)
  {
    ZeroMembers();
    niCheckIsOK(apFile,;);

    ni::Ptr<ni::iDataTable> dt = ni::GetLang()->CreateDataTable("webcam");
    if (!ni::GetLang()->SerializeDataTable("xml", ni::eSerializeMode_Read, dt, apFile)) {
      niWarning("Can't read webcam datatable.");
      return;
    }

    static tBool _escapiInitialized = ni::eFalse;
    if (!_escapiInitialized) {
      ESCAPI_init();
      int numCaptureDevices = ESCAPI_countCaptureDevices();
      niLog(Info, niFmt("Found '%d' webcam.", numCaptureDevices));
      niLoop(i,numCaptureDevices) {
        niLog(Info, niFmt("Webcam %d: %s", i, _getWebcamName(i)));
      }
      _escapiInitialized = eTrue;
    }
    mWebcamIndex = -1;

    tI32 webcamIndex = dt->GetIntDefault("device_index",0);
    tI32 preferredmode = dt->GetIntDefault("preferred_mode",-1);
    mCapture.mWidth = dt->GetIntDefault("width",640);
    mCapture.mHeight = dt->GetIntDefault("height",480);
    niLog(Info,niFmt("Initializing webcam: %d, mode: %d, width: %d, height: %d.",
                     webcamIndex, preferredmode, mCapture.mWidth, mCapture.mHeight));

    const tBool bSwapRGB = dt->GetBoolDefault("swap_rgb",eFalse);

    mptrTargetBitmap = mptrGraphics->CreateBitmap2D(mCapture.mWidth,mCapture.mHeight, bSwapRGB ? WEBCAM_PIXEL_FORMAT2 : WEBCAM_PIXEL_FORMAT1);
    if (!mptrTargetBitmap.IsOK()) {
      niError(niFmt(_A("Can't create target bitmap for webcam '%s', index: %d, name: '%s'."),
                    apFile->GetSourcePath(), webcamIndex, _getWebcamName(webcamIndex)));
      return;
    }
    mptrTargetBitmap->Clearf(sColor4f::One());
    mCapture.mTargetBuf = (int*)mptrTargetBitmap->GetData();

    if (niFlagIs(mFlags,eVideoDecoderFlags_TargetTexture)) {
      Ptr<iTexture> ptrTex = mptrGraphics->CreateTexture(
          NULL,
          eBitmapType_2D,
          mptrTargetBitmap->GetPixelFormat()->GetFormat(),
          0,
          mCapture.mWidth, mCapture.mHeight, 0,
          eTextureFlags_Dynamic|eTextureFlags_Overlay);
      if (!ptrTex.IsOK()) {
        niError(niFmt(_A("Can't create target texture for webcam  webcam '%s', index: %d, name: '%s'."),
                      apFile->GetSourcePath(), webcamIndex, _getWebcamName(webcamIndex)));
        return;
      }

      mpTargetTexture = niNew sVideoDecoderTexture(apGraphics,ahspName,this,ptrTex);
    }

    Ptr<iFile> _fp = apFile;
    Ptr<cVideoDecoderWebcam> _this = this;
#ifdef WEBCAM_ASYNC_INIT
    niExec(IO,_this,_fp,preferredmode,webcamIndex) {
#endif

      _this->mptrTargetBitmap->Clearf(sColor4f::Green());
      if (niFlagIs(_this->mFlags,eVideoDecoderFlags_TargetTexture)) {
        _this->mptrGraphics->BlitBitmapToTexture(_this->mptrTargetBitmap,_this->mpTargetTexture->mptrTexture,0);
      }

      tF32 timeStart = ni::TimerInSeconds();
      if (!ESCAPI_initCapture(webcamIndex, preferredmode, &_this->mCapture)) {
        _this->mptrTargetBitmap->Clearf(sColor4f::Red());
        if (niFlagIs(_this->mFlags,eVideoDecoderFlags_TargetTexture)) {
          _this->mptrGraphics->BlitBitmapToTexture(_this->mptrTargetBitmap,_this->mpTargetTexture->mptrTexture,0);
        }

        niError(niFmt(_A("Can't init capture for webcam '%s', index: %d, name: '%s'."),
                      _fp->GetSourcePath(), webcamIndex, _getWebcamName(webcamIndex)));
        return
#ifdef WEBCAM_ASYNC_INIT
            ni::eFalse
#endif
            ;
      }
      niDebugFmt(("... initCapture time: %gs", ni::TimerInSeconds()-timeStart));

      timeStart = ni::TimerInSeconds();
      ESCAPI_doCapture(webcamIndex);
      niDebugFmt(("... doCapture time: %gs", ni::TimerInSeconds()-timeStart));

      _this->mWebcamIndex = webcamIndex;
      _this->mbPaused = eFalse;

#ifdef WEBCAM_ASYNC_INIT
      return ni::eTrue;
    };
#endif
  }

  ///////////////////////////////////////////////
  ~cVideoDecoderWebcam() {
    niSafeDelete(mpTargetTexture);
    if (mWebcamIndex >= 0) {
      ESCAPI_deinitCapture(mWebcamIndex);
    }
    Invalidate();
  }

  ///////////////////////////////////////////////
  void __stdcall Invalidate() {
  }

  ///////////////////////////////////////////////
  void __stdcall ZeroMembers() {
    mpTargetTexture = NULL;
  }

  ///////////////////////////////////////////////
  ni::tBool __stdcall IsOK() const {
    niClassIsOK(cVideoDecoderWebcam);
    return ni::eTrue;
  }

  ///////////////////////////////////////////////
  virtual const achar* __stdcall GetVideoDecoderName() const {
    return _A("Webcam");
  }

  ///////////////////////////////////////////////
  virtual tF64 __stdcall GetVideoFps() const {
    return 1.0/25.0;
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
    if (!mbPaused && mWebcamIndex >= 0 && afFrameTime > 0.0f) {
      if (!ESCAPI_isCaptureDone(mWebcamIndex))
        return eTrue;

      if (niFlagIs(mFlags,eVideoDecoderFlags_TargetTexture)) {
        mptrGraphics->BlitBitmapToTexture(mptrTargetBitmap,mpTargetTexture->mptrTexture,0);
      }

      ESCAPI_doCapture(mWebcamIndex);
    }

    return eTrue;
  }

 private:
  Ptr<iGraphics>           mptrGraphics;
  const tVideoDecoderFlags mFlags;
  sVideoDecoderTexture*    mpTargetTexture;
  Ptr<iBitmap2D>           mptrTargetBitmap;
  SimpleCapParams          mCapture;
  tBool                    mbPaused;
  Ptr<iFile>               mptrFile;
  int                      mWebcamIndex;

  niEndClass(cVideoDecoderWebcam);
};

iVideoDecoder* CreateVideoDecoder_Webcam(iGraphics* apGraphics, iHString* ahspName, iFile* apFile, tVideoDecoderFlags aFlags) {
  return niNew cVideoDecoderWebcam(apGraphics,ahspName,apFile,aFlags);
}

#endif
