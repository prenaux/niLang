#pragma once
#ifndef __NIVULKANMEMORYALLOCATOR_H_EFF909D3_8022_9942_8018_4478EF411760__
#define __NIVULKANMEMORYALLOCATOR_H_EFF909D3_8022_9942_8018_4478EF411760__

#include <niLang/Types.h>
#include <niLang/Utils/ThreadImpl.h>

#ifdef niVulkanMemoryAllocator_Implement
#define VMA_IMPLEMENTATION
#elif defined niVulkanMemoryAllocator_Include
// nothing
#else
#error "E/niVulkanMemoryAllocator_Implement or niVulkanMemoryAllocator_Include must be defined before including niVulkanMemoryAllocator.h"
#endif

#define VMA_ASSERT(expr) niDebugAssert(expr)

// VMA has this disabled by default, putting it here in case we want to enable
// it in the future. Comment from VMA: Assert that will be called very often,
// like inside data structures e.g. operator[]. Making it non-empty can make
// program slow.
#define VMA_HEAVY_ASSERT(expr) //niDebugAssert(expr)

#define VMA_SYSTEM_ALIGNED_MALLOC(size, alignment) ni::ni_aligned_malloc(size, alignment, __FILE__, __LINE__, __FUNCTION__)
#define VMA_SYSTEM_ALIGNED_FREE(ptr) ni::ni_free(ptr, __FILE__, __LINE__, __FUNCTION__)

// Log is very verbose, prints for each allocation
#ifdef ENABLE_VMA_LOG
#define VMA_DEBUG_LOG(format, ...) niDebugFmt(("VMA: " format, __VA_ARGS__))
#else
#define VMA_DEBUG_LOG(format, ...)
#endif

#define VMA_LEAK_LOG_FORMAT(format, ...) niWarning(niFmt("VMA_LEAK: " format, __VA_ARGS__))
#define VMA_ASSERT_LEAK(expr) niPanicAssert(expr) // if (!(expr)) { niWarning(niFmt("VMA_ASSERT_LEAK: " #expr)); }

struct NiVmaMutex {
	NiVmaMutex() {}
	~NiVmaMutex() {}
	void Lock() { _mutex.ThreadLock(); }
	void Unlock() { _mutex.ThreadUnlock(); }
	void LockRead() { _mutex.ThreadLock(); }
	void UnlockRead() { _mutex.ThreadUnlock(); }
	void LockWrite() { _mutex.ThreadLock(); }
	void UnlockWrite() { _mutex.ThreadUnlock(); }
	ni::ThreadMutex _mutex;
};

#define VMA_MUTEX NiVmaMutex
#define VMA_RW_MUTEX NiVmaMutex

EA_DISABLE_CLANG_WARNING(-Wnullability-completeness);
EA_DISABLE_CLANG_WARNING(-Wunused-private-field);
#include "include/vk_mem_alloc.h"
EA_RESTORE_CLANG_WARNING()
EA_RESTORE_CLANG_WARNING()

#endif // __NIVULKANMEMORYALLOCATOR_H_EFF909D3_8022_9942_8018_4478EF411760__
