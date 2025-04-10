#include <CrashLogger.hpp>
#include <psapi.h>
#include <GameAPI.h>
#include <PluginManager.h>

#define PRINT_HEAPS 1
#define PRINT_POOLS 0

namespace CrashLogger::Memory
{
	std::stringstream output;

	extern void Process(EXCEPTION_POINTERS* info)
	try 
	{
		const auto hProcess = GetCurrentProcess();


		PROCESS_MEMORY_COUNTERS_EX2 pmc = {};
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
		}
	}
	catch (...) { output << "Failed to log memory." << '\n'; }

	extern std::stringstream& Get() { output.flush(); return output; }
}