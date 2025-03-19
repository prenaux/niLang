#pragma once
#ifndef __STL_TUPLE_H_7E9B1898_ADB5_4849_AEEC_6C643BEE4E44__
#define __STL_TUPLE_H_7E9B1898_ADB5_4849_AEEC_6C643BEE4E44__

#include "EASTL/tuple.h"

namespace astl {

template <typename... Ts>
using tuple = eastl::tuple<Ts&...>;

using eastl::apply;

template <size_t... Is>
using index_sequence = eastl::index_sequence<Is...>;

template <size_t N>
using make_index_sequence = eastl::make_index_sequence<N>;

using eastl::make_tuple;

}
#endif // __STL_TUPLE_H_7E9B1898_ADB5_4849_AEEC_6C643BEE4E44__
