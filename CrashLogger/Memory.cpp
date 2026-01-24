#include "CrashLogger.hpp"
#include <psapi.h>
#include "GameAPI.h"
#include "PluginManager.h"
#include <SafeWrite.h>
#include <Script.h>
#include "PDB.h"
#include "SharedMem.h"

#define PRINT_HEAPS 1
#define PRINT_POOLS 0

namespace CrashLogger::Memory
{
	std::stringstream output;

	constexpr uintptr_t TARGET_ALLOC = 0x00401F00;
	constexpr uintptr_t TARGET_FREE = 0x00401F20;
	constexpr size_t PATCH_SIZE = 6;

	// globals for trampoline pointers and original function pointers
	using Alloc_t = void* (__cdecl*)(size_t);
	using Free_t = void(__cdecl*)(void*);

	Alloc_t g_origAlloc = nullptr;
	Free_t  g_origFree = nullptr;
	void* g_allocTramp = nullptr;
	void* g_freeTramp = nullptr;

	struct AllocInfo { uint32_t caller; uint32_t size; };
	static std::unordered_map<void*, AllocInfo> g_activeAllocs;
	static std::mutex g_allocMutex;

	constexpr size_t THREAD_BUF_SZ = 1 << 18;
	constexpr size_t MAX_THREAD_LOGS = 64;

	std::atomic<uint64_t> g_totalActiveBytes{ 0 };
	static std::atomic<uint64_t> g_overflowCount{ 0 };

	static std::atomic<bool> g_profilingEnabled{ false };
	static std::atomic<bool> g_profilingStarted{ false };

	std::thread g_memoryProfilerThread;


	struct ThreadLog
	{
		AllocEvent buf[THREAD_BUF_SZ];
		std::atomic<uint32_t> writeIndex{ 0 };
		uint32_t lastConsumed = 0;

		void reset()
		{
			writeIndex.store(0, std::memory_order_relaxed);
			lastConsumed = 0;
		}
	};

	// Global registration array (fixed size, lock-free CAS insertion)
	static std::atomic<ThreadLog*> g_threadLogs[MAX_THREAD_LOGS] = {};

	// Helper to find/claim a slot for this thread
	static ThreadLog* RegisterCurrentThreadLog(ThreadLog* local)
	{
		// Check if already registered
		for (size_t i = 0; i < MAX_THREAD_LOGS; ++i)
		{
			ThreadLog* t = g_threadLogs[i].load(std::memory_order_acquire);
			if (t == local) return t;
		}

		// Try to insert into an empty slot
		for (size_t i = 0; i < MAX_THREAD_LOGS; ++i)
		{
			ThreadLog* expected = nullptr;
			if (g_threadLogs[i].compare_exchange_strong(expected, local,
				std::memory_order_acq_rel, std::memory_order_acquire))
			{
				return local;
			}
		}

		// No slot available
		return nullptr;
	}

	thread_local ThreadLog g_localThreadLog;

	static inline ThreadLog* GetThreadLog()
	{
		static thread_local bool registered = false;
		static thread_local bool disabled = false;

		if (disabled)
			return nullptr;

		if (!registered)
		{
			if (!RegisterCurrentThreadLog(&g_localThreadLog))
			{
				// No slot available – stop recording on this thread
				g_overflowCount.fetch_add(1, std::memory_order_relaxed);
				disabled = true;
				return nullptr;
			}

			registered = true;
		}

		return &g_localThreadLog;
	}

	inline void RecordAlloc(uint32_t caller, uint32_t size, void* ptr) noexcept
	{
		ThreadLog* tlog = GetThreadLog();
		if (!tlog) return;

		uint32_t idx = tlog->writeIndex.load(std::memory_order_relaxed);
		uint32_t slot = idx % THREAD_BUF_SZ;

		tlog->buf[slot].ptr = ptr;
		tlog->buf[slot].size = size;
		tlog->buf[slot].caller = caller;
		tlog->buf[slot].type = 0; // alloc

		tlog->writeIndex.store(idx + 1, std::memory_order_release);
	}

	inline void RecordFree(uint32_t caller, void* ptr) noexcept
	{
		ThreadLog* tlog = GetThreadLog();
		if (!tlog) return;

		uint32_t idx = tlog->writeIndex.load(std::memory_order_relaxed);
		uint32_t slot = idx % THREAD_BUF_SZ;

		tlog->buf[slot].ptr = ptr;
		tlog->buf[slot].size = 0;
		tlog->buf[slot].caller = caller;
		tlog->buf[slot].type = 1; // free

		tlog->writeIndex.store(idx + 1, std::memory_order_release);
	}

	// Trampoline creation
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

		return trampoline;
	}

	bool PatchJump(uintptr_t target, size_t patchSize, void* hookFunc)
	{
		DWORD old;
		if (!VirtualProtect(reinterpret_cast<void*>(target), 5, PAGE_EXECUTE_READWRITE, &old))
			return false;

		uintptr_t hookAddr = reinterpret_cast<uintptr_t>(hookFunc);
		int32_t rel = static_cast<int32_t>(hookAddr - (target + 5));
		uint8_t jmp = 0xE9;
		memcpy(reinterpret_cast<void*>(target), &jmp, 1);
		memcpy(reinterpret_cast<void*>(target + 1), &rel, 4);
		VirtualProtect(reinterpret_cast<void*>(target), patchSize, old, &old);
		return true;
	}

	// Hook functions
	void* __cdecl Hooked_FormHeapAlloc(size_t size)
	{
		if (g_shm)
		{
			g_shm->header.debugCounter.fetch_add(1, std::memory_order_relaxed);
			g_shm->header.lastHeartbeat.store(GetTickCount64(), std::memory_order_relaxed);
		}

		uint32_t caller = reinterpret_cast<uint32_t>(_ReturnAddress());
		void* result = g_origAlloc(size);

		if (!result) return nullptr;

		// Record allocation event if profiling is active
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

	void FlushThreadLogsToSharedMemory()
	{
		for (size_t i = 0; i < MAX_THREAD_LOGS; ++i)
		{
			ThreadLog* t = g_threadLogs[i].load(std::memory_order_acquire);
			if (!t) continue;

			uint32_t produced = t->writeIndex.load(std::memory_order_acquire);
			uint32_t start = t->lastConsumed;

			// Handle overwrite / overflow
			if (produced - start > THREAD_BUF_SZ)
			{
				g_overflowCount.fetch_add(produced - start - THREAD_BUF_SZ,
					std::memory_order_relaxed);
				start = produced - THREAD_BUF_SZ;
			}

			for (uint32_t idx = start; idx < produced; ++idx)
			{
				TryWriteShared(t->buf[idx % THREAD_BUF_SZ]);
			}

			t->lastConsumed = produced;
		}
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

	// Helper to determine appropriate depth based on caller
	UInt32 GetDepthForCaller(const std::string& callerSymbol)
	{
		// Buffer/array allocations - no vtable
		if (callerSymbol.find("Buffer") != std::string::npos ||
			callerSymbol.find("Array") != std::string::npos ||
			callerSymbol.find("Insert") != std::string::npos)
		{
			return 0;  // Skip resolution, these are raw buffers
		}

		// NiGeometryData_ReadBinary allocates vertex/normal/UV buffers (raw floats/vectors)
		if (callerSymbol.find("NiGeometryData_ReadBinary") != std::string::npos)
		{
			return 0;  // Raw geometry data, no vtable
		}

		// Direct object allocations - vtable is at offset 0
		if (callerSymbol.find("new") != std::string::npos ||
			callerSymbol.find("CreateCopy") != std::string::npos ||
			callerSymbol.find("ReadBinary") != std::string::npos)
		{
			return 1;  // Allocated object has vtable at ptr[0]
		}

		// ExtraData allocations - usually pointer to object
		if (callerSymbol.find("Extra") != std::string::npos)
		{
			return 2;
		}

		// Default: try shallow first
		return 2;
	}

	// Safe wrapper that won't crash on invalid pointers
	std::string SafeGetLineForObject(void* ptr, UInt32 depth)
	{
		if (!ptr) return "<null>";

		// Check if memory is readable before dereferencing
		MEMORY_BASIC_INFORMATION mbi;
		if (VirtualQuery(ptr, &mbi, sizeof(mbi)) == 0)
		{
			return "<invalid memory>";
		}

		if (!(mbi.State & MEM_COMMIT) ||
			!(mbi.Protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE)))
		{
			return "<inaccessible memory>";
		}

		void* ptrCopy = ptr;  // Make non-const copy
		void** objectPtr = &ptrCopy;
		std::string result = Stack::GetLineForObject(objectPtr, depth);
		return result.empty() ? "<unresolved>" : result;
	};

	// Get module name from address (faster and more reliable than full symbol resolution)
	std::string GetModuleFromAddress(uint32_t address, HANDLE hProcess)
	{
		HMODULE hMods[1024];
		DWORD cbNeeded;

		if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
		{
			for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
			{
				MODULEINFO modInfo;
				if (GetModuleInformation(hProcess, hMods[i], &modInfo, sizeof(modInfo)))
				{
					uintptr_t base = reinterpret_cast<uintptr_t>(modInfo.lpBaseOfDll);
					uintptr_t end = base + modInfo.SizeOfImage;

					if (address >= base && address < end)
					{
						char modName[MAX_PATH];
						if (GetModuleBaseName(hProcess, hMods[i], modName, sizeof(modName)))
						{
							return std::format("{}+0x{:X}", modName, address - base);
						}
					}
				}
			}
		}
		return std::format("0x{:08X}", address);
	}

	// Safe symbol resolution - try module name first, fall back to PDB if needed
	std::string SafeGetSymbol(uint32_t addr, HANDLE hProcess, bool tryFullSymbols)
	{
		// Always get module info (fast and reliable)
		std::string moduleInfo = GetModuleFromAddress(addr, hProcess);

		// Only try full symbol resolution if explicitly requested and enabled
		if (tryFullSymbols)
		{
			std::string sym = PDB::GetSymbol(addr, hProcess);
			if (!sym.empty())
			{
				return sym;
			}
		}

		return moduleInfo;
	}

	void TickMemoryProfiler()
	{
		if (!g_profilingEnabled.load(std::memory_order_acquire))
			return;

		static uint32_t counter = 0;
		if ((++counter & 0x3F) == 0) // every ~64 calls
		{
			FlushThreadLogsToSharedMemory();
		}
	}

	void MemoryProfilerThread()
	{
		while (g_profilingStarted.load(std::memory_order_acquire))
		{
			g_shm->header.alive.fetch_add(1, std::memory_order_release); // heartbeat

			TickMemoryProfiler();
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}


	void StartMemoryProfiler()
	{
		g_profilingEnabled.store(true, std::memory_order_release);
		g_profilingStarted.store(true, std::memory_order_release);
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
		_MESSAGE("%s",helperPath);

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

	extern void Process(EXCEPTION_POINTERS* info)
		try
	{
		const auto hProcess = GetCurrentProcess();

		PROCESS_MEMORY_COUNTERS_EX pmc = {};
		pmc.cb = sizeof(pmc);

		// Get physical memory size
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

			FlushThreadLogsToSharedMemory();
		}
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