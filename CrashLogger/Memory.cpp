#include "CrashLogger.hpp"
#include <psapi.h>
#include "GameAPI.h"
#include "PluginManager.h"
#include "obse_common/SafeWrite.h"
#include <Script.h>
#include "PDB.h"
#include "SharedMem.h"
#include <intrin.h>
#include <atomic>
#include <mutex>
#include <thread>
#include <format>

namespace CrashLogger::Memory
{
	std::stringstream output;

	constexpr uintptr_t TARGET_ALLOC = 0x00401F00;
	constexpr uintptr_t TARGET_FREE = 0x00401F20;
	constexpr size_t PATCH_SIZE = 6;

	using Alloc_t = void* (__cdecl*)(size_t);
	using Free_t = void(__cdecl*)(void*);

	Alloc_t g_origAlloc = nullptr;
	Free_t  g_origFree = nullptr;
	void* g_allocTramp = nullptr;
	void* g_freeTramp = nullptr;

	std::atomic<uint64_t> g_totalActiveBytes{ 0 };

	static std::atomic<bool> g_profilingEnabled{ false };
	static std::atomic<bool> g_profilingStarted{ false };

	std::thread g_memoryProfilerThread;

	constexpr uint32_t RING_CAP = 1u << 19;
	constexpr uint32_t RING_MASK = RING_CAP - 1;
	constexpr uint32_t DRAIN_BATCH = 4096;
	constexpr uint32_t PRODUCER_WAIT_US = 5000;
	static_assert((RING_CAP& (RING_CAP - 1)) == 0, "RING_CAP must be a power of two");

	struct Cell
	{
		std::atomic<uint32_t> seq;
		AllocEvent ev;
	};

	static Cell* g_ring = nullptr;
	alignas(64) static std::atomic<uint32_t> g_head{ 0 };
	alignas(64) static uint32_t g_tail = 0;

	static std::mutex g_flushMutex;
	static std::atomic<int> g_flushWaiters{ 0 };
	static std::atomic<bool> g_saturated{ false };
	static LONGLONG g_producerBudgetTicks = 0;

	static std::atomic<uint64_t> g_droppedAllocs{ 0 };
	static std::atomic<uint64_t> g_droppedFrees{ 0 };
	static std::atomic<uint64_t> g_consumed{ 0 };
	static std::atomic<uint64_t> g_sharedStalls{ 0 };
	static std::atomic<uint32_t> g_highWater{ 0 };
	static std::atomic<uint32_t> g_heapUsedAtStart{ 0 };
	static std::atomic<bool> g_heapUsedAtStartKnown{ false };

	static thread_local int t_suppress = 0;
	struct SuppressScope
	{
		SuppressScope() { ++t_suppress; }
		~SuppressScope() { --t_suppress; }
	};

	inline bool IsHeapCritical()
	{
		if (!g_formHeap)
			return false;

		const float used = float(g_formHeap->field_014);
		const float total = float(g_formHeap->field_00C);
		if (total <= 0.0f)
			return false;

		return (used / total) >= HEAP_ALWAYS;
	}

	static bool InitRing()
	{
		if (g_ring)
			return true;

		auto* mem = static_cast<Cell*>(VirtualAlloc(
			nullptr, sizeof(Cell) * RING_CAP, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
		if (!mem)
			return false;

		for (uint32_t i = 0; i < RING_CAP; ++i)
			mem[i].seq.store(i, std::memory_order_relaxed);

		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		g_producerBudgetTicks = (freq.QuadPart * PRODUCER_WAIT_US) / 1000000;

		g_head.store(0, std::memory_order_relaxed);
		g_tail = 0;
		g_ring = mem;
		return true;
	}

	static inline bool TryPush(uint32_t caller, uint32_t sizeAndType, void* ptr) noexcept
	{
		uint32_t pos = g_head.load(std::memory_order_relaxed);
		Cell* cell;
		for (;;)
		{
			cell = &g_ring[pos & RING_MASK];
			const int32_t dif = static_cast<int32_t>(cell->seq.load(std::memory_order_acquire) - pos);
			if (dif == 0)
			{
				if (g_head.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed))
					break;
			}
			else if (dif < 0)
			{
				return false;
			}
			else
			{
				pos = g_head.load(std::memory_order_relaxed);
			}
		}

		cell->ev.ptr = ptr;
		cell->ev.sizeAndType = sizeAndType;
		cell->ev.caller = caller;
		cell->seq.store(pos + 1, std::memory_order_release);
		return true;
	}

	static inline void Record(uint32_t caller, uint32_t sizeAndType, void* ptr,
		std::atomic<uint64_t>& dropCounter) noexcept
	{
		if (t_suppress || !g_ring)
			return;

		if (TryPush(caller, sizeAndType, ptr))
			return;

		if (!g_saturated.load(std::memory_order_relaxed))
		{
			LARGE_INTEGER now;
			QueryPerformanceCounter(&now);
			const LONGLONG deadline = now.QuadPart + g_producerBudgetTicks;
			do
			{
				SwitchToThread();
				if (TryPush(caller, sizeAndType, ptr))
					return;
				QueryPerformanceCounter(&now);
			}
			while (now.QuadPart < deadline);

			g_saturated.store(true, std::memory_order_relaxed);
		}

		dropCounter.fetch_add(1, std::memory_order_relaxed);
	}

	inline void RecordAlloc(uint32_t caller, uint32_t size, void* ptr) noexcept
	{
		const uint32_t s = size > AE_SIZE_MASK ? AE_SIZE_MASK : size;
		Record(caller, s, ptr, g_droppedAllocs);
	}

	inline void RecordFree(uint32_t caller, void* ptr) noexcept
	{
		Record(caller, AE_TYPE_FREE, ptr, g_droppedFrees);
	}

	static inline bool WriteShared(AllocEvent& ev)
	{
		TryWriteShared(ev);
		return true;
	}

	static uint32_t DrainRing(uint32_t maxEvents)
	{
		if (!g_ring)
			return 0;

		uint32_t tail = g_tail;

		const uint32_t pending = g_head.load(std::memory_order_relaxed) - tail;
		if (pending > g_highWater.load(std::memory_order_relaxed))
			g_highWater.store(pending, std::memory_order_relaxed);

		uint32_t n = 0;
		while (n < maxEvents)
		{
			Cell& c = g_ring[tail & RING_MASK];
			if (c.seq.load(std::memory_order_acquire) != tail + 1)
				break;

			if (!WriteShared(c.ev))
			{
				g_sharedStalls.fetch_add(1, std::memory_order_relaxed);
				break;
			}

			c.seq.store(tail + RING_CAP, std::memory_order_release);
			++tail;
			++n;
		}

		g_tail = tail;
		g_consumed.fetch_add(n, std::memory_order_relaxed);

		if (g_saturated.load(std::memory_order_relaxed) &&
			(g_head.load(std::memory_order_relaxed) - tail) < RING_CAP / 2)
		{
			g_saturated.store(false, std::memory_order_relaxed);
		}

		return n;
	}

	void FlushThreadLogsToSharedMemory()
	{
		SuppressScope suppress;

		std::unique_lock<std::mutex> lk(g_flushMutex, std::defer_lock);
		g_flushWaiters.fetch_add(1, std::memory_order_relaxed);
		for (int i = 0; i < 200 && !lk.try_lock(); ++i)
			Sleep(1);
		g_flushWaiters.fetch_sub(1, std::memory_order_relaxed);

		if (!lk.owns_lock())
			return;

		for (int b = 0; b < 1024; ++b)
		{
			if (DrainRing(DRAIN_BATCH) == 0)
				break;
		}
	}

	void* CreateTrampoline(uintptr_t target, size_t patchSize)
	{
		if (patchSize < 5) return nullptr;
		size_t trampSize = patchSize + 5;
		uint8_t* trampoline = static_cast<uint8_t*>(
			VirtualAlloc(nullptr, trampSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE)
			);
		if (!trampoline) return nullptr;

		memcpy(trampoline, reinterpret_cast<void*>(target), patchSize);

		uintptr_t retAddr = target + patchSize;
		uintptr_t jmpFrom = reinterpret_cast<uintptr_t>(trampoline + patchSize);
		int32_t relBack = static_cast<int32_t>(retAddr - (jmpFrom + 5));
		trampoline[patchSize] = 0xE9;
		*reinterpret_cast<int32_t*>(trampoline + patchSize + 1) = relBack;

		FlushInstructionCache(GetCurrentProcess(), trampoline, trampSize);
		return trampoline;
	}

	bool PatchJump(uintptr_t target, size_t patchSize, void* hookFunc)
	{
		DWORD old;
		if (!VirtualProtect(reinterpret_cast<void*>(target), patchSize, PAGE_EXECUTE_READWRITE, &old))
			return false;

		uint8_t* p = reinterpret_cast<uint8_t*>(target);
		const int32_t rel = static_cast<int32_t>(reinterpret_cast<uintptr_t>(hookFunc) - (target + 5));
		p[0] = 0xE9;
		memcpy(p + 1, &rel, 4);
		for (size_t i = 5; i < patchSize; ++i)
			p[i] = 0x90;

		VirtualProtect(reinterpret_cast<void*>(target), patchSize, old, &old);
		FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<void*>(target), patchSize);
		return true;
	}

	void* __cdecl Hooked_FormHeapAlloc(size_t size)
	{
		uint32_t caller = reinterpret_cast<uint32_t>(_ReturnAddress());
		void* result = g_origAlloc(size);

		if (!result) return nullptr;

		if (g_profilingEnabled.load(std::memory_order_acquire))
		{
			RecordAlloc(caller, static_cast<uint32_t>(size), result);
		}

		return result;
	}

	void __cdecl Hooked_FormHeapFree(void* ptr)
	{
		if (!ptr)
		{
			g_origFree(ptr);
			return;
		}

		uint32_t caller = reinterpret_cast<uint32_t>(_ReturnAddress());

		if (g_profilingEnabled.load(std::memory_order_acquire))
		{
			RecordFree(caller, ptr);
		}

		g_origFree(ptr);
	}

	bool InstallAllocHook()
	{
		g_allocTramp = CreateTrampoline(TARGET_ALLOC, PATCH_SIZE);
		if (!g_allocTramp) return false;
		g_origAlloc = reinterpret_cast<Alloc_t>(g_allocTramp);
		return PatchJump(TARGET_ALLOC, PATCH_SIZE, &Hooked_FormHeapAlloc);
	}

	bool InstallFreeHook()
	{
		g_freeTramp = CreateTrampoline(TARGET_FREE, PATCH_SIZE);
		if (!g_freeTramp) return false;
		g_origFree = reinterpret_cast<Free_t>(g_freeTramp);
		return PatchJump(TARGET_FREE, PATCH_SIZE, &Hooked_FormHeapFree);
	}

	void TickMemoryProfiler()
	{
		SuppressScope suppress;
		std::lock_guard<std::mutex> lk(g_flushMutex);
		DrainRing(DRAIN_BATCH);
	}

	void MemoryProfilerThread()
	{
		SuppressScope suppress;

		while (g_profilingStarted.load(std::memory_order_acquire))
		{
			uint32_t n;
			{
				std::lock_guard<std::mutex> lk(g_flushMutex);
				n = DrainRing(DRAIN_BATCH);
			}

			if (n == 0)
				Sleep(1);
			else if (g_flushWaiters.load(std::memory_order_relaxed) > 0)
				Sleep(1);
		}
	}

	void StartMemoryProfiler()
	{
		if (g_profilingStarted.load(std::memory_order_acquire))
			return;

		if (!InitRing())
		{
			_ERROR("Memory profiler: failed to allocate event ring");
			return;
		}

		if (g_formHeap)
		{
			g_heapUsedAtStart.store(static_cast<uint32_t>(g_formHeap->field_014), std::memory_order_relaxed);
			g_heapUsedAtStartKnown.store(true, std::memory_order_relaxed);
		}

		g_profilingStarted.store(true, std::memory_order_release);
		g_profilingEnabled.store(true, std::memory_order_release);
		g_memoryProfilerThread = std::thread(MemoryProfilerThread);
		g_memoryProfilerThread.detach();
	}

	void LaunchHelper()
	{

		_MESSAGE("Launching CrashLoggerHelper.exe...");

		char helperPath[MAX_PATH]{};

		if (!GetModuleFileNameA(g_hThisDLL, helperPath, MAX_PATH))
			return;

		char* slash = strrchr(helperPath, '\\');
		if (!slash)
			return;

		slash[1] = 0;
		strcat_s(helperPath, "CrashLoggerHelper.exe");
		_MESSAGE("%s", helperPath);

		if (GetFileAttributesA(helperPath) == INVALID_FILE_ATTRIBUTES)
		{
			_ERROR("CrashLoggerHelper.exe not found: %s", helperPath);
			return;
		}

		char cmdLine[64];
		sprintf_s(cmdLine, "%lu", GetCurrentProcessId());

		STARTUPINFOA si{};
		si.cb = sizeof(si);
		PROCESS_INFORMATION pi{};

		BOOL ok = CreateProcessA(
			helperPath,
			cmdLine,
			nullptr,
			nullptr,
			FALSE,
			CREATE_NO_WINDOW,
			nullptr,
			nullptr,
			&si,
			&pi
		);

		if (!ok)
		{
			_ERROR("Failed to launch helper (%lu)", GetLastError());
		}
		else
		{
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
			_MESSAGE("Done.");
		}
	}

	static SIZE_T CommittedBytesInRange(uintptr_t start, uintptr_t end)
	{
		SIZE_T total = 0;
		uintptr_t addr = start;
		while (addr < end)
		{
			MEMORY_BASIC_INFORMATION mbi;
			if (!VirtualQuery(reinterpret_cast<void*>(addr), &mbi, sizeof(mbi)))
				break;

			const uintptr_t regionEnd = reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize;
			const uintptr_t clipEnd = regionEnd < end ? regionEnd : end;
			if (mbi.State == MEM_COMMIT)
				total += clipEnd - addr;

			if (regionEnd <= addr)
				break;
			addr = regionEnd;
		}
		return total;
	}

	extern void Process(EXCEPTION_POINTERS* info)
		try
		{
			const auto hProcess = GetCurrentProcess();

			PROCESS_MEMORY_COUNTERS_EX pmc = {};
			pmc.cb = sizeof(pmc);

			MEMORYSTATUSEX memoryStatus;
			memoryStatus.dwLength = sizeof(memoryStatus);
			GlobalMemoryStatusEx(&memoryStatus);
			if (GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc)))
			{
				DWORDLONG virtUsage = memoryStatus.ullTotalVirtual - memoryStatus.ullAvailVirtual;
				DWORDLONG physUsage = pmc.PrivateUsage;
				output << "Process' Memory:" << '\n';
				output << std::format("Physical Usage: {}", GetMemoryUsageString(physUsage, memoryStatus.ullTotalPhys)) << '\n';
				output << std::format("Virtual  Usage: {}", GetMemoryUsageString(virtUsage, memoryStatus.ullTotalVirtual)) << '\n';
			}

			if (g_formHeap)
			{
				UInt32 usedHeapMemory = 0;
				UInt32 totalHeapMemory = 0;

				output << "\n================================\n";
				output << "\nGame's Memory:" << '\n';

				UInt8* mainHeap = (UInt8*)g_formHeap->field_018;
				UInt8* mainHeapEnd = mainHeap + g_formHeap->field_00C;
				SIZE_T used = g_formHeap->field_014;
				SIZE_T total = g_formHeap->field_00C;

#if PRINT_HEAPS
				SIZE_T start = reinterpret_cast<std::uintptr_t>(mainHeap);
				SIZE_T end = reinterpret_cast<std::uintptr_t>(mainHeapEnd);
				output << std::format("{:30}   {}   ({:08X} - {:08X})", "FormHeap",
					GetMemoryUsageString(used, total), start, end) << '\n';
#endif
				const SIZE_T committed = CommittedBytesInRange(
					reinterpret_cast<uintptr_t>(mainHeap), reinterpret_cast<uintptr_t>(mainHeapEnd));
				output << std::format("{:30}   {}", "FormHeap committed pages",
					GetMemoryUsageString(committed, total)) << '\n';

				usedHeapMemory += used;
				totalHeapMemory += total;

				SIZE_T uiPoolMemory = 0;
				SIZE_T uiTotalPoolMemory = 0;
#if PRINT_POOLS
				output << "\nPools:" << '\n';
#endif
				for (UInt32 i = 0; i < 256; i++)
				{
					MemoryPool* pPool = g_memoryHeap_poolsByAddress[i];
					if (!pPool) continue;

					SIZE_T poolUsed = pPool->field_10C;
					SIZE_T poolTotal = pPool->field_110;

					uiPoolMemory += poolUsed;
					uiTotalPoolMemory += poolTotal;
#if PRINT_POOLS
					SIZE_T poolStart = reinterpret_cast<SIZE_T>(pPool->field_108);
					SIZE_T poolEnd = poolStart + pPool->field_110;
					output << std::format("{:30}   {}   ({:08X} - {:08X})", pPool->m_name,
						GetMemoryUsageString(poolUsed, poolTotal), poolStart, poolEnd) << '\n';
#endif
				}

				output << std::format("\nTotal Heap Memory: {}", GetMemoryUsageString(usedHeapMemory, totalHeapMemory)) << '\n';
				output << std::format("Total Pool Memory: {}", GetMemoryUsageString(uiPoolMemory, uiTotalPoolMemory)) << '\n';
				output << std::format("Total Memory:      {}", GetMemoryUsageString(usedHeapMemory + uiPoolMemory, totalHeapMemory + uiTotalPoolMemory)) << '\n';
			}

			FlushThreadLogsToSharedMemory();

			const uint64_t droppedA = g_droppedAllocs.load(std::memory_order_relaxed);
			const uint64_t droppedF = g_droppedFrees.load(std::memory_order_relaxed);
			const uint32_t pending = g_head.load(std::memory_order_relaxed) - g_tail;

			output << "\n================================\n";
			output << "\nMemory profiler:" << '\n';
			output << std::format("  Events delivered to helper: {}", g_consumed.load(std::memory_order_relaxed)) << '\n';
			output << std::format("  Events still in ring:       {}", pending) << '\n';
			output << std::format("  Ring high-water mark:       {} / {}", g_highWater.load(std::memory_order_relaxed), RING_CAP) << '\n';
			output << std::format("  Dropped allocs / frees:     {} / {}", droppedA, droppedF) << '\n';
			output << std::format("  Shared-queue refusals:      {}", g_sharedStalls.load(std::memory_order_relaxed)) << '\n';
			if (g_heapUsedAtStartKnown.load(std::memory_order_relaxed))
			{
				output << std::format("  FormHeap in use when profiling began: {} bytes (blocks allocated earlier are untracked)",
					g_heapUsedAtStart.load(std::memory_order_relaxed)) << '\n';
			}
			output << ((droppedA + droppedF) == 0
				? "  Status: no events dropped\n"
				: "  Status: INCOMPLETE - events were dropped, live set is unreliable\n");
		}
		catch (...)
		{
			output << "Failed to log memory." << '\n';
		}

		extern std::stringstream& Get()
		{
			output.flush();
			return output;
		}
}