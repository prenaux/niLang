#ifndef ASTL_QUEUE
#define ASTL_QUEUE

#include "stl_alloc.h"
#include "EASTL/queue.h"

namespace astl {

ASTL_RAW_ALLOCATOR_IMPL(queue);

template <class _Tp, class _Sequence = astl::deque<_Tp,ASTL_ALLOCATOR(_Tp,queue)> >
using queue = eastl::queue<_Tp, _Sequence>;

}  // namespace astl
#endif /* ASTL_QUEUE */
