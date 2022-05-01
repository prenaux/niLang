#pragma once
#ifndef __LIST_H_033B4714_1794_3140_93F1_62ACDB301F07__
#define __LIST_H_033B4714_1794_3140_93F1_62ACDB301F07__

#include "stl_alloc.h"
#include "EASTL/list.h"

namespace astl {

ASTL_RAW_ALLOCATOR_IMPL(list);

template <typename T, ASTL_TMPL_PARAM_ALLOCATOR(_Alloc,T,list)>
using list = eastl::list<T,_Alloc>;

}  // namespace astl
#endif // __LIST_H_033B4714_1794_3140_93F1_62ACDB301F07__
