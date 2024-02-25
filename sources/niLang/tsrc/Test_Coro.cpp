#include "stdafx.h"

#if (__has_include(<coroutine>) && (defined(niJSCC) || !defined(__clang__))) ||        \
  (__has_include(<experimental/coroutine>) && defined(__clang__))
#define niCoroutines

#if (__has_include(<experimental/coroutine>) && defined(__clang__))

// Don't complain about your own stupid, thanks...
EA_DISABLE_CLANG_WARNING(-Wdeprecated-experimental-coroutine);

#include <experimental/coroutine>
namespace std_coro = std::experimental;
#else
#include <coroutine>
namespace std_coro = std;
#endif
#else
#pragma message "W/Coroutines not supported!"
#endif

#if defined niCoroutines

namespace ni {

struct iGenerator : public iRunnable {
  virtual ni::tBool __stdcall IsDone() = 0;
  virtual ni::tBool __stdcall Resume() = 0;
  virtual ni::Var __stdcall GetValue() const = 0;
};

// References:
// - https://www.youtube.com/watch?v=J7fYddslH0Q&t=1004s
struct CoVarGenerator {
  struct promise_type;

  std_coro::coroutine_handle<promise_type> mHandle;

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

  //
  // the promise is the central point, its where the data should be stored
  //
  struct promise_type
  // or std_coro::coroutine_traits<CoVarGenerator, ...>
  // (cf https://en.cppreference.com/w/cpp/coroutine/coroutine_traits)
  {
    ni::Var mValue { niVarNull };

    promise_type(ni::Var aValue)
        : mValue(aValue)
    {}

    promise_type()
    {}

    // promise_type(T...); // optional
    // ---- Start
    CoVarGenerator get_return_object() {
      return CoVarGenerator {
        std_coro::coroutine_handle<promise_type>::from_promise(*this)
      };
    }

    std_coro::suspend_always initial_suspend() { return {}; } // start suspended
    // std_coro::suspend_never initial_suspend() { return {}; } // run right away

    // ---- Shutdown
    void unhandled_exception() {}
    SuspendAlways final_suspend() noexcept { return {}; }

    // ---- Yield
    std_coro::suspend_always yield_value(const ni::Var& aValue) {
      mValue = aValue;
      return {};
    }

    // ---- Return
    void return_value(const ni::Var& aValue) {
      mValue = aValue;
    }
  };

  CoVarGenerator(std_coro::coroutine_handle<promise_type> aHandle)
      : mHandle(aHandle) {}

  niFn(ni::tBool) IsDone() {
    return mHandle.done();
  }

  ni::tBool Resume() {
    if (mHandle.done())
      return ni::eFalse;
    mHandle.resume();
    return ni::eTrue;
  }

  ni::Var GetValue() const {
    return mHandle.promise().mValue;
  }

  ni::Var __stdcall Run() {
    if (!Resume())
      return niVarNull;
    return GetValue();
  }
};

struct sGenerator : public ImplRC<
  iGenerator,eImplFlags_DontInherit1,iRunnable>
{
  sGenerator(CoVarGenerator& aGenerator)
      : _generator(aGenerator)
  {
  }

  virtual ni::tBool __stdcall IsDone() niImpl {
    return _generator.IsDone();
  }

  virtual ni::tBool __stdcall Resume() niImpl {
    return _generator.Resume();
  }

  virtual ni::Var __stdcall GetValue() const niImpl  {
    return _generator.GetValue();
  }

  virtual ni::Var __stdcall Run() niImpl  {
    return _generator.Run();
  }

  ni::CoVarGenerator _generator;
};

Nonnull<iGenerator> CreateGenerator(CoVarGenerator&& aGenerator) {
  return MakeNonnull<sGenerator>(aGenerator);
}

#if 0
// I dont see a good use case for copying a generator?
Nonnull<iGenerator> CreateGenerator(CoVarGenerator& aGenerator) {
  return MakeNonnull<sGenerator>(aGenerator);
}
#endif

}

ni::CoVarGenerator hello_coro() {
  int x = 1;
  niDefer {
    niDebugFmt(("... hello_coro deferred"));
  };

  niDebugFmt(("... hello_coro 1"));
  x *= 10;
  co_yield ni::Var(1 * x);

  niDebugFmt(("... hello_coro 2"));
  x *= 10;
  co_yield ni::Var(2 * x);

  niDebugFmt(("... hello_coro 3"));
  x *= 10;
  co_yield ni::Var(3 * x);

  niDebugFmt(("... hello_coro return 123"));
  co_return 123;
}

struct FCoro {
};

TEST_FIXTURE(FCoro,OnStack) {
  niDebugFmt(("... before hello_coro called"));
  ni::CoVarGenerator c = hello_coro();
  niDebugFmt(("... hello_coro called"));
  for (;;) {
    if (!c.Resume())
      break;
    niDebugFmt((".. CoVarGenerator.GetValue(): '%s'", c.GetValue()));
  }
  niDebugFmt(("... done"));
}

TEST_FIXTURE(FCoro,Generator) {
  niLetMut g = ni::CreateGenerator(hello_coro());
  niDebugFmt(("... Generator created"));
  while (!g->IsDone()) {
    niDebugFmt((".. Run(): '%s'", g->Run()));
  }
  niDebugFmt(("... Generator done"));
}

TEST_FIXTURE(FCoro,GeneratorLambda) {
  niLetMut g = ni::CreateGenerator([]() -> ni::CoVarGenerator {
    niDefer {
      niDebugFmt(("... coro_lambda deferred"));
    };
    int x = 1;
    niLoop(i,5) {
      niDebugFmt(("... coro_lambda 1"));
      co_yield ni::Var(i * x);
      x *= 10;
    }

    co_return niVarNull;
  }());
  niDebugFmt(("... GeneratorLambda created"));
  while (!g->IsDone()) {
    niDebugFmt((".. Run(): '%s'", g->Run()));
  }
  niDebugFmt(("... GeneratorLambda done"));
}

#endif // niCoroutines
