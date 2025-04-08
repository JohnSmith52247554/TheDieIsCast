#pragma once

#include "Config.h"

#include <chrono>

#if ALLOCATION_INDICATOR
#undef new
#include <map>
void* operator new(unsigned int size, const char* file, int line);
void* operator new(unsigned int size);
void operator delete(void* ptr) noexcept;
#endif

namespace Debug
{
#if ALLOCATION_INDICATOR
	struct AllocationInfo
	{
		size_t size;
		std::string filename;
		int line;
	};

	void checkMemoryLeaks();
	void initialize();
	void shutdown();

	extern unsigned int alloc_count;
	extern size_t alloc_size;
	extern std::map<void*, AllocationInfo>* allocations;
#endif

	class Timer
	{
	private:
		std::chrono::steady_clock::time_point begin;
	public:
		Timer();
		~Timer();

		const float getEscapeTime() const;
	};
}

#if ALLOCATION_INDICATOR
#define new new(__FILE__, __LINE__)
#endif