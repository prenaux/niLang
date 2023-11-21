#include "stdafx.h"
#include <niLang/Utils/Trace.h>

/*
  Some test result (on Win8, Core i7 3930K @ 4Ghz):

  Decoding atmo_loop_01.wav|ogg|opus

  DecodeOGG1 is using Tremor
  DecodeOGG2 is using STB

  // On PC
  [DecodeWAV]  [Decode] 0.00755 secs
  [DecodeOGG1] [Decode] 0.16129 secs
  [DecodeOGG2] [Decode] 0.04777 secs
  [DecodeOPUS] [Decode] 0.15731 secs

  // In Chrome
  [DecodeWAV]  [Decode] 0.02000 secs
  [DecodeOGG1] [Decode] 1.42200 secs
  [DecodeOGG2] [Decode] 0.35700 secs
  [DecodeOPUS] [Decode] 0.38700 secs

  // In FF
  [DecodeWAV]  [Decode] 0.00641 secs
  [DecodeOGG1] [Decode] 0.48208 secs
  [DecodeOGG2] [Decode] 0.11228 secs
  [DecodeOPUS] [Decode] 0.19236 secs

  // In IE11
  [DecodeWAV]  [Decode] 0.56863 secs
  [DecodeOGG1] [Decode] 4.32245 secs
  [DecodeOGG2] [Decode] 0.65347 secs
  [DecodeOPUS] [Decode] 1.55278 secs

 */

using namespace ni;

niDeclareModuleTrace_(Test_niSound,Play);
#define SHOULD_PLAY_SOUND niModuleShouldTrace_(Test_niSound,Play)

struct FSoundDecode {
  QPtr<iSoundFactory> mptrSoundFactory;

  FSoundDecode() {
    mptrSoundFactory = niCreateInstance(niSound,SoundFactory,niVarNull,niVarNull);
  }
  ~FSoundDecode() {
    mptrSoundFactory->Invalidate();
  }
};

TEST_FIXTURE(FSoundDecode,DecodeWAV) {
  CHECK_RETURN_IF_FAILED(mptrSoundFactory.IsOK());

  {
    Ptr<iFile> fpSound = ni::GetLang()->URLOpen("Test_niSound://atmo_loop_01.wav");
    CHECK_RETURN_IF_FAILED(fpSound.IsOK());

    Ptr<iSoundBuffer> sndBuffer;
    {
      TEST_TIMING_BEGIN(Decode);
      sndBuffer = mptrSoundFactory->CreateSoundBuffer(fpSound,eFalse,_H(fpSound->GetSourcePath()));
      TEST_TIMING_END();
    }
    CHECK_RETURN_IF_FAILED(sndBuffer.IsOK());
  }
}

TEST_FIXTURE(FSoundDecode,DecodeWAVEJUNK) {
  CHECK_RETURN_IF_FAILED(mptrSoundFactory.IsOK());

  {
    Ptr<iFile> fpSound = ni::GetLang()->URLOpen("Test_niSound://weapon_13_shoot_1.wav");
    CHECK_RETURN_IF_FAILED(fpSound.IsOK());

    Ptr<iSoundBuffer> sndBuffer;
    {
      TEST_TIMING_BEGIN(Decode);
      sndBuffer = mptrSoundFactory->CreateSoundBuffer(fpSound,eFalse,_H(fpSound->GetSourcePath()));
      TEST_TIMING_END();
    }
    CHECK_RETURN_IF_FAILED(sndBuffer.IsOK());
  }
}

TEST_FIXTURE(FSoundDecode,DecodeOGG) {
  CHECK_RETURN_IF_FAILED(mptrSoundFactory.IsOK());

  {
    Ptr<iFile> fpSound = ni::GetLang()->URLOpen("Test_niSound://atmo_loop_01.ogg");
    CHECK_RETURN_IF_FAILED(fpSound.IsOK());

    Ptr<iSoundBuffer> sndBuffer;
    {
      TEST_TIMING_BEGIN(Decode);
      sndBuffer = mptrSoundFactory->CreateSoundBuffer(fpSound,eFalse,_H(fpSound->GetSourcePath()));
      TEST_TIMING_END();
    }
    CHECK_RETURN_IF_FAILED(sndBuffer.IsOK());
  }
}
