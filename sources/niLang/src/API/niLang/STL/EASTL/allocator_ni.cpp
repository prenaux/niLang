#include "internal/config.h"
#include "allocator_ni.h"

#if EASTL_ALLOCATOR_NI

namespace eastl
{

void* allocator_ni::allocate(size_t n, int /*flags*/)
{
  return ni::ni_malloc(n,__FILE__,__LINE__,"eastl::allocator_ni::ni_malloc");
}

void* allocator_ni::allocate(size_t n, size_t alignment, size_t alignmentOffset, int /*flags*/)
{
  // We check for (offset % alignmnent == 0) instead of (offset == 0) because
  // any block which is aligned on e.g. 64 also is aligned at an offset of 64
  // by definition.
  if ((alignmentOffset % alignment) == 0)  {
    return ni::ni_aligned_malloc(n, alignment, __FILE__,__LINE__,"eastl::allocator_ni::ni_aligned_malloc");
  }
  return NULL;
}

void allocator_ni::deallocate(void* p, size_t /*n*/)
{
  ni::ni_free(p,__FILE__,__LINE__,"eastl::allocator_ni::ni_free");
}

/// gDefaultAllocator
/// Default global allocator_ni instance.
EASTL_API allocator_ni  gDefaultAllocatorNi;
EASTL_API allocator_ni* gpDefaultAllocatorNi = &gDefaultAllocatorNi;

EASTL_API allocator_ni* GetDefaultAllocatorNi()
{
  return gpDefaultAllocatorNi;
}

EASTL_API allocator_ni* SetDefaultAllocatorNi(allocator_ni* pAllocator)
{
  allocator_ni* const pPrevAllocator = gpDefaultAllocatorNi;
  gpDefaultAllocatorNi = pAllocator;
  return pPrevAllocator;
}

} // namespace eastl


#endif // EASTL_ALLOCATOR_NI
