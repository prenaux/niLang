#pragma once
#ifndef __IOSPROCESS_50BD34A2_37BC_4088_9518_47212EC83EE5_H__
#define __IOSPROCESS_50BD34A2_37BC_4088_9518_47212EC83EE5_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "Types.h"

#if niMinFeatures(15)
#include "Utils/CollectionImpl.h"

namespace ni {
struct iRegex;
struct iFile;

/** \addtogroup niLang
 * @{
 */

//! OS Process spawning flags.
enum eOSProcessSpawnFlags
{
  //! No flags/default
  eOSProcessSpawnFlags_None = 0,
  //! Create stdin/out/err.
  //! \remark Indicates that we want to redirect stdin/ou/err to our own
  //!         handles. The implication is that we can and must drain
  //!         stdout/err to retrieve its output.
  eOSProcessSpawnFlags_StdFiles = niBit(0),
  //! Indicates that we want different pipes for StdOut and StdErr.
  //! \remark This means that both stdout and stderr need to be drained.
  eOSProcessSpawnFlags_DifferentStdOutAndStdErr = niBit(1),
  //! Create the process in sand boxed environment.
  //! \remark A process in a SandBox will have limited rights to access sensitive informations in the OS.
  //! \remark Not all OS's are capable of creating sandboxed processes.
  eOSProcessSpawnFlags_SandBox = niBit(2),
  //! Create the process an independant process.
  //! \remark This is needed to launch other applications independantly, such as in the case of a launcher starting another app.
  eOSProcessSpawnFlags_Detached = niBit(3),
  //! \internal
  eOSProcessSpawnFlags_ForceDWORD = 0xFFFFFFFF
};

//! OS Process spawning flags type. \see ni::eOSProcessSpawnFlags
typedef tU32 tOSProcessSpawnFlags;

//! OS Process standard file objects.
enum eOSProcessFile
{
  //! Stdin file object.
  eOSProcessFile_StdIn = 0,
  //! Stdout file object.
  eOSProcessFile_StdOut = 1,
  //! Stderr file object.
  eOSProcessFile_StdErr = 2,
  //! \internal
  eOSProcessFile_Last = 3,
  //! \internal
  eOSProcessFile_ForceDWORD = 0xFFFFFFFF
};

//! OS Process object interface.
struct iOSProcess : public iUnknown {
  niDeclareInterfaceUUID(iOSProcess,0xf297a4c9,0x1c07,0x4e6e,0xb8,0x04,0x23,0x36,0xf8,0x76,0x0f,0xad);

  //! Get the path of the executable from which the process has been spawned.
  //! {Property}
  //! \remark Might be empty.
  virtual const ni::achar* __stdcall GetExePath() const = 0;
  //! Get the command line used to spawn the process.
  //! {Property}
  //! \remark Might be empty.
  virtual const ni::achar* __stdcall GetCommandLine() const = 0;

  //! Get the process's PID.
  //! {Property}
  virtual tInt __stdcall GetPID() const = 0;

  //! Get the parent process's PID.
  //! {Property}
  virtual tInt __stdcall GetParentPID() const = 0;
  //! Check whether the parent process is a parent of this process.
  //! \return The distance of the parent, 1 is the direct parent, 2
  //!         is the parent of the parent, and so on. Returns 0 if the
  //!         specified PID is not the parent of this process.
  virtual tU32 __stdcall IsParentProcess(tInt aParentPID) const = 0;

  //! Get whether the process is the current process.
  //! {Property}
  virtual tBool __stdcall GetIsCurrent() const = 0;

  //! Get the termination status (exit code) of the process and return true
  //! if the status indicates that the process crashed.
  //! {Property}
  virtual tBool __stdcall GetDidCrash() const = 0;

  //! Attempts to kill the process, giving it a specified exit code.
  //! \param  anExitCode the exit code to give to the process once terminated.
  //! \param abWait: if true wait for the process to be actually be terminated before returning.
  //! \return true if successful, false otherwise.
  virtual tBool __stdcall Kill(tInt anExitCode, tBool abWait) = 0;

  //! Wait for the process to exit.
  //! \return true if the process exited cleanly within the given time limit.
  virtual tBool __stdcall Wait(tU32 anTimeoutMs) = 0;
  //! Waits for process to exit. In POSIX systems, if the process hasn't been
  //! signaled then returns the exit code; otherwise it's considered a
  //! failure. On Windows the exit code is always returned.
  //! \return 'x' is true the process exited cleanly within the given time
  //!         limit, false otherwise. 'y' is the process's exit code.
  virtual sVec2i __stdcall WaitForExitCode(tU32 anTimeoutMs) = 0;

  //! Terminates the process with extreme prejudice. The given result code will
  //! be the exit code of the process. If the process has already exited, this
  //! will do nothing.
  virtual void __stdcall Terminate(tInt aResultCode) = 0;

  //! Get a standard file object beloging to the process.
  //! {Property}
  virtual iFile* __stdcall GetFile(eOSProcessFile aFile) const = 0;
};

//! OS Process enum sink.
struct iOSProcessEnumSink : public iUnknown {
  niDeclareInterfaceUUID(iOSProcessEnumSink,0xc3edac6c,0xa769,0x441d,0x89,0x0c,0xab,0x4c,0xe2,0x99,0xd2,0x3c);

  //! Called when a process has been found.
  //! \return false to interrupt the enumeration, true to continue.
  virtual ni::tBool __stdcall OnOSProcessEnumSink(tIntPtr aPID, tIntPtr aParentPID, const ni::achar* aaszExeName) = 0;
};

//! OS Process Manager
struct iOSProcessManager : public iUnknown {
  niDeclareInterfaceUUID(iOSProcessManager,0xd396e429,0x444e,0x46ff,0xa2,0xa4,0xbf,0x66,0x4b,0x32,0x80,0x90);

  //! Return the id of the current process.
  //! {Property}
  virtual tInt __stdcall GetCurrentProcessID() const = 0;
  //! Return a process object of the current process.
  //! {Property}
  virtual iOSProcess* __stdcall GetCurrentProcess() const = 0;
  //! Create a process object from the specified process id.
  virtual iOSProcess* __stdcall CreateProcess(tInt aPID) = 0;

  //! Spawn a new process. \see ni::iOSProcessManager::SpawnProcessEx
  virtual iOSProcess* __stdcall SpawnProcess(const ni::achar* aaszCmdLine, tOSProcessSpawnFlags aSpawn) = 0;
  //! Spawn a new process with the specified working directory and environment variables.
  //! \param aaszCmdLine the command line of the process to spawn.
  //! \param aaszWorkDir if not NULL specifies the working directory from where the process will start.
  //! \param apEnvs if not NULL specifies the environment variables that will be set for the new process.
  //! \param aSpawn the spawn flags. \see ni::eOSProcessSpawnFlags
  virtual iOSProcess* __stdcall SpawnProcessEx(
      const ni::achar* aaszCmdLine, const achar* aaszWorkDir,
      const tStringCMap* apEnvs, tOSProcessSpawnFlags aSpawn) = 0;

  //! Enumerates all processes.
  //! \return The number of processes enumerated.
  //! \remark If apSink is NULL returns the number of processes that passed the filter.
  virtual tU32 __stdcall EnumProcesses(ni::iRegex* apFilter, iOSProcessEnumSink* apSink) = 0;

  //! Get the current working directory.
  //! {Property}
  virtual cString __stdcall GetCwd() const = 0;

  //! Get the current environment variables.
  //! {Property}
  virtual Ptr<tStringCMap> __stdcall GetEnviron() const = 0;
};

#ifndef niNoProcess
//! Get the OS process manager.
niExportFunc(ni::iOSProcessManager*) GetOSProcessManager();
#endif

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // #if niMinFeatures(15)
#endif // __IOSPROCESS_50BD34A2_37BC_4088_9518_47212EC83EE5_H__
