#pragma once
#ifndef __NIAPPLIBCONSOLE_H_F3321484_AFBF_7F4A_8033_62F8E236E019__
#define __NIAPPLIBCONSOLE_H_F3321484_AFBF_7F4A_8033_62F8E236E019__

#include <niLang/Utils/CmdLine.h>
#include <niLang/Utils/ConcurrentImpl.h>
#include <niLang/STL/vector.h>
#include <niLang/STL/utils.h>

namespace app {

//! Send a notification to application host.
extern void AppNotifyHost(const char* aCmd);

void AppConsoleLoop(const ni::achar* aAppName,
                    ni::Ptr<ni::iRunnable> aLoop,
                    ni::Ptr<ni::iRunnable> aShutdown);

}
#endif // __NIAPPLIBCONSOLE_H_F3321484_AFBF_7F4A_8033_62F8E236E019__
