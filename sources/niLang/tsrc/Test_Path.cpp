#include "stdafx.h"
#include "../src/API/niLang/Utils/Path.h"

static const char* const _kTestPathAbsWithDrive = "n:/not_a_dir/not_a_file.noext";
static const char* const _kTestPathAbsVol = "/not_a_volume/not_a_dir/not_a_file.noext";
static const char* const _kTestPathRel = "not_a_dir/not_a_file.noext";

struct FPath {
};

using namespace ni;

TEST_FIXTURE(FPath,IsAbsPath) {
  CHECK_EQUAL(eTrue, StrIsAbsolutePath("\\foobar"));
  CHECK_EQUAL(eTrue, StrIsAbsolutePath("/foobar"));
  CHECK_EQUAL(eTrue, StrIsAbsolutePath("n:/foobar"));
  CHECK_EQUAL(eTrue, StrIsAbsolutePath("n:\\foobar"));
  CHECK_EQUAL(eTrue, StrIsAbsolutePath("N:/foobar"));
  CHECK_EQUAL(eTrue, StrIsAbsolutePath("N:\\foobar"));
  CHECK_EQUAL(eFalse, StrIsAbsolutePath("n:foobar"));
  CHECK_EQUAL(eFalse, StrIsAbsolutePath(":foobar"));
  CHECK_EQUAL(eFalse, StrIsAbsolutePath("foobar:"));
  CHECK_EQUAL(eFalse, StrIsAbsolutePath(NULL));
  CHECK_EQUAL(eFalse, StrIsAbsolutePath("nn:/foobar"));

  const cPath pathAbsDrive(_kTestPathAbsWithDrive);
  CHECK_EQUAL(eTrue, pathAbsDrive.IsAbsolute());

  const cPath pathAbsVol(_kTestPathAbsVol);
  CHECK_EQUAL(eTrue, pathAbsVol.IsAbsolute());

  const cPath pathRel(_kTestPathRel);
  CHECK_EQUAL(eFalse, pathRel.IsAbsolute());
}

TEST_FIXTURE(FPath,PathRel) {
  const cString cwd = ni::GetLang()->GetProperty("ni.app.cwd");
  niDebugFmt(("... cwd: '%s'", cwd));
  CHECK(!cwd.IsEmpty());

  CHECK_EQUAL(cwd + _ASTR(_kTestPathRel), ni::GetRootFS()->GetAbsolutePath(_kTestPathRel));
}

TEST_FIXTURE(FPath,PathAbsVol) {
  cString r = ni::GetRootFS()->GetAbsolutePath(_kTestPathAbsVol);
  // skip the drive if one has been added
  const char* rp = r.Chars();
  if (StrStartsWithDriveSep(rp+1)) {
    rp += 2;
  }
  CHECK_EQUAL(_ASTR(_kTestPathAbsVol), _ASTR(rp));
}

TEST_FIXTURE(FPath,PathAbsWithDrive) {
  CHECK_EQUAL(_ASTR(_kTestPathAbsWithDrive), ni::GetRootFS()->GetAbsolutePath(_kTestPathAbsWithDrive));
}

TEST_FIXTURE(FPath,Base) {
  {
    const cPath pathAbsDrive(_kTestPathAbsWithDrive);
    CHECK_EQUAL(pathAbsDrive.GetPath(), cPath("n:/not_a_dir/", "not_a_file.noext").GetPath());
    CHECK_EQUAL(pathAbsDrive.GetPath(), cPath("n:/not_a_dir", "not_a_file.noext").GetPath());
  }

  {
    const cPath pathAbsVol(_kTestPathAbsVol);
    CHECK_EQUAL(pathAbsVol.GetPath(), cPath("/not_a_volume/not_a_dir/", "not_a_file.noext").GetPath());
    CHECK_EQUAL(pathAbsVol.GetPath(), cPath("/not_a_volume/not_a_dir", "not_a_file.noext").GetPath());
  }

  {
    const cPath pathRel(_kTestPathRel);
    CHECK_EQUAL(pathRel.GetPath(), cPath("not_a_dir/", "not_a_file.noext").GetPath());
    CHECK_EQUAL(pathRel.GetPath(), cPath("not_a_dir", "not_a_file.noext").GetPath());
  }

  {
    const cPath pathRel(_kTestPathRel);
    cPath path = _kTestPathRel;

    path = _kTestPathRel;

    CHECK_EQUAL(eFalse, path.IsAbsolute());

    path.PushDirectory();
    {
      path.AddDirectoryBack("back_dir");
      CHECK_EQUAL(_ASTR("not_a_dir/back_dir/not_a_file.noext"), path.GetPath());
      path.AddDirectoryBack("back_dir2");
      CHECK_EQUAL(_ASTR("not_a_dir/back_dir/back_dir2/not_a_file.noext"), path.GetPath());
      path.RemoveDirectoryBack();
      CHECK_EQUAL(_ASTR("not_a_dir/back_dir/not_a_file.noext"), path.GetPath());
    }
    path.PopDirectory();
    CHECK_EQUAL(pathRel.GetPath(), path.GetPath());

    path.PushDirectory();
    {
      path.AddDirectoryFront("front_dir");
      CHECK_EQUAL(_ASTR("front_dir/not_a_dir/not_a_file.noext"), path.GetPath());
      path.AddDirectoryFront("front_dir2");
      CHECK_EQUAL(_ASTR("front_dir2/front_dir/not_a_dir/not_a_file.noext"), path.GetPath());
    }
    path.PopDirectory();
    CHECK_EQUAL(pathRel.GetPath(), path.GetPath());

    path.SetExtension("");
    CHECK_EQUAL(_ASTR("not_a_dir/not_a_file"), path.GetPath());
    CHECK_EQUAL(_ASTR(""), path.GetExtension());

    path.SetExtension("noext");
    CHECK_EQUAL(_ASTR("not_a_dir/not_a_file.noext"), path.GetPath());
    CHECK_EQUAL(_ASTR("noext"), path.GetExtension());
    CHECK_EQUAL(_ASTR("not_a_file"), path.GetFileNoExt());
    CHECK_EQUAL(pathRel.GetPath(), path.GetPath());

    path.SetExtension(NULL);
    CHECK_EQUAL(_ASTR("not_a_dir/not_a_file"), path.GetPath());

    path.SetExtension(".foo");
    CHECK_EQUAL(_ASTR("not_a_dir/not_a_file.foo"), path.GetPath());
    CHECK_EQUAL(_ASTR("not_a_file"), path.GetFileNoExt());
    CHECK_EQUAL(_ASTR("foo"), path.GetExtension());

    path.SetFileNoExt("roger");
    CHECK_EQUAL(_ASTR("not_a_dir/roger.foo"), path.GetPath());

    path.SetFileNoExt("BarFile");
    path.SetDirectory("foodir");
    CHECK_EQUAL(_ASTR("foodir/BarFile.foo"), path.GetPath());
  }

  {
    const cPath pathAbsDrive(_kTestPathAbsWithDrive);
    cPath path = _kTestPathAbsWithDrive;

    path.PushDirectory();
    {
      // Disabled in debug builds because it'll assert and fail the test...
// #if !defined niDebug
      // path.AddDirectoryFront("front_dir");
      // CHECK_EQUAL(_ASTR("front_dir/N:/not_a_dir/not_a_file.noext"), path.GetPath());
// #endif
    }
    path.PopDirectory();
    CHECK_EQUAL(pathAbsDrive.GetPath(), path.GetPath());
  }

  {
    cPath path;
    path.SetProtocol("http")
        .SetFile("BarFile.foo")
        .SetDirectory("foodir");
    CHECK_EQUAL(_ASTR("http://foodir/BarFile.foo"), path.GetPath());

    path.SetProtocol("proto");
    CHECK_EQUAL(_ASTR("proto://foodir/BarFile.foo"), path.GetPath());

    path.SetDirectory("file://some/dir");
    CHECK_EQUAL(_ASTR("file://some/dir/BarFile.foo"), path.GetPath());

    path.SetDirectory("another/place");
    CHECK_EQUAL(_ASTR("file://another/place/BarFile.foo"), path.GetPath());
    CHECK_EQUAL(_ASTR("file://another/place/"), path.GetDirectory());
    CHECK_EQUAL(_ASTR("another/place/"), path.GetDirectoryNoProtocol());

    path.RemoveDirectoryBack().AddDirectoryBack("back");
    CHECK_EQUAL(_ASTR("file://another/back/BarFile.foo"), path.GetPath());

    path.AddDirectoryFront("fagiano");
    CHECK_EQUAL(_ASTR("file://fagiano/another/back/BarFile.foo"), path.GetPath());
  }
}
