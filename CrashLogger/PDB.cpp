#include "PDB.h"
#include <DbgHelp.h>
#include <windows.h>
#include <string>
#include <string_view>
#include <format>
#include <algorithm>
#include <IDebugLog.h>
#include "SharedMem.h"

#pragma comment(lib, "DbgHelp.lib")

namespace CrashLogger::PDB
{

    std::string GetModule(UInt32 eip, HANDLE process)
    {
        IMAGEHLP_MODULE module{};
        module.SizeOfStruct = sizeof(IMAGEHLP_MODULE);

        if (!SymGetModuleInfo(process, eip, &module))
            return {};

        return std::string(module.ModuleName ? module.ModuleName : "");
    }

    UInt32 GetModuleBase(UInt32 eip, HANDLE process)
    {
        IMAGEHLP_MODULE module{};
        module.SizeOfStruct = sizeof(IMAGEHLP_MODULE);

        if (!SymGetModuleInfo(process, eip, &module))
            return 0;

        return static_cast<UInt32>(module.BaseOfImage);
    }

    std::string GetSymbol(UInt32 eip, HANDLE process)
    {
        char symbolBuffer[sizeof(SYMBOL_INFO) + 255];
        auto* symbol = reinterpret_cast<SYMBOL_INFO*>(symbolBuffer);

        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = 254;

        DWORD64 displacement = 0;
        if (!SymFromAddr(process, eip, &displacement, symbol))
            return {};

        // "Name+0xDISPLACEMENT"
        return std::format("{}+0x{:X}", symbol->Name, static_cast<unsigned long long>(displacement));
    }

    std::string GetLine(UInt32 eip, HANDLE process)
    {
        IMAGEHLP_LINE line{};
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE);

        DWORD displacement = 0;
        if (!SymGetLineFromAddr(process, eip, &displacement, &line))
            return {};

        if (!line.FileName)
            return {};

        return std::format("{}:{}", line.FileName, line.LineNumber);
    }


    static bool IsReadable(HANDLE process, const void* p, size_t size)
    {
        if (!p || !process)
            return false;

        MEMORY_BASIC_INFORMATION mbi{};
        if (!VirtualQueryEx(process, p, &mbi, sizeof(mbi)))
            return false;

        if (mbi.State != MEM_COMMIT)
            return false;

        if (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD))
            return false;

        return size <= mbi.RegionSize;
    }


    static std::string GetObjectClassNameSafe(void* objBase, HANDLE process) noexcept
    {
        if (!objBase || !process)
            return {};

        // Read vtable pointer from object
        if (!IsReadable(process, objBase, sizeof(void*)))
            return {};

        void** vtbl = *reinterpret_cast<void***>(objBase);
        if (!vtbl)
            return {};

        // RTTI usually at vtbl[-1]
        if (!IsReadable(process, vtbl - 1, sizeof(void*)))
            return {};

        void* colPtr = vtbl[-1];
        if (!colPtr)
            return {};

        if (!IsReadable(process, colPtr, sizeof(RTTILocator)))
            return {};

        auto* rtti = static_cast<RTTILocator*>(colPtr);
        if (!rtti->type || !IsReadable(process, rtti->type, sizeof(RTTIType)))
            return {};

        RTTIType* type = rtti->type;

        // Check name pointer
        if (!IsReadable(process, type->name, 4))
            return {};

        if (type->name[0] != '.' || type->name[1] != '?')
            return {};

        // Ensure null-terminated within MAX_PATH
        for (UInt32 i = 0; i < MAX_PATH; i++)
        {
            if (!IsReadable(process, &type->name[i], 1))
                return {};

            if (type->name[i] == '\0')
                return std::string(type->name);
        }

        return {};
    }

    std::string GetObjectClassNameInternal2(void* objBase, HANDLE hProcess)
    {
        try
        {
            if (!objBase)
                return {};

            void** obj = static_cast<void**>(objBase);
            if (!obj[0])
                return {};

            auto** vtbl = reinterpret_cast<RTTILocator**>(obj[0]);
            auto* rtti = vtbl[-1];
            if (!rtti || !rtti->type || !rtti->type->name)
                return {};

            RTTIType* type = rtti->type;

            if ((type->name[0] == '.') && (type->name[1] == '?'))
            {
                for (UInt32 i = 0; i < MAX_PATH; i++)
                {
                    if (type->name[i] == '\0')
                        return std::string(type->name);
                }
            }

            return {};
        }
        catch (...)
        {
            return {};
        }
    }


    std::string GetClassNameFromRTTI(void* object, HANDLE process) noexcept
    {
        _MESSAGE("rtti1");

        if (!object || !process)
            return {};

        std::string decorated = GetObjectClassNameSafe(object, process);

        _MESSAGE("rtti2: decorated='%s'", decorated.c_str());
        if (decorated.empty())
            return {};

        if (decorated.size() < 2 || decorated[0] != '.' || decorated[1] != '?')
        {
            _MESSAGE("rtti3: bad decorated prefix");
            return {};
        }

        _MESSAGE("rtti4: undecorating");

        char tempBuffer[MAX_PATH]{};

        BOOL ok = UnDecorateSymbolName(
            decorated.c_str() + 1,
            tempBuffer,
            MAX_PATH - 1,
            UNDNAME_NO_ARGUMENTS
        );

        tempBuffer[MAX_PATH - 1] = '\0';

        _MESSAGE("rtti5: undecorated buffer='%s' ok=%d", tempBuffer, ok);

        if (!ok)
            return {};

        size_t safeLen = strnlen(tempBuffer, MAX_PATH);
        if (safeLen == MAX_PATH)
        {
            _MESSAGE("rttiX: undecorated NOT null-terminated (invalid RTTI)");
            return {};
        }

        std::string undec = std::string(tempBuffer, safeLen);

        if (undec.size() > 200)
        {
            _MESSAGE("rttiX: undecorated suspiciously large");
            return {};
        }

        // Strip "class " only if present
        constexpr std::string_view classPrefix = "class ";

        if (undec.rfind(classPrefix, 0) == 0 &&
            undec.size() > classPrefix.size())
        {
            return undec.substr(classPrefix.size());
        }

        return undec;
    }


    std::string GetClassNameFromPDB(void* object, HANDLE process) noexcept
    {
        if (!object || !process)
            return {};
        _MESSAGE("pdb1");
        // Make sure reading 4 bytes at object is safe
        if (!IsReadable(process, object, sizeof(UInt32)))
            return {};
        _MESSAGE("pdb2");
        UInt32 addr = 0;
        std::memcpy(&addr, object, sizeof(addr));  // avoids UB, respects readability check
        _MESSAGE("pdb3");
        if (!addr)
            return {};
        _MESSAGE("pdb4");
        std::string sym = GetSymbol(addr, process);
        if (sym.empty())
            return {};
        _MESSAGE("pdb5");
        // "Name+0x1234" -> "Name"
        if (auto plusPos = sym.find('+'); plusPos != std::string::npos)
            sym = sym.substr(0, plusPos);
        _MESSAGE("pdb6");
        // "Foo::Bar::`vftable'" -> "Foo::Bar"
        if (auto vftPos = sym.find("::`vftable'"); vftPos != std::string::npos)
            sym = sym.substr(0, vftPos);
        _MESSAGE("pdb7");
        return sym;
    }


    std::string GetClassNameFromRTTIorPDB(void* object, HANDLE process) noexcept
    {
        if (!object || !process)
            return {};

        if (auto rtti = GetClassNameFromRTTI(object, process); !rtti.empty())
            return rtti;

        return GetClassNameFromPDB(object, process);
    }
}
