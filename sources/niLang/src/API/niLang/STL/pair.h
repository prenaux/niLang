#pragma once
#ifndef __PAIR_H_B73CE45B_19B0_2142_80C9_BC79A3C1D64D__
#define __PAIR_H_B73CE45B_19B0_2142_80C9_BC79A3C1D64D__

#include "EASTL/utility.h"

namespace astl {

template <typename K, typename V>
using pair = eastl::pair<K,V>;

using eastl::make_pair;

} // namespace astl
#endif // __PAIR_H_B73CE45B_19B0_2142_80C9_BC79A3C1D64D__
