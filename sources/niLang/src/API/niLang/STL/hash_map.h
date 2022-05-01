#pragma once
#ifndef __HASH_MAP_H_2069C775_6EC9_5143_8F30_57A8E259CFDB__
#define __HASH_MAP_H_2069C775_6EC9_5143_8F30_57A8E259CFDB__

#include "stl_alloc.h"
#include "EASTL/hash_map.h"

namespace astl {

ASTL_RAW_ALLOCATOR_IMPL(hash_map);

template <class _Key,
          class _Tp,
          class _Hash = eastl::hash<_Key>,
          class _Pred = eastl::equal_to<_Key>,
          ASTL_TMPL_PARAM_PAIR_ALLOCATOR(_Alloc,const _Key,_Tp,hash_map) >
using hash_map = eastl::hash_map<_Key,_Tp,_Hash,_Pred,_Alloc>;

}  // namespace astl
#endif // __HASH_MAP_H_2069C775_6EC9_5143_8F30_57A8E259CFDB__
