#include "stdafx.h"

#if (__has_include(<coroutine>) && !defined(__clang__)) ||        \
  (__has_include(<experimental/coroutine>) && defined(__clang__))
#define niCoroutines
#if (__has_include(<experimental/coroutine>) && defined(__clang__))
#include <experimental/coroutine>
namespace std_coro = std::experimental;
#else
#include <coroutine>
#define niCoroNS(SYM) std::SYM
namespace std_coro = std;
#endif
#else
#error "E/Coroutines not supported!"
#endif

// References:
// - https://www.youtube.com/watch?v=J7fYddslH0Q&t=1004s
struct CoHelloR {
  struct promise_type;

  // Awaitable interface
  struct Awaitable {
    bool await_ready() noexcept;
    void await_suspend(std_coro::coroutine_handle<promise_type>) noexcept;
    void await_resume() noexcept;
  };

  // Equivalent to std_coro::suspend_always
  struct SuspendAlways {
    bool await_ready() noexcept { return false; }
    void await_suspend(std_coro::coroutine_handle<promise_type>) noexcept {}
    void await_resume() noexcept {}
  };

  struct promise_type {
    CoHelloR get_return_object() { return {}; }
    // niCoroNS(suspend_always) initial_suspend() { return {}; }
    std_coro::suspend_never initial_suspend() { return {}; }
    void return_void() {}
    void unhandled_exception() {}
    SuspendAlways final_suspend() noexcept { return {}; }
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
