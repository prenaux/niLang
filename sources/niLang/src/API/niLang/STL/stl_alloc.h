#pragma once
#ifndef __ALLOC_H_DC8709E9_3388_CD41_8EC6_5E98807EAE93__
#define __ALLOC_H_DC8709E9_3388_CD41_8EC6_5E98807EAE93__

#include "../Types.h"
#include "pair.h"

namespace astl {

#ifndef ASTL_TEMPLATE_NULL
#define ASTL_TEMPLATE_NULL template <>
#endif

#ifndef ASTL_TMPL_PARAM
#define ASTL_TMPL_PARAM(CLASSNAME, DEFVAL) typename CLASSNAME = DEFVAL
#endif

#define ASTL_RAW_ALLOCATOR_IMPL(NAME)                                   \
  struct raw_allocator_##NAME {                                         \
    static void* allocate(size_t n, int /*flags*/ = 0) {                \
      return niMalloc(n);                                               \
    }                                                                   \
    static void* aligned_allocate(size_t n, size_t alignment, size_t alignmentOffset, int /*flags*/ = 0) { \
      if ((alignmentOffset % alignment) == 0)  {                        \
        return niAlignedMalloc(n, alignment);                           \
      }                                                                 \
      return NULL;                                                      \
    }                                                                   \
    static void deallocate(void* p, size_t n) {                         \
      niUnused(n);                                                      \
      niFree(p);                                                        \
    }                                                                   \
  };

#define ASTL_ALLOCATOR(TYPE, NAME) \
  astl::astl_allocator<TYPE, raw_allocator_##NAME>
#define ASTL_TMPL_PARAM_ALLOCATOR(CLASSNAME, TYPE, NAME) \
  ASTL_TMPL_PARAM(CLASSNAME, ASTL_ALLOCATOR(TYPE, NAME) )

#define ASTL_PAIR_ALLOCATOR(T1, T2, NAME) \
  astl::astl_allocator<astl::pair<T1,T2>, raw_allocator_##NAME>
#define ASTL_TMPL_PARAM_PAIR_ALLOCATOR(CLASSNAME, T1, T2, NAME) \
  ASTL_TMPL_PARAM(CLASSNAME, ASTL_PAIR_ALLOCATOR(T1,T2,NAME) )

template <typename T, typename _RawAlloc>
class astl_allocator
{
 public:
	typedef _RawAlloc raw_allocator;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef T* pointer;
  typedef const T* const_pointer;
  typedef T& reference;
  typedef const T& const_reference;
  typedef T value_type;

  astl_allocator(const char* = NULL) {}

  astl_allocator(const astl_allocator&) {}

  astl_allocator(const astl_allocator&, const char*) {}

  astl_allocator& operator=(const astl_allocator&) { return *this; }

  bool operator==(const astl_allocator&) { return true; }

  bool operator!=(const astl_allocator&) { return false; }

  void* allocate(size_t n, int flags = 0) {
    return reinterpret_cast<pointer>(raw_allocator::allocate(n,flags));
  }
  void* allocate(size_t n, size_t alignment, size_t alignmentOffset, int flags = 0) {
    return reinterpret_cast<pointer>(raw_allocator::aligned_allocate(n,alignment,alignmentOffset,flags));
  }
  void deallocate(void* p, size_t n) {
    raw_allocator::deallocate(p,n);
  }

  const char* get_name() const { return "astl_allocator"; }

  void set_name(const char*) {}
};

// [operator ==(a1, a2)] returns true only if the storage allocated by the
// allocator a1 can be deallocated through a2. Establishes reflexive,
// symmetric, and transitive relationship. Does not throw exceptions.
template <class T, class U, class _RawAlloc>
bool operator==(const astl_allocator<T,_RawAlloc>&, const astl_allocator<U,_RawAlloc>&) { return true; }
template <class T, class U, class _RawAlloc>
bool operator!=(const astl_allocator<T,_RawAlloc>&, const astl_allocator<U,_RawAlloc>&) { return false; }

}
#endif // __ALLOC_H_DC8709E9_3388_CD41_8EC6_5E98807EAE93__
