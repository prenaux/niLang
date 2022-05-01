#pragma once
#ifndef __DEQUE_H_9D7F5A10_78CF_BD4F_80C9_FC27C068E21E__
#define __DEQUE_H_9D7F5A10_78CF_BD4F_80C9_FC27C068E21E__

#include "stl_alloc.h"
#include "EASTL/deque.h"

namespace astl {

ASTL_RAW_ALLOCATOR_IMPL(deque);

template <typename _Tp, ASTL_TMPL_PARAM_ALLOCATOR(_Alloc,_Tp,deque)>
using deque = eastl::deque<_Tp,_Alloc>;

}  // namespace astl
#endif // __DEQUE_H_9D7F5A10_78CF_BD4F_80C9_FC27C068E21E__
