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

    struct TypeDescriptor {
        void* pVFTable;   // 0x00
        void* spare;      // 0x04
        char name[1];     // 0x08
    };

    // Unified function: local fast path + remote safe path
    std::string GetObjectClassNameImpl(void* objBase, HANDLE process) 
    {
        if (!objBase)
            return {};

        if (logFile)
        {
            //LOG("inside GetObjectClassNameImpl");
        }
        HANDLE effective = process ? process : GetCurrentProcess();
        if (effective == GetCurrentProcess())
        {

            std::string result = GetObjectClassNameInternal2(objBase, effective);
            const char* decorated = result.c_str();
            if (!decorated || decorated[0] == '\0')
                return {};

            // Basic sanity for RTTI strings
            if (decorated[0] != '.' || decorated[1] != '?')
                return {};

            return std::string(decorated);
        }
        if (logFile)
        {
            //LOG("asd");
        }

        auto addr = reinterpret_cast<std::uintptr_t>(objBase);
        if ((addr & 0x3) != 0)
            return {};
        if (logFile)
        {
            //LOG("fgh");
        }
        MEMORY_BASIC_INFORMATION mbi{};
        // 1) Ensure object itself is readable in remote process
        if (!VirtualQueryEx(effective, objBase, &mbi, sizeof(mbi)))
            return {};
        if (mbi.State != MEM_COMMIT)
            return {};
        if (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD))
            return {};
        if (mbi.RegionSize < sizeof(void*))
            return {};
        // 2) Read vtable pointer from remote object: [objBase] -> vtblRemote
        void* vtblRemote = nullptr;
        SIZE_T bytesRead = 0;
        if (!ReadProcessMemory(effective, objBase, &vtblRemote, sizeof(vtblRemote), &bytesRead) ||
            bytesRead != sizeof(vtblRemote) ||
            !vtblRemote)
        {
            return {};
        }
        if (!VirtualQueryEx(effective, vtblRemote, &mbi, sizeof(mbi)))
            return {};
        if (mbi.State != MEM_COMMIT)
            return {};
        if (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD))
            return {};
        void* locatorRemote = nullptr;
        void* locatorSlotRemote =
            static_cast<void*>(static_cast<std::uint8_t*>(vtblRemote) - sizeof(void*));
        if (!VirtualQueryEx(effective, locatorSlotRemote, &mbi, sizeof(mbi)))
            return {};
        if (mbi.State != MEM_COMMIT)
            return {};
        if (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD))
            return {};
        if (mbi.RegionSize < sizeof(void*))
            return {};

        bytesRead = 0;
        if (!ReadProcessMemory(effective, locatorSlotRemote,
            &locatorRemote, sizeof(locatorRemote), &bytesRead) ||
            bytesRead != sizeof(locatorRemote) ||
            !locatorRemote)
        {
            return {};
        }

        RTTILocator locator{};
        if (!VirtualQueryEx(effective, locatorRemote, &mbi, sizeof(mbi)))
            return {};
        if (mbi.State != MEM_COMMIT)
            return {};
        if (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD))
            return {};
        if (mbi.RegionSize < sizeof(locator))
            return {};

        bytesRead = 0;
        if (!ReadProcessMemory(effective, locatorRemote,
            &locator, sizeof(locator), &bytesRead) ||
            bytesRead != sizeof(locator))
        {
            return {};
        }

        if (!locator.type)
            return {};

        TypeDescriptor typePrefix{};
        if (!VirtualQueryEx(effective, locator.type, &mbi, sizeof(mbi)))
            return {};
        if (mbi.State != MEM_COMMIT)
            return {};
        if (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD))
            return {};
        if (mbi.RegionSize < sizeof(TypeDescriptor))
            return {};

        bytesRead = 0;
        if (!ReadProcessMemory(effective, locator.type,
            &typePrefix, sizeof(typePrefix), &bytesRead) ||
            bytesRead != sizeof(typePrefix))
        {
            return {};
        }
        constexpr SIZE_T nameOffset = offsetof(TypeDescriptor, name);
        std::uintptr_t nameRemoteAddr =
            reinterpret_cast<std::uintptr_t>(locator.type) + nameOffset;

        char nameBuf[MAX_PATH]{};

        if (!VirtualQueryEx(effective, reinterpret_cast<void*>(nameRemoteAddr), &mbi, sizeof(mbi)))
            return {};
        if (mbi.State != MEM_COMMIT)
            return {};
        if (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD))
            return {};

        SIZE_T toRead = (mbi.RegionSize < MAX_PATH) ? mbi.RegionSize : MAX_PATH;
        bytesRead = 0;

        if (!ReadProcessMemory(effective,
            reinterpret_cast<void*>(nameRemoteAddr),
            nameBuf,
            toRead,
            &bytesRead) ||
            bytesRead == 0)
        {
            return {};
        }

        nameBuf[MAX_PATH - 1] = '\0';
        if (nameBuf[0] != '.' || nameBuf[1] != '?')
            return {};

        size_t len = strnlen(nameBuf, MAX_PATH);
        if (len == 0 || len == MAX_PATH)
            return {};

        return std::string(nameBuf, len);
    }


    std::string GetClassNameFromRTTI(void* object, HANDLE process) noexcept
    {
        const char* emptyStr = "";

        std::string decorated;
        if (logFile)
        {
            //LOG("About to call function");
        }

        try
        {
            decorated = GetObjectClassNameImpl(object, process);
        }
        catch (...)
        {
        }

        if (decorated.empty())
            return {};

        if (decorated.size() < 2 || decorated[0] != '.' || decorated[1] != '?')
        {
            return {};
        }

        char tempBuffer[MAX_PATH]{};

        BOOL ok = UnDecorateSymbolName(
            decorated.c_str() + 1,
            tempBuffer,
            MAX_PATH - 1,
            UNDNAME_NO_ARGUMENTS
        );

        tempBuffer[MAX_PATH - 1] = '\0';

        if (!ok)
            return {};

        size_t safeLen = strnlen(tempBuffer, MAX_PATH);
        if (safeLen == MAX_PATH)
        {
            return {};
        }

        std::string undec = std::string(tempBuffer, safeLen);

        if (undec.size() > 200)
        {
            return {};
        }

        constexpr std::string_view classPrefix = "class ";

        if (undec.rfind(classPrefix, 0) == 0 &&
            undec.size() > classPrefix.size())
        {
            return undec.substr(classPrefix.size());
        }

        return undec;
    }


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

    std::string& GetClassNameGetSymbol(void* object, std::string& buffer, HANDLE hProcess)
    {
        buffer = GetSymbol(*((UInt32*)object), hProcess);
        return buffer;
    }

    std::string& GetClassNameFromPDBSEH(void* object, std::string& buffer, HANDLE hProcess)
        try { GetClassNameGetSymbol(object, buffer, hProcess); return buffer; }
    catch (...) { return buffer; }


    std::string GetClassNameFromPDB(void* object, HANDLE hProcess) noexcept
    {
        std::string name;
        GetClassNameFromPDBSEH(object, name, hProcess);
        return name.substr(0, name.find("::`vftable'"));
    }


    std::string GetClassNameFromRTTIorPDB(void* object, HANDLE process) noexcept
    {
        if (logFile)
        {
            //LOG("1");
        }
        if (!object || !process)
            return {};
        if (logFile)
        {
            //LOG("2");
        }
        if (auto rtti = GetClassNameFromRTTI(object, process); !rtti.empty())
            return rtti;

        if (logFile)
        {
            //LOG("3");
        }
        return GetClassNameFromPDB(object, process);
    }
}
