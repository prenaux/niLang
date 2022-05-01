#pragma once
#ifndef __PROCESSUTILS_H_19DABF7F_719B_4E37_8DE8_ABA02E9B15F5__
#define __PROCESSUTILS_H_19DABF7F_719B_4E37_8DE8_ABA02E9B15F5__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../IOSProcess.h"

#ifdef niWindows
#define niExeExtension ".exe"
#else
#define niExeExtension ""
#endif

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

struct sRunProcessResult {
  sRunProcessResult() : succeeded(0), exitCode(-1) {}
  tI32 succeeded;
  tI32 exitCode;
  cString strStdout;
};

inline sRunProcessResult RunProcess(iOSProcessManager* apProcMan, const achar* aaszCmd,
                                    iFile* apStdOut = NULL, ThreadMutex* apSyncStdOutMutex = NULL,
                                    tU32 anTimeoutMs = eInvalidHandle)
{
  sRunProcessResult res;
  niCheckIsOK(apProcMan, res);

  Ptr<iOSProcess> proc = apProcMan->SpawnProcess(aaszCmd, eOSProcessSpawnFlags_StdFiles);
  niCheckIsOK(proc,res);

  for (;;) {
    Ptr<iFile> procStdOut = proc->GetFile(eOSProcessFile_StdOut);
    tU32 validCount = 0;
    if (procStdOut.IsOK() && !procStdOut->GetPartialRead()) {
      ++validCount;
      cString line = procStdOut->ReadStringLine();
      if (apStdOut) {
        if (apSyncStdOutMutex) {
          apSyncStdOutMutex->ThreadLock();
        }
        if (!line.empty()) {
          apStdOut->WriteString(line.Chars());
        }
        apStdOut->WriteString("\n");
        if (apSyncStdOutMutex) {
          apSyncStdOutMutex->ThreadUnlock();
        }
      }
      else {
        res.strStdout += line + "\n";
      }
    }
    if (validCount == 0)
      break;
  }

  sVec2i r = proc->WaitForExitCode(anTimeoutMs);
  res.succeeded = r.x;
  res.exitCode = r.y;
  return res;
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
}
#endif // __PROCESSUTILS_H_19DABF7F_719B_4E37_8DE8_ABA02E9B15F5__
