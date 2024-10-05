#include "stdafx.h"
#include <niLang/Utils/Trace.h>

using namespace ni;

niDeclareModuleTrace_(Test_niSound,Play);
#define SHOULD_PLAY_SOUND niModuleShouldTrace_(Test_niSound,Play)

struct FSoundFactory {
  QPtr<iSoundFactory> mptrSoundFactory;

  FSoundFactory() {
    mptrSoundFactory = niCreateInstance(niSound,SoundFactory,niVarNull,niVarNull);
  }
  ~FSoundFactory() {
    mptrSoundFactory->Invalidate();
  }

  tBool PlayOnce(TEST_PARAMS_FUNC, const tF32 afMaxPlayTime, Ptr<iSoundSource> sndSource, tF32 afSpeed = 1.0f) {
    tU32 numLoops = 0;
    CHECK(sndSource->Play2D(1,afSpeed,0));

    tF64 lastDisplayedTime = ni::TimerInSeconds();
    const tF64 startTime = ni::TimerInSeconds();
    do {
      const tF64 currentTime = ni::TimerInSeconds();
      ni::GetLang()->UpdateFrameTime(currentTime);
      mptrSoundFactory->Update(ni::GetLang()->GetFrameTime());
      ++numLoops;
      if (currentTime - lastDisplayedTime > 0.5f) {
        niDebugFmt(("... play time: %gs", currentTime - startTime));
        lastDisplayedTime = currentTime;
      }
      if ((currentTime - startTime) > afMaxPlayTime) {
        niDebugFmt(("... play time timeout: %gs", ni::TimerInSeconds() - startTime));
        return eFalse;
      }
      SleepMs(10);
    } while(sndSource->GetIsPlaying());

    niDebugFmt(("... full play time: %gs, loops: %d", ni::TimerInSeconds() - startTime, numLoops));

    CHECK_NOT_EQUAL(1,numLoops);
    CHECK(numLoops > 5);

    return eTrue;
  };
};

TEST_FIXTURE(FSoundFactory,Base) {
  CHECK(mptrSoundFactory.IsOK());
  CHECK(mptrSoundFactory->StartupDriver(eInvalidHandle,eSoundFormat_Stereo16,44100,0));
}

TEST_FIXTURE(FSoundFactory,Silent_ClickWAV) {
  CHECK(mptrSoundFactory.IsOK());

  niLet silentDriverIndex = mptrSoundFactory->GetDriverIndex(_H("Silent"));
  CHECK(mptrSoundFactory->StartupDriver(silentDriverIndex,eSoundFormat_Stereo16,44100,0));

  Ptr<iSoundDriver> activeDriver = mptrSoundFactory->GetDriver(mptrSoundFactory->GetActiveDriver());
  CHECK_RETURN_IF_FAILED(activeDriver.IsOK());
  CHECK_RETURN_IF_FAILED(activeDriver->GetName() == _H("Silent"));

  {
    Ptr<iFile> fpSound = ni::GetLang()->URLOpen("Test_niSound://click.wav");
    CHECK_RETURN_IF_FAILED(mptrSoundFactory.IsOK());

    Ptr<iSoundBuffer> sndBuffer = mptrSoundFactory->CreateSoundBuffer(fpSound,eFalse,_H(fpSound->GetSourcePath()));
    CHECK_RETURN_IF_FAILED(sndBuffer.IsOK());

    Ptr<iSoundSource> sndSource = mptrSoundFactory->CreateSoundSource(sndBuffer);
    CHECK_RETURN_IF_FAILED(sndSource.IsOK());

    niLoop(i,3) {
      CHECK(PlayOnce(TEST_PARAMS_CALL, 0.5, sndSource));
    }
  }
}

#if !defined NO_CONSOLE_AUDIO
TEST_FIXTURE(FSoundFactory,ClickWAV) {
  CHECK(mptrSoundFactory.IsOK());
  CHECK(mptrSoundFactory->StartupDriver(eInvalidHandle,eSoundFormat_Stereo16,44100,0));

  {
    Ptr<iFile> fpSound = ni::GetLang()->URLOpen("Test_niSound://click.wav");
    CHECK_RETURN_IF_FAILED(mptrSoundFactory.IsOK());

    Ptr<iSoundBuffer> sndBuffer = mptrSoundFactory->CreateSoundBuffer(fpSound,eFalse,_H(fpSound->GetSourcePath()));
    CHECK_RETURN_IF_FAILED(sndBuffer.IsOK());

    Ptr<iSoundSource> sndSource = mptrSoundFactory->CreateSoundSource(sndBuffer);
    CHECK_RETURN_IF_FAILED(sndSource.IsOK());

    niLoop(i,3) {
      CHECK(PlayOnce(TEST_PARAMS_CALL, 0.5, sndSource));
    }
  }
}

TEST_FIXTURE(FSoundFactory,Mono8WAV) {
  CHECK(mptrSoundFactory.IsOK());
  CHECK(mptrSoundFactory->StartupDriver(eInvalidHandle,eSoundFormat_Stereo16,44100,0));

  {
    Ptr<iFile> fpSound = ni::GetLang()->URLOpen("Test_niSound://lobby_chat.wav");
    CHECK_RETURN_IF_FAILED(mptrSoundFactory.IsOK());

    Ptr<iSoundBuffer> sndBuffer = mptrSoundFactory->CreateSoundBuffer(fpSound,eFalse,_H(fpSound->GetSourcePath()));
    CHECK_RETURN_IF_FAILED(sndBuffer.IsOK());

    CHECK_EQUAL(eSoundFormat_Mono8, sndBuffer->GetFormat());
    CHECK_EQUAL(22050, sndBuffer->GetFrequency());

    Ptr<iSoundSource> sndSource = mptrSoundFactory->CreateSoundSource(sndBuffer);
    CHECK_RETURN_IF_FAILED(sndSource.IsOK());

    niLoop(i,3) {
      CHECK(PlayOnce(TEST_PARAMS_CALL, 1.0, sndSource));
    }
  }
}

TEST_FIXTURE(FSoundFactory,WAVEJUNK) {
  CHECK(mptrSoundFactory.IsOK());
  CHECK(mptrSoundFactory->StartupDriver(eInvalidHandle,eSoundFormat_Stereo16,44100,0));

  {
    Ptr<iFile> fpSound = ni::GetLang()->URLOpen("Test_niSound://weapon_13_shoot_1.wav");
    CHECK_RETURN_IF_FAILED(mptrSoundFactory.IsOK());

    Ptr<iSoundBuffer> sndBuffer = mptrSoundFactory->CreateSoundBuffer(fpSound,eFalse,_H(fpSound->GetSourcePath()));
    CHECK_RETURN_IF_FAILED(sndBuffer.IsOK());

    CHECK_EQUAL(eSoundFormat_Stereo16, sndBuffer->GetFormat());
    CHECK_EQUAL(44100, sndBuffer->GetFrequency());

    Ptr<iSoundSource> sndSource = mptrSoundFactory->CreateSoundSource(sndBuffer);
    CHECK_RETURN_IF_FAILED(sndSource.IsOK());

    niLoop(i,3) {
      CHECK(PlayOnce(TEST_PARAMS_CALL, 1.0, sndSource));
    }
  }
}

TEST_FIXTURE(FSoundFactory,ClickOGG) {
  CHECK(mptrSoundFactory.IsOK());
  CHECK(mptrSoundFactory->StartupDriver(eInvalidHandle,eSoundFormat_Stereo16,44100,0));

  {
    Ptr<iFile> fpSound = ni::GetLang()->URLOpen("Test_niSound://click.ogg");
    CHECK_RETURN_IF_FAILED(mptrSoundFactory.IsOK());

    Ptr<iSoundBuffer> sndBuffer = mptrSoundFactory->CreateSoundBuffer(fpSound,eFalse,_H(fpSound->GetSourcePath()));
    CHECK_RETURN_IF_FAILED(sndBuffer.IsOK());

    Ptr<iSoundSource> sndSource = mptrSoundFactory->CreateSoundSource(sndBuffer);
    CHECK_RETURN_IF_FAILED(sndSource.IsOK());

    niLoop(i,3) {
      CHECK(PlayOnce(TEST_PARAMS_CALL, 0.5, sndSource));
    }
  }
}

TEST_FIXTURE(FSoundFactory,StreamWAV) {
  CHECK(mptrSoundFactory.IsOK());
  CHECK(mptrSoundFactory->StartupDriver(eInvalidHandle,eSoundFormat_Stereo16,44100,0));

  {
    Ptr<iFile> fpSound = ni::GetLang()->URLOpen("Test_niSound://atmo_loop_01.wav");
    CHECK_RETURN_IF_FAILED(mptrSoundFactory.IsOK());

    Ptr<iSoundBuffer> sndBuffer;
    {
      TEST_TIMING_BEGIN(Decode);
      sndBuffer = mptrSoundFactory->CreateSoundBuffer(fpSound,eTrue,_H(fpSound->GetSourcePath()));
      TEST_TIMING_END();
    }
    CHECK_RETURN_IF_FAILED(sndBuffer.IsOK());

    Ptr<iSoundSource> sndSource = mptrSoundFactory->CreateSoundSource(sndBuffer);
    CHECK_RETURN_IF_FAILED(sndSource.IsOK());

    if (SHOULD_PLAY_SOUND) {
      CHECK(PlayOnce(TEST_PARAMS_CALL, 3, sndSource, 10.0f));
    }
  }
}

TEST_FIXTURE(FSoundFactory,StreamOGG) {
  CHECK(mptrSoundFactory.IsOK());
  CHECK(mptrSoundFactory->StartupDriver(eInvalidHandle,eSoundFormat_Stereo16,44100,0));

  {
    Ptr<iFile> fpSound = ni::GetLang()->URLOpen("Test_niSound://atmo_loop_01.ogg");
    CHECK_RETURN_IF_FAILED(mptrSoundFactory.IsOK());

    Ptr<iSoundBuffer> sndBuffer;
    {
      TEST_TIMING_BEGIN(Decode);
      sndBuffer = mptrSoundFactory->CreateSoundBuffer(fpSound,eTrue,_H(fpSound->GetSourcePath()));
      TEST_TIMING_END();
    }
    CHECK_RETURN_IF_FAILED(sndBuffer.IsOK());

    Ptr<iSoundSource> sndSource = mptrSoundFactory->CreateSoundSource(sndBuffer);
    CHECK_RETURN_IF_FAILED(sndSource.IsOK());

    if (SHOULD_PLAY_SOUND) {
      CHECK(PlayOnce(TEST_PARAMS_CALL, 3, sndSource, 10.0f));
    }
  }
}
#endif
