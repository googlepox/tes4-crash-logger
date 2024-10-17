#include <CrashLogger.hpp>
#include <obse_common/SafeWrite.h>

#define SYMOPT_EX_WINE_NATIVE_MODULES 1000

constexpr UInt32 ce_printStackCount = 256;

namespace CrashLogger::PDB
{
	extern std::string GetModule(UInt32 eip, HANDLE process)
	{
		IMAGEHLP_MODULE module = { 0 };
		module.SizeOfStruct = sizeof(IMAGEHLP_MODULE);
		if (!Safe_SymGetModuleInfo(process, eip, &module)) return "";

		return module.ModuleName;
	}

	extern UInt32 GetModuleBase(UInt32 eip, HANDLE process)
	{
		IMAGEHLP_MODULE module = { 0 };
		module.SizeOfStruct = sizeof(IMAGEHLP_MODULE);
		if (!Safe_SymGetModuleInfo(process, eip, &module)) return 0;

		return module.BaseOfImage;
	}

	extern std::string GetSymbol(UInt32 eip, HANDLE process)
	{
		char symbolBuffer[sizeof(SYMBOL_INFO) + 255];
		const auto symbol = (SYMBOL_INFO*)symbolBuffer;

		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		symbol->MaxNameLen = 254;
		DWORD64 offset = 0;

		if (!Safe_SymFromAddr(process, eip, &offset, symbol)) return "";

		const std::string functioName = symbol->Name;

		return std::format("{}+0x{:0X}", functioName, offset);
	}

	extern std::string GetLine(UInt32 eip, HANDLE process)
	{
		char lineBuffer[sizeof(IMAGEHLP_LINE) + 255];
		const auto line = (IMAGEHLP_LINE*)lineBuffer;
		line->SizeOfStruct = sizeof(IMAGEHLP_LINE);

		DWORD offset = 0;

		if (!Safe_SymGetLineFromAddr(process, eip, &offset, line)) return "";

		return std::format("{}:{:d}", line->FileName, line->LineNumber);
	}

	std::string& GetClassNameGetSymbol(void* object, std::string& buffer)
	{
		buffer = GetSymbol(*((UInt32*)object), GetCurrentProcess());
		return buffer;
	}

	std::string& GetClassNameFromPDBSEH(void* object, std::string& buffer)
		try { GetClassNameGetSymbol(object, buffer); return buffer; }
	catch (...) { return buffer; }


	std::string GetClassNameFromPDB(void* object)
	{
		std::string name;
		GetClassNameFromPDBSEH(object, name);
		return name.substr(0, name.find("::`vftable'"));
	}

	struct RTTIType
	{
		void* typeInfo;
		UInt32	pad;
		char	name[0];
	};

	struct RTTILocator
	{
		UInt32		sig, offset, cdOffset;
		RTTIType* type;
	};

	// use the RTTI information to return an object's class name
	const char* GetObjectClassNameInternal2(void* objBase)
	{
		__try {
			const char* result = "";
			void** obj = (void**)objBase;
			RTTILocator** vtbl = (RTTILocator**)obj[0];
			RTTILocator* rtti = vtbl[-1];
			RTTIType* type = rtti->type;

			// starts with .?AV
			if ((type->name[0] == '.') && (type->name[1] == '?'))
			{
				// is at most MAX_PATH chars long
				for (UInt32 i = 0; i < MAX_PATH; i++) if (type->name[i] == 0)
				{
					result = type->name;
					break;
				}
			}
			return result;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return "";
		}
	}

	std::string GetClassNameFromRTTI(void* object)
	{
		std::string name = GetObjectClassNameInternal2(object);
		// Starts with .?AV, ends with @@
//		return name.substr(4, name.size() - 6);

		char buffer[MAX_PATH];
		Safe_UnDecorateSymbolName(name.substr(1, name.size() - 1).c_str(), buffer, MAX_PATH, UNDNAME_NO_ARGUMENTS);
		name = buffer;

		return name.substr(6, name.size() - 6);
	}

	extern std::string GetClassNameFromRTTIorPDB(void* object)
	{
		if (const auto str = GetClassNameFromRTTI(object); !str.empty()) return str;
		return GetClassNameFromPDB(object);
		//if (const auto str = GetClassNameFromPDB(object); !str.contains("0x")) return str;
	}
};

namespace CrashLogger
{

	void Log(EXCEPTION_POINTERS* info)
	{
		const auto begin = std::chrono::system_clock::now();

		Playtime::Process(info);
		Exception::Process(info);
		//Thread::Process(info);
		//Memory::Process(info);
		//_MESSAGE("processing device");
		//Device::Process(info);
		Calltrace::Process(info);
		//_MESSAGE("processing registry");
		Registry::Process(info);
		Stack::Process(info);
		//Mods::Process(info)
		Install::Process(info);
		Modules::Process(info);
		//AssetTracker::Process(info);

		const auto processing = std::chrono::system_clock::now();

		_MESSAGE("%s", Playtime::Get().str().c_str());
		_MESSAGE("%s", Exception::Get().str().c_str());
		//_MESSAGE("%s", Thread::Get().str().c_str());
		_MESSAGE("%s", Calltrace::Get().str().c_str());
		_MESSAGE("================================");
		_MESSAGE("%s", Registry::Get().str().c_str());
		_MESSAGE("================================");
		_MESSAGE("%s", Stack::Get().str().c_str());
		_MESSAGE("================================");
		//_MESSAGE("%s", Device::Get().str().c_str());
		//_MESSAGE("================================");
		//_MESSAGE("%s", Memory::Get().str());
		//_MESSAGE("================================");
		//_MESSAGE("%s", Mods::Get().str());
		//_MESSAGE("================================");
		//_MESSAGE("%s", AssetTracker::Get().str());
		//_MESSAGE("================================");
		_MESSAGE("%s", Modules::Get().str().c_str());
		_MESSAGE("================================");
		_MESSAGE("%s", Install::Get().str().c_str());

		const auto printing = std::chrono::system_clock::now();

		const auto timeProcessing = std::chrono::duration_cast<std::chrono::milliseconds>(processing - begin);

		const auto timePrinting = std::chrono::duration_cast<std::chrono::milliseconds>(printing - processing);

		_MESSAGE("%s", std::format("Processed in {:d} ms, printed in {:d} ms", timeProcessing.count(), timePrinting.count()).c_str());

		//Logger::Copy();

		Safe_SymCleanup(GetCurrentProcess());
	};

	static LPTOP_LEVEL_EXCEPTION_FILTER s_originalFilter = nullptr;

	LONG WINAPI Filter(EXCEPTION_POINTERS* info) {
		static bool caught = false;
		bool ignored = false;
		if (caught) ignored = true;
		else
		{
			caught = true;
			try { Log(info); }
			catch (...) {};
		}
		if (s_originalFilter) s_originalFilter(info); // don't return
		return !ignored ? EXCEPTION_CONTINUE_SEARCH : EXCEPTION_EXECUTE_HANDLER;
	};

	LPTOP_LEVEL_EXCEPTION_FILTER WINAPI FakeSetUnhandledExceptionFilter(__in LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter) {
		s_originalFilter = lpTopLevelExceptionFilter;
		return nullptr;
	}

	extern void Init()
	{
		Playtime::Init();

		s_originalFilter = SetUnhandledExceptionFilter(&Filter);

		SafeWrite32(0x00A281B4, (UInt32)&FakeSetUnhandledExceptionFilter);
	}
}