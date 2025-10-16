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
	std::map<UInt32, UInt32> allocMap;
	static const UInt32 TARGET = 0x00401F00;
	static uint8_t g_origBytes[16] = { 0 };
	static size_t  g_patchSize = 6;
	static uint8_t* g_trampoline = nullptr;
	typedef void* (__cdecl* FormHeapAllocate_t)(size_t);
	static FormHeapAllocate_t g_originalAlloc = nullptr;

	// Helper: create trampoline copying g_patchSize bytes + jmp back
	bool CreateTrampoline()
	{
		// backup original bytes
		memcpy(g_origBytes, (void*)TARGET, g_patchSize);

		// allocate RWX memory for trampoline
		const size_t trampSize = g_patchSize + 5; // copied bytes + jmp rel32 back
		g_trampoline = (uint8_t*)VirtualAlloc(nullptr, trampSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if (!g_trampoline) return false;

		// copy the original bytes
		memcpy(g_trampoline, (void*)TARGET, g_patchSize);

		// append jmp back to original code at TARGET + g_patchSize
		uintptr_t origContinue = (uintptr_t)TARGET + g_patchSize;
		uintptr_t jmpFrom = (uintptr_t)(g_trampoline + g_patchSize);
		int32_t relBack = static_cast<int32_t>(origContinue - (jmpFrom + 5));

		g_trampoline[g_patchSize + 0] = 0xE9; // JMP rel32
		*reinterpret_cast<int32_t*>(g_trampoline + g_patchSize + 1) = relBack;

		// set callable pointer
		g_originalAlloc = reinterpret_cast<FormHeapAllocate_t>(g_trampoline);
		return true;
	}

	void* __cdecl new_FormHeapAllocate(size_t allocSize)
	{
		// Get caller return address
		void* returnAddr = _ReturnAddress();

		UInt32 caller = reinterpret_cast<UInt32>(returnAddr);

		// Log safely (no heap alloc)
		//LogCallerAddr(caller, allocSize);
		
		allocMap[caller]++;

		// Call original via trampoline
		if (g_originalAlloc) {
			return g_originalAlloc(allocSize);
		}

		// Fallback if trampoline failed (risky)
		typedef void* (__cdecl* RawAlloc_t)(size_t);
		RawAlloc_t raw = reinterpret_cast<RawAlloc_t>(TARGET);
		return raw(allocSize);
	}

	// Install hook: write JMP at target -> new_FormHeapAllocate
	bool InstallAllocHook()
	{
		if (!CreateTrampoline()) return false;

		uintptr_t hookAddr = reinterpret_cast<uintptr_t>(&new_FormHeapAllocate);
		int32_t relToHook = static_cast<int32_t>(hookAddr - (TARGET + 5));

		// write opcode 0xE9 then rel32 using SafeWrite
		SafeWrite8(TARGET, 0xE9);
		SafeWrite32(TARGET + 1, static_cast<UInt32>(relToHook));

		return true;
	}

	// Remove hook: restore original bytes and free trampoline
	void RemoveHook()
	{
		// restore original bytes (use VirtualProtect-aware write)
		DWORD old;
		VirtualProtect((LPVOID)TARGET, g_patchSize, PAGE_EXECUTE_READWRITE, &old);
		memcpy((void*)TARGET, g_origBytes, g_patchSize);
		FlushInstructionCache(GetCurrentProcess(), (void*)TARGET, g_patchSize);
		VirtualProtect((LPVOID)TARGET, g_patchSize, old, &old);

		if (g_trampoline) {
			VirtualFree(g_trampoline, 0, MEM_RELEASE);
			g_trampoline = nullptr;
			g_originalAlloc = nullptr;
		}
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

			
			DumpAllocationsToFile(allocMap, hProcess);
		}
	}
	catch (...) { output << "Failed to log memory." << '\n'; }

	extern std::stringstream& Get() { output.flush(); return output; }
}