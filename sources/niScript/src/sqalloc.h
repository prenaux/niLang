#ifndef __SQALLOC_H_C4041C01_E26F_4175_BB5E_31614A3AF08D__
#define __SQALLOC_H_C4041C01_E26F_4175_BB5E_31614A3AF08D__

#include "sqconfig.h"

struct tagSQObject;
struct sSQDeepCloneGuardSet {
  ni::tU32               _depth;
  sSQDeepCloneGuardSet() : _depth(0) {}
};
typedef sSQDeepCloneGuardSet tSQDeepCloneGuardSet;

template <typename T>
struct SQAllocator : ni::cMemImpl {};

#define SQ_ALLOCATOR(TYPE) SQAllocator<TYPE>

#endif // __SQALLOC_H_C4041C01_E26F_4175_BB5E_31614A3AF08D__
