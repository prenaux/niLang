#include "stdafx.h"

#include "../src/API/niLang/IFile.h"
#include "../src/API/niLang/ILang.h"
#include "../src/API/niLang/Utils/CryptoUtils.h"

using namespace ni;

static const achar* const _hashedDir = "hashDir/";

static achar const* _URL[] = {
  "http://bitscollider.com/img/ni-logo.png",
  "..\\..\\data\\niUI\\error.dds",
  "../../data/niUI/error.dds",
  "file://img/ni-logo.png",
  "img/ni-logo.png",
  "HTTP://BITSCOLLIDER.COM/IMG/NI-LOGO.PNG",
  "a://foo://bar",
  "file://../../data/niUI/error.dds",
  "/Work/niLang/data/niUI/error.dds",
  "\\My Work\\www\\bitscollider-2\\static\\img\\ni-logo.png",
};

struct FHashedDir {
  Ptr<ni::iFileSystem> hashedFS;
  FHashedDir() {
    cString dir = UnitTest::GetTestOutputFilePath(_hashedDir);
    ni::GetRootFS()->FileMakeDir(dir.Chars());
    Ptr<iFileSystem> baseFS = ni::GetLang()->CreateFileSystemDir(
      dir.Chars(),
      eFileSystemRightsFlags_Read|eFileSystemRightsFlags_Write|eFileSystemRightsFlags_Create|eFileSystemRightsFlags_Delete);
    hashedFS = ni::GetLang()->CreateFileSystemHashed(baseFS.ptr());
  }
};

TEST_FIXTURE(FHashedDir, GetAbsolutePath) {
  niDebugFmt(("... hashedFS baseContainer: %s", hashedFS->GetBaseContainer()));
  niLoop(i, niCountOf(_URL)) {
    cPath path = _URL[i];
    cPath absPath = hashedFS->GetAbsolutePath(_URL[i]).Chars();
    niDebugFmt(("... URL%d: %s -> %s", i, path.GetPath(), absPath.GetPath()));
    CHECK(!path.GetPath().contains(_hashedDir));
    CHECK(absPath.GetPath().contains(_hashedDir));
    CHECK_EQUAL(absPath.GetExtension(), path.GetExtension());
  }

  // Hash shouldn't include the file extension in its computation
  cString hash1 = hashedFS->GetAbsolutePath("some/path/file.foo");
  cString hash2 = hashedFS->GetAbsolutePath("some/path/file.bar");
  cString hash3 = hashedFS->GetAbsolutePath("some/path/file");
  niDebugFmt(("... HASH1: %s", hash1));
  niDebugFmt(("... HASH2: %s", hash2));
  niDebugFmt(("... HASH3: %s", hash3));
  CHECK_EQUAL(_ASTR("84bd8a5e2b52e6a93984ecb611860db21478f178"), cPath(hash1.Chars()).GetFileNoExt());
  CHECK_EQUAL(cPath(hash1.Chars()).GetFileNoExt(), cPath(hash2.Chars()).GetFileNoExt());
  CHECK_EQUAL(cPath(hash2.Chars()).GetFileNoExt(), cPath(hash3.Chars()).GetFileNoExt());
}

TEST_FIXTURE(FHashedDir, FileOpen) {
  cString fileName = "test|file:withsome\\wierd/char";
  // fileName += ni::ToString(ni::RandInt());
  fileName += ".txt";
  niDebugFmt(("... fileName: %s", fileName));
  {
    Ptr<iFile> fpWrite = hashedFS->FileOpen(fileName.Chars(), eFileOpenMode_Write);
    niDebugFmt(("... fpWrite: %s", fpWrite->GetSourcePath()));
    fpWrite->WriteString("This is some test text");
  }
  {
    Ptr<iFile> fpRead = hashedFS->FileOpen(fileName.Chars(), eFileOpenMode_Read);
    niDebugFmt(("... fpRead: %s", fpRead->GetSourcePath()));
    const cString read = fpRead->ReadString();
    niDebugFmt(("... read: %s", read));
    CHECK_EQUAL(_ASTR("This is some test text"), read);
  }
}
