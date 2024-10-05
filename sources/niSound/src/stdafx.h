#ifndef __STDAFX_61780673_H__
#define __STDAFX_61780673_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niSound.h"

using namespace ni;

#if defined niWindows
iSoundDriver* __stdcall New_SoundDriverWaveOut();

#elif defined niJSCC
iSoundDriver* __stdcall New_SoundDriverSDL();

#elif defined niAndroid
#define NO_SOUND
// iSoundDriver* __stdcall New_SoundDriverJNI();

#elif defined niIOS || defined niOSX
iSoundDriver* __stdcall New_SoundDriverOSX();

#elif defined niLinuxDesktop
iSoundDriver* __stdcall New_SoundDriverALSA();

#elif defined niQNX || defined niLinuxDesktop
#define NO_SOUND

#else
#error "Platform not supported."

#endif

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __STDAFX_61780673_H__
