#ifndef ASTL_ARRAY
#define ASTL_ARRAY

#include "EASTL/array.h"

namespace astl {

template <typename T, size_t N = 1>
using array = eastl::array<T,N>;

}  // namespace astl
#endif /* ASTL_ARRAY */
