#include "CrashLogger.hpp"
#include "SafeWrite.h"
#include "Logging.hpp"
#include <signal.h>
#include "SharedMem.h"

#define SYMOPT_EX_WINE_NATIVE_MODULES 1000

constexpr UInt32 ce_printStackCount = 256;

namespace CrashLogger
{
	void trans_func( unsigned int u, EXCEPTION_POINTERS* pExp )
	{
		throw SE_Exception();
	}

	void LogPlaytime(EXCEPTION_POINTERS* info) {
		__try {
			Playtime::Process(info);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			_MESSAGE("Failed to log playtime.");
		}
	}

	void LogException(EXCEPTION_POINTERS* info) {
		__try {
			Exception::Process(info);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			_MESSAGE("Failed to log exception.");
		}
	}

	void LogThread(EXCEPTION_POINTERS* info) {
		__try {
			Thread::Process(info);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			_MESSAGE("Failed to log thread.");
		}
	}

	void LogCalltrace(EXCEPTION_POINTERS* info) {
		__try {
			Calltrace::Process(info);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			_MESSAGE("Failed to log calltrace.");
		}
	}

	void LogRegistry(EXCEPTION_POINTERS* info) {
		__try {
			Registry::Process(info);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			_MESSAGE("Failed to log registers.");
		}
	}

	void LogStack(EXCEPTION_POINTERS* info) {
		__try {
			Stack::Process(info);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			_MESSAGE("Failed to log stack.");
		}
	}

	void LogInstall(EXCEPTION_POINTERS* info) {
		__try {
			Install::Process(info);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			_MESSAGE("Failed to log install.");
		}
	}

	void LogMemory(EXCEPTION_POINTERS* info) {
		__try {
			Memory::Process(info);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			_MESSAGE("Failed to log memory.");
		}
	}

	void LogDevice(EXCEPTION_POINTERS* info) {
		__try {
			Device::Process(info);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			_MESSAGE("Failed to log device.");
		}
	}

	void LogGameData(EXCEPTION_POINTERS* info)
	{
		__try
		{
			GameData::Process(info);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			_MESSAGE("Failed to log game data.");
		}
	}

	void Log(EXCEPTION_POINTERS* info)
	{
		const auto begin = std::chrono::system_clock::now();

		_MESSAGE("Processing playtime");
		LogPlaytime(info);
		_MESSAGE("Processing exception");
		LogException(info);
		_MESSAGE("Processing thread");
		LogThread(info);
		_MESSAGE("Processing memory");
		LogMemory(info);
		//_MESSAGE("Processing device");
		//LogDevice(info);
		_MESSAGE("Processing calltrace");
		LogCalltrace(info);
		_MESSAGE("Processing registers");
		LogRegistry(info);
		_MESSAGE("Processing stack");
		LogStack(info);
		//_MESSAGE("Processing mods);
		//Mods::Process(info)
		_MESSAGE("Processing install");
		LogInstall(info);
		//_MESSAGE("processing game data");
		//GameData::Process(info);
		//AssetTracker::Process(info);

		const auto processing = std::chrono::system_clock::now();

		_MESSAGE("%s", Playtime::Get().str().c_str());
		_MESSAGE("%s", Exception::Get().str().c_str());
		_MESSAGE("%s", Thread::Get().str().c_str());
		_MESSAGE("================================");
		_MESSAGE("%s", Calltrace::Get().str().c_str());
		_MESSAGE("================================\n");
		_MESSAGE("%s", Registry::Get().str().c_str());
		_MESSAGE("================================\n");
		_MESSAGE("%s", Stack::Get().str().c_str());
		_MESSAGE("================================\n");
		//_MESSAGE("%s", Device::Get().str().c_str());
		//_MESSAGE("================================");
		_MESSAGE("%s", Memory::Get().str().c_str());
		_MESSAGE("================================\n");
		LogGameData(info);
		//_MESSAGE("================================\n");
		//_MESSAGE("================================");
		//_MESSAGE("%s", Mods::Get().str());
		//_MESSAGE("================================");
		//_MESSAGE("%s", AssetTracker::Get().str());
		//_MESSAGE("================================");
		Modules::Process(info);
		//_MESSAGE("%s", Modules::Get().str().c_str());
		_MESSAGE("================================\n");
		_MESSAGE("%s", Install::Get().str().c_str());

		const auto printing = std::chrono::system_clock::now();

		const auto timeProcessing = std::chrono::duration_cast<std::chrono::milliseconds>(processing - begin);

		const auto timePrinting = std::chrono::duration_cast<std::chrono::milliseconds>(printing - processing);

		_MESSAGE("%s", std::format("Processed in {:d} ms, printed in {:d} ms", timeProcessing.count(), timePrinting.count()).c_str());

		Logger::Copy();

		SymCleanup(GetCurrentProcess());
	};

	void AttemptLog(EXCEPTION_POINTERS* info) {
		__try { Log(info); }
		__except (EXCEPTION_EXECUTE_HANDLER) {
			_MESSAGE("Failed to log exception info");
		};
	}

	static LPTOP_LEVEL_EXCEPTION_FILTER s_originalFilter = nullptr;

	static PVOID handle;

	static bool caught = false;

	LONG WINAPI LogVectored(EXCEPTION_POINTERS* ExceptionInfo) {
		if (ExceptionInfo->ExceptionRecord->ExceptionCode == 0x406D1388) {
			//Log(ExceptionInfo);
			return EXCEPTION_CONTINUE_EXECUTION;
		}
		if(ExceptionInfo->ExceptionRecord->ExceptionCode == 0x40010006){
			_MESSAGE("%s",   reinterpret_cast<char*>(ExceptionInfo->ExceptionRecord->ExceptionInformation[1]));
			return EXCEPTION_CONTINUE_EXECUTION;
		}
		if (!caught) {
			caught = true;
			//_MESSAGE("From Vectored Handler");
			AttemptLog(ExceptionInfo);

			RemoveVectoredExceptionHandler(handle);
		}
		return EXCEPTION_CONTINUE_SEARCH;
	}
	void AddVectoredException() {
		handle = AddVectoredExceptionHandler(0, &LogVectored);
	}

	LONG WINAPI Filter(EXCEPTION_POINTERS* info) {

		bool ignored = false;
		if (caught) ignored = true;
		else
		{
			caught = true;
			AttemptLog(info);
		}
		if (s_originalFilter) return s_originalFilter(info);
		return  EXCEPTION_CONTINUE_SEARCH;
	};

	LPTOP_LEVEL_EXCEPTION_FILTER WINAPI FakeSetUnhandledExceptionFilter(__in LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter) {
		s_originalFilter = lpTopLevelExceptionFilter;
		return nullptr;
	}

	void InstallExceptionFilterHook()
	{
		s_originalFilter = SetUnhandledExceptionFilter(&Filter);

		SafeWrite32(0x00A281B4, (UInt32)&FakeSetUnhandledExceptionFilter);
	}

	extern void Init()
	{
		InitSharedMemory(true);

		Playtime::Init();

		InstallExceptionFilterHook();

		Memory::InstallAllocHook();
		Memory::InstallFreeHook();

		Memory::StartMemoryProfiler();
		Memory::LaunchHelper();

		AddVectoredException();
	}
}
