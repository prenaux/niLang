#include "stdafx.h"
#include "../src/API/niLang/IFile.h"
#include "../src/API/niLang/ILang.h"

struct FURLFile {
};

using namespace ni;

static achar const* _URL[] = {
  /* 0 */ "http://bitscollider.com/img/ni-logo.png",
  /* 1 */ "..\\..\\data\\niUI\\error.dds",
  /* 2 */ "../../data/niUI/error.dds",
  /* 3 */ "file://img/ni-logo.png",
  /* 4 */ "img/ni-logo.png",
  /* 5 */ "HTTP://BITSCOLLIDER.COM/IMG/NI-LOGO.PNG",
  /* 6 */ "a://foo://bar",
  /* 7 */ "file://../../data/niUI/error.dds",
  /* 8 */ "/Work/niLang/data/niUI/error.dds",
  /* 9 */ "\\My Work\\www\\bitscollider-2\\static\\img\\ni-logo.png",
};

const tI32 knMaxProtocolSearchLen = 16;

TEST_FIXTURE(FURLFile,StringURL) {
  niDebugFmt(("... URL[0]: %s", _URL[0]));

  CHECK_EQUAL(4, StrFindProtocol(_URL[0]));
  CHECK_EQUAL(-1, StrFindProtocol(_URL[1]));
  CHECK_EQUAL(-1, StrFindProtocol(_URL[2]));
  CHECK_EQUAL(4, StrFindProtocol(_URL[3]));
  CHECK_EQUAL(-1, StrFindProtocol(_URL[4]));
  CHECK_EQUAL(4, StrFindProtocol(_URL[5]));
  CHECK_EQUAL(1, StrFindProtocol(_URL[6]));

  CHECK_EQUAL(_ASTR("http"), StringURLGetProtocol(_URL[0]));
  CHECK_EQUAL(_ASTR("bitscollider.com/img/ni-logo.png"), StringURLGetPath(_URL[0]));

  CHECK_EQUAL(_ASTR("HTTP"), StringURLGetProtocol(_URL[5]));
  CHECK_EQUAL(_ASTR("BITSCOLLIDER.COM/IMG/NI-LOGO.PNG"), StringURLGetPath(_URL[5]));

  CHECK_EQUAL(_ASTR("file"), StringURLGetProtocol(_URL[3]));
  CHECK_EQUAL(_ASTR("img/ni-logo.png"), StringURLGetPath(_URL[3]));

  CHECK_EQUAL(_ASTR(""), StringURLGetProtocol(_URL[1]));
  CHECK_EQUAL(_ASTR("..\\..\\data\\niUI\\error.dds"), StringURLGetPath(_URL[1]));

  CHECK_EQUAL(_ASTR("a"), StringURLGetProtocol(_URL[6]));
  CHECK_EQUAL(_ASTR("foo://bar"), StringURLGetPath(_URL[6]));

  CHECK_EQUAL(-1, StrFindProtocol(""));
  CHECK_EQUAL(_ASTR(""), StringURLGetProtocol(""));
  CHECK_EQUAL(_ASTR(""), StringURLGetPath(""));
}

TEST_FIXTURE(FURLFile,URLHandler) {
  CHECK_EQUAL(_ASTR("file"), ni::GetLang()->URLGetProtocol(_URL[1]));
  CHECK_EQUAL(_ASTR("file"), ni::GetLang()->URLGetProtocol(_URL[2]));
  CHECK_EQUAL(_ASTR("file"), ni::GetLang()->URLGetProtocol(_URL[7]));
  CHECK_EQUAL(_ASTR("file"), ni::GetLang()->URLGetProtocol(_URL[8]));

  CHECK_EQUAL(_ASTR("file"), ni::GetLang()->URLGetProtocol(_URL[3]));
  Ptr<iURLFileHandler> fileHandler = ni::GetLang()->URLGetHandler(_URL[3]);
  CHECK(fileHandler.IsOK());

  CHECK_EQUAL(_ASTR("default"), ni::GetLang()->URLGetProtocol(_URL[4]));
  Ptr<iURLFileHandler> defaultHandler = ni::GetLang()->URLGetHandler(_URL[4]);
  CHECK(defaultHandler.IsOK());

  CHECK(fileHandler.ptr() != defaultHandler.ptr());
}

TEST_FIXTURE(FURLFile,URLOpen) {
  niDebugFmt(("... URL[7]: %s", _URL[7]));
  Ptr<iFile> fileFP = ni::GetLang()->URLOpen(_URL[7]);
  CHECK_RETURN_IF_FAILED(fileFP.IsOK());
  CHECK_EQUAL(152,fileFP->GetSize());

  niDebugFmt(("... URL[2]: %s", _URL[2]));
  Ptr<iFile> defaultFP = ni::GetLang()->URLOpen(_URL[2]);
  CHECK_RETURN_IF_FAILED(defaultFP.IsOK());
  CHECK_EQUAL(fileFP->GetSize(), defaultFP->GetSize());
  CHECK_EQUAL(152,fileFP->GetSize());

  {
    cString absPath = ni::GetLang()->GetEnv("WORK");
    absPath += "/niLang/data/niUI/error.dds";
    niDebugFmt(("... URL absPath: %s", absPath));
    Ptr<iFile> fileFP = ni::GetLang()->URLOpen(absPath.Chars());
    CHECK_RETURN_IF_FAILED(fileFP.IsOK());
  }
}

TEST_FIXTURE(FURLFile,PathURL) {
  niDebugFmt(("... URL[0]: %s", _URL[0]));
  ni::cPath pathURL(_URL[0]);

  CHECK_EQUAL(_ASTR("http://bitscollider.com/img/ni-logo.png"),
              pathURL.GetPath());

  pathURL.SetFile("narf.foo");
  CHECK_EQUAL(_ASTR("http://bitscollider.com/img/narf.foo"),
              pathURL.GetPath());

  CHECK(pathURL.HasProtocol());
  CHECK_EQUAL(_ASTR("http"), pathURL.GetProtocol());

  pathURL.SetProtocol("dummy");
  CHECK(pathURL.HasProtocol());
  CHECK_EQUAL(_ASTR("dummy"), pathURL.GetProtocol());
  CHECK_EQUAL(_ASTR("dummy://bitscollider.com/img/narf.foo"),
              pathURL.GetPath());

  pathURL.SetProtocol("");
  CHECK(!pathURL.HasProtocol());
  CHECK_EQUAL(_ASTR(""), pathURL.GetProtocol());
  CHECK_EQUAL(_ASTR("bitscollider.com/img/narf.foo"),
              pathURL.GetPath());


  // make sure nothing changes if we set the protocol to empty again
  pathURL.SetProtocol("");
  CHECK(!pathURL.HasProtocol());
  CHECK_EQUAL(_ASTR(""), pathURL.GetProtocol());
  CHECK_EQUAL(_ASTR("bitscollider.com/img/narf.foo"),
              pathURL.GetPath());

  // add back a protocol
  pathURL.SetProtocol("foo");
  CHECK(pathURL.HasProtocol());
  CHECK_EQUAL(_ASTR("foo"), pathURL.GetProtocol());
  CHECK_EQUAL(_ASTR("foo://bitscollider.com/img/narf.foo"),
              pathURL.GetPath());
}
