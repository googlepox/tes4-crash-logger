#include <CrashLogger.hpp>
#include <psapi.h>
#include <GameAPI.h>
#include <Hooks_Memory.cpp>
#include <PluginManager.h>

#define _WIN32_WINNT 0x0A000010

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

		// If NVHR is used, the number will be 0
		if (g_formHeap) {
			/*_MESSAGE("field4: %u", g_formHeap->field_004);
			_MESSAGE("field8: %u", g_formHeap->field_008);
			_MESSAGE("fieldc: %u", g_formHeap->field_00C);
			_MESSAGE("field10: %u", g_formHeap->field_010);
			_MESSAGE("field14: %u", g_formHeap->field_014);
			_MESSAGE("field18: %u", g_formHeap->field_018);
			_MESSAGE("field1c: %u", g_formHeap->field_01C);
			_MESSAGE("field20: %u", g_formHeap->field_020);
			_MESSAGE("field24: %u", g_formHeap->field_024);
			_MESSAGE("field28: %u", g_formHeap ->field_028);
			_MESSAGE("field30: %u", g_formHeap ->field_030);
			_MESSAGE("field34: %u", g_formHeap ->field_034);
			_MESSAGE("field38: %u", g_formHeap ->field_038);
			_MESSAGE("field3c: %u", g_formHeap ->field_03C);
			_MESSAGE("field40: %u", g_formHeap ->field_040);
			_MESSAGE("field44: %u", g_formHeap ->field_044);
			_MESSAGE("field48: %u", g_formHeap ->field_048);
			_MESSAGE("field4c: %u", g_formHeap ->field_04C);
			_MESSAGE("field50: %u", g_formHeap ->field_050);
			_MESSAGE("field54: %u", g_formHeap ->field_054);
			_MESSAGE("field58: %u", g_formHeap ->field_058);
			_MESSAGE("field5c: %u", g_formHeap ->field_05C);
			_MESSAGE("field60: %u", g_formHeap ->field_060);
			_MESSAGE("field64: %u", g_formHeap ->field_064);
			_MESSAGE("field168: %u", g_formHeap ->field_168);
			_MESSAGE("fiel16c: %u", g_formHeap ->field_16C);
			_MESSAGE("field16d: %u", g_formHeap ->field_16D);*/
			UInt32 usedHeapMemory = 0;
			UInt32 totalHeapMemory = 0;

			output << "\n================================\n";

			output << "\nGame's Memory:" << '\n';

			UInt8* mainHeap = (UInt8*)g_formHeap->field_018;
			UInt8* mainHeapEnd = mainHeap + g_formHeap->field_00C;
			SIZE_T used = g_formHeap->field_01C;
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