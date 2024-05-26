#include "stdafx.h"
#include <niLang/Utils/Trace.h>

using namespace ni;

niDeclareModuleTrace_(Test_niSound,Play);
#define SHOULD_PLAY_SOUND niModuleShouldTrace_(Test_niSound,Play)

struct FMusic {
  QPtr<iSoundFactory> mptrSoundFactory;

  FMusic() {
    mptrSoundFactory = niCreateInstance(niSound,SoundFactory,niVarNull,niVarNull);
  }
  ~FMusic() {
    mptrSoundFactory->Invalidate();
  }

  void UpdateFor(const tF32 afMaxPlayTime) {
    tF64 lastDisplayedTime = ni::TimerInSeconds();
    const tF64 startTime = ni::TimerInSeconds();
    do {
      const tF64 currentTime = ni::TimerInSeconds();
      ni::GetLang()->UpdateFrameTime(currentTime);
      mptrSoundFactory->Update(ni::GetLang()->GetFrameTime());
      if (currentTime - lastDisplayedTime > 0.5f) {
        niDebugFmt(("... update time: %gs", currentTime - startTime));
        lastDisplayedTime = currentTime;
      }
      if (currentTime - startTime > afMaxPlayTime) {
        return;
      }
      SleepMs(10);
    } while(1);
  }

  tBool PlayMusic(TEST_PARAMS_FUNC, const tF32 afMaxPlayTime, Ptr<iSoundSource> sndSource, tF32 afSpeed = 1.0f) {
    tU32 numLoops = 0;
    CHECK(mptrSoundFactory->MusicPlay(sndSource,1,afSpeed,1));

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
      if (currentTime - startTime > afMaxPlayTime) {
        return eTrue;
      }
      SleepMs(10);
    } while(sndSource->GetIsPlaying());

    niDebugFmt(("... full play time: %gs", ni::TimerInSeconds() - startTime));

    CHECK_NOT_EQUAL(1,numLoops);
    CHECK(numLoops > 10);

    return eTrue;
  };
};

#if !defined NO_CONSOLE_AUDIO
TEST_FIXTURE(FMusic,Music1) {
  CHECK(mptrSoundFactory.IsOK());
  CHECK(mptrSoundFactory->StartupDriver(0,eSoundFormat_Stereo16,44100,0));

  {
    Ptr<iFile> fpSound = ni::GetLang()->URLOpen("Test_niSound://music_punch.ogg");
    CHECK_RETURN_IF_FAILED(mptrSoundFactory.IsOK());

    Ptr<iSoundBuffer> sndBuffer = mptrSoundFactory->CreateSoundBuffer(fpSound,eFalse,_H(fpSound->GetSourcePath()));
    CHECK_RETURN_IF_FAILED(sndBuffer.IsOK());

    Ptr<iSoundSource> sndSource = mptrSoundFactory->CreateSoundSource(sndBuffer);
    CHECK_RETURN_IF_FAILED(sndSource.IsOK());

    CHECK(PlayMusic(TEST_PARAMS_CALL, 2.6f, sndSource));
  }

  mptrSoundFactory->MusicStop(1.0f);
  UpdateFor(1.1f);
}

TEST_FIXTURE(FMusic,Music2) {
  CHECK(mptrSoundFactory.IsOK());
  CHECK(mptrSoundFactory->StartupDriver(0,eSoundFormat_Stereo16,44100,0));

  {
    Ptr<iFile> fpSound = ni::GetLang()->URLOpen("Test_niSound://music_punch.ogg");
    CHECK_RETURN_IF_FAILED(mptrSoundFactory.IsOK());

    Ptr<iSoundBuffer> sndBuffer = mptrSoundFactory->CreateSoundBuffer(fpSound,eFalse,_H(fpSound->GetSourcePath()));
    CHECK_RETURN_IF_FAILED(sndBuffer.IsOK());

    Ptr<iSoundSource> sndSource = mptrSoundFactory->CreateSoundSource(sndBuffer);
    CHECK_RETURN_IF_FAILED(sndSource.IsOK());

    CHECK(PlayMusic(TEST_PARAMS_CALL, 2.6f, sndSource));
  }

  {
    Ptr<iFile> fpSound = ni::GetLang()->URLOpen("Test_niSound://music_sketch8.ogg");
    CHECK_RETURN_IF_FAILED(mptrSoundFactory.IsOK());

    Ptr<iSoundBuffer> sndBuffer = mptrSoundFactory->CreateSoundBuffer(fpSound,eFalse,_H(fpSound->GetSourcePath()));
    CHECK_RETURN_IF_FAILED(sndBuffer.IsOK());

    Ptr<iSoundSource> sndSource = mptrSoundFactory->CreateSoundSource(sndBuffer);
    CHECK_RETURN_IF_FAILED(sndSource.IsOK());

    CHECK(PlayMusic(TEST_PARAMS_CALL, 2.6f, sndSource));
  }

  mptrSoundFactory->MusicStop(1.0f);
  UpdateFor(1.1f);
}
#endif
