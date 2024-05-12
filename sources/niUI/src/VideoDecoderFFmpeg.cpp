// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"
#include "VideoUtils.h"

#if defined NI_VIDEO_USE_THEORAPLAYER && defined NI_VIDEO_USE_FFMPEG

#include "theoraplayer/TheoraVideoManager.h"
#include "theoraplayer/TheoraVideoFrame.h"
#include "VideoDecoderTexture.h"
#include <niLang/Utils/ConcurrentImpl.h>


#define READ_BUFFER_SIZE 4096
#define FFMPEG_LOG_LEVEL AV_LOG_WARNING


#define VIDEO_OUTPUTMODE        TH_RGBX
#define VIDEO_PIXELFORMAT "R8G8B8A8"

#include <niLang/Utils/TimerSleep.h>
#include <niLang/Utils/ConcurrentImpl.h>

#include "VideoDecoderTexture.h"
#include "TheoraVideoClip.h"


#define FFMPEG_ASYNC_INIT
#define FFMPEG_PIXEL_FORMAT "R8G8B8A8"

#define FFMPEG_TRACE(aFmt) niDebugFmt(aFmt)

struct cVideoDecoderFFmpeg : public ni::ImplRC<ni::iVideoDecoder,ni::eImplFlags_Default>
{
  niBeginClass(cVideoDecoderFFmpeg);


  enum eFFmpegStatus {
    eFFmpegStatus_Created = 0,
    eFFmpegStatus_Stopped,
    eFFmpegStatus_Reconnecting,
    eFFmpegStatus_Connecting,
    eFFmpegStatus_Decoding
  };

  ///////////////////////////////////////////////
  cVideoDecoderFFmpeg(iGraphics* apGraphics, iHString* ahspName, iFile* apFile, tVideoDecoderFlags aFlags)
      : mFlags(aFlags)
      , mptrGraphics(apGraphics)
      , mbUpdateOnBind(eTrue)
  {
    niCheckIsOK(apFile,;);
    mpTargetTexture = NULL;
    ni::Ptr<ni::iDataTable> dt = ni::GetLang()->CreateDataTable("ipcam");
    if (!ni::GetLang()->SerializeDataTable("xml", ni::eSerializeMode_Read, dt, apFile)) {
      niWarning("ffmpeg: can't read ipcam datatable.");
      return;
    }

    apFile->SeekSet(0);
    mbIsShutdown = eFalse;
    mConfig = apFile;
    mnWidth = dt->GetIntDefault("width",640);
    mnHeight = dt->GetIntDefault("height",480);
    mnFPS = dt->GetIntDefault("fps",10);
    mnStatusSquareSize = dt->GetIntDefault("status_square_size",6);
    mstrURL = dt->GetStringDefault("url","");
    if (mstrURL.empty()) {
      niError("No source URL specified.");
      return;
    }

    mptrTargetBitmap = mptrGraphics->CreateBitmap2D(mnWidth, mnHeight, FFMPEG_PIXEL_FORMAT);
    if (!mptrTargetBitmap.IsOK()) {
      niError(niFmt("Can't create target bitmap for ipcam '%s'.",
                    apFile->GetSourcePath()));
      return;
    }

    _UpdateBitmap(NULL);

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

    mExecutor = ni::GetConcurrent()->CreateExecutorThreadPool(1);
    mbDecoding = eFalse;
    niLog(Info,niFmt("Initialized ipcam: %s, width: %d, height: %d.",
                     mstrURL, mnWidth, mnHeight));
  }

  ///////////////////////////////////////////////
  ~cVideoDecoderFFmpeg() {
    FFMPEG_TRACE(("Destroying ipcam %p: %s, width: %d, height: %d.",
                 (tIntPtr)this, mstrURL, mnWidth, mnHeight));
    Invalidate();
    niSafeDelete(mpTargetTexture);
  }

  ///////////////////////////////////////////////
  void __stdcall Invalidate() {
    if (mbIsShutdown) return;
    mbIsShutdown = eTrue;
    if (mpTheoraVideoClip) {
      Theora_destroyVideoClip(mpTheoraVideoClip);
      mpTheoraVideoClip = NULL;
    }
    mExecutor->Shutdown(0);
  }

  ///////////////////////////////////////////////
  ni::tBool __stdcall IsOK() const {
    niClassIsOK(cVideoDecoderFFmpeg);
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual const achar* __stdcall GetVideoDecoderName() const {
    return _A("FFmpeg");
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
    if (mbPaused) return eTrue;
    _UpdateTarget(afFrameTime);

    if (mbTargetBitmapDirty) {
      if (niFlagIs(mFlags,eVideoDecoderFlags_TargetTexture) && mpTargetTexture) {
        mptrGraphics->BlitBitmapToTexture(mptrTargetBitmap,mpTargetTexture->mptrTexture,0);
      }
      if (mpTheoraVideoClip)
        mpTheoraVideoClip->popFrame();
      mbDecoding = eFalse;
      mbTargetBitmapDirty = eFalse;
    }
    return eTrue;
  }

  sRectf _FitToCenter(tF32 w1, tF32 h1, tF32 w2, tF32 h2) {
    const tF32 innerAspectRatio = ni::FDiv(w1, h1);
    const tF32 outerAspectRatio = ni::FDiv(w2, h2);
    const tF32 resizeFactor = (innerAspectRatio >= outerAspectRatio) ? ni::FDiv(w2, w1) : ni::FDiv(h2, h1);
    const tF32 w = w1 * resizeFactor;
    const tF32 h = h1 * resizeFactor;
    return sRectf((w2 - w) / 2.f, (h2 - h) / 2.f, w, h);
  }

  void _UpdateBitmap(iBitmap2D* aptrBitmap) {
    if (aptrBitmap) {
      if (aptrBitmap->GetWidth() != mptrTargetBitmap->GetWidth() || aptrBitmap->GetHeight() != mptrTargetBitmap->GetHeight()) {
        sRectf r = _FitToCenter(aptrBitmap->GetWidth(),aptrBitmap->GetHeight(),
                                mptrTargetBitmap->GetWidth(),mptrTargetBitmap->GetHeight());
        mptrTargetBitmap->BlitStretch(
          aptrBitmap, 0, 0, r.x, r.y,
          aptrBitmap->GetWidth(),
          aptrBitmap->GetHeight(),
          r.GetWidth(),
          r.GetHeight());
      }
      else {
        mptrTargetBitmap = aptrBitmap;
      }
    }
    else {
      mptrTargetBitmap->Clearf(sVec4f::Zero());
    }
    mbTargetBitmapDirty = eTrue;
  }

  void _UpdateTarget(tF32 afFrameTime) {
    if (mbDecoding)
      return;
    WeakPtr<cVideoDecoderFFmpeg> _this = this;
    auto run = ni::Runnable([_this, afFrameTime] {
      QPtr<cVideoDecoderFFmpeg> vd(_this);
      if (!vd.IsOK()) return eTrue;
      vd->mbDecoding = eTrue;
      if (!vd->mpTheoraVideoClip) {
        vd->mConfig->SeekSet(0);
        vd->mpTheoraVideoClip = Theora_createVideoClip(vd->mConfig,VIDEO_OUTPUTMODE,2,false);
      }

      if (vd->mpTheoraVideoClip) {

        vd->mpTheoraVideoClip->update(afFrameTime);
        if (vd->mpTheoraVideoClip->mSeekFrame >= 0) {
          vd->mpTheoraVideoClip->doSeek();
        }
        vd->mpTheoraVideoClip->decodedAudioCheck();

        if (vd->mpTheoraVideoClip->decodeNextFrame())
        {
          TheoraVideoFrame* f = vd->mpTheoraVideoClip->getNextFrame();
          if (f) {
            vd->_UpdateBitmap(f->getBitmap(vd->mptrGraphics, VIDEO_PIXELFORMAT));
          }
        }
        else {
          niWarning("Video decode error, destory Theora_destroyVideoClip.");
          Theora_destroyVideoClip(vd->mpTheoraVideoClip);
          vd->mpTheoraVideoClip = NULL;
        }
      }

      if (!vd->mpTheoraVideoClip) {
        ni::SleepMs(2000);
      }

      vd->mbDecoding = eFalse;
      return eTrue;
    });

    // ni::GetConcurrent()->GetExecutorIO()->Submit(run);
    mExecutor->Submit(run);
  }

  tBool                    mbIsShutdown;
  Ptr<iFile>               mConfig;
  Ptr<iExecutor>           mExecutor;
  TheoraVideoClip*         mpTheoraVideoClip;
  tF32                     mfSpeed;
  tU32                     mnLastUpdateFrameId;
  tBool                    mbTargetDirty;
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
  tBool                    mbDecoding;
  tBool                    mbPaused;
  QPtr<iFuture>            mptrFuture;
  tU32                     mnNumPartDecoded;
  tI32                     mnStatusSquareSize;
  tU32                     mnStatus;

  niEndClass(cVideoDecoderFFmpeg);
};

iVideoDecoder* CreateVideoDecoder_FFmpeg(iGraphics* apGraphics, iHString* ahspName, iFile* apFile, tVideoDecoderFlags aFlags) {
  return niNew cVideoDecoderFFmpeg(apGraphics,ahspName,apFile,aFlags);
}
#endif
