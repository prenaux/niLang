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

  std_coro::coroutine_handle<promise_type> mHandle;

  CoHelloR(std_coro::coroutine_handle<promise_type> aHandle)
      : mHandle(aHandle) {}

  void resume() {
    mHandle.resume();
  }

  // Awaitable interface
  struct Awaitable {
    bool await_ready() noexcept;
    void await_suspend(std_coro::coroutine_handle<promise_type>) noexcept;
    void await_resume()  noexcept;
  };

  // Equivalent to std_coro::suspend_always
  struct SuspendAlways {
    bool await_ready() noexcept { return false; }
    void await_suspend(std_coro::coroutine_handle<promise_type>) noexcept {}
    void await_resume() noexcept {}
  };

  // https://en.cppreference.com/w/cpp/coroutine/coroutine_traits
  struct promise_type /* or std_coro::coroutine_traits<CoHelloR, ...> */ {
    // promise_type(T...); // optional
    // ---- Start
    CoHelloR get_return_object() {
      return CoHelloR {
        std_coro::coroutine_handle<promise_type>::from_promise(*this)
      };
    }
    std_coro::suspend_always initial_suspend() { return {}; } // start suspended
    // std_coro::suspend_never initial_suspend() { return {}; } // run right away
    // ---- Shutdown
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
  CoHelloR c = hello_coro();
  c.resume();
}
