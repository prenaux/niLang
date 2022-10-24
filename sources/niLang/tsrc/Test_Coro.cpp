#include "stdafx.h"

#if (__has_include(<coroutine>) && !defined(__clang__)) || \
  (__has_include(<experimental/coroutine>) && defined(__clang__))
#define niCoroutines
#if (__has_include(<experimental/coroutine>) && defined(__clang__))
#include <experimental/coroutine>
#define niCoroNS(SYM) std::experimental::SYM
#else
#include <coroutine>
#define niCoroNS(SYM) std::SYM
#endif
#else
#error "E/Coroutines not supported!"
#endif

struct CoHelloR {

  struct promise_type {
    CoHelloR get_return_object() { return {}; }
    niCoroNS(suspend_always) initial_suspend() { return {}; }
    void return_void() {}
    void unhandled_exception() {}
    niCoroNS(suspend_always) final_suspend() noexcept { return {}; }
  };
};

CoHelloR hello_coro() {
  niDebugFmt(("... hello_coro"));
  co_return;
}

struct FCoro {
};

TEST_FIXTURE(FCoro,Hello) {
  hello_coro();
}
