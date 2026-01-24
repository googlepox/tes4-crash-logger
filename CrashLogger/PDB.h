#pragma once
#include <string>
#include <Windows.h>

namespace CrashLogger::PDB
{
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

    std::string GetModule(UInt32 eip, HANDLE process);
    UInt32 GetModuleBase(UInt32 eip, HANDLE process);
    std::string GetSymbol(UInt32 eip, HANDLE process);
    std::string GetLine(UInt32 eip, HANDLE process);

    std::string GetClassNameFromRTTIorPDB(void* object);
}
