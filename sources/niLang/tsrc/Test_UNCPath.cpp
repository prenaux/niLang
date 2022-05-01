#include "stdafx.h"

#ifdef niWinDesktop
#include "../src/API/niLang/Platforms/Win32/Win32_Redef.h"
#include "../src/API/niLang/Platforms/Win32/Win32_UTF.h"
#include <windows.h>
#include <intrin.h>

using namespace ni;

struct FUNCPath {
};

niExportFunc(achar*) FixSystemDir(achar* aaszOutput, const achar* aaszBuffer, const achar* aaszToCreate, tBool abCreate);

static cString _Win32GetSpecialFolderPersonal() {
  achar buffer[AMAX_PATH] = {0};
  ni::Windows::utf8_SHGetSpecialFolderPath(NULL, buffer, CSIDL_PROFILE, 1);
  return buffer;
}

static cString _Win32GetFolderPath(int flag) {
  achar buffer[AMAX_PATH] = {0};
  ni::Windows::utf8_SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, flag, buffer);
  return buffer;
}

static cString _GetHomeDir() {
  achar aaszOutput[AMAX_PATH];
  achar buffer[AMAX_PATH] = {0};
  ni::Windows::utf8_SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, SHGFP_TYPE_DEFAULT, buffer);
  return FixSystemDir(aaszOutput,buffer,NULL,eFalse);
}

TEST_FIXTURE(FUNCPath,ResolveUNC) {
  cString dirGetSpecialFolderPersonal = _Win32GetSpecialFolderPersonal();
  cString dirGetFolderPathDefault = _Win32GetFolderPath(SHGFP_TYPE_DEFAULT);
  cString dirGetFolderPathCurrent = _Win32GetFolderPath(SHGFP_TYPE_CURRENT);

  niDebugFmt(("... dirGetSpecialFolderPersonal: %s",dirGetSpecialFolderPersonal));
  niDebugFmt(("... dirGetFolderPathDefault: %s",dirGetFolderPathDefault));
  niDebugFmt(("... dirGetFolderPathCurrent %s",dirGetFolderPathCurrent));
  // the old way we looking for home should be the same as we call SHGFP_TYPE_CURRENT
  CHECK_EQUAL(dirGetSpecialFolderPersonal, dirGetFolderPathCurrent);

  // check if we are not starting with the uncpath from windows
  CHECK(!dirGetFolderPathDefault.StartsWith("\\\\"));

  cString homeDir = _GetHomeDir();
  cString home = ni::GetLang()->GetProperty("ni.dirs.home");
  niDebugFmt(("... ni.dirs.home: %s",home));
  niDebugFmt(("... homeDir: %s",homeDir));

  // check if we are not starting with the uncpath from windows
  CHECK(!home.StartsWith("\\\\"));

  // check if the home dir property is correct
  CHECK_EQUAL(home, homeDir);
}
#endif // end of #ifdef niWinDesktop
