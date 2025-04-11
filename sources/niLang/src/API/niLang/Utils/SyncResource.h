#ifndef __SYNCRESOURCE_H_FB00E394_7077_486E_8939_A30168AA9A3C__
#define __SYNCRESOURCE_H_FB00E394_7077_486E_8939_A30168AA9A3C__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "ThreadImpl.h"
#include "../STL/tuple.h"

/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

namespace astl {

// Adoption tag that matches std::adopt_lock
struct adopt_lock_t {
  explicit adopt_lock_t() = default;
};
inline constexpr adopt_lock_t adopt_lock{};

} // namespace astl

namespace ni {

inline void ThreadYield()
{
  #ifdef niWin32
  Sleep(0);
  #elif defined niPosix
  sched_yield();
  #elif defined niNoThreads
  // do nothing...
  #else
    #error "ThreadYield not implemented on this platform!"
  #endif
}

inline void ThreadLock(ThreadMutex& m0)
{
  m0.ThreadLock();
}

template <class... MutexTypes>
inline void ThreadLock(ThreadMutex& m0, ThreadMutex& m1, MutexTypes&... mutexes)
{
  // Track which mutexes we need to lock
  constexpr size_t n = sizeof...(MutexTypes) + 2; // +2 for m0, m1
  ThreadMutex* mutex_ptrs[] = { &m0, &m1, &mutexes... };
  bool locked[n] = { false };
  size_t lock_count = 0;

  // Starting position (which we'll rotate)
  size_t start_pos = 0;

  while (lock_count < n) {
    // Try each mutex in the current rotation
    for (size_t i = 0; i < n; ++i) {
      size_t idx = (start_pos + i) % n;

      // Skip already locked mutexes
      if (locked[idx])
        continue;

      // Use non-blocking ThreadTryLock for ALL mutexes
      if (mutex_ptrs[idx]->ThreadTryLock()) {
        locked[idx] = true;
        lock_count++;
      }
      else {
        // On failure, rotate and try again
        // If we didn't get all locks, we need to release any locks we did get
        if (lock_count > 0 && lock_count < n) {
          for (size_t j = 0; j < n; j++) {
            if (locked[j]) {
              mutex_ptrs[j]->ThreadUnlock();
              locked[j] = false;
            }
          }
          lock_count = 0;
        }

        // Change starting position and try again
        start_pos = (start_pos + 1) % n;
        ni::ThreadYield();
        break;
      }
    }
  }
}

inline void ThreadUnlock(ThreadMutex& m0)
{
  m0.ThreadUnlock();
}

template <class... MutexTypes>
inline void ThreadUnlock(ThreadMutex& m0, ThreadMutex& m1, MutexTypes&... mutexes)
{
  // Unlock all mutexes (order doesn't matter for unlocking)
  (mutexes.ThreadUnlock(), ...);
  m1.ThreadUnlock();
  m0.ThreadUnlock();
}

template <typename T>
struct sSyncResourceAccess {
  T& _resource;
  ThreadMutex* _mutex = nullptr;

  sSyncResourceAccess(T& resource, ThreadMutex& mutex)
      : _resource(resource)
      , _mutex(&mutex)
  {
    _mutex->ThreadLock();
  }

  sSyncResourceAccess(T& resource, ThreadMutex& mutex, astl::adopt_lock_t)
      : _resource(resource)
      , _mutex(&mutex)
  {
    // lock is already acquired externally (astl::adopt_lock)
  }

  // Move constructor (transfers lock ownership)
  sSyncResourceAccess(sSyncResourceAccess&& other) noexcept
      : _resource(other._resource)
      , _mutex(other._mutex)
  {
    other._mutex = nullptr; // transfer lock ownership
  }

  // Destructor unlocks if mutex is still held
  ~sSyncResourceAccess()
  {
    if (_mutex) {
      _mutex->ThreadUnlock();
    }
  }

  sSyncResourceAccess(const sSyncResourceAccess&) = delete;
  sSyncResourceAccess& operator=(const sSyncResourceAccess&) = delete;

  T* operator->()
  {
    return &_resource;
  }
  T& operator*()
  {
    return _resource;
  }

  T& operator=(T&& aRight)
  {
    _resource = astl::move(aRight);
    return _resource;
  }

  T& operator=(ain<T> aRight)
  {
    _resource = aRight;
    return _resource;
  }

  operator T&()
  {
    return _resource;
  }
  operator const T&() const
  {
    return _resource;
  }
};

template <typename T>
struct sSyncResource {
  T _value;
  mutable ThreadMutex _mutex;

 public:
  sSyncResource()
      : _value()
  {
  }

  template <typename... Args>
  sSyncResource(Args&&... args)
      : _value(astl::forward<Args>(args)...)
  {
  }

  sSyncResourceAccess<T> Lock()
  {
    return sSyncResourceAccess<T>(_value, _mutex);
  }

  sSyncResourceAccess<const T> Lock() const
  {
    return sSyncResourceAccess<const T>(_value, _mutex);
  }

  sSyncResourceAccess<T> Lock(astl::adopt_lock_t)
  {
    return sSyncResourceAccess<T>(_value, _mutex, astl::adopt_lock);
  }

  sSyncResourceAccess<const T> Lock(astl::adopt_lock_t) const
  {
    return sSyncResourceAccess<const T>(_value, _mutex, astl::adopt_lock);
  }
};

template <typename R>
inline auto SyncLock(R& resource)
{
  return resource.Lock();
}

template <typename R1, typename R2>
inline auto SyncLock(R1& r1, R2& r2)
{
  ThreadLock(r1._mutex, r2._mutex);
  return astl::make_tuple(r1.Lock(astl::adopt_lock), r2.Lock(astl::adopt_lock));
}

template <typename R1, typename R2, typename R3>
inline auto SyncLock(R1& r1, R2& r2, R3& r3)
{
  ThreadLock(r1._mutex, r2._mutex, r3._mutex);
  return astl::make_tuple(r1.Lock(astl::adopt_lock), r2.Lock(astl::adopt_lock),
                          r3.Lock(astl::adopt_lock));
}

template <typename R1, typename R2, typename R3, typename R4>
inline auto SyncLock(R1& r1, R2& r2, R3& r3, R4& r4)
{
  ThreadLock(r1._mutex, r2._mutex, r3._mutex, r4._mutex);
  return astl::make_tuple(r1.Lock(astl::adopt_lock), r2.Lock(astl::adopt_lock),
                          r3.Lock(astl::adopt_lock), r4.Lock(astl::adopt_lock));
}

} // namespace ni

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
#endif // __SYNCRESOURCE_H_FB00E394_7077_486E_8939_A30168AA9A3C__
