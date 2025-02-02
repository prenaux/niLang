// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niLang/Types.h"
#include "API/niLang/Utils/UnknownImpl.h"
#include "API/niLang/Utils/Sync.h"
#include "API/niLang/StringDef.h"
#include "API/niLang/STL/EASTL/EABase/eabase.h"
#include "API/niLang/STL/atomic.h"
#include <stdio.h>

// #define USE_MEMORY_TRACKING
// #define USE_NEDMALLOC
// #define USE_MIMALLOC
// #define USE_BUMPALLOCATOR
// #define COUNT_WEAK_PTR
// #define TRACE_WEAK_PTR
#define COUNT_ALLOCS

// Use FluidStudios for memory tracking
#if defined USE_MEMORY_TRACKING
#pragma message("# MemoryAllocator: USE_MEMORY_TRACKING")

// Just include the cpp here so we don't have to add it to the all projects
#include "../../thirdparty/FluidStudios/MemoryManager/mmgr.cpp"
#define _CRT_SECURE_NO_WARNINGS 1

// Use ned malloc
#elif defined USE_NEDMALLOC
#pragma message("# MemoryAllocator: USE_NEDMALLOC")

#include "nedmalloc/nedmalloc.h"

// Use mimalloc
#elif defined USE_MIMALLOC
#pragma message("# MemoryAllocator: USE_MIMALLOC")

#include "mimalloc/include/mimalloc.h"

// Use bump allocator for all allocations
#elif defined USE_BUMPALLOCATOR
#pragma message("# MemoryAllocator: USE_BUMPALLOCATOR")

// Use system allocator
#else
#pragma message("# MemoryAllocator: system allocator")

#endif

#if 0
#define IF_TRACE_MEM(KIND)                        \
  static int _count = 0;                          \
  if ((++_count > 200000) && ((_count % 10000) == 0))
#endif

#ifdef niJSCC
#define ALIGN_ALL_ALLOCS
#endif

#define MEM_MAX(a, b) ((a) > (b) ? (a) : (b))
#ifdef VECTORMATH_USE_SSE
#define VECTORMATH_MIN_ALIGN 16
#else
#define VECTORMATH_MIN_ALIGN 0
#endif
#define MIN_ALLOC_ALIGNMENT MEM_MAX(VECTORMATH_MIN_ALIGN, EA_PLATFORM_MIN_MALLOC_ALIGNMENT)

namespace ni {

#if defined USE_MIMALLOC
static void ni_mimalloc_print(const char* aText, void*) {
  niPrint(aText);
}
#endif

#if defined USE_BUMPALLOCATOR
// This is meant as a reference implementation for a "this is the fastest you
// can allocate memory" allocator. Or at least it shouldnt have big horrible
// bottlenecks.
struct sBumpAllocator {
  static constexpr size_t CHUNK_SIZE = 64 * 1024 * 1024;

  astl::atomic<char*> current;
  char* end;
  __sync_mutex();

  void* alloc_aligned(size_t size, size_t alignment) {
#if 0
    return _aligned_malloc(size,alignment);
#else
    while (true) {
      char* old_current = current.load(astl::memory_order_relaxed);
      char* aligned_ptr = (char*)(((uintptr_t)old_current + alignment - 1) & ~(alignment - 1));
      char* new_current = aligned_ptr + size;

      if (new_current > end) {
        __sync_lock();
        if (old_current != current.load(std::memory_order_relaxed)) continue;
        current = (char*)::malloc(CHUNK_SIZE);
        end = current + CHUNK_SIZE;
        continue;
      }

      if (current.compare_exchange_weak(old_current, new_current,
                                        std::memory_order_release,
                                        std::memory_order_relaxed)) {
        return aligned_ptr;
      }
    }
#endif
  }

  void* alloc(size_t size) {
    return this->alloc_aligned(size, MIN_ALLOC_ALIGNMENT);
  }

  void* realloc(void* ptr, size_t new_size, size_t alignment) {
#if 0
    return _aligned_realloc(ptr,new_size,alignment);
#else
    if (!ptr) return alloc_aligned(new_size, alignment);
    // Always allocate new + copy since we can't safely know the original size
    void* new_ptr = alloc_aligned(new_size, alignment);
    // Copy new_size bytes - if this is too much, the original allocation was bigger anyway
    memcpy(new_ptr, ptr, new_size);
    return new_ptr;
#endif
  }

  void free(void* ptr) {
    niUnused(ptr);
    // noop
  }
};
static sBumpAllocator _bumpAllocator;
#endif

// We always add mpWeakPtr because on 32-bit OS's the header must be 8 bytes
// anyway and on 64bit systems we have more memory available and the alloc
// size will already consume our 8 bytes no matter what.
struct sObjectMemoryHeader {
  struct sWeakPtrImpl* mpWeakPtr;
#ifdef ni32
  int __padding;
#endif
};
niCAssert(sizeof(sObjectMemoryHeader) == niStandardTypesAlignment);

static struct {
  sObjectMemoryHeader _header;
  char _buffer[8];
} _memZero = { { nullptr }, { 0, 0, 0, 0, 0, 0, 0, 0 } };
niCAssert(sizeof(_memZero) == (sizeof(sObjectMemoryHeader) + 8));

#define CHECK_ZERO_MEMORY() \
  niDebugAssert(((tU32*)_memZero._buffer)[0] == 0 && ((tU32*)_memZero._buffer)[1] == 0)

static __forceinline void* _internal_aligned_malloc(
  size_t size, size_t alignment,
  const char *f, int l, const char *sf)
{
#if defined USE_MEMORY_TRACKING
	void* ptr = mmgrAllocator(f, l, sf, m_alloc_malloc, align, size);
#elif defined USE_NEDMALLOC
	void* ptr = nedalloc::nedmemalign(alignment, size);
#elif defined USE_MIMALLOC
	void* ptr = mi_malloc_aligned(size, alignment);
#elif defined USE_BUMPALLOCATOR
	void* ptr = _bumpAllocator.alloc_aligned(size, alignment);
#elif defined _MSC_VER
	void* ptr = _aligned_malloc(size, alignment);
#else
	void* ptr;
	alignment = alignment > sizeof(void*) ? alignment : sizeof(void*);
	if (posix_memalign(&ptr, alignment, size))
	{
		ptr = nullptr;
	}
#endif
	return ptr;
}

#ifdef ALIGN_ALL_ALLOCS
static __forceinline void* _internal_malloc(size_t size, const char *f, int l, const char *sf)
{
	return _internal_aligned_malloc(size, MIN_ALLOC_ALIGNMENT, f, l, sf);
}
#else
static __forceinline void* _internal_malloc(size_t size, const char *f, int l, const char *sf)
{
  niUnused(f); niUnused(l); niUnused(sf);
#if defined USE_MEMORY_TRACKING
	void* ptr = mmgrAllocator(f, l, sf, m_alloc_malloc, MIN_ALLOC_ALIGNMENT, size);
#elif defined USE_NEDMALLOC
	void* ptr = nedalloc::nedmalloc(size);
#elif defined USE_MIMALLOC
	void* ptr = mi_malloc(size);
#elif defined USE_BUMPALLOCATOR
	void* ptr = _bumpAllocator.alloc(size);
#elif defined _MSC_VER
	void* ptr = _aligned_malloc(size, MIN_ALLOC_ALIGNMENT);
#else
	void* ptr = malloc(size);
#endif
	return ptr;
}
#endif

static __forceinline void* _internal_realloc(
  void* ptr, size_t size,
  const char *f, int l, const char *sf)
{
  niUnused(f); niUnused(l); niUnused(sf);
#if defined USE_MEMORY_TRACKING
  void* reallocPtr = mmgrReallocator(f, l, sf, m_alloc_realloc, size, ptr);
#elif defined USE_NEDMALLOC
	void* reallocPtr = nedalloc::nedrealloc(ptr, size);
#elif defined USE_MIMALLOC
	void* reallocPtr = mi_realloc(ptr, size);
#elif defined USE_BUMPALLOCATOR
	void* reallocPtr = _bumpAllocator.realloc(ptr, size, MIN_ALLOC_ALIGNMENT);
#elif defined _MSC_VER
	void* reallocPtr = _aligned_realloc(ptr, size, MIN_ALLOC_ALIGNMENT);
#else
	void* reallocPtr = realloc(ptr, size);
#endif
	return reallocPtr;
}

static __forceinline void _internal_free(void* ptr, const char *f, int l, const char *sf)
{
  niUnused(f); niUnused(l); niUnused(sf);
#if defined USE_MEMORY_TRACKING
  mmgrDeallocator(f, l, sf, m_alloc_free, ptr);
#elif defined USE_NEDMALLOC
  nedalloc::nedfree(ptr);
#elif defined USE_MIMALLOC
  mi_free(ptr);
#elif defined USE_BUMPALLOCATOR
  _bumpAllocator.free(ptr);
#elif defined _MSC_VER
	_aligned_free(ptr);
#else
	free(ptr);
#endif
}

#ifdef COUNT_ALLOCS
static SyncCounter _numAlloc(0);
static SyncCounter _numFree(0);
static SyncCounter _numRealloc(0);
#endif

niExportFunc(void*) ni_malloc(size_t size, const char *f, int l, const char *sf)
{
  CHECK_ZERO_MEMORY();
  if (size == 0) {
    return _memZero._buffer;
  }
#ifdef COUNT_ALLOCS
  _numAlloc.Inc();
#endif
  void* r = _internal_malloc(size,f,l,sf);
#ifdef IF_TRACE_MEM
  IF_TRACE_MEM(ni_malloc) {
    niDebugFmt(("... ni_malloc: size: %d, f: %s, l: %d, sf: %s",
                size, f, l, sf));
  }
#endif
  return r;
}

niExportFunc(void*) ni_aligned_malloc(size_t size, size_t alignment,const char *f, int l, const char *sf)
{
  CHECK_ZERO_MEMORY();
  if (size == 0) {
    return _memZero._buffer;
  }
#ifdef COUNT_ALLOCS
  _numAlloc.Inc();
#endif
  void* r = _internal_aligned_malloc(size,alignment,f,l,sf);
#ifdef IF_TRACE_MEM
  IF_TRACE_MEM(ni_aligned_malloc) {
    niDebugFmt(("... ni_aligned_malloc: size: %d, alignment: %d, f: %s, l: %d, sf: %s",
                size, alignment, f, l, sf));
  }
#endif
  return r;
}

niExportFunc(void*) ni_realloc(void* ptr, size_t size, const char *f, int l, const char *sf)
{
  CHECK_ZERO_MEMORY();
  if ((ptr == nullptr) || (ptr == _memZero._buffer)) {
    return ni_malloc(size, f, l, sf);
  }
#ifdef COUNT_ALLOCS
  _numFree.Inc(); _numAlloc.Inc();
  _numRealloc.Inc();
#endif
  void* r = _internal_realloc(ptr,size,f,l,sf);
#ifdef IF_TRACE_MEM
  IF_TRACE_MEM(ni_realloc) {
    niDebugFmt(("... ni_realloc: ptr: %p, size: %d, f: %s, l: %d, sf: %s",
                (tIntPtr)ptr, size, f, l, sf));
  }
#endif
  return r;
}

niExportFunc(void) ni_free(void* ptr, const char *f, int l, const char *sf)
{
  CHECK_ZERO_MEMORY();
  if ((ptr == nullptr) || (ptr == _memZero._buffer)) {
    return;
  }
#ifdef COUNT_ALLOCS
  _numFree.Inc();
#endif
  _internal_free(ptr,f,l,sf);
  // tracing in ni_free leads to crashes....
#if defined IF_TRACE_MEM && 0
  IF_TRACE_MEM(ni_free) {
    niDebugFmt(("... ni_free: ptr: %p, f: %s, l: %d, sf: %s",
                (tIntPtr)ptr, f, l, sf));
  }
#endif
}

#ifdef _DEBUG
struct iWeakPtr : public iUnknown {
  niDeclareInterfaceUUID(iWeakPtr,0x211f691c,0x4dff,0x4093,0xae,0x37,0x52,0xec,0x6a,0xc0,0x1b,0xd5);
};
#endif

#ifdef COUNT_WEAK_PTR
SYNC_INT_TYPE _nWeakPtrCount = 0;
#define WEAK_PTR_COUNT _nWeakPtrCount
#else
#define WEAK_PTR_COUNT -1
#endif

struct sWeakPtrImpl : public iUnknown {
private:
  __sync_mutex();
  __sync_ptr(iUnknown) mpObserved;
  SYNC_INT_TYPE mprotected_nNumRefs;

public:
  sWeakPtrImpl(iUnknown* apObserved) : mpObserved(apObserved), mprotected_nNumRefs(0) {
#ifdef COUNT_WEAK_PTR
    SYNC_INCREMENT(&_nWeakPtrCount);
#endif
  }
  ~sWeakPtrImpl() {
    __sync_local_ptr(iUnknown,pObserved);
    niDebugAssert(pObserved.ptr() == nullptr); // Observed object should have been nulled
#ifdef COUNT_WEAK_PTR
    SYNC_DECREMENT(&_nWeakPtrCount);
#endif
#ifdef TRACE_WEAK_PTR
    niDebugFmt(("... Destructing WeakPtr %p ; %d weak pointers in total.",
                (tIntPtr)this,
                WEAK_PTR_COUNT));
#endif
  }

  virtual tI32 __stdcall AddRef() niOverride {
    niImpl_AddRef();
  }
  virtual tI32 __stdcall Release() niOverride {
    niImpl_Release();
  }
  virtual tI32 __stdcall SetNumRefs(tI32 anNumRefs) niOverride {
    niImpl_SetNumRefs(anNumRefs);
  }
  virtual tI32 __stdcall GetNumRefs() const niOverride {
    niImpl_GetNumRefs();
  }
  virtual void __stdcall DeleteThis() niOverride {
    delete this;
  }
  virtual void __stdcall Invalidate() niOverride {
  }
  virtual tBool __stdcall IsOK() const niOverride {
    __sync_local_ptr(iUnknown,pObserved);
    return pObserved.IsOK();
  }
  iUnknown* __stdcall QueryInterface(const tUUID& aIID) niOverride {
#ifdef _DEBUG
    if (aIID == niGetInterfaceUUID(iWeakPtr)) {
      return (iUnknown*)this;
    }
#endif
    __sync_local_ptr(iUnknown,pObserved);
    iUnknown* o = pObserved.ptr();
    return o ? o->QueryInterface(aIID) : (iUnknown*)nullptr;
  }
  void __stdcall ListInterfaces(iMutableCollection* apLst, tU32 anListFlags) const niOverride {
    __sync_local_ptr(iUnknown,pObserved);
    iUnknown* o = pObserved.ptr();
    if (o) {
      o->ListInterfaces(apLst,anListFlags);
    }
  }

  tIntPtr GetObservedIntPtr() const {
    __sync_local_ptr(iUnknown,pObserved);
    return (tIntPtr)pObserved.ptr();
  }

  void SetObservedNull() {
    __sync_set(mpObserved,(iUnknown*)nullptr);
  }

  iUnknown* Deref() {
    __sync_local_ptr(iUnknown,pObserved);
    iUnknown* o = pObserved.ptr();
    if (o) {
      if (o->GetNumRefs() <= 0) {
#ifdef _DEBUG
        niLog(Debug,niFmt("Deref: Trying to revive %p with weak pointer.", (tIntPtr)o));
#endif
        return nullptr;
      }
      return o;
    }
    return nullptr;
  }
};

#define MEMORY_INIT_HEADER(P,SIZE)                  \
  ((sObjectMemoryHeader*)(P))->mpWeakPtr = nullptr;

#define MEMORY_GET_HEADER(apObjectPtr)                                  \
  ((sObjectMemoryHeader*)(((tPtr)(apObjectPtr))-sizeof(sObjectMemoryHeader)))

#ifdef TRACE_WEAK_PTR
#define OBJECT_MEMORY_DESTRUCT(apObjectPtr)                             \
  if (apObjectPtr->mpWeakPtr) {                                         \
    niDebugFmt(("... Invalidating WeakPtr %p for object %p ; %d weak pointers in total.", \
                (tIntPtr)basePtr->mpWeakPtr,                            \
                (tIntPtr)basePtr->mpWeakPtr->GetObservedIntPtr(),       \
                WEAK_PTR_COUNT));                                       \
    apObjectPtr->mpWeakPtr->SetObservedNull();                          \
    apObjectPtr->mpWeakPtr->Release();                                  \
  }
#else
#define OBJECT_MEMORY_DESTRUCT(apObjectPtr)     \
  if (apObjectPtr->mpWeakPtr) {                 \
    apObjectPtr->mpWeakPtr->SetObservedNull();  \
    apObjectPtr->mpWeakPtr->Release();          \
  }
#endif

#ifdef COUNT_ALLOCS
static SyncCounter _numObjectAlloc(0);
static SyncCounter _numObjectFree(0);
#endif

niExportFunc(void*) ni_object_alloc(size_t anSize, const achar* file, int line, const achar* fun)
{
  niDebugAssert(anSize > 0);
  CHECK_ZERO_MEMORY();
#ifdef COUNT_ALLOCS
  _numObjectAlloc.Inc();
#endif

  const tSize nAllocSize = sizeof(sObjectMemoryHeader)+anSize;
  void* p = ni_malloc(nAllocSize, file, line, fun);
  // XXX: Using niDebugAssert because if this triggers we're probably not
  // gonna achieve much since a single object is likely to be a small
  // allocation and whatever a niPanicAssert would try to allocate would also
  // fall over and crash somewhere else... If we one day care about this we
  // should write a proper test for this somehow - which won't be trivial.
  niDebugAssert(p != nullptr);
  memset(p,0,nAllocSize);
  {
    MEMORY_INIT_HEADER(p, nAllocSize);
  }

  // printf("... OBJECT ALLOC: %db, %s:%d in %s\n", anSize, file?file:"NF", line, fun?fun:"NF");
  return ((tPtr)p)+sizeof(sObjectMemoryHeader);
}

niExportFunc(void) ni_object_free(void* apObjectPtr, const char* file, int line, const char* fun) {
  CHECK_ZERO_MEMORY();
#ifdef COUNT_ALLOCS
  _numObjectFree.Inc();
#endif

  sObjectMemoryHeader* basePtr = MEMORY_GET_HEADER(apObjectPtr);
  niDebugAssert((void*)basePtr != (void*)&_memZero);
  OBJECT_MEMORY_DESTRUCT(basePtr);
  ni_free(basePtr, file, line, fun);
  // printf("... OBJECT FREE: %s:%d in %s\n", file?file:"NF", line, fun?fun:"NF");
}

niExportFunc(tBool) ni_object_has_weak_ptr(iUnknown* apObject) {
  sObjectMemoryHeader* basePtr = MEMORY_GET_HEADER(apObject);
  return basePtr->mpWeakPtr != nullptr;
}

niExportFunc(iUnknown*) ni_object_get_weak_ptr(iUnknown* apObject) {
#if 0
  // Discourage improper usage of the API ; assert if apObjectPtr is nullptr or
  // if it is already a weak pointer object.
  niPanicAssert(apObject != nullptr
                && apObject->QueryInterface(niGetInterfaceUUID(iWeakPtr)) == nullptr);
#endif

  // Ensure that we get the actual base object. That is mainly to handle the case where
  // get_weak_ptr() is called with a weak pointer as parameter.
  iUnknown* pBaseObject = apObject ? apObject->QueryInterface(niGetInterfaceUUID(iUnknown)) : nullptr;
  if (!pBaseObject)
    return nullptr;

  sObjectMemoryHeader* basePtr = MEMORY_GET_HEADER(pBaseObject);
  if (!basePtr->mpWeakPtr) {
    basePtr->mpWeakPtr = new sWeakPtrImpl(pBaseObject);
    // Once a weak pointer is created the observed object 'owns' a reference
    // to the weak pointer. This is to prevent alloc/destroy of weak pointers
    // in a loop which would add pressure on the memory allocator for little
    // benefit.
    basePtr->mpWeakPtr->AddRef();
#ifdef TRACE_WEAK_PTR
    niDebugFmt(("... New WeakPtr %p for object %p ; %d weak pointers in total.",
                (tIntPtr)basePtr->mpWeakPtr,
                (tIntPtr)basePtr->mpWeakPtr->mpObjectMemory,
                WEAK_PTR_COUNT));
#endif
  }
  return basePtr->mpWeakPtr;
}

niExportFunc(iUnknown*) ni_object_deref_weak_ptr(iUnknown* apWeakPtr) {
  if (!apWeakPtr)
    return nullptr;
#ifdef _DEBUG
  // Check for improper usage of the API ; assert if apWeakPtr not a weak pointer.
  niDebugAssert(apWeakPtr->QueryInterface(niGetInterfaceUUID(iWeakPtr)) != nullptr);
#endif
  sWeakPtrImpl* weakPtr = (sWeakPtrImpl*)apWeakPtr;
  return weakPtr->Deref();
}

niExportFunc(sVec4i*) ni_mem_get_stats(sVec4i* apStats) {
  niPanicAssert(apStats != nullptr);
#ifdef COUNT_ALLOCS
  apStats->x = _numAlloc.Get();
  apStats->y = _numFree.Get();
  apStats->z = _numObjectAlloc.Get();
  apStats->w = _numObjectFree.Get();
#else
  *apStats = sVec4i::Zero();
#endif
  return apStats;
}

niExportFunc(void) ni_mem_dump_report() {
#if defined USE_MEMORY_TRACKING
  mmgrDumpMemoryReport();
#elif defined USE_MIMALLOC
  niLog(Info,"--- MIMALLOC REPORT BEGIN ---");
  mi_stats_print_out(ni_mimalloc_print, nullptr);
  niLog(Info,"--- MIMALLOC REPORT END ---");
#endif
}

niExportFunc(void) ni_mem_dump_leaks() {
#ifdef USE_MEMORY_TRACKING
  mmgrDumpMemoryLeakReport();
#endif
}

}
