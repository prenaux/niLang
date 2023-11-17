#pragma once
#ifndef __TEST_UTILS_H_5A7925D6_22A0_524F_B61C_00C7D593C3B5__
#define __TEST_UTILS_H_5A7925D6_22A0_524F_B61C_00C7D593C3B5__

namespace ni {

static ni::cString _GetHTTPSTestCasesUrl(const achar* aPath) {
  return niFmt("https://www.bytecollider.com/test_cases/%s", aPath);
}

}
#endif // __TEST_UTILS_H_5A7925D6_22A0_524F_B61C_00C7D593C3B5__
