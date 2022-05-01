#ifndef ASTL_SET
#define ASTL_SET

#include "stl_alloc.h"
#include "EASTL/set.h"

namespace astl {

ASTL_RAW_ALLOCATOR_IMPL(set);

template <class _Key, ASTL_TMPL_PARAM(_Compare,eastl::less<_Key>),
          ASTL_TMPL_PARAM_ALLOCATOR(_Alloc,_Key,set)>
using set = eastl::set<_Key,_Compare,_Alloc>;

}  // namespace astl
#endif /* ASTL_SET */
