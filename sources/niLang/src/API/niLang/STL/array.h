#ifndef ASTL_ARRAY
#define ASTL_ARRAY

#include <array>

namespace astl {

template <typename T, size_t N = 1>
using array = std::array<T,N>;

}  // namespace astl
#endif /* ASTL_ARRAY */
