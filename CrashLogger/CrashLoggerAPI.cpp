#include "CrashLoggerAPI.h"
#include "CrashLogger.hpp"
#include <filesystem>
#include <format>
#include "SharedMem.h"

namespace CrashLogger
{
    bool IsReadablePage(DWORD protect)
    {
        if (protect & (PAGE_NOACCESS | PAGE_GUARD))
            return false;

        switch (protect & 0xFF)   // low bits are type, high bits are modifiers
        {
        case PAGE_READONLY:
        case PAGE_READWRITE:
        case PAGE_WRITECOPY:
        case PAGE_EXECUTE_READ:
        case PAGE_EXECUTE_READWRITE:
        case PAGE_EXECUTE_WRITECOPY:
            return true;
        default:
            return false;
        }
    }

    bool GetStringForClassLabel(
        void* object,
        std::string& labelName,
        std::string& objectName,
        std::string& description,
        HANDLE hProcess)
    {
        if (!object) return false;  // Add null check

        try
        {
            static bool filled = false;
            if (!filled)
            {
                Labels::FillLabels();
                filled = true;
            }
            for (const auto& label : Labels::Label::GetAll())
            {
                if (label && label->Satisfies(object))
                {
                    labelName = label->GetLabelName();
                    objectName = label->GetName(object, hProcess);
                    description = label->GetDescription(object);
                    return true;
                }
            }
        }
        catch (...) {}
        return false;
    }

    bool GetAsString(
        HANDLE process,
        const void* remotePtr,
        std::string& labelName,
        std::string& outString)
    {
        if (!remotePtr || !process)
            return false;

        MEMORY_BASIC_INFORMATION mbi{};
        if (!VirtualQueryEx(process, remotePtr, &mbi, sizeof(mbi)))
            return false;

        // Must be committed, non-guard, readable page
        if (mbi.State != MEM_COMMIT)
            return false;

        if (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD))
            return false;

        // Accept typical readable protections
        const DWORD prot = (mbi.Protect & 0xFF);
        switch (prot)
        {
        case PAGE_READONLY:
        case PAGE_READWRITE:
        case PAGE_WRITECOPY:
        case PAGE_EXECUTE_READ:
        case PAGE_EXECUTE_READWRITE:
        case PAGE_EXECUTE_WRITECOPY:
            break;
        default:
            return false;
        }

        const char* remoteStr = static_cast<const char*>(remotePtr);

        // Calculate how many bytes we *could* safely read from this region
        const size_t maxReadable =
            (uintptr_t)mbi.BaseAddress + mbi.RegionSize - (uintptr_t)remoteStr;
        const size_t maxCheck = (std::min)(maxReadable, size_t(MAX_PATH));

        if (maxCheck == 0)
            return false;

        // Read that chunk into a local buffer
        char buffer[MAX_PATH]{};
        SIZE_T bytesRead = 0;
        if (!ReadProcessMemory(process, remoteStr, buffer, maxCheck, &bytesRead) || bytesRead == 0)
            return false;

        // Now we validate and find the actual length *inside our buffer*
        size_t actualLen = 0;
        for (size_t i = 0; i < bytesRead; i++)
        {
            char c = buffer[i];
            if (c == '\0')
            {
                actualLen = i;
                break;
            }

            // Allow printable ASCII plus common whitespace
            if (c < 0x20 || c > 0x7E)
            {
                if (c != '\t' && c != '\n' && c != '\r')
                    return false;
            }
        }

        // Must have found null terminator and be reasonable length
        if (actualLen == 0 || actualLen >= bytesRead || actualLen < 3)
            return false;

        std::string sanitized = SanitizeString(std::string(buffer, actualLen));
        if (sanitized.size() < 3)
            return false;

        labelName = "String";
        outString = std::move(sanitized);
        return true;
    }


    ResolveResult ResolveObject(void** object, HANDLE hProcess)
    {
        ResolveResult out;

        if (!object)
            return out;

        std::string label, name, desc;
        //LOG("GetStringForClassLabel");
        if (GetStringForClassLabel(object, label, name, desc, hProcess))
        {
            out.kind = ResolveKind::Label;
            out.label = label;
            out.name = name;
            out.description = desc;
            return out;
        }

        //LOG("GetClassNameFromRTTIorPDB");
        if (const auto rtti = PDB::GetClassNameFromRTTIorPDB(object, hProcess);
            !rtti.empty())
        {
            out.kind = ResolveKind::RTTI;
            out.label = "RTTI";
            out.name = rtti;
            return out;
        }

        //LOG("GetAsString");
        if (GetAsString(hProcess, object, label, desc))
        {
            out.kind = ResolveKind::String;
            out.label = label;
            out.description = desc;
            return out;
        }

        return out;
    }
}