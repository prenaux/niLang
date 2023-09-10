#include "stdafx.h"
#include <niLang/Utils/TimerSleep.h>

using namespace ni;

struct FVideoDecoder {
  FVideoDecoder() {
  }
  ~FVideoDecoder() {
  }
};

struct IPCam : public ni::cWidgetSinkImpl<> {
  Ptr<iTexture> mTexture;
  Ptr<iOverlay> mOverlay;
  cString tex1 = "";
  cString tex2 = "";
  tBool useA = false;
  tF32 test = 3;

  TEST_CONSTRUCTOR(IPCam) {
    TEST_STEPS(750);
  }
  ~IPCam() {
  }

  tBool __stdcall OnSinkAttached() niImpl {
    const cString dataDir = ni::GetLang()->GetProperty("ni.dirs.data");
    tex1 = dataDir + "/test/ipcam/CCTV-01.ipcam";
    tex2 = dataDir + "/test/ipcam/CCTV-02.ipcam";
    niLog(Info, "IPCam::OnSinkAttached");
    return eTrue;
  };

  tBool __stdcall OnDestroy() niImpl {
    niLog(Info, "IPCam::OnDestroy");
    return eTrue;
  }

  tBool __stdcall OnSinkDetached() niImpl {
    niLog(Info, "IPCam::OnSinkDetached");
    return eTrue;
  }

  tBool __stdcall OnPaint(const sVec2f &avMousePos, iCanvas *apCanvas) niImpl {
    test += ni::GetLang()->GetFrameTime();
    float k = 1; // wait timer
    if (test > k) {
      test -= k;
      useA = !useA;
      tHStringPtr path = _H(useA ? tex1.Chars() : tex2.Chars());
      mTexture = mpWidget->GetGraphics()->CreateTextureFromRes(path, NULL, eTextureFlags_Default);
      niDebugFmt(("Switch to %s", path));
    }

    if (mTexture.IsOK())
    {
      if (!mOverlay.IsOK())
      {
        mOverlay = mpWidget->GetGraphics()->CreateOverlayTexture(mTexture);
      }
      mOverlay->GetMaterial()->SetChannelTexture(eMaterialChannel_Base, mTexture);
      apCanvas->BlitOverlay(Rectf(0, 0, 512, 512), mOverlay);
    }
    return eFalse;
  }
};

// Skipped because its doing an infinite loop atm.
// SKIP: TEST_FIXTURE_WIDGET(FVideoDecoder, IPCam);

struct Player : public ni::cWidgetSinkImpl<> {
  Ptr<iTexture> mTexture;
  Ptr<iOverlay> mOverlay;
  cString tex1 = "";
  cString tex2 = "";
  tBool useA = false;
  tF32 test = 3;

  TEST_CONSTRUCTOR(Player) {
    TEST_STEPS(7500);
  }
  ~Player() {
  }

  tBool __stdcall OnSinkAttached() niImpl {
    const cString dataDir = ni::GetLang()->GetProperty("ni.dirs.data");
    tex1 = dataDir + "/test/ipcam/cut_50_FPS.ogv";
    // tex1 = dataDir + "/test/ipcam/cut_24_FPS.ogv";
    niLog(Info, "Player::OnSinkAttached");

    mTexture = mpWidget->GetGraphics()->CreateTextureFromRes(_H(tex1), NULL, eTextureFlags_Overlay);
    mOverlay = mpWidget->GetGraphics()->CreateOverlayTexture(mTexture);
    mOverlay->GetMaterial()->SetChannelTexture(eMaterialChannel_Base, mTexture);
    return eTrue;
  };

  tBool __stdcall OnKeyDown(eKey aKey, tU32 aKeyMod) niImpl {

    if (aKey == eKey_n1) {
      QPtr<iVideoDecoder> decoder = mOverlay->GetMaterial()->GetChannelTexture(eMaterialChannel_Base);
      if (decoder.IsOK()) {
        decoder->SetPause(!decoder->GetPause());
      }
    }
    else if (aKey == eKey_n2) {
      mOverlay->GetMaterial()->SetChannelTexture(eMaterialChannel_Base, NULL);
      mTexture = NULL;
    }
    return eTrue;
  }

  tBool __stdcall OnDestroy() niImpl {
    niLog(Info, "Player::OnDestroy");
    return eTrue;
  }

  tBool __stdcall OnSinkDetached() niImpl {
    niLog(Info, "Player::OnSinkDetached");
    return eTrue;
  }

  tBool __stdcall OnPaint(const sVec2f &avMousePos, iCanvas *apCanvas) niImpl {
    if (mTexture.IsOK() && mOverlay.IsOK()) {
      apCanvas->BlitOverlay(Rectf(0, 0, mTexture->GetWidth() * 0.25, mTexture->GetHeight()*0.25), mOverlay);
    }
    return eFalse;
  }
};

// TEST_FIXTURE_WIDGET(FVideoDecoder, Player);
