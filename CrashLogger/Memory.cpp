#include "CrashLogger.hpp"
#include <psapi.h>
#include "GameAPI.h"
#include "PluginManager.h"
#include <SafeWrite.h>

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

	constexpr size_t THREAD_BUF_SZ = 1 << 18;   // 4096 entries per thread (tune)
	constexpr size_t MAX_THREAD_LOGS = 64;

	std::atomic<uint64_t> g_totalActiveBytes{ 0 };

	// ---------- event struct ----------
	struct AllocEvent {
		void* ptr;
		uint32_t size;
		uint32_t caller;   // RVA/VA as uint32
		uint8_t  kind;     // 0 = alloc, 1 = free
		uint8_t  pad[3];
	};

	// ---------- per-thread buffer ----------
	struct ThreadLog {
		// circular/append-only buffer (single-producer: the thread)
		AllocEvent buf[THREAD_BUF_SZ];

		// write index: number of events produced so far (monotonic)
		std::atomic<uint32_t> writeIndex{ 0 };

		// consumer uses this to remember where it consumed up to
		uint32_t lastConsumed = 0;

		// convenience
		void reset() { writeIndex.store(0, std::memory_order_relaxed); lastConsumed = 0; }
	};

	// global registration array (fixed size, lock-free CAS insertion)
	static std::atomic<ThreadLog*> g_threadLogs[MAX_THREAD_LOGS] = {};

	// helper to find/claim a slot for this thread
	static ThreadLog* RegisterCurrentThreadLog(ThreadLog* local)
	{
		// if already in table (someone else registered same pointer) return
		for (size_t i = 0; i < MAX_THREAD_LOGS; ++i) {
			ThreadLog* t = g_threadLogs[i].load(std::memory_order_acquire);
			if (t == local) return t;
		}

		// try to insert into an empty slot
		for (size_t i = 0; i < MAX_THREAD_LOGS; ++i) {
			ThreadLog* expected = nullptr;
			if (g_threadLogs[i].compare_exchange_strong(expected, local,
				std::memory_order_acq_rel, std::memory_order_acquire)) {
				return local;
			}
		}

		// no slot available: return nullptr => logging disabled for this thread
		return nullptr;
	}

	// thread_local buffer (no heap)
	thread_local ThreadLog g_localThreadLog;

	// get current thread log pointer (register on first use)
	static inline ThreadLog* GetThreadLog()
	{
		static thread_local bool registered = false;
		if (!registered) {
			ThreadLog* result = RegisterCurrentThreadLog(&g_localThreadLog);
			registered = true; // even if result==nullptr we won't retry (avoid allocations)
			(void)result;
		}
		// we return pointer even if not in global registry — draining iterates only registered ones
		return &g_localThreadLog;
	}

	// ---------- Safe record functions (callable inside hooks) ----------

	inline void RecordAlloc(uint32_t caller, uint32_t size, void* ptr) noexcept
	{
		ThreadLog* tlog = GetThreadLog();
		if (!tlog) return;

		// producer writes event into buffer, then increments writeIndex (release)
		uint32_t idx = tlog->writeIndex.load(std::memory_order_relaxed);
		uint32_t slot = idx % THREAD_BUF_SZ;

		// write payload (plain stores)
		tlog->buf[slot].ptr = ptr;
		tlog->buf[slot].size = size;
		tlog->buf[slot].caller = caller;
		tlog->buf[slot].kind = 0; // alloc

		// publish the event (release ensures prior writes visible to consumer)
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
		tlog->buf[slot].kind = 1; // free

		tlog->writeIndex.store(idx + 1, std::memory_order_release);
	}

	// returns pointer to trampoline (callable), or nullptr on failure
	void* CreateTrampoline(uintptr_t target, size_t patchSize)
	{
		if (patchSize < 5) return nullptr;
		size_t trampSize = patchSize + 5;
		uint8_t* trampoline = static_cast<uint8_t*>(
			VirtualAlloc(nullptr, trampSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE)
			);
		if (!trampoline) return nullptr;

		// copy original bytes
		memcpy(trampoline, reinterpret_cast<void*>(target), patchSize);

		// jmp back to original function after overwritten bytes
		uintptr_t retAddr = target + patchSize;
		uintptr_t jmpFrom = reinterpret_cast<uintptr_t>(trampoline + patchSize);
		int32_t relBack = static_cast<int32_t>(retAddr - (jmpFrom + 5));
		trampoline[patchSize] = 0xE9;
		*reinterpret_cast<int32_t*>(trampoline + patchSize + 1) = relBack;

		return trampoline;
	}

	bool PatchJump(uintptr_t target, void* hookFunc)
	{
		DWORD old;
		if (!VirtualProtect(reinterpret_cast<void*>(target), 5, PAGE_EXECUTE_READWRITE, &old)) return false;
		uintptr_t hookAddr = reinterpret_cast<uintptr_t>(hookFunc);
		int32_t rel = static_cast<int32_t>(hookAddr - (target + 5));
		uint8_t jmp = 0xE9;
		memcpy(reinterpret_cast<void*>(target), &jmp, 1);
		memcpy(reinterpret_cast<void*>(target + 1), &rel, 4);
		VirtualProtect(reinterpret_cast<void*>(target), 5, old, &old);
		return true;
	}

	void* __cdecl Hooked_FormHeapAlloc(size_t size)
	{
		uint32_t caller = reinterpret_cast<uint32_t>(_ReturnAddress());
		void* result = nullptr;
		if (g_origAlloc) result = g_origAlloc(size);
		else {
			using RawAlloc_t = void* (__cdecl*)(size_t);
			RawAlloc_t raw = reinterpret_cast<RawAlloc_t>(0x00401F00); // fallback only
			result = raw(size);
		}
		if (result) RecordAlloc(caller, static_cast<uint32_t>(size), result);
		return result;
	}

	void __cdecl Hooked_FormHeapFree(void* ptr)
	{
		uint32_t caller = reinterpret_cast<uint32_t>(_ReturnAddress());
		// record free first or after; recording after is fine
		RecordFree(caller, ptr);
		if (g_origFree) g_origFree(ptr);
		else {
			using RawFree_t = void(__cdecl*)(void*);
			RawFree_t raw = reinterpret_cast<RawFree_t>(0x00401F20);
			raw(ptr);
		}
	}

	void DrainThreadLogs()
	{
		// iterate registered slots
		for (size_t i = 0; i < MAX_THREAD_LOGS; ++i) {
			ThreadLog* t = g_threadLogs[i].load(std::memory_order_acquire);
			if (!t) continue;

			// read producer index (number of events produced)
			uint32_t produced = t->writeIndex.load(std::memory_order_acquire);
			uint32_t start = t->lastConsumed;
			// process events start .. produced-1
			for (uint32_t idx = start; idx < produced; ++idx) {
				uint32_t slot = idx % THREAD_BUF_SZ;
				const AllocEvent& ev = t->buf[slot];

				// apply event
				if (ev.kind == 0) {
					std::scoped_lock lock(g_allocMutex);
					g_activeAllocs[ev.ptr] = { ev.caller, ev.size };
					g_totalActiveBytes.fetch_add(ev.size, std::memory_order_relaxed);
				}
				else {
					std::scoped_lock lock(g_allocMutex);
					auto it = g_activeAllocs.find(ev.ptr);
					if (it != g_activeAllocs.end()) {
						g_totalActiveBytes.fetch_sub(it->second.size, std::memory_order_relaxed);
						g_activeAllocs.erase(it);
					}
				}
			}
			// mark consumed
			t->lastConsumed = produced;
		}
	}

	void StartDrainThread()
	{
		static std::atomic<bool> running{ true };
		std::thread([] {
			SetThreadDescription(GetCurrentThread(), L"AllocDrainThread");
			while (true) {
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
				DrainThreadLogs();
			}
			}).detach();
	}

	bool InstallAllocHook()
	{
		g_allocTramp = CreateTrampoline(TARGET_ALLOC, PATCH_SIZE);
		if (!g_allocTramp) return false;
		g_origAlloc = reinterpret_cast<Alloc_t>(g_allocTramp);
		return PatchJump(TARGET_ALLOC, &Hooked_FormHeapAlloc);
	}

	bool InstallFreeHook()
	{
		g_freeTramp = CreateTrampoline(TARGET_FREE, PATCH_SIZE);
		if (!g_freeTramp) return false;
		g_origFree = reinterpret_cast<Free_t>(g_freeTramp);
		return PatchJump(TARGET_FREE, &Hooked_FormHeapFree);
		StartDrainThread();
	}

	void DumpAllocStats(HANDLE hProcess)
	{
		DrainThreadLogs();
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		DrainThreadLogs();

		char workingDirectory[MAX_PATH];
		char symbolPath[MAX_PATH];
		char altSymbolPath[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, workingDirectory);
		GetEnvironmentVariable("_NT_SYMBOL_PATH", symbolPath, MAX_PATH);
		GetEnvironmentVariable("_NT_ALTERNATE_SYMBOL_PATH", altSymbolPath, MAX_PATH);
		std::string lookPath = std::format("{};{}\\Data\\OBSE\\plugins;{};{}", workingDirectory, workingDirectory, symbolPath, altSymbolPath);

		char newSymbolPath[MAX_PATH] = {};
		if (!SymGetSearchPath(hProcess, newSymbolPath, MAX_PATH)) {
			SymCleanup(hProcess);
			SymInitialize(hProcess, lookPath.c_str(), TRUE);
		}

		std::unordered_map<uint32_t, uint64_t> callerBytes;
		std::unordered_map<uint32_t, uint32_t> callerCounts;

		{
			std::scoped_lock lock(g_allocMutex);
			for (auto& [ptr, info] : g_activeAllocs) {
				callerBytes[info.caller] += info.size;
				callerCounts[info.caller] += 1;
			}
		}

		// Move into sortable vector
		struct CallerStat {
			uint32_t caller;
			uint64_t bytes;
			uint32_t count;
		};

		std::vector<CallerStat> stats;
		stats.reserve(callerBytes.size());
		for (auto& [caller, bytes] : callerBytes)
			stats.push_back({ caller, bytes, callerCounts[caller] });

		// Sort descending by total bytes
		std::sort(stats.begin(), stats.end(),
			[](const CallerStat& a, const CallerStat& b) {
				return a.bytes > b.bytes;
			});

		std::ofstream out("AllocDump.txt");
		out << "===== Active Allocations by Caller =====\n";
		out << std::format("{:<50} {:>12} {:>12}\n", "Function", "Count", "Bytes");
		out << "--------------------------------------------------------------\n";

		for (auto& stat : stats) {
			std::string sym = PDB::GetSymbol(stat.caller, hProcess);
			out << std::format("{:<50} {:>12} {:>12}\n", sym, stat.count, stat.bytes);
		}

		out << "--------------------------------------------------------------\n";
		out << std::format("Total Active Allocations: {:>12} bytes ({:.2f} MB)\n",
			g_totalActiveBytes.load(),
			g_totalActiveBytes.load() / (1024.0 * 1024.0));

		out.close();

		_MESSAGE("Memory dump complete: %zu entries, total active %.2f MB",
			stats.size(),
			g_totalActiveBytes.load() / (1024.0 * 1024.0));

		out.close();
	}

	void DumpAllocationsToFile(const std::map<UInt32, UInt32>& allocMap, HANDLE hProcess)
	{
		_MESSAGE("Dumping Allocations to file");
		// Copy entries into a sortable vector
		std::vector<std::pair<UInt32, UInt32>> sortedEntries(allocMap.begin(), allocMap.end());

		// Sort descending by count
		std::sort(sortedEntries.begin(), sortedEntries.end(),
			[](const auto& a, const auto& b) {
				return a.second > b.second;
			});

		// Open file
		std::ofstream output("AllocDump.txt", std::ios::out | std::ios::trunc);
		if (!output.is_open()) {
			_MESSAGE("Failed to open AllocDump.txt for writing.");
			return;
		}

		output << "=== Allocation Dump ===\n\n";

		char workingDirectory[MAX_PATH];
		char symbolPath[MAX_PATH];
		char altSymbolPath[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, workingDirectory);
		GetEnvironmentVariable("_NT_SYMBOL_PATH", symbolPath, MAX_PATH);
		GetEnvironmentVariable("_NT_ALTERNATE_SYMBOL_PATH", altSymbolPath, MAX_PATH);
		std::string lookPath = std::format("{};{}\\Data\\OBSE\\plugins;{};{}", workingDirectory, workingDirectory, symbolPath, altSymbolPath);

		char newSymbolPath[MAX_PATH] = {};
		if (!SymGetSearchPath(hProcess, newSymbolPath, MAX_PATH)) {
			SymCleanup(hProcess);
			SymInitialize(hProcess, lookPath.c_str(), TRUE);
		}

		for (const auto& [addr, count] : sortedEntries) {
			std::string symbol = PDB::GetSymbol(addr, hProcess);
			if (symbol.empty())
				symbol = std::format("0x{:08X}", addr);

			output << std::format("Count: {:6} | 0x{:08X} | {}\n", count, addr, symbol);
		}

		output << "\n=== End of Dump ===\n";
		output.close();

		_MESSAGE("Wrote allocation dump to AllocDump.txt (%zu entries)", sortedEntries.size());
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
		if ( GetProcessMemoryInfo( hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc)) )
		{
			DWORDLONG virtUsage = memoryStatus.ullTotalVirtual - memoryStatus.ullAvailVirtual;
			DWORDLONG physUsage = pmc.PrivateUsage;
			output << "Process' Memory:" << '\n';
			output << std::format("Physical Usage: {}", GetMemoryUsageString(physUsage, memoryStatus.ullTotalPhys)) << '\n';
			output << std::format("Virtual  Usage: {}", GetMemoryUsageString(virtUsage, memoryStatus.ullTotalVirtual)) << '\n';
		}

		if (g_formHeap) {
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
			
			/*if (stats.uiHeapOverhead == sizeof(ZeroOverheadHeap)) {
				start = reinterpret_cast<SIZE_T>(static_cast<ZeroOverheadHeap*>(heap)->pHeap);
				end = start + static_cast<ZeroOverheadHeap*>(heap)->uiSize;
			}
			else {
				start = reinterpret_cast<SIZE_T>(static_cast<MemoryHeap*>(heap)->pMemHeap);
				end = start + static_cast<MemoryHeap*>(heap)->uiMemHeapSize;
			}*/

			output << std::format("{:30}	 {}	  ({:08X} - {:08X})", "FormHeap", GetMemoryUsageString(used, total), start, end) << '\n';
#endif
			usedHeapMemory += used;
			totalHeapMemory += total;

			



			SIZE_T uiPoolMemory = 0;
			SIZE_T uiTotalPoolMemory = 0;
#if PRINT_POOLS
			output << "\nPools:" << '\n';
#endif
			for (UInt32 i = 0; i < 256; i++) {
				MemoryPool* pPool = g_memoryHeap_poolsByAddress[i];
				if (!pPool)
					continue;

				SIZE_T used = pPool->field_10C;
				SIZE_T total = pPool->field_110;

				uiPoolMemory += used;
				uiTotalPoolMemory += total;
#if PRINT_POOLS
				SIZE_T start = reinterpret_cast<SIZE_T>(pPool->field_108);
				SIZE_T end = start + pPool->field_110;
				output << std::format("{:30}	 {}	  ({:08X} - {:08X})", pPool->m_name, GetMemoryUsageString(used, total), start, end) << '\n';
#endif
			}

			output << std::format("\nTotal Heap Memory: {}", GetMemoryUsageString(usedHeapMemory, totalHeapMemory)) << '\n';
			output << std::format("Total Pool Memory: {}", GetMemoryUsageString(uiPoolMemory, uiTotalPoolMemory)) << '\n';
			output << std::format("Total Memory:      {}", GetMemoryUsageString(usedHeapMemory + uiPoolMemory, totalHeapMemory + uiTotalPoolMemory)) << '\n';

			
			DumpAllocStats(hProcess);
		}
	}
	catch (...) { output << "Failed to log memory." << '\n'; }

	extern std::stringstream& Get() { output.flush(); return output; }
}