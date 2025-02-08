#pragma once
#ifndef __MAINIMPL_H_FE396F86_3119_4BA8_8CF6_A007B4DA8BBE__
#define __MAINIMPL_H_FE396F86_3119_4BA8_8CF6_A007B4DA8BBE__

#include <niLang/Utils/CrashReport.h>
#if !defined niMainImpl_NoCrashReportHandler
niCrashReport_DeclareHandler();
#endif

#include <niLang/Utils/Asan.h>
#if !defined niMainImpl_NoAsanOptions
niAsanDefaultOptions();
#endif

#ifdef niWindows
#include "../Platforms/Win32/Win32_DelayLoadImpl.h"
#endif

#endif // __MAINIMPL_H_FE396F86_3119_4BA8_8CF6_A007B4DA8BBE__
