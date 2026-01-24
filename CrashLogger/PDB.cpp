#include "PDB.h"
#include <DbgHelp.h>
#include "Utilities.hpp"

namespace CrashLogger::PDB
{
	extern std::string GetModule(UInt32 eip, HANDLE process)
	{
		IMAGEHLP_MODULE module = { 0 };
		module.SizeOfStruct = sizeof(IMAGEHLP_MODULE);
		if (!SymGetModuleInfo(process, eip, &module)) return "";

		return module.ModuleName;
	}

	extern UInt32 GetModuleBase(UInt32 eip, HANDLE process)
	{
		IMAGEHLP_MODULE module = { 0 };
		module.SizeOfStruct = sizeof(IMAGEHLP_MODULE);
		if (!SymGetModuleInfo(process, eip, &module)) return 0;

		return module.BaseOfImage;
	}

	extern std::string GetSymbol(UInt32 eip, HANDLE process)
	{
		char symbolBuffer[sizeof(SYMBOL_INFO) + 255];
		const auto symbol = (SYMBOL_INFO*)symbolBuffer;
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		symbol->MaxNameLen = 254;
		DWORD64 offset = 0;
		if (!SymFromAddr(process, eip, &offset, symbol)) return "";
		const std::string functioName = symbol->Name;
		return std::format("{}+0x{:0X}", functioName, offset);
	}

	extern std::string GetLine(UInt32 eip, HANDLE process)
	{
		char lineBuffer[sizeof(IMAGEHLP_LINE) + 255];
		const auto line = (IMAGEHLP_LINE*)lineBuffer;
		line->SizeOfStruct = sizeof(IMAGEHLP_LINE);

		DWORD offset = 0;

		if (!SymGetLineFromAddr(process, eip, &offset, line)) return "";

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

	// use the RTTI information to return an object's class name
	const char* GetObjectClassNameInternal2(void* objBase)
	{
		__try
		{
			const char* result = "";
			void** obj = (void**)objBase;
			RTTILocator** vtbl = (RTTILocator**)obj[0];
			RTTILocator* rtti = vtbl[-1];
			RTTIType* type = rtti->type;

			if (!type) return "";
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
		__except (ExceptionFilter(GetExceptionCode()))
		{
			return "";
		}
	}

	static bool IsReadable(const void* p, size_t size = sizeof(void*))
	{
		MEMORY_BASIC_INFORMATION mbi{};
		if (!VirtualQuery(p, &mbi, sizeof(mbi)))
			return false;

		if (mbi.State != MEM_COMMIT)
			return false;

		if (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD))
			return false;

		return size <= mbi.RegionSize;
	}

	const char* GetObjectClassNameSafe(void* objBase)
	{
		if (!objBase) return "";

		// 1) Validate object base
		if (!IsReadable(objBase, sizeof(void*)))
			return "";

		// 2) Read vtable pointer
		void** vtbl = *(void***)objBase;
		if (!vtbl)
			return "";

		// 3) Validate vtable memory (at least one pointer BEFORE it)
		if (!IsReadable(vtbl - 1, sizeof(void*)))
			return "";

		// 4) Read RTTI locator POINTER VALUE
		void* colPtr = vtbl[-1];
		if (!colPtr)
			return "";

		// 5) Validate RTTI locator structure memory
		if (!IsReadable(colPtr, sizeof(PDB::RTTILocator)))
			return "";

		auto* rtti = static_cast<PDB::RTTILocator*>(colPtr);

		// 6) Validate RTTIType pointer
		if (!rtti->type || !IsReadable(rtti->type, sizeof(PDB::RTTIType)))
			return "";

		auto* type = rtti->type;

		// 7) Validate name pointer
		if (!IsReadable(type->name, 4))
			return "";

		// 8) MSVC RTTI names start with ".?AV"
		if (type->name[0] != '.' || type->name[1] != '?')
			return "";

		// 9) Ensure null termination
		for (UInt32 i = 0; i < MAX_PATH; i++)
		{
			if (!IsReadable(&type->name[i], 1))
				return "";

			if (type->name[i] == '\0')
				return type->name;
		}

		return "";
	}

	std::string GetClassNameFromRTTI(void* object)
	{
		//std::string name = GetObjectClassNameInternal2(object);
		std::string name = GetObjectClassNameSafe(object);
		if (name.empty()) return name;
		// Starts with .?AV, ends with @@
//		return name.substr(4, name.size() - 6);

		char buffer[MAX_PATH];
		UnDecorateSymbolName(name.substr(1, name.size() - 1).c_str(), buffer, MAX_PATH, UNDNAME_NO_ARGUMENTS);
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