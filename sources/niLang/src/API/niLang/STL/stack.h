#ifndef ASTL_STACK
#define ASTL_STACK

#include "stl_alloc.h"
#include "deque.h"
#include "EASTL/stack.h"

namespace astl {

ASTL_RAW_ALLOCATOR_IMPL(stack);

template <class _Tp, class _Sequence = astl::deque<_Tp,ASTL_ALLOCATOR(_Tp,stack)> >
using stack = eastl::stack<_Tp, _Sequence>;

}  // namespace astl
#endif /* ASTL_STACK */
