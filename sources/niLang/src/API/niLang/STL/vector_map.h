#ifndef __STL_VECTOR_MAP_H_06231B71_79B6_4011_ADEF_9F7403F1E047__
#define __STL_VECTOR_MAP_H_06231B71_79B6_4011_ADEF_9F7403F1E047__

#include "stl_alloc.h"
#include "EASTL/vector_map.h"

namespace astl {

ASTL_RAW_ALLOCATOR_IMPL(vector_map);

template <class _Key, class _Tp,
          ASTL_TMPL_PARAM(_Compare, eastl::less<_Key>),
          ASTL_TMPL_PARAM_PAIR_ALLOCATOR(_Alloc,const _Key,_Tp,map)>
using vector_map = eastl::vector_map<_Key,_Tp,_Compare,_Alloc>;

}  // namespace astl
#endif // __STL_VECTOR_MAP_H_06231B71_79B6_4011_ADEF_9F7403F1E047__
