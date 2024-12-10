#pragma once
#ifndef __ASAN_H_208044CE_8BB6_EF11_94C3_2943A3C07827__
#define __ASAN_H_208044CE_8BB6_EF11_94C3_2943A3C07827__

#ifdef niLinux
//
// The LeakDetector on Linux (somehow not an issue on macOS?) detects some
// allocations in global variables as leaks. In my opinion, that's incorrect,
// but I may have missed something. Still, it renders LeakDetector unusable,
// as there’s no way to "fix" those leaks since they are allocated in global
// variables only once. This is a significant problem in UnitTest, as tests
// frequently fail during development. As a result, detect_leaks is disabled
// by default on Linux.
//
/*
See https://stackoverflow.com/a/51061314

You can run with export ASAN_OPTIONS=detect_leaks=0 or add a function to your application:
```
#ifdef __cplusplus
extern "C"
#endif
const char* __asan_default_options() { return "detect_leaks=0"; }
```

See [Asan flags wiki](https://github.com/google/sanitizers/wiki/AddressSanitizerFlags) and [common sanitizer flags](https://github.com/google/sanitizers/wiki/SanitizerCommonFlags) wiki for more details.

*/
#define niAsanDefaultOptions() niExternC const char* __asan_default_options() { return "detect_leaks=0"; }
#else
#define niAsanDefaultOptions()
#endif

#endif // __ASAN_H_208044CE_8BB6_EF11_94C3_2943A3C07827__
