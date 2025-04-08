#include "Debug.h"

#if ALLOCATION_INDICATOR
#undef new

#include <mutex>
#include <thread>
std::mutex allocMutex;
thread_local bool in_custom_new = false;

void* operator new(unsigned int size, const char* file, int line)
{
	if (size == 0)
		size = 1;
	Debug::alloc_count++;
	Debug::alloc_size += size;

	if (size >= 512)
		std::cout << "file: " << file << "\nline: " << line << "\nsize: " << size << "\n\n";

	if (in_custom_new)
		return std::malloc(size);

	in_custom_new = true;
	void* ptr = std::malloc(size);
	if (ptr != nullptr && Debug::allocations != nullptr)
	{
		std::lock_guard<std::mutex> lock(allocMutex);
		(*Debug::allocations)[ptr] = { size, file, line };
	}
	in_custom_new = false;
	return ptr;
}

void* operator new(unsigned int size)
{
	if (size == 0)
		size = 1;
	Debug::alloc_count++;
	Debug::alloc_size += size;

	return std::malloc(size);
}

void operator delete(void* ptr) noexcept
{
	if (Debug::allocations != nullptr && !in_custom_new)
	{
		std::lock_guard<std::mutex> lock(allocMutex);
		in_custom_new = true;
		Debug::allocations->erase(ptr);
		in_custom_new = false;
	}
	std::free(ptr);
}
#endif

namespace Debug
{
#if ALLOCATION_INDICATOR
	unsigned int Debug::alloc_count = 0U;
	size_t Debug::alloc_size = 0U;
	std::map<void*, Debug::AllocationInfo>* Debug::allocations = nullptr;

	void checkMemoryLeaks() 
	{
		if (!allocations->empty()) 
		{
			std::cout << "Memory leaks detected!" << std::endl;
			for (const auto& pair : *allocations) 
				std::cout << "Leak of " << pair.second.size << " bytes at address: " << pair.first <<  "\nfile name: " << pair.second.filename << "\nline: " << pair.second.line << std::endl;
		}
		else
			std::cout << "No memory leaks detected." << std::endl;
	}

	void initialize() 
	{
		allocations = new (std::malloc(sizeof(std::map<void*, AllocationInfo>))) std::map<void*, AllocationInfo>();
	}

	void shutdown()
	{
		in_custom_new = true;
		allocations->~map();
		std::free(allocations);
		allocations = nullptr;
		in_custom_new = false;
	}

#endif

	Timer::Timer()
	{
		begin = std::chrono::steady_clock::now();
	}

	Timer::~Timer()
	{
		auto end = std::chrono::steady_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
		//std::cout << "time cunsume " << duration.count() << "ns\n";
	}

	const float Timer::getEscapeTime() const
	{
		auto end = std::chrono::steady_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
		return duration.count();
	}
}

#if ALLOCATION_INDICATOR
#define new new(__FILE__, __LINE__)
#endif