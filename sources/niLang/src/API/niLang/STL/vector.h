#ifndef ASTL_VECTOR
#define ASTL_VECTOR

#include "stl_alloc.h"
#include "EASTL/vector.h"

namespace astl {

ASTL_RAW_ALLOCATOR_IMPL(vector);

template <typename _Tp, ASTL_TMPL_PARAM_ALLOCATOR(_Alloc,_Tp,vector)>
using vector = eastl::vector<_Tp,_Alloc>;

}  // namespace astl
#endif /* ASTL_VECTOR */
