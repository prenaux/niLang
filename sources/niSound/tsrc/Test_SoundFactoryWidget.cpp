#include "stdafx.h"
#include <niAppLib.h>

using namespace ni;

struct FSoundFactoryWidget {
};

struct ClickWAV : public ni::cWidgetSinkImpl<> {
  QPtr<iSoundFactory> mptrSoundFactory;
  Ptr<iSoundSource> sndSource;
  tU32 numPlay;

  TEST_CONSTRUCTOR(ClickWAV) {
    m_numSteps = 0xffffff;
    numPlay = 3;
  }

  tBool __stdcall OnSinkAttached() niImpl {
    mptrSoundFactory = niCreateInstance(niSound,SoundFactory,niVarNull,niVarNull);
    CHECK(mptrSoundFactory->StartupDriver(0,eSoundFormat_Stereo16,44100,NULL));

    Ptr<iFile> fpSound = ni::GetLang()->URLOpen("Test_niSound://click.wav");
    CHECK(mptrSoundFactory.IsOK());

    Ptr<iSoundBuffer> sndBuffer = mptrSoundFactory->CreateSoundBuffer(fpSound,eFalse,_H(fpSound->GetSourcePath()));
    CHECK(sndBuffer.IsOK());

    sndSource = mptrSoundFactory->CreateSoundSource(sndBuffer);
    CHECK(sndSource.IsOK());

    CHECK(sndSource->Play2D(1,1,0));
    return eTrue;
  }

  tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niImpl {
    static tF64 lastDisplayedTime = ni::TimerInSeconds();
    static tF64 startTime = ni::TimerInSeconds();

    const tF64 currentTime = ni::TimerInSeconds();
    mptrSoundFactory->Update(ni::GetLang()->GetFrameTime());
    if (currentTime - lastDisplayedTime > 0.5f) {
      niDebugFmt(("... playing: %gs", currentTime - startTime));
      lastDisplayedTime = currentTime;
    }
    if (currentTime - startTime > 1.0f) {
      niDebugFmt(("... play time timeout: %gs", ni::TimerInSeconds() - startTime));
      m_numSteps = 0;
    }
    if (!sndSource->GetIsPlaying()) {
      niDebugFmt(("... play time final: %gs", ni::TimerInSeconds() - startTime));
      if (--numPlay == 0) {
        m_numSteps = 0;
      }
      else {
        lastDisplayedTime = ni::TimerInSeconds();
        startTime = ni::TimerInSeconds();
        CHECK(sndSource->Play2D(1,1,0));
      }
    }

    return eFalse;
  }
};
TEST_FIXTURE_WIDGET(FSoundFactoryWidget,ClickWAV);

struct AtmoWAV : public ni::cWidgetSinkImpl<> {
  QPtr<iSoundFactory> mptrSoundFactory;
  Ptr<iSoundSource> sndSource;

  TEST_CONSTRUCTOR(AtmoWAV) {
    m_numSteps = 0xffffff;
  }

  tBool __stdcall OnSinkAttached() niImpl {
    mptrSoundFactory = niCreateInstance(niSound,SoundFactory,niVarNull,niVarNull);
    CHECK(mptrSoundFactory->StartupDriver(0,eSoundFormat_Stereo16,44100,NULL));

    Ptr<iFile> fpSound = ni::GetLang()->URLOpen("Test_niSound://atmo_loop_01.wav");
    CHECK(mptrSoundFactory.IsOK());

    Ptr<iSoundBuffer> sndBuffer = mptrSoundFactory->CreateSoundBuffer(fpSound,eTrue,_H(fpSound->GetSourcePath()));
    CHECK(sndBuffer.IsOK());

    sndSource = mptrSoundFactory->CreateSoundSource(sndBuffer);
    CHECK(sndSource.IsOK());

    CHECK(sndSource->Play2D(1,5,0));
    return eTrue;
  }

  tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niImpl {
    static tF64 lastDisplayedTime = ni::TimerInSeconds();
    static tF64 startTime = ni::TimerInSeconds();

    const tF64 currentTime = ni::TimerInSeconds();
    mptrSoundFactory->Update(ni::GetLang()->GetFrameTime());
    if (currentTime - lastDisplayedTime > 0.5f) {
      niDebugFmt(("... playing: %gs", currentTime - startTime));
      lastDisplayedTime = currentTime;
    }
    if (currentTime - startTime > 25.0f) {
      niDebugFmt(("... play time timeout: %gs", ni::TimerInSeconds() - startTime));
      m_numSteps = 0;
    }
    if (!sndSource->GetIsPlaying()) {
      niDebugFmt(("... play time final: %gs", ni::TimerInSeconds() - startTime));
      m_numSteps = 0;
    }

    return eFalse;
  }
};
TEST_FIXTURE_WIDGET(FSoundFactoryWidget,AtmoWAV);

struct AtmoOGG : public ni::cWidgetSinkImpl<> {
  QPtr<iSoundFactory> mptrSoundFactory;
  Ptr<iSoundSource> sndSource;

  TEST_CONSTRUCTOR(AtmoOGG) {
    m_numSteps = 0xffffff;
  }

  tBool __stdcall OnSinkAttached() niImpl {
    mptrSoundFactory = niCreateInstance(niSound,SoundFactory,niVarNull,niVarNull);
    CHECK(mptrSoundFactory->StartupDriver(0,eSoundFormat_Stereo16,44100,NULL));

    Ptr<iFile> fpSound = ni::GetLang()->URLOpen("Test_niSound://atmo_loop_01.ogg");
    CHECK(mptrSoundFactory.IsOK());

    Ptr<iSoundBuffer> sndBuffer = mptrSoundFactory->CreateSoundBuffer(fpSound,eTrue,_H(fpSound->GetSourcePath()));
    CHECK(sndBuffer.IsOK());

    sndSource = mptrSoundFactory->CreateSoundSource(sndBuffer);
    CHECK(sndSource.IsOK());

    CHECK(sndSource->Play2D(1,5,0));
    return eTrue;
  }

  tBool __stdcall OnPaint(const sVec2f& avMousePos, iCanvas* apCanvas) niImpl {
    static tF64 lastDisplayedTime = ni::TimerInSeconds();
    static tF64 startTime = ni::TimerInSeconds();

    const tF64 currentTime = ni::TimerInSeconds();
    mptrSoundFactory->Update(ni::GetLang()->GetFrameTime());
    if (currentTime - lastDisplayedTime > 0.5f) {
      niDebugFmt(("... playing: %gs", currentTime - startTime));
      lastDisplayedTime = currentTime;
    }
    if (currentTime - startTime > 25.0f) {
      niDebugFmt(("... play time timeout: %gs", ni::TimerInSeconds() - startTime));
      m_numSteps = 0;
    }
    if (!sndSource->GetIsPlaying()) {
      niDebugFmt(("... play time final: %gs", ni::TimerInSeconds() - startTime));
      m_numSteps = 0;
    }

    return eFalse;
  }
};
TEST_FIXTURE_WIDGET(FSoundFactoryWidget,AtmoOGG);
