#ifndef ASTL_ATOMIC
#define ASTL_ATOMIC

#include <atomic>

namespace astl {

template <typename T>
using atomic = std::atomic<T>;

}  // namespace astl
#endif /* ASTL_ATOMIC */
