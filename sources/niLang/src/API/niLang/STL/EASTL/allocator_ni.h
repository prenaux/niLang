/////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
/////////////////////////////////////////////////////////////////////////////


#ifndef EASTL_ALLOCATOR_NI_H
#define EASTL_ALLOCATOR_NI_H

#include "internal/config.h"

#if EASTL_ALLOCATOR_NI

namespace eastl
{
	///////////////////////////////////////////////////////////////////////////////
	// allocator_ni
	//
	// Implements an EASTL allocator that uses malloc/free as opposed to
	// new/delete or PPMalloc Malloc/Free.
	//
	// Example usage:
	//      vector<int, allocator_ni> intVector;
	//
	class EASTL_API allocator_ni
	{
	public:
		allocator_ni(const char* = NULL) {}

		allocator_ni(const allocator_ni&) {}

		allocator_ni(const allocator_ni&, const char*) {}

		allocator_ni& operator=(const allocator_ni&) { return *this; }

		bool operator==(const allocator_ni&) { return true; }

		bool operator!=(const allocator_ni&) { return false; }

		void* allocate(size_t n, int /*flags*/ = 0);

		void* allocate(size_t n, size_t alignment, size_t alignmentOffset, int /*flags*/ = 0);

		void deallocate(void* p, size_t /*n*/);

		const char* get_name() const { return "allocator_ni"; }

		void set_name(const char*) {}
	};
	inline bool operator==(const allocator_ni&, const allocator_ni&) { return true; }
	inline bool operator!=(const allocator_ni&, const allocator_ni&) { return false; }

	EASTL_API allocator_ni* GetDefaultAllocatorNi();
	EASTL_API allocator_ni* SetDefaultAllocatorNi(allocator_ni* pAllocator);

} // namespace eastl

#endif

#endif // Header include guard
