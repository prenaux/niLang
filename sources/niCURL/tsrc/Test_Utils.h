#ifndef __TEST_UTILS_H_5A7925D6_22A0_524F_B61C_00C7D593C3B5__
#define __TEST_UTILS_H_5A7925D6_22A0_524F_B61C_00C7D593C3B5__

namespace ni {

static ni::cString _GetHTTPSTestCasesUrl(const achar* aPath)
{
  return niFmt("https://www.bytecollider.com/test_cases/%s", aPath);
}

// Start with:
//   cd "$WORK/../pierre/sources/nas/web/bitscollider/test_cases"
//   . hat repos php_8
//   php -S 0.0.0.0:4444
static ni::cString _GetLocalTestCasesUrl(const achar* aPath)
{
  return niFmt("http://localhost:4444/%s", aPath);
}

} // namespace ni
#endif // __TEST_UTILS_H_5A7925D6_22A0_524F_B61C_00C7D593C3B5__
