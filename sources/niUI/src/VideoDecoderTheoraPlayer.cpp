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

class cVideoDecoderTheoraPlayer : public ni::ImplRC<ni::iVideoDecoder,ni::eImplFlags_Default>
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
    mbDecoding = eFalse;
    mnNumLoops = eInvalidHandle;
    mnLastUpdateFrameId = eInvalidHandle;
    mptrGraphics = apGraphics;
    mpTargetTexture = niNew sVideoDecoderTexture(apGraphics,ahspName,this,NULL);
    niLog(Info,niFmt("Video started loading: '%s'",apFile->GetSourcePath()));

    const tBool withAlpha = StrEndsWith(apFile->GetSourcePath(),".ogw");

    iExecutor* pLoadingExecutor =
        ni::GetConcurrent()->GetExecutorIO()
        // ni::GetConcurrent()->GetExecutorMain()
        ;

    Ptr<iFile> _fp = apFile;
    Ptr<cVideoDecoderTheoraPlayer> _this = this;
    niExec_(pLoadingExecutor,_this,_fp,withAlpha) {
      TheoraVideoClip* pTheoraVideoClip = Theora_createVideoClip(
          _fp,
          withAlpha ? VIDEO_OUTPUTMODE_ALPHA : VIDEO_OUTPUTMODE,
          2,
          false);
      if (!pTheoraVideoClip) {
        niError(niFmt("Can't create the video clip '%s'.",_fp->GetSourcePath()));
        return ni::eFalse;
      }

      niExec(Main,_this,pTheoraVideoClip,_fp,withAlpha)  {
        _this->mpTheoraVideoClip = pTheoraVideoClip;

        if (niFlagIs(_this->mFlags,eVideoDecoderFlags_TargetTexture)) {
          Ptr<iTexture> ptrTex = _this->mptrGraphics->CreateTexture(
              NULL,
              eBitmapType_2D,
              VIDEO_PIXELFORMAT,
              0,
              pTheoraVideoClip->getWidth(), pTheoraVideoClip->getHeight(), 0,
              eTextureFlags_Dynamic);
          if (!ptrTex.IsOK()) {
            niError(niFmt(_A("Can't create target texture for video '%s'."),_fp->GetSourcePath()));
            return ni::eFalse;
          }
          _this->mpTargetTexture->mptrTexture = ptrTex;
        }

        niLog(Info,niFmt(
            "Video created: '%s' %s, %dx%d, %gsecs, %gfps\n",
            _fp->GetSourcePath(),
            withAlpha ? "rgba" : "rgb",
            pTheoraVideoClip->getWidth(),
            pTheoraVideoClip->getHeight(),
            _this->GetLength(),
            _this->GetVideoFps()));

        _this->SetNumLoops(_this->mnNumLoops);
        _this->SetSpeed(_this->mfSpeed);
        _this->SetPause(_this->mbPaused);

        return ni::eTrue;
      };

      return ni::eTrue;
    };

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
    if (!mpTheoraVideoClip)
      return eTrue;

    if (afFrameTime > 0.0f && !mbPaused) {
      mpTheoraVideoClip->update(afFrameTime);
      mbTargetDirty = eTrue;
    }

    if (abUpdateTarget && mbTargetDirty && (mnLastUpdateFrameId != ni::GetLang()->GetFrameNumber())) {
      _UpdateTarget();
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

  void _UpdateTarget() {
    if (!mpTheoraVideoClip)
      return; // no decoding...

    if (mbDecoding)
      return;

    Ptr<cVideoDecoderTheoraPlayer> _this = this;
    niExec(CPU,_this) {
      _this->mbDecoding = eTrue;

      // if user requested seeking, do that then.
      if (_this->mpTheoraVideoClip->mSeekFrame >= 0) {
        _this->mpTheoraVideoClip->doSeek();
      }

      _this->mpTheoraVideoClip->decodedAudioCheck();
      _this->mpTheoraVideoClip->decodeNextFrame();

      TheoraVideoFrame* f = _this->mpTheoraVideoClip->getNextFrame();
      if (f) {
        niExec(Main,_this,f) {
          _this->mptrTargetBitmap = f->getBitmap(_this->mptrGraphics, VIDEO_PIXELFORMAT);
          if (niFlagIs(_this->mFlags,eVideoDecoderFlags_TargetTexture) && _this->mpTargetTexture) {
            _this->mptrGraphics->BlitBitmapToTexture(_this->mptrTargetBitmap,_this->mpTargetTexture->mptrTexture,0);
          }
          _this->mpTheoraVideoClip->popFrame();
          _this->mbDecoding = eFalse;
          return eTrue;
        };
      }
      else {
        _this->mbDecoding = eFalse;
      }

      return eTrue;
    };
  }

 private:
  Ptr<iGraphics>           mptrGraphics;
  const tVideoDecoderFlags mFlags;
  TheoraVideoClip*         mpTheoraVideoClip;
  sVideoDecoderTexture*    mpTargetTexture;
  Ptr<iBitmap2D>           mptrTargetBitmap;
  tBool                    mbPaused;
  tBool                    mbDecoding;
  tU32                     mnNumLoops;
  tF32                     mfSpeed;
  tU32                     mnLastUpdateFrameId;
  tBool                    mbTargetDirty;

  niEndClass(cVideoDecoderTheoraPlayer);
};

iVideoDecoder* CreateVideoDecoder_TheoraPlayer(iGraphics* apGraphics, iHString* ahspName, iFile* apFile, tVideoDecoderFlags aFlags) {
  niCheckIsOK(apFile,NULL);
  return niNew cVideoDecoderTheoraPlayer(apGraphics,ahspName,apFile,aFlags);
}

#endif
