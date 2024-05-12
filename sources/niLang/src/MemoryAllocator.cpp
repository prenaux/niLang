// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "API/niLang/Types.h"
#include "API/niLang/Utils/UnknownImpl.h"
#include "API/niLang/Utils/Sync.h"
#include "API/niLang/StringDef.h"
#include "API/niLang/STL/EASTL/EABase/eabase.h"
#include <stdio.h>

// #define USE_MEMORY_TRACKING
// #define COUNT_WEAK_PTR
// #define TRACE_WEAK_PTR

#if defined USE_MEMORY_TRACKING
// Just include the cpp here so we don't have to add it to the all projects
#include "../../thirdparty/FluidStudios/MemoryManager/mmgr.cpp"
#endif

namespace ni {

#if defined USE_MTUNER
// using rmem.h
#define MTUNER_ALLOC(_handle, _ptr, _size, _overhead)                  rmemAlloc((_handle), (_ptr), (uint32_t)(_size), (uint32_t)(_overhead))
#define MTUNER_ALIGNED_ALLOC(_handle, _ptr, _size, _overhead, _align)  rmemAllocAligned((_handle), (_ptr), (uint32_t)(_size), (uint32_t)(_overhead), (uint32_t)(_align))
#define MTUNER_REALLOC(_handle, _ptr, _size, _overhead, _prevPtr)      rmemRealloc((_handle), (_ptr), (uint32_t)(_size), (uint32_t)(_overhead), (_prevPtr))
#define MTUNER_FREE(_handle, _ptr)                                     rmemFree((_handle), (_ptr))
#else
#define MTUNER_ALLOC(_handle, _ptr, _size, _overhead)
#define MTUNER_ALIGNED_ALLOC(_handle, _ptr, _size, _overhead, _align)
#define MTUNER_REALLOC(_handle, _ptr, _size, _overhead, _prevPtr)
#define MTUNER_FREE(_handle, _ptr)
#endif

#define MEM_MAX(a, b) ((a) > (b) ? (a) : (b))
#ifdef VECTORMATH_USE_SSE
#define VECTORMATH_MIN_ALIGN 16
#else
#define VECTORMATH_MIN_ALIGN 0
#endif
#define MIN_ALLOC_ALIGNMENT MEM_MAX(VECTORMATH_MIN_ALIGN, EA_PLATFORM_MIN_MALLOC_ALIGNMENT)

#ifdef niJSCC
#define ALIGN_ALL_ALLOCS
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

#if defined(USE_MEMORY_TRACKING)

#define _CRT_SECURE_NO_WARNINGS 1

static __forceinline void* _internal_aligned_malloc(
  size_t size, size_t align,
  const char *f, int l, const char *sf)
{
	void* pMemAlign = mmgrAllocator(f, l, sf, m_alloc_malloc, align, size);

	// If using MTuner, report allocation to rmem.
	MTUNER_ALIGNED_ALLOC(0, pMemAlign, size, 0, align);

	// Return handle to allocated memory.
	return pMemAlign;
}

static __forceinline void* _internal_malloc(size_t size, const char *f, int l, const char *sf)
{
	return _internal_aligned_malloc(size, MIN_ALLOC_ALIGNMENT, f, l, sf);
}

static __forceinline  void* _internal_realloc(
  void* ptr, size_t size,
  const char *f, int l, const char *sf)
{
	void* pRealloc = mmgrReallocator(f, l, sf, m_alloc_realloc, size, ptr);

	// If using MTuner, report reallocation to rmem.
	MTUNER_REALLOC(0, pRealloc, size, 0, ptr);

	// Return handle to reallocated memory.
	return pRealloc;
}

static __forceinline void _internal_free(void* ptr, const char *f, int l, const char *sf)
{
	// If using MTuner, report free to rmem.
	MTUNER_FREE(0, ptr);

	mmgrDeallocator(f, l, sf, m_alloc_free, ptr);
}

#else // defined(USE_MEMORY_TRACKING) || defined(USE_MTUNER)

static __forceinline void* _internal_aligned_malloc(
  size_t size, size_t alignment,
  const char *f, int l, const char *sf)
{
#ifdef _MSC_VER
	void* ptr = _aligned_malloc(size, alignment);
#else
	void* ptr;
	alignment = alignment > sizeof(void*) ? alignment : sizeof(void*);
	if (posix_memalign(&ptr, alignment, size))
	{
		ptr = nullptr;
	}
#endif

	MTUNER_ALIGNED_ALLOC(0, ptr, size, 0, alignment);

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

#ifdef _MSC_VER
	void* ptr = _aligned_malloc(size, MIN_ALLOC_ALIGNMENT);
	MTUNER_ALIGNED_ALLOC(0, ptr, size, 0, MIN_ALLOC_ALIGNMENT);
#else
	void* ptr = malloc(size);
	MTUNER_ALLOC(0, ptr, size, 0);
#endif

	return ptr;
}
#endif

static __forceinline void* _internal_realloc(
  void* ptr, size_t size,
  const char *f, int l, const char *sf)
{
  niUnused(f); niUnused(l); niUnused(sf);

#ifdef _MSC_VER
	void* reallocPtr = _aligned_realloc(ptr, size, MIN_ALLOC_ALIGNMENT);
#else
	void* reallocPtr = realloc(ptr, size);
#endif

	MTUNER_REALLOC(0, reallocPtr, size, 0, ptr);

	return reallocPtr;
}

static __forceinline void _internal_free(void* ptr, const char *f, int l, const char *sf)
{
	MTUNER_FREE(0, ptr);

#ifdef _MSC_VER
	_aligned_free(ptr);
#else
	free(ptr);
#endif
}

#endif // defined(USE_MEMORY_TRACKING) || defined(USE_MTUNER)

static SyncCounter _numAlloc(0);
static SyncCounter _numFree(0);

niExportFunc(void*) ni_malloc(size_t size, const char *f, int l, const char *sf)
{
  CHECK_ZERO_MEMORY();
  if (size == 0) {
    return _memZero._buffer;
  }
  _numAlloc.Inc();
  return _internal_malloc(size,f,l,sf);
}

niExportFunc(void*) ni_aligned_malloc(size_t size, size_t alignment,const char *f, int l, const char *sf)
{
  CHECK_ZERO_MEMORY();
  if (size == 0) {
    return _memZero._buffer;
  }
  _numAlloc.Inc();
  return _internal_aligned_malloc(size,alignment,f,l,sf);
}

niExportFunc(void*) ni_realloc(void* ptr, size_t size, const char *f, int l, const char *sf)
{
  CHECK_ZERO_MEMORY();
  if ((ptr == nullptr) || (ptr == _memZero._buffer)) {
    return ni_malloc(size, f, l, sf);
  }

  _numFree.Inc(); _numAlloc.Inc();
  return _internal_realloc(ptr,size,f,l,sf);
}

niExportFunc(void) ni_free(void* ptr, const char *f, int l, const char *sf)
{
  CHECK_ZERO_MEMORY();
  if ((ptr == nullptr) || (ptr == _memZero._buffer)) {
    return;
  }
  _numFree.Inc();
  _internal_free(ptr,f,l,sf);
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

static SyncCounter _numObjectAlloc(0);
static SyncCounter _numObjectFree(0);

niExportFunc(void*) ni_object_alloc(size_t anSize, const achar* file, int line, const achar* fun)
{
  niDebugAssert(anSize > 0);
  CHECK_ZERO_MEMORY();
  _numObjectAlloc.Inc();

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
  _numObjectFree.Inc();

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
  apStats->x = _numAlloc.Get();
  apStats->y = _numFree.Get();
  apStats->z = _numObjectAlloc.Get();
  apStats->w = _numObjectFree.Get();
  return apStats;
}

niExportFunc(void) ni_mem_dump_report() {
#ifdef USE_MEMORY_TRACKING
  mmgrDumpMemoryReport();
#endif
}

niExportFunc(void) ni_mem_dump_leaks() {
#ifdef USE_MEMORY_TRACKING
  mmgrDumpMemoryLeakReport();
#endif
}

}
