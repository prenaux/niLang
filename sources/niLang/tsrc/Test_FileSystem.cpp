#include "stdafx.h"
#include "../src/API/niLang/IFile.h"
#include "../src/API/niLang/ILang.h"
#include "../src/FileFd.h"
#include "../src/API/niLang/Utils/TimerSleep.h"
#include <niLang/Utils/Trace.h>
#include <niLang/Utils/FileEnum.h>
#include <niLang/Utils/URLFileHandler.h>

struct FFileSystem {
};

using namespace ni;

static cString _GetCurrentDrive() {
#ifdef niWindows
  const char szDrive[] = { (char)(_getdrive()&0xff) + 'a' - 1, ':', 0 };
  return szDrive;
#else
  return AZEROSTR;
#endif
}

TEST_FIXTURE(FFileSystem,GetAbsolutePath) {
  const cString dataDir = ni::GetLang()->GetProperty("ni.dirs.data");
  CHECK(StrIsPathSep(dataDir.back()));
  CHECK(StrIsAbsolutePath(dataDir.Chars()));

  {
    cString abs = ni::GetRootFS()->GetAbsolutePath("../../data/niUI/error.dds");
    CHECK_EQUAL(dataDir + "niUI/error.dds", abs);
  }

  {
    cString abs = ni::GetRootFS()->GetAbsolutePath("../../data/niUI");
    CHECK_EQUAL(dataDir + "niUI", abs);
  }

  {
    cString abs = ni::GetRootFS()->GetAbsolutePath("../../data/niUI/");
    CHECK_EQUAL(dataDir + "niUI/", abs);
  }

  {
    cString abs = ni::GetRootFS()->GetAbsolutePath("./.././../data/niUI/");
    CHECK_EQUAL(dataDir + "niUI/", abs);
  }

  {
    cString abs = ni::GetRootFS()->GetAbsolutePath("n:/a/b/c/../data/../niUI/");
    CHECK_EQUAL(_ASTR("n:/a/b/niUI/"), abs);
  }

  {
    cString abs = ni::GetRootFS()->GetAbsolutePath("N:/a/b/c/../data/../niUI/");
    CHECK_EQUAL(_ASTR("n:/a/b/niUI/"), abs);
  }

  {
    cString abs = ni::GetRootFS()->GetAbsolutePath("/a/b/c/../data/../niUI/");
    CHECK_EQUAL(_GetCurrentDrive() + _ASTR("/a/b/niUI/"), abs);
  }

  {
    const cString binDir = ni::GetLang()->GetProperty("ni.dirs.bin");
    cString abs = ni::GetRootFS()->GetAbsolutePath("a/b///c//../data/.//../niUI///");
    CHECK_EQUAL(binDir + _ASTR("a/b/niUI/"), abs);
  }
}

TEST_FIXTURE(FFileSystem,GetAbsolutePathUnicodePath) {
  const cString dataDir = ni::GetLang()->GetProperty("ni.dirs.data");
  CHECK(StrIsPathSep(dataDir.back()));
  CHECK(StrIsAbsolutePath(dataDir.Chars()));

  cString abs = ni::GetRootFS()->GetAbsolutePath("../../data/你好αβ/error.dds");

  CHECK_EQUAL(cString(dataDir + "你好αβ/error.dds"), abs);
}

TEST_FIXTURE(FFileSystem,GetAbsolutePathExpandHome) {
  const cString homeDir = ni::GetLang()->GetProperty("ni.dirs.home");
  CHECK(StrIsPathSep(homeDir.back()));
  CHECK(StrIsAbsolutePath(homeDir.Chars()));

  {
    cString abs = ni::GetRootFS()->GetAbsolutePath("~/data/你好αβ/error.dds");
    CHECK_EQUAL(cString(homeDir + "data/你好αβ/error.dds"), abs);
  }

  {
    cString abs = ni::GetRootFS()->GetAbsolutePath("~/data/你好αβ/../error.dds");
    CHECK_EQUAL(cString(homeDir + "data/error.dds"), abs);
  }

  {
    cString abs = ni::GetRootFS()->GetAbsolutePath("~/data/../你好αβ/../error.dds");
    CHECK_EQUAL(cString(homeDir + "error.dds"), abs);
  }

  {
    cString abs = ni::GetRootFS()->GetAbsolutePath("~/data/../你好/../αβ/error.dds");
    CHECK_EQUAL(cString(homeDir + "αβ/error.dds"), abs);
  }
}

TEST_FIXTURE(FFileSystem,Dirs) {
  {
    cString path = ni::GetLang()->GetProperty("ni.dirs.home");
    niTraceFmt(("home: '%s'", path));
    CHECK(ni::DirExists(path.Chars()));
  }

  {
    cString path = ni::GetLang()->GetProperty("ni.dirs.documents");
    niTraceFmt(("documents: '%s'", path));
    CHECK(ni::DirExists(path.Chars()));
    CHECK(path.icontains("documents") || path.icontains("docs"));
  }

  {
    cString path = ni::GetLang()->GetProperty("ni.dirs.temp");
    niTraceFmt(("temp: '%s'", path));
    CHECK(ni::DirExists(path.Chars()));
  }

  {
    cString path = ni::GetLang()->GetProperty("ni.dirs.logs");
    niTraceFmt(("logs: '%s'", path));
    CHECK(ni::DirExists(path.Chars()));
    CHECK(path.icontains("log"));
  }

  {
    cString path = ni::GetLang()->GetProperty("ni.dirs.config");
    niTraceFmt(("config: '%s'", path));
    CHECK(ni::DirExists(path.Chars()));
    CHECK(path.icontains("config") || path.icontains("cfg"));
  }
}

TEST_FIXTURE(FFileSystem,RootFSCaseInsensitive) {
  // The root file system and FileSystemDir should be case insensitive.

  cString workDir = cPath(ni::GetLang()->GetEnv("WORK"),AZEROSTR).GetPath();
  niTraceFmt(("workDir: '%s'", workDir));
  CHECK(ni::DirExists(workDir.c_str()));

  // We use a temp folder in the WORK directory because its usually case
  // sensitive on Pierre's macOS dev machines.
  cString workDirTmp = cPath(workDir,AZEROSTR).AddDirectoryBack("_tmp/Test_niLang").GetPath();
  ni::GetRootFS()->FileMakeDir(workDirTmp.Chars());
  niTraceFmt(("workDir _tmp: '%s'", workDirTmp));
  CHECK(ni::DirExists(workDirTmp.Chars()));

  cString workDirTmpLwr = workDirTmp;
  workDirTmpLwr.ToLower();

  const cPath filePath = cPath(workDirTmp,"heLLoTest.txt");
  const cPath filePathLwr = cPath(workDirTmpLwr,"hellotest.txt");

  // Create a hellotest.txt file with wierd casing to make sure we can open it
  // even if the file system is case sensitive.
  {
    Ptr<iFile> fpHello = ni::GetRootFS()->FileOpen(filePath.Chars(), eFileOpenMode_Write);
    fpHello->WriteString("Hello test!");
  }

  CHECK(ni::FileExists(cPath(workDirTmp,"heLLoTest.txt").Chars()));

  const tBool isCaseSensitive = [&]() {
    FILE* fp = fopen(cPath(workDirTmp,"hellotest.txt").Chars(),"rb");
    tBool r = (fp == NULL);
    if (fp) {
      fclose(fp);
    }
    return r;
  }();
  niTraceFmt(("... isCaseSensitive: %y", isCaseSensitive));

  // File enum should enumerate ignoring case
  CHECK_EQUAL(1,ni::GetRootFS()->FileEnum(filePath.Chars(),eFileAttrFlags_AllFiles,NULL));
  CHECK_EQUAL(1,ni::GetRootFS()->FileEnum(filePathLwr.Chars(),eFileAttrFlags_AllFiles,NULL));

  {
    FindFile ff;
    const tBool found = ff.First(filePath.Chars());
    CHECK(found);
    CHECK_EQUAL(workDirTmp, ff.DirName());
    CHECK_EQUAL("heLLoTest.txt", ff.FileName());
    CHECK_EQUAL(11, ff.FileSize()); // size of the test string we write in the file
    CHECK_EQUAL(1, ff.FileAttribs()&eFileAttrFlags_File);
  }

  // FindFile should be ignoring case
  {
    FindFile ff;
    const tBool found = ff.First(filePathLwr.Chars());
    CHECK(found);
    CHECK(workDirTmp.IEq(ff.DirName()));
    CHECK_EQUAL("heLLoTest.txt", ff.FileName());
    CHECK_EQUAL(11, ff.FileSize()); // size of the test string we write in the file
    CHECK_EQUAL(1, ff.FileAttribs()&eFileAttrFlags_File);
  }

  // FindFile path should find the case sensitive filename
  CHECK_EQUAL(filePath.GetPath(), FindFirstFilePath(cPath(workDirTmp,"hellotest.txt").Chars()).GetPath());
  CHECK(filePath.GetPath().IEq(FindFirstFilePath(cPath(workDirTmpLwr,"hellotest.txt").Chars()).GetPath()));

  // Make dir / rmdir should base case insensitive
  {
    const cPath dirPath = cPath(workDirTmp,NULL).AddDirectoryBack("craZZZyDir");
    const cPath dirPathLwr = cPath(workDirTmp,NULL).AddDirectoryBack("crazzzydir");
    niTraceFmt(("...dirPath: '%s'", dirPath.GetPath()));
    niTraceFmt(("...dirPathLwr: '%s'", dirPathLwr.GetPath()));

    CHECK_EQUAL(eTrue,ni::GetRootFS()->FileMakeDir(dirPath.Chars()));
    CHECK_EQUAL(dirPath.GetPath(), FindFirstDirPath(dirPath.Chars()).GetPath());
    CHECK(dirPath.GetPath().IEq(FindFirstDirPath(dirPathLwr.Chars()).GetPath()));
    {
      FindFile ff;
      const tBool found = ff.First(dirPath.Chars());
      CHECK_EQUAL(eTrue,found);
      // DirName should be the parent folder
      CHECK_EQUAL(workDirTmp, _ASTR(ff.DirName()));
      // FileName should be the folder itself
      CHECK_EQUAL(_ASTR("craZZZyDir"), _ASTR(ff.FileName()));
    }

    CHECK_EQUAL(eTrue,ni::DirExists(dirPath.Chars()));
    CHECK_EQUAL(eTrue,ni::DirExists(dirPathLwr.Chars())); // DirExists should be case insensitive

    CHECK_EQUAL(eTrue,ni::GetRootFS()->FileDeleteDir(dirPathLwr.Chars()));

    CHECK_EQUAL(eFalse,ni::DirExists(dirPath.Chars()));
    CHECK_EQUAL(eFalse,ni::DirExists(dirPathLwr.Chars()));
  }

  // File exist should be ignoring case
  CHECK_EQUAL(1,ni::GetRootFS()->FileExists(cPath(workDirTmp,"hellotest.txt").Chars(),eFileAttrFlags_AllFiles) &
              eFileAttrFlags_File);

  // afopen the test file
  // Note: We can't do this test on Windows because of DLL boundary issues, we
  // can't use a CRT object created in another DLL/module.
#if !defined niWindows
  {
    cString openedPath;
    FILE* fp = afopen(filePathLwr.Chars(), "rb", &openedPath);
    CHECK(fp != NULL);
    if (fp) {
      char buf[6] = {0};
      fread(buf,5,1,fp);
      CHECK_EQUAL(_ASTR("Hello"),_ASTR(buf));
      CHECK_EQUAL(filePath.GetPath(), openedPath);
    }
  }
#endif

  // Delete the test file
  {
    CHECK_EQUAL(eTrue,ni::FileExists(filePath.Chars()));
    CHECK_EQUAL(eTrue,ni::FileExists(filePathLwr.Chars()));

    CHECK_EQUAL(eTrue,ni::GetRootFS()->FileDelete(filePathLwr.Chars()));

    CHECK_EQUAL(eFalse,ni::FileExists(filePath.Chars()));
    CHECK_EQUAL(eFalse,ni::FileExists(filePathLwr.Chars()));
  }
}

TEST_FIXTURE(FFileSystem,FindExeCI) {
  achar exePathBuff[niStackBufferSize];
  cPath exePath(ni_get_exe_path(exePathBuff));

  cString exeFileName = exePath.GetFile();
  niDebugFmt(("... exeFileName: %s", exeFileName));

  CHECK_EQUAL(eTrue,ni::FileExists(exeFileName.Chars()));
  CHECK_EQUAL(eTrue,ni::FileExists((_ASTR("./") + exeFileName).Chars()));

  {
    cString exeFileNameLwr = exeFileName;
    exeFileNameLwr.ToLower();
    niDebugFmt(("... exeFileNameLwr: %s", exeFileNameLwr));
    CHECK_EQUAL(eTrue,ni::FileExists(exeFileNameLwr.Chars()));
    CHECK_EQUAL(eTrue,ni::FileExists((_ASTR("./") + exeFileNameLwr).Chars()));
    CHECK_EQUAL(eFalse,ni::FileExists((_ASTR("./squirrel_") + exeFileNameLwr).Chars()));
  }

  {
    cString exeFileNameUpr = exeFileName;
    exeFileNameUpr.ToUpper();
    niDebugFmt(("... exeFileNameUpr: %s", exeFileNameUpr));
    CHECK_EQUAL(eTrue,ni::FileExists(exeFileNameUpr.Chars()));
    CHECK_EQUAL(eTrue,ni::FileExists((_ASTR("./") + exeFileNameUpr).Chars()));
    CHECK_EQUAL(eFalse,ni::FileExists((_ASTR("./squirrel_") + exeFileNameUpr).Chars()));
  }
}

TEST_FIXTURE(FFileSystem,ModuleDataDir) {
  cString moduleDataDir = GetModuleDataDir("niLang","Test_niLang");
  niDebugFmt(("... moduleDataDir: '%s'", moduleDataDir));
  ni::RegisterModuleDataDirDefaultURLFileHandler("niLang","Test_niLang");
  Ptr<iFile> fp = ni::GetLang()->URLOpen("Test_niLang://hello-file-system.txt");
  CHECK_RETURN_IF_FAILED(fp.IsOK());
  CHECK_EQUAL(18, fp->GetSize());
  CHECK_EQUAL(_ASTR("Hello File System"), fp->ReadStringLine());
}

TEST_FIXTURE(FFileSystem,GetTime) {
  const cString filePathA = UnitTest::GetTestOutputFilePath("fs_get_time_a.txt");
  const cString filePathB = UnitTest::GetTestOutputFilePath("fs_get_time_b.txt");
  const tI64 currentTimeSecs = ni::GetCurrentTime()->GetUnixTimeSecs();
  CHECK_NOT_EQUAL(0,currentTimeSecs);

  // Test time that are the within the same second
  {
    ni::GetRootFS()->FileDelete(filePathA.Chars());
    Ptr<iFile> fp = ni::GetRootFS()->FileOpen(filePathA.Chars(), eFileOpenMode_Write);
    fp->WriteString("Hello test A!");
  }
  {
    ni::GetRootFS()->FileDelete(filePathB.Chars());
    Ptr<iFile> fp = ni::GetRootFS()->FileOpen(filePathB.Chars(), eFileOpenMode_Write);
    fp->WriteString("Hello test B!");
  }

  Ptr<iFile> fpA = ni::GetRootFS()->FileOpen(filePathA.Chars(), eFileOpenMode_Read);
  Ptr<iTime> ta = ni::CreateTimeZero();
  CHECK(fpA->GetTime(eFileTime_LastWrite,ta));
  Ptr<iFile> fpB = ni::GetRootFS()->FileOpen(filePathB.Chars(), eFileOpenMode_Read);
  Ptr<iTime> tb = ni::CreateTimeZero();
  CHECK(fpB->GetTime(eFileTime_LastWrite,tb));
  niDebugFmt(("... ta: %d - %s", ta->GetUnixTimeSecs(), ta->Format(NULL)));
  niDebugFmt(("... tb: %d - %s", tb->GetUnixTimeSecs(), tb->Format(NULL)));
  CHECK_NOT_EQUAL(0,ta->GetUnixTimeSecs());
  CHECK_EQUAL(0,ta->Compare(tb));
  CHECK_CLOSE(0,tb->GetUnixTimeSecs()-ta->GetUnixTimeSecs(),1);
  CHECK_CLOSE(0,currentTimeSecs-ta->GetUnixTimeSecs(),1);
}

TEST_FIXTURE(FFileSystem,GetTime1) {
  const cString filePathA = UnitTest::GetTestOutputFilePath("fs_get_time_a.txt");
  const cString filePathB = UnitTest::GetTestOutputFilePath("fs_get_time_b.txt");
  const tI64 currentTimeSecs = ni::GetCurrentTime()->GetUnixTimeSecs();
  CHECK_NOT_EQUAL(0,currentTimeSecs);

  // Test time that are the within 1 seconds
  {
    ni::GetRootFS()->FileDelete(filePathA.Chars());
    Ptr<iFile> fp = ni::GetRootFS()->FileOpen(filePathA.Chars(), eFileOpenMode_Write);
    fp->WriteString("Hello test A!");
  }
  ni::SleepMs(1100);
  {
    ni::GetRootFS()->FileDelete(filePathB.Chars());
    Ptr<iFile> fp = ni::GetRootFS()->FileOpen(filePathB.Chars(), eFileOpenMode_Write);
    fp->WriteString("Hello test B!");
  }

  Ptr<iFile> fpA = ni::GetRootFS()->FileOpen(filePathA.Chars(), eFileOpenMode_Read);
  Ptr<iTime> ta = ni::CreateTimeZero();
  CHECK(fpA->GetTime(eFileTime_LastWrite,ta));
  Ptr<iFile> fpB = ni::GetRootFS()->FileOpen(filePathB.Chars(), eFileOpenMode_Read);
  Ptr<iTime> tb = ni::CreateTimeZero();
  CHECK(fpB->GetTime(eFileTime_LastWrite,tb));
  niDebugFmt(("... ta: %d - %s", ta->GetUnixTimeSecs(), ta->Format(NULL)));
  niDebugFmt(("... tb: %d - %s", tb->GetUnixTimeSecs(), tb->Format(NULL)));
  CHECK_NOT_EQUAL(0,ta->GetUnixTimeSecs());
  CHECK_EQUAL(-1,ta->Compare(tb));
  CHECK_NOT_EQUAL(tb->GetUnixTimeSecs(),ta->GetUnixTimeSecs());
  CHECK_CLOSE(1,tb->GetUnixTimeSecs()-ta->GetUnixTimeSecs(),1);
  CHECK_CLOSE(0,currentTimeSecs-ta->GetUnixTimeSecs(),1);
}
