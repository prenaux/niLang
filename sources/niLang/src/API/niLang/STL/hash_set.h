#pragma once
#ifndef __HASH_SET_H_97424047_80A3_8344_B655_5798A7AC95AD__
#define __HASH_SET_H_97424047_80A3_8344_B655_5798A7AC95AD__

#include "stl_alloc.h"
#include "EASTL/hash_set.h"

namespace astl {

ASTL_RAW_ALLOCATOR_IMPL(hash_set);

template <class _Value,
          class _Hash = eastl::hash<_Value>,
          class _Pred = eastl::equal_to<_Value>,
          ASTL_TMPL_PARAM_ALLOCATOR(_Alloc,_Value,hash_set) >
using hash_set = eastl::hash_set<_Value,_Hash,_Pred,_Alloc>;

}  // namespace astl
#endif // __HASH_SET_H_97424047_80A3_8344_B655_5798A7AC95AD__
