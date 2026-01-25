#include "PDB.h"
#include <DbgHelp.h>
#include <windows.h>
#include <iostream>
#include <string>

#pragma comment(lib, "DbgHelp.lib")  

namespace CrashLogger::PDB
{
    std::string GetModule(UInt32 eip, HANDLE process)
    {
        IMAGEHLP_MODULE module = {};
        module.SizeOfStruct = sizeof(module);
        if (!SymGetModuleInfo(process, eip, &module)) return "";
        return module.ModuleName;
    }

    UInt32 GetModuleBase(UInt32 eip, HANDLE process)
    {
        IMAGEHLP_MODULE module = {};
        module.SizeOfStruct = sizeof(module);
        if (!SymGetModuleInfo(process, eip, &module)) return 0;
        return module.BaseOfImage;
    }

    std::string GetSymbol(UInt32 eip, HANDLE process)
    {
        char symbolBuffer[sizeof(SYMBOL_INFO) + 255];
        SYMBOL_INFO* symbol = reinterpret_cast<SYMBOL_INFO*>(symbolBuffer);
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = 254;

        DWORD64 offset = 0;
        if (!SymFromAddr(process, eip, &offset, symbol)) return "";

        // Format with hex offset, not decimal
        return std::format("{}+0x{:X}", symbol->Name, offset);
    }

    std::string GetLine(UInt32 eip, HANDLE process)
    {
        char lineBuffer[sizeof(IMAGEHLP_LINE) + 255];
        IMAGEHLP_LINE* line = reinterpret_cast<IMAGEHLP_LINE*>(lineBuffer);
        line->SizeOfStruct = sizeof(IMAGEHLP_LINE);

        DWORD offset = 0;
        if (!SymGetLineFromAddr(process, eip, &offset, line)) return "";
        return std::format("{}:{}", line->FileName, line->LineNumber);
    }

    bool IsReadable(const void* p, size_t size)
    {
        MEMORY_BASIC_INFORMATION mbi{};
        if (!VirtualQuery(p, &mbi, sizeof(mbi))) return false;
        if (mbi.State != MEM_COMMIT) return false;
        if (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD)) return false;
        return size <= mbi.RegionSize;
    }

    const char* GetObjectClassNameSafe(void* objBase)
    {
        if (!objBase) return "";
        if (!IsReadable(objBase, sizeof(void*))) return "";

        void** vtbl = *(void***)objBase;
        if (!vtbl) return "";
        if (!IsReadable(vtbl - 1, sizeof(void*))) return "";

        void* colPtr = vtbl[-1];
        if (!colPtr) return "";
        if (!IsReadable(colPtr, sizeof(RTTILocator))) return "";

        RTTILocator* rtti = static_cast<RTTILocator*>(colPtr);
        if (!rtti->type || !IsReadable(rtti->type, sizeof(RTTIType))) return "";

        RTTIType* type = rtti->type;
        if (!IsReadable(type->name, 4)) return "";
        if (type->name[0] != '.' || type->name[1] != '?') return "";

        for (UInt32 i = 0; i < MAX_PATH; i++)
        {
            if (!IsReadable(&type->name[i], 1)) return "";
            if (type->name[i] == '\0') return type->name;
        }
        return "";
    }

    std::string GetClassNameFromRTTI(void* object)
    {
        std::string name = GetObjectClassNameSafe(object);
        if (name.empty()) return "";

        char buffer[MAX_PATH] = {};
        UnDecorateSymbolName(name.c_str() + 1, buffer, MAX_PATH, UNDNAME_NO_ARGUMENTS);
        std::string undecorated = buffer;

        if (undecorated.size() > 6)
            return undecorated.substr(6);
        return undecorated;
    }

    std::string GetClassNameFromPDB(void* object, HANDLE hProcess)
    {
        if (!object) return "";

        // Check alignment
        if ((reinterpret_cast<uintptr_t>(object) & 0x3) != 0)
            return "";

        // Validate readable
        if (!IsReadable(object, sizeof(void*)))
            return "";

        std::string name;
        try
        {
            // Read the vtable pointer
            void* vtablePtr = *reinterpret_cast<void**>(object);
            if (!vtablePtr) return "";

            // Verify vtable is in read-only/executable memory
            MEMORY_BASIC_INFORMATION mbi{};
            if (!VirtualQuery(vtablePtr, &mbi, sizeof(mbi)))
                return "";

            if (!(mbi.Protect & (PAGE_EXECUTE_READ | PAGE_READONLY)))
                return "";

            // Get symbol for vtable address
            name = GetSymbol(reinterpret_cast<UInt32>(vtablePtr), hProcess);
        }
        catch (...)
        {
            return "";
        }

        // Look for vftable marker
        size_t pos = name.find("::`vftable'");
        if (pos == std::string::npos)
            return "";  // Not a vtable symbol

        return name.substr(0, pos);
    }

    std::string GetClassNameFromRTTIorPDB(void* object, HANDLE hProcess)
    {
        if (!object) return "";

        // Check alignment first
        if ((reinterpret_cast<uintptr_t>(object) & 0x3) != 0)
            return "";

        // Try RTTI first (more reliable)
        std::string str = GetClassNameFromRTTI(object);
        if (!str.empty())
        {
            // Additional validation: reject obvious garbage
            if (str.find('+') != std::string::npos)
                return "";
            if (str.find("Rtl") == 0 || str.find("Nt") == 0)
                return "";
            return str;
        }

        // Try PDB symbols
        str = GetClassNameFromPDB(object, hProcess);
        if (!str.empty())
        {
            // Reject function offsets
            if (str.find('+') != std::string::npos)
                return "";
            // Reject system functions
            if (str.find("Rtl") == 0 || str.find("Nt") == 0 ||
                str.find("BCrypt") == 0 || str.find("Ordinal") == 0)
                return "";
        }

        return str;
    }
}
