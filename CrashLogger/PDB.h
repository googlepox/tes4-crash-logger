#pragma once
#include <Windows.h>
#include <string>
#include "ITypes.h"

namespace CrashLogger::PDB
{
    struct RTTIType
    {
        void* typeInfo;
        UInt32 pad;
        char name[0];
    };

    struct RTTILocator
    {
        UInt32 sig;
        UInt32 offset;
        UInt32 cdOffset;
        RTTIType* type;
    };

    std::string GetModule(UInt32 eip, HANDLE process);
    UInt32 GetModuleBase(UInt32 eip, HANDLE process);
    std::string GetSymbol(UInt32 eip, HANDLE process);
    std::string GetLine(UInt32 eip, HANDLE process);
    std::string IsReadable(const void* p, size_t size);
    std::string GetObjectClassNameImpl(void* objBase, HANDLE process) ;
    std::string GetObjectClassNameInternal2(void* objBase, HANDLE process);
    std::string GetClassNameFromRTTI(void* object, HANDLE process) noexcept;
    std::string GetClassNameFromPDB(void* object, HANDLE process) noexcept;
    std::string GetClassNameFromRTTIorPDB(void* object, HANDLE process) noexcept;

}
