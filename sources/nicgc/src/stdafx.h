#pragma once
#ifndef __STDAFX_H_7F51E070_4174_9B4A_AE37_18D637FFB83E__
#define __STDAFX_H_7F51E070_4174_9B4A_AE37_18D637FFB83E__
#pragma once

#include <niLang.h>

#define CHECK_ARGV(I,MSG) if ((I) >= argc) { ErrorHelp("Missing argument: " MSG); }

#ifdef niDebug
#define TRACE_CMD_LINE(MSG) niDebugFmt(MSG)
#else
#define TRACE_CMD_LINE(MSG)
#endif

#endif // __STDAFX_H_7F51E070_4174_9B4A_AE37_18D637FFB83E__
