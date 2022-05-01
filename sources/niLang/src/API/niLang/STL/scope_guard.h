#pragma once
#ifndef __SCOPE_GUARD_H_6A722F15_3212_46D9_B403_09FC3A228CA4__
#define __SCOPE_GUARD_H_6A722F15_3212_46D9_B403_09FC3A228CA4__

#include "../Types.h"

#ifdef niCPP_Lambda

#include "preproc.h"
#include "EASTL/algorithm.h"

namespace astl {

template <typename TReleaseFun>
struct scope_guard
{
  scope_guard(TReleaseFun afunRelease)
      : _active(true)
      , _funRelease(eastl::move(afunRelease))
  {}

  // allows move semantic (value is semantic is not allowed)
  scope_guard(scope_guard&& aScopeGuard)
      : _active(aScopeGuard._active)
      , _funRelease(eastl::move(aScopeGuard._funRelease))
  {
    aScopeGuard.dismiss();
  }

  ~scope_guard() {
    if (_active) {
      _funRelease();
    }
  }

  inline void dismiss() {
    _active = false;
  }
  inline bool is_active() {
    return _active;
  }

 private:
  bool _active;
  TReleaseFun _funRelease;

  scope_guard();
  scope_guard(const scope_guard&);
  scope_guard& operator = (const scope_guard&);
};


// Make a scope guard, guaranteeing that if acquire throws release will be called
template <typename TAcquireFun, typename TReleaseFun>
static scope_guard<TReleaseFun> make_scope_guard(TAcquireFun&& afunAcquire, TReleaseFun&& afunRelease) {
  auto scope = astl::scope_guard<TReleaseFun>(eastl::forward<TReleaseFun>(afunRelease));
  afunAcquire();
  return scope;
}

template<typename TReleaseFun>
static scope_guard< TReleaseFun > make_scope_guard(TReleaseFun&& afunRelease) {
  return scope_guard< TReleaseFun >(eastl::forward<TReleaseFun>(afunRelease));
}

struct scope_guard_on_exit {
  template <typename TReleaseFun>
  inline scope_guard<TReleaseFun> operator << (TReleaseFun&& afunRelease) {
    return scope_guard<TReleaseFun>(afunRelease);
  }
};

#define ASTL_SCOPE_EXIT auto ASTL_ANONYMOUS_VARIABLE(__scope_exit) = astl::scope_guard_on_exit() << [&]()
#define niDefer ASTL_SCOPE_EXIT

}
#else
#error "The C++ compiler must support Lambda & auto."
#endif

#endif // __SCOPE_GUARD_H_6A722F15_3212_46D9_B403_09FC3A228CA4__
