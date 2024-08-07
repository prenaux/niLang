#include "stdafx.h"

#include "../src/API/niLang/IOSProcess.h"
#include "../src/API/niLang/IFile.h"
#include "../src/API/niLang/IRegex.h"
#include "../src/API/niLang/Utils/TimerSleep.h"

using namespace ni;

class cFilePosTracker : public ImplRC<iFileBase,eImplFlags_Default>
{
 public:
  cFilePosTracker(iFileBase* apBase)
    : mBase(apBase)
  {
    mnPos = 0;
  }

  ~cFilePosTracker() {
  }

  //// iFile ////////////////////////////////
  inline tFileFlags __stdcall GetFileFlags() const {
    return mBase->GetFileFlags();
  }

  inline tBool  __stdcall Seek(tI64 offset) {
    if (mBase->Seek(offset)) {
      mnPos += offset;
      return eTrue;
    }
    else {
      return eFalse;
    }
  }
  inline tBool  __stdcall SeekSet(tI64 offset) {
    if (mBase->SeekSet(offset)) {
      mnPos = offset;
      return eTrue;
    }
    else {
      return eFalse;
    }
  }
  inline tBool __stdcall SeekEnd(tI64 offset) {
    if (mBase->SeekEnd(offset)) {
      const tSize size = this->GetSize();
      mnPos = (offset < size) ? (size - offset) : 0;
      return eTrue;
    }
    else {
      return eFalse;
    }
  }

  inline tSize  __stdcall ReadRaw(void* pOut, tSize nSize) {
    const tSize read = mBase->ReadRaw(pOut,nSize);
    mnPos += read;
    return read;
  }
  inline tSize  __stdcall WriteRaw(const void* apIn, tSize nSize) {
    const tSize written = mBase->WriteRaw(apIn,nSize);
    mnPos += written;
    return written;
  }

  inline tI64 __stdcall Tell() {
    return mnPos;
  }
  inline tI64 __stdcall GetSize() const {
    return mBase->GetSize();
  }
  inline const achar* __stdcall GetSourcePath() const {
    return NULL;
  }

  inline tBool __stdcall Flush()  {
    mBase->Flush();
    return eTrue;
  }

  inline tBool __stdcall GetTime(eFileTime aFileTime, iTime* apTime) const {
    return mBase->GetTime(aFileTime,apTime);
  }
  inline tBool __stdcall SetTime(eFileTime aFileTime, const iTime* apTime) {
    return mBase->SetTime(aFileTime,apTime);
  }
  inline tBool __stdcall Resize(tI64 newSize) {
    return mBase->Resize(newSize);
  }

 protected:
  ni::Nonnull<iFileBase> mBase;
  tI64 mnPos;
};

//--------------------------------------------------------------------------------------------
//
//  Child process main
//
//--------------------------------------------------------------------------------------------
#if !defined niNoProcess

#include "../src/Platform.h"

/*

  A Reader0 -> B Writer0 -> B Reader1 -> A Writer1

*/

static const int kPrintLoopCount = 10;

#define RUN_IN_SIGNAL_HANDLER_V(PROC,ARGC,ARGV)       \
  __try { return PROC(ARGC,ARGV); }                   \
  __except(EXCEPTION_EXECUTE_HANDLER) { return -1; }

int ChildProcessMain_Process__(int argc, const ni::achar** argv) {
  cString mode = (argc >= 3) ? argv[2] : _A("not specified");
  Ptr<iOSProcessManager> _pman;
  _pman = ni::GetOSProcessManager();
  Ptr<iOSProcess> cur = _pman->GetCurrentProcess();
  Ptr<iFile> fpOut = cur->GetFile(eOSProcessFile_StdOut);
  Ptr<iFile> fpErr = cur->GetFile(eOSProcessFile_StdErr);

  fpErr->WriteString(niFmt("TESTCHILD Started in mode '%s'.\n",mode));
  if (mode == _A("regular")) {
  }
  else if (mode == _A("hang")) {
    fpErr->WriteString("... hang ...\n");
    while (1) {}
    fpErr->WriteString("... hang done ...\n");
  }
  else if (mode == _A("stdfiles")) {
    Ptr<iFile> fpIn = cur->GetFile(eOSProcessFile_StdIn);

    // simple task, just uppercase whathever is sent over...
    tU32 c = 0;
    while (++c < 10) {
      cString str = fpIn->ReadString();
      // log to stderr
      fpErr->WriteStringZ(niFmt("CHILD RECEIVED: %s\n",str));
      fpErr->Flush();
      str.ToUpper();
      // write result to stdin
      fpOut->WriteStringZ(str.Chars());
      fpOut->Flush();
      if (str == _A("EXIT"))
        break;
    }
  }
  else if (mode == _A("wait")) {
    cString strWaitMs = (argc >= 4) ? argv[3] : _A("100");
    tU32 waitMs = ni::Min(strWaitMs.Long(),5000);
    ni::SleepMs(waitMs);
  }
  else if (mode == _A("crash")) {
    tU8* overflowed = (tU8*)0x0; // Do it at zero otherwise the process may
    // hang while exiting because the allocator gets corrupted.
    // Could happen at zero as-well, however the memory protection
    // should be enforced earlier since zero is also a NULL
    // pointer which is supposed to always be invalid.
    tU8* p = overflowed;
    niLoop(i,1024*1024*1024) {
      *p++ = (tU8)i;
    }
    ni::GetStdOut()->WriteString((char*)p);
  }
  else if (mode == _A("printloop")) {
    niLoop(i,kPrintLoopCount) {
      fpOut->WriteString(niFmt("... TEXT %d\n", i));
      ni::SleepMs(100);
    }
  }

  fpOut->WriteString("Done.\n");
  return 0;
}
int ChildProcessMain_Process(int argc, const ni::achar** argv) {
  //     RUN_IN_SIGNAL_HANDLER_V(ChildProcessMain_Process__,argc,argv);
  return ChildProcessMain_Process__(argc,argv);
}


bool Test_ChildProcess_Start(int argc, const char** argv, int& ret) {
  if (argc >= 2 && ni::StrCmp(_A("__Process"),argv[1]) == 0) {
    fprintf(stdout,"# Process test mode [PID:%d]\n",ni::GetOSProcessManager()->GetCurrentProcessID());
    fflush(stdout);
    ret = ChildProcessMain_Process(argc,argv);
  }
  else {
    return false;
  }
  return true;
}
#endif

//--------------------------------------------------------------------------------------------
//
//  Tests
//
//--------------------------------------------------------------------------------------------
struct FProcess {
  Ptr<iOSProcessManager> _pman;
  FProcess() {
    _pman = ni::GetOSProcessManager();
  }
  ~FProcess() {
  }
};

TEST_FIXTURE(FProcess,Manager) {
  CHECK(_pman.IsOK());
}

TEST_FIXTURE(FProcess,ThisProcess) {
  Ptr<iOSProcess> cur = _pman->GetCurrentProcess();
  CHECK_RETURN_IF_FAILED(cur.IsOK());
  cString o;
  o << _A("## THIS PROCESS ##\n");
  o << _A("- ExePath: ") << cur->GetExePath() << AEOL;
  o << _A("- CmdLine: ") << cur->GetCommandLine() << AEOL;
  TEST_PRINT(o.Chars());
  CHECK_EQUAL(kTrue,cur->GetIsCurrent());
}

TEST_FIXTURE(FProcess,ThisFiles) {
  Ptr<iOSProcess> cur = _pman->GetCurrentProcess();
  CHECK(cur.IsOK());
  Ptr<iFile> fpIn = cur->GetFile(eOSProcessFile_StdIn);
  CHECK_RETURN_IF_FAILED(fpIn.IsOK());
  Ptr<iFile> fpOut = cur->GetFile(eOSProcessFile_StdOut);
  CHECK_RETURN_IF_FAILED(fpOut.IsOK());
  Ptr<iFile> fpErr = cur->GetFile(eOSProcessFile_StdErr);
  CHECK_RETURN_IF_FAILED(fpErr.IsOK());

  CHECK(niFlagIs(fpIn->GetFileFlags(),eFileFlags_Read));
  CHECK(niFlagIsNot(fpIn->GetFileFlags(),eFileFlags_Write));
  CHECK(niFlagIs(fpOut->GetFileFlags(),eFileFlags_Write));
  CHECK(niFlagIsNot(fpOut->GetFileFlags(),eFileFlags_Read));
  CHECK(niFlagIs(fpErr->GetFileFlags(),eFileFlags_Write));
  CHECK(niFlagIsNot(fpErr->GetFileFlags(),eFileFlags_Read));

  fpOut->WriteString(_A("StdOut: Hello World from ThisProcess file handle !\n"));
  fpOut->Flush();

  fpErr->WriteString(_A("StdErr: Hello World from ThisProcess file handle !\n"));
  fpErr->Flush();
}

TEST_FIXTURE(FProcess,Environ) {
  // astl::map<cString,cString>* environ;
  Ptr<tStringCMap> envs = _pman->GetEnvs();
  // for (auto it : *envs) {
    // niDebugFmt(("... %s = %s", it.first, it.second));
  // }
  cString path = astl::get_default(*envs,"PATH","");
  CHECK(path.IsNotEmpty());
}

TEST_FIXTURE(FProcess,Cwd) {
  cString cwd = _pman->GetCwd();
  // niDebugFmt(("... cwd: %s", cwd));
  CHECK(cwd.IsNotEmpty());
  cString oa = niFmt("%s-%s",
                     ni::GetProperty("ni.loa.osx"),
                     ni::GetProperty("ni.loa.arch"));
  CHECK(cwd.EndsWith(oa.Chars()));
}

#if !defined niNoProcess
TEST_FIXTURE(FProcess,EnumAll) {
  struct MyEnum : public ImplLocal<iOSProcessEnumSink> {
    TEST_PARAMS_DECL;
    tU32 _count;
    MyEnum(TEST_PARAMS_FUNC) : TEST_PARAMS_CONS, _count(0) {}

    virtual ni::tBool __stdcall OnOSProcessEnumSink(tIntPtr aPID, tIntPtr aParentPID, const ni::achar* aaszExeName) {
      cString o;
      o << "[" << _count++ << "]: "
        << "pid = " << (tI64)aPID << ", "
        << "ppid = " << (tI64)aParentPID << ", "
        << aaszExeName;
      niPrintln(o.Chars());
      return ni::eTrue;
    }
  } myEnum(TEST_PARAMS_CALL);

  niDebugFmt((_A("- Processes: EnumAll - ")));
  tU32 numProcesses = _pman->EnumProcesses(NULL,&myEnum);
  CHECK(numProcesses > 10); // fair to assume that at least 10 procs will be running on any modern os..
}

TEST_FIXTURE(FProcess,EnumTestExe) {
  struct MyEnum : public ImplLocal<iOSProcessEnumSink> {
    TEST_PARAMS_DECL;
    tU32 _count;
    MyEnum(TEST_PARAMS_FUNC) : TEST_PARAMS_CONS, _count(0) {}

    virtual ni::tBool __stdcall OnOSProcessEnumSink(tIntPtr aPID, tIntPtr aParentPID, const ni::achar* aaszExeName) {
      cString o;
      o << "[" << _count++ << "]: "
        << "pid = " << (tI64)aPID << ", "
        << "ppid = " << (tI64)aParentPID << ", "
        << aaszExeName;
      niPrintln(o.Chars());
      return ni::eTrue;
    }
  } myEnum(TEST_PARAMS_CALL);

  niDebugFmt((_A("- Processes: EnumTestExe - ")));
  Ptr<iRegex> filter = ni::CreateFilePatternRegex(_A("Test_niLang_*"),NULL);
  const tBool atLeastOne_Test_niLang = (_pman->EnumProcesses(filter,&myEnum) >= 1);
  CHECK_EQUAL(eTrue,atLeastOne_Test_niLang);
}

TEST_FIXTURE(FProcess,Spawn) {
  AUTO_WARNING_MODE();

  Ptr<iOSProcess> cur = _pman->GetCurrentProcess();
  CHECK(cur.IsOK());

  cString cmdLine;
  cmdLine << cur->GetExePath();
  cmdLine << " __Process regular";
  Ptr<iOSProcess> spawned = _pman->SpawnProcess(
      cmdLine.Chars(),
      eOSProcessSpawnFlags_None);
  CHECK(spawned.IsOK());
  if (spawned.IsOK()) {
    CHECK(spawned->Wait(3000));
  }
}

TEST_FIXTURE(FProcess,Spawn5) {
  AUTO_WARNING_MODE();
  Ptr<iOSProcess> cur = _pman->GetCurrentProcess();
  CHECK(cur.IsOK());

  cString cmdLine;
  cmdLine << cur->GetExePath();
  cmdLine << " __Process regular";
  Ptr<iOSProcess> spawned[5];

  niLoop(i,niCountOf(spawned)) {
    spawned[i] = _pman->SpawnProcess(
        cmdLine.Chars(),
        eOSProcessSpawnFlags_None);
    CHECK_RETURN_IF_FAILED(spawned[i].IsOK());
  }
  niLoop(i,niCountOf(spawned)) {
    CHECK(spawned[i]->Wait(3000));
  }
}

TEST_FIXTURE(FProcess,SpawnWait) {
  AUTO_WARNING_MODE();

  Ptr<iOSProcess> cur = _pman->GetCurrentProcess();
  CHECK(cur.IsOK());

  cString cmdLine;
  cmdLine << cur->GetExePath();
  cmdLine << " __Process wait 500";
  Ptr<iOSProcess> spawned = _pman->SpawnProcess(
      cmdLine.Chars(),
      eOSProcessSpawnFlags_None);
  CHECK_RETURN_IF_FAILED(spawned.IsOK());

  // should still be waiting...
  CHECK(!spawned->Wait(400));
  // should be done then...
  CHECK(spawned->Wait(500));
}

TEST_FIXTURE(FProcess,SpawnCrash) {
#ifdef niWindows
  // On windows run the test only if we're explicitly targeting the fixture.
  // That's because we're getting the JIT debugger which breaks the automated
  // run of the test case (we have to click on the dialog to discard it...)
  if (!UnitTest::runFixtureName.IEq(m_testName)) {
    return;
  }
#endif

  AUTO_WARNING_MODE();

  Ptr<iOSProcess> cur = _pman->GetCurrentProcess();
  CHECK(cur.IsOK());

  cString cmdLine;
  cmdLine << cur->GetExePath();
  cmdLine << " __Process crash";
  Ptr<iOSProcess> spawned = _pman->SpawnProcess(
      cmdLine.Chars(),
      eOSProcessSpawnFlags_None);
  CHECK_RETURN_IF_FAILED(spawned.IsOK());

  // wait a tiny bit for it to crash...
  CHECK(spawned->Wait(eInvalidHandle));
  {
    AUTO_WARNING_MODE();
    CHECK(spawned->GetDidCrash());
  }
}

TEST_FIXTURE(FProcess,SpawnHangTerminate) {
  Ptr<iOSProcess> cur = _pman->GetCurrentProcess();
  CHECK(cur.IsOK());

  cString cmdLine;
  cmdLine << cur->GetExePath();
  cmdLine << " __Process hang";
  Ptr<iOSProcess> spawned = _pman->SpawnProcess(
      cmdLine.Chars(),
      eOSProcessSpawnFlags_None);
  CHECK_RETURN_IF_FAILED(spawned.IsOK());

  // wait a tiny bit for it to keep hanging..
  CHECK(!spawned->Wait(500));

  {
    AUTO_WARNING_MODE();
    CHECK(!spawned->GetDidCrash());
  }

  spawned->Terminate(123);

  sVec2i r = spawned->WaitForExitCode(eInvalidHandle);
  niDebugFmt(("spawned->WaitForExitCode(): %s", r));
#ifdef niWindows
  CHECK_EQUAL(1,r.x);
  CHECK_EQUAL(123,r.y);
#else
  // posix platform don't support a custom exit code when forcefully
  // terminated
  CHECK_EQUAL(0,r.x);
  CHECK_EQUAL(eInvalidHandle,r.y);
#endif
}

TEST_FIXTURE(FProcess,SpawnStdFiles) {
  Ptr<iOSProcess> cur = _pman->GetCurrentProcess();
  CHECK(cur.IsOK());

  cString cmdLine;
  cmdLine << cur->GetExePath();
  cmdLine << " __Process stdfiles";
  Ptr<iOSProcess> spawned = _pman->SpawnProcess(
      cmdLine.Chars(),
      eOSProcessSpawnFlags_StdFiles);
  CHECK_RETURN_IF_FAILED(spawned.IsOK());

  Ptr<iFile> fpTo;
  {
    fpTo = spawned->GetFile(eOSProcessFile_StdIn);
    Nonnull<cFilePosTracker> fpToTrackerBase { niNew cFilePosTracker(fpTo->GetFileBase()) };
    fpTo = ni::GetLang()->CreateFile(fpToTrackerBase);
    CHECK_RETURN_IF_FAILED(fpTo.IsOK());
    CHECK(niFlagIs(fpTo->GetFileFlags(),eFileFlags_Write));
    CHECK(niFlagIsNot(fpTo->GetFileFlags(),eFileFlags_Read));
  }

  Ptr<iFile> fpErr = spawned->GetFile(eOSProcessFile_StdErr);
  CHECK_RETURN_IF_FAILED(fpErr.IsOK());
  CHECK(niFlagIs(fpErr->GetFileFlags(),eFileFlags_Read));
  CHECK(niFlagIsNot(fpErr->GetFileFlags(),eFileFlags_Write));

  Ptr<iFile> fpFrom;
  {
    fpFrom = spawned->GetFile(eOSProcessFile_StdOut);
    CHECK(fpErr.ptr() == fpFrom.ptr());
    Nonnull<cFilePosTracker> fpFromTrackerBase { niNew cFilePosTracker(fpFrom->GetFileBase()) };
    fpFrom = ni::GetLang()->CreateFile(fpFromTrackerBase);
    CHECK_RETURN_IF_FAILED(fpFrom.IsOK());
    CHECK(niFlagIs(fpFrom->GetFileFlags(),eFileFlags_Read));
    CHECK(niFlagIsNot(fpFrom->GetFileFlags(),eFileFlags_Write));
  }

  cString r;
  // skip the test program initial message
  r = fpFrom->ReadStringLine();
  niDebugFmt(("TEST RECEIVED INITIAL: '%s'", r));
  CHECK(r.contains(_A("test mode")));

  {
    niDebugFmt(("TEST1 SEND"));
    const tSize tellBeforeWrite = fpTo->Tell();
    fpTo->WriteStringZ(_A("hello"));
    fpTo->Flush();
    const tSize wroteBytes = fpTo->Tell()-tellBeforeWrite;
    niDebugFmt(("TEST1 SENT: %d bytes", wroteBytes));

    niDebugFmt(("TEST1 CHILD STDERR: '%s'", fpErr->ReadString()));
    const tSize tellBeforeRead = fpFrom->Tell();
    r = fpFrom->ReadString();
    const tSize receivedBytes = fpFrom->Tell()-tellBeforeRead;
    niDebugFmt(("TEST1 RECEIVED: %d bytes, '%s'", receivedBytes, r));
    CHECK_EQUAL(_ASTR("HELLO"),r);
    CHECK_EQUAL(6,receivedBytes);
    CHECK_EQUAL(wroteBytes,receivedBytes);
  }

  fpTo->WriteStringZ(_A("world!"));
  fpTo->Flush();
  niDebugFmt(("TEST2 CHILD STDERR: '%s'", fpErr->ReadString()));
  r = fpFrom->ReadString();
  niDebugFmt(("TEST2 RECEIVED: '%s'", r));
  CHECK_EQUAL(_ASTR("WORLD!"),r);

  fpTo->WriteStringZ(_A("exit"));
  fpTo->Flush();
  niDebugFmt(("TEST3 CHILD STDERR: '%s'", fpErr->ReadString()));
  r = fpFrom->ReadString();
  niDebugFmt(("TEST3 RECEIVED: '%s'", r));
  CHECK_EQUAL(_ASTR("EXIT"),r);

  tBool cleanClose = spawned->Wait(1000);
  CHECK(cleanClose);
  if (!cleanClose) {
    spawned->Terminate(-1);
  }
}

TEST_FIXTURE(FProcess,SpawnStdFilesDifferentStdOutAndStdErr) {
#if !defined niWindows
  AUTO_WARNING_MODE();
#endif

  Ptr<iOSProcess> cur = _pman->GetCurrentProcess();
  CHECK(cur.IsOK());

  cString cmdLine;
  cmdLine << cur->GetExePath();
  cmdLine << " __Process stdfiles";
  Ptr<iOSProcess> spawned = _pman->SpawnProcess(
      cmdLine.Chars(),
      eOSProcessSpawnFlags_StdFiles|
      eOSProcessSpawnFlags_DifferentStdOutAndStdErr);
  CHECK_RETURN_IF_FAILED(spawned.IsOK());

  Ptr<iFile> fpTo = spawned->GetFile(eOSProcessFile_StdIn);
  CHECK_RETURN_IF_FAILED(fpTo.IsOK());
  CHECK(niFlagIs(fpTo->GetFileFlags(),eFileFlags_Write));
  CHECK(niFlagIsNot(fpTo->GetFileFlags(),eFileFlags_Read));

  Ptr<iFile> fpFrom = spawned->GetFile(eOSProcessFile_StdOut);
  CHECK_RETURN_IF_FAILED(fpFrom.IsOK());
  CHECK(niFlagIs(fpFrom->GetFileFlags(),eFileFlags_Read));
  CHECK(niFlagIsNot(fpFrom->GetFileFlags(),eFileFlags_Write));

  Ptr<iFile> fpErr = spawned->GetFile(eOSProcessFile_StdErr);
  CHECK_RETURN_IF_FAILED(fpErr.IsOK());
  CHECK(niFlagIs(fpErr->GetFileFlags(),eFileFlags_Read));
  CHECK(niFlagIsNot(fpErr->GetFileFlags(),eFileFlags_Write));

  CHECK(fpErr.ptr() != fpFrom.ptr());

  cString r;
  // skip the test program initial message
  r = fpFrom->ReadStringLine();
  CHECK(r.contains(_A("test mode")));
  niDebugFmt(("TEST RECEIVED: %s", r));

  fpTo->WriteStringZ(_A("hello"));
  fpTo->Flush();
  niDebugFmt(("TEST CHILD STDERR: %s", fpErr->ReadString()));
  r = fpFrom->ReadString();
  CHECK_EQUAL(_ASTR("HELLO"),r);
  niDebugFmt(("TEST RECEIVED: %s", r));

  fpTo->WriteStringZ(_A("world!"));
  fpTo->Flush();
  niDebugFmt(("TEST CHILD STDERR: %s", fpErr->ReadString()));
  r = fpFrom->ReadString();
  CHECK_EQUAL(_ASTR("WORLD!"),r);
  niDebugFmt(("TEST RECEIVED: %s", r));

  fpTo->WriteStringZ(_A("exit"));
  fpTo->Flush();
  niDebugFmt(("TEST CHILD STDERR: %s", fpErr->ReadString()));
  r = fpFrom->ReadString();
  CHECK_EQUAL(_ASTR("EXIT"),r);
  niDebugFmt(("TEST RECEIVED: %s", r));

  tBool cleanClose = spawned->Wait(1000);
  CHECK(cleanClose);
  if (!cleanClose) {
    spawned->Terminate(-1);
  }
}

TEST_FIXTURE(FProcess,SpawnPrintLoop) {
  Ptr<iOSProcess> cur = _pman->GetCurrentProcess();
  CHECK(cur.IsOK());

  cString cmdLine;
  cmdLine << cur->GetExePath();
  cmdLine << " __Process printloop";
  Ptr<iOSProcess> spawned = _pman->SpawnProcess(
      cmdLine.Chars(),
      eOSProcessSpawnFlags_StdFiles);
  CHECK_RETURN_IF_FAILED(spawned.IsOK());

  Ptr<iFile> fpFrom = spawned->GetFile(eOSProcessFile_StdOut);
  CHECK_RETURN_IF_FAILED(fpFrom.IsOK());
  CHECK(niFlagIs(fpFrom->GetFileFlags(),eFileFlags_Read));
  CHECK(niFlagIsNot(fpFrom->GetFileFlags(),eFileFlags_Write));

  int count = 0;
  while (1) {
    cString r = fpFrom->ReadStringLine();
    if (r.empty() && fpFrom->GetPartialRead())
      break;
    if (!r.StartsWith("D/")) {
      niDebugFmt(("RECEIVED: %s", r));
      ++count;
    }
  }

  // +3 lines are printed in addition to the loop
  CHECK_EQUAL(3+kPrintLoopCount,count);

  tBool cleanClose = spawned->Wait(1000);
  CHECK(cleanClose);
  if (!cleanClose) {
    spawned->Terminate(-1);
  }
}
#endif // niNoProcess
