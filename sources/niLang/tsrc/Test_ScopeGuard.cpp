#include "stdafx.h"
#include <niLang/STL/scope_guard.h>

namespace astl {

}

struct FScopeGuard {
};

TEST_FIXTURE(FScopeGuard,Base) {
  int scopeAcquireCount = 0;
  int scopeReleaseCount = 0;
  int scope2ReleaseCount = 0;
  {
    auto guard = astl::make_scope_guard(
        [&]() {
          niDebugFmt(("... ScopeAcquire"));
          ++scopeAcquireCount;
        },
        [&]() {
          niDebugFmt(("... ScopeRelease"));
          ++scopeReleaseCount;
        });

    auto guard_rel = astl::make_scope_guard(
        [&]() {
          niDebugFmt(("... Scope2Release"));
          ++scope2ReleaseCount;
        });
  }
  CHECK_EQUAL(1, scopeAcquireCount);
  CHECK_EQUAL(1, scopeReleaseCount);
  CHECK_EQUAL(1, scope2ReleaseCount);
}

TEST_FIXTURE(FScopeGuard,ScopeExit) {
  int scopeExitCount = 0;
  int scopeDontExitCount = 0;
  do
  {
    ASTL_SCOPE_EXIT {
      niDebugFmt(("... ScopeExit"));
      ++scopeExitCount;
    };
    niDebugFmt(("... ScopeStuff"));

    break;

    ASTL_SCOPE_EXIT {
      niDebugFmt(("... DontExit Me"));
      ++scopeDontExitCount;
    };
  } while(0);
  CHECK_EQUAL(1, scopeExitCount);
  CHECK_EQUAL(0, scopeDontExitCount);
}
