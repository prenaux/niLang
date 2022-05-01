#pragma once
#ifndef __MAP_H_C7390176_6BFD_A249_8532_DC55F7EF725A__
#define __MAP_H_C7390176_6BFD_A249_8532_DC55F7EF725A__

#include "stl_alloc.h"
#include "EASTL/map.h"

namespace astl {

ASTL_RAW_ALLOCATOR_IMPL(map);

template <class _Key, class _Tp,
          ASTL_TMPL_PARAM(_Compare, eastl::less<_Key>),
          ASTL_TMPL_PARAM_PAIR_ALLOCATOR(_Alloc,const _Key,_Tp,map)>
using map = eastl::map<_Key,_Tp,_Compare,_Alloc>;

}  // namespace astl
#endif // __MAP_H_C7390176_6BFD_A249_8532_DC55F7EF725A__
