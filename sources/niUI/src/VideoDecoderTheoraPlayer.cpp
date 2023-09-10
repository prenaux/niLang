#include "stdafx.h"
#include "VideoUtils.h"

#if NI_VIDEO_USE_THEORAPLAYER

#include "theoraplayer/TheoraVideoManager.h"
#include "theoraplayer/TheoraVideoFrame.h"
#include "VideoDecoderTexture.h"
#include <niLang/Utils/ConcurrentImpl.h>

#define VIDEO_OUTPUTMODE        TH_RGBX
#define VIDEO_OUTPUTMODE_ALPHA  TH_RGBA
#define VIDEO_PIXELFORMAT "R8G8B8A8"
#define VIDEO_PIXELFORMAT_BPP 4

class cVideoDecoderTheoraPlayer : public ni::cIUnknownImpl<ni::iVideoDecoder,ni::eIUnknownImplFlags_Default>
{
  niBeginClass(cVideoDecoderTheoraPlayer);

 public:
  ///////////////////////////////////////////////
  cVideoDecoderTheoraPlayer(iGraphics* apGraphics, iHString* ahspName, iFile* apFile, tVideoDecoderFlags aFlags)
      : mFlags(aFlags)
      , mbUpdateOnBind(eTrue)
  {
    mfSpeed = 1.0f;
    mbPaused = eFalse;
    mnNumLoops = eInvalidHandle;
    mnLastUpdateFrameId = eInvalidHandle;
    mptrGraphics = apGraphics;
    mpTargetTexture = niNew sVideoDecoderTexture(apGraphics,ahspName,this,NULL);
    niLog(Info,niFmt("Video started loading: '%s'",apFile->GetSourcePath()));

    const tBool withAlpha = StrEndsWith(apFile->GetSourcePath(),".ogw");
    Ptr<iFile> _fp = apFile;
    mpTheoraVideoClip = Theora_createVideoClip(
      _fp,
      withAlpha ? VIDEO_OUTPUTMODE_ALPHA : VIDEO_OUTPUTMODE,
      4,
      false,
      false);

    if (!mpTheoraVideoClip) {
      niError(niFmt("Can't create the video clip '%s'.",_fp->GetSourcePath()));
      return;
    }

    if (!mpTheoraVideoClip->load(_fp)) {
      niError("Couldn't load the video clip.");
      return;
    }

    if (niFlagIs(mFlags,eVideoDecoderFlags_TargetTexture)) {
      Ptr<iTexture> ptrTex = mptrGraphics->CreateTexture(
        NULL,
        eBitmapType_2D,
        VIDEO_PIXELFORMAT,
        0,
        mpTheoraVideoClip->getWidth(), mpTheoraVideoClip->getHeight(), 0,
        ni::eTextureFlags_Dynamic|ni::eTextureFlags_Overlay);

      if (!ptrTex.IsOK()) {
        niError(niFmt(_A("Can't create target texture for video '%s'."),_fp->GetSourcePath()));
        return;
      }
      mpTargetTexture->mptrTexture = ptrTex;
    }

    niLog(Info,niFmt(
            "Video created: '%s' %s, %dx%d, %gsecs, %gfps\n",
            _fp->GetSourcePath(),
            withAlpha ? "rgba" : "rgb",
            mpTheoraVideoClip->getWidth(),
            mpTheoraVideoClip->getHeight(),
            GetLength(),
            GetVideoFps()));

    SetNumLoops(mnNumLoops);
    SetSpeed(mfSpeed);
    SetPause(mbPaused);
    mpExecutor = GetConcurrent()->CreateExecutorThreadPool(1);
  }

  ///////////////////////////////////////////////
  ~cVideoDecoderTheoraPlayer() {
    niSafeDelete(mpTargetTexture);
    if (mpTheoraVideoClip) {
      niLog(Info,niFmt("Destroying video clip %p", (tIntPtr)mpTheoraVideoClip));
      Theora_destroyVideoClip(mpTheoraVideoClip);
      mpTheoraVideoClip = NULL;
    }
  }

  ///////////////////////////////////////////////
  ni::tBool __stdcall IsOK() const {
    niClassIsOK(cVideoDecoderTheoraPlayer);
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual const achar* __stdcall GetVideoDecoderName() const {
    return _A("THEORAPLAYER");
  }
  virtual tF64 __stdcall GetVideoFps() const {
    return mpTheoraVideoClip ? mpTheoraVideoClip->getFPS() : 1.0/24.0 ;
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
  virtual tF64 __stdcall GetLength() const {
    return mpTheoraVideoClip ? mpTheoraVideoClip->getDuration() : 0;
  }
  virtual void __stdcall SetTime(tF64 afTime) {
    if (mpTheoraVideoClip) {
      mpTheoraVideoClip->seek(afTime);
    }
  }
  virtual tF64 __stdcall GetTime() const {
    return mpTheoraVideoClip ? mpTheoraVideoClip->getTimePosition() : 0;
  }
  virtual void __stdcall SetPause(tBool abPause) {
    mbPaused = abPause;
    if (mpTheoraVideoClip) {
      mbPaused ? mpTheoraVideoClip->pause() : mpTheoraVideoClip->play();
    }
  }
  virtual tBool __stdcall GetPause() const {
    return mbPaused;
  }
  virtual void __stdcall SetNumLoops(tU32 anNumLoops) {
    mnNumLoops = anNumLoops;
  }
  virtual tU32 __stdcall GetNumLoops() const {
    return mnNumLoops;
  }
  virtual void __stdcall SetSpeed(tF32 afSpeed) {
    mfSpeed = ni::Clamp(afSpeed,0.0f,1000.0f);
    if (mpTheoraVideoClip) {
      mpTheoraVideoClip->setPlaybackSpeed(mfSpeed);
    }
  }
  virtual tF32 __stdcall GetSpeed() const {
    return mfSpeed;
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
    if (!mpTheoraVideoClip || !mpTargetTexture)
      return eTrue;

    if (afFrameTime > 0.0f && !mbPaused) {
      mpTheoraVideoClip->update(afFrameTime);
      mbTargetDirty = eTrue;

      WeakPtr<cVideoDecoderTheoraPlayer> _this = this;
      mpExecutor->Execute(ni::Runnable([_this]() {
        QPtr<cVideoDecoderTheoraPlayer> player(_this);
        if (!player.IsOK()) return eTrue;
        auto clip = player->mpTheoraVideoClip;
        if (!clip) return eTrue;

        if (clip->mSeekFrame >= 0) {
          clip->doSeek();
        }
        clip->decodedAudioCheck();
        clip->decodeNextFrame();
        return eTrue;
      }));
    }

    if (abUpdateTarget && mbTargetDirty && (mnLastUpdateFrameId != ni::GetLang()->GetFrameNumber())) {
      auto f = mpTheoraVideoClip->getNextFrame();
      if (f) {
        mptrTargetBitmap = f->getBitmap(mptrGraphics, VIDEO_PIXELFORMAT);
        if (niFlagIs(mFlags,eVideoDecoderFlags_TargetTexture) && mpTargetTexture) {
          mptrGraphics->BlitBitmapToTexture(mptrTargetBitmap,mpTargetTexture->mptrTexture,0);
        }
        mpTheoraVideoClip->popFrame();
      }
      mnLastUpdateFrameId = ni::GetLang()->GetFrameNumber();
      mbTargetDirty = eFalse;
    }

    const tF32 videoLen = this->GetLength();
    tF32 currentTime = this->GetTime();
    if ((mnNumLoops > 0) && (currentTime > videoLen)) {
      while ((mnNumLoops > 0) && (currentTime > videoLen)) {
        if (mnNumLoops != eInvalidHandle) {
          --mnNumLoops;
        }
        currentTime -= videoLen;
      }
      this->SetTime(currentTime);
    }
    return eTrue;
  }

 private:
  Ptr<iGraphics>           mptrGraphics;
  const tVideoDecoderFlags mFlags;
  TheoraVideoClip*         mpTheoraVideoClip;
  sVideoDecoderTexture*    mpTargetTexture;
  Ptr<iBitmap2D>           mptrTargetBitmap;
  tBool                    mbPaused;
  tU32                     mnNumLoops;
  tF32                     mfSpeed;
  tU32                     mnLastUpdateFrameId;
  tBool                    mbTargetDirty;
  Ptr<iExecutor>           mpExecutor;

  niEndClass(cVideoDecoderTheoraPlayer);
};

iVideoDecoder* CreateVideoDecoder_TheoraPlayer(iGraphics* apGraphics, iHString* ahspName, iFile* apFile, tVideoDecoderFlags aFlags) {
  niCheckIsOK(apFile,NULL);
  return niNew cVideoDecoderTheoraPlayer(apGraphics,ahspName,apFile,aFlags);
}

#endif
