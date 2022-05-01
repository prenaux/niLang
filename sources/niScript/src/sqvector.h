#ifndef __SQVECTOR_H_F01BAED7_665F_4301_B8A2_6F82689CE075__
#define __SQVECTOR_H_F01BAED7_665F_4301_B8A2_6F82689CE075__

#include "sqalloc.h"

#define SQ_VECTOR_TYPEDEF(T,NAME)                       \
  ASTL_RAW_ALLOCATOR_IMPL(sq_##NAME);                   \
  typedef astl::vector<T,ASTL_ALLOCATOR(T,sq_##NAME)> NAME;

#endif // __SQVECTOR_H_F01BAED7_665F_4301_B8A2_6F82689CE075__
