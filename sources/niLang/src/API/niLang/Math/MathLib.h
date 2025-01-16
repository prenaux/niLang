#pragma once
#ifndef __MATH_H_B1B63586_C9EC_4727_B3A3_36AA22915CBF__
#define __MATH_H_B1B63586_C9EC_4727_B3A3_36AA22915CBF__

#define MATH_IMPL_INCLUDE_DEPS
#define MATH_IMPL_FORWARDS 0
#define MATH_IMPL_BEGIN namespace ni {
#define MATH_IMPL_END }
#define MATH_IMPL_FN(TYPE) [[nodiscard]] __forceinline TYPE __stdcall
#define MATH_IMPL_FN_VOID __forceinline void __stdcall
#define MATH_IMPL_CONST_OVR
#include "MathImpl.h"

#endif // __MATH_H_B1B63586_C9EC_4727_B3A3_36AA22915CBF__
