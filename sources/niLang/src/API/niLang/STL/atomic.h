#ifndef ASTL_ATOMIC
#define ASTL_ATOMIC

#include <atomic>

namespace astl {

template <typename T>
using atomic = std::atomic<T>;

using std::memory_order_relaxed;
using std::memory_order_consume;
using std::memory_order_acquire;
using std::memory_order_release;
using std::memory_order_acq_rel;
using std::memory_order_seq_cst;

}  // namespace astl
#endif /* ASTL_ATOMIC */
