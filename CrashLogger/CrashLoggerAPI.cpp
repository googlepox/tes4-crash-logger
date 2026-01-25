#include "CrashLoggerAPI.h"
#include "CrashLogger.hpp"
#include <filesystem>
#include <format>
#include "IDebugLog.h"

namespace CrashLogger
{
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
        const void* object,
        std::string& labelName,
        std::string& string)
    {
        if (!object)
            return false;

        MEMORY_BASIC_INFORMATION mbi{};
        if (!VirtualQuery(object, &mbi, sizeof(mbi)))
            return false;

        // Must be readable committed memory
        if (mbi.State != MEM_COMMIT)
            return false;
        if (!(mbi.Protect & (PAGE_READONLY | PAGE_READWRITE)))
            return false;

        const char* cstr = static_cast<const char*>(object);

        // Don't read past this region
        const size_t maxReadable =
            (uintptr_t)mbi.BaseAddress + mbi.RegionSize - (uintptr_t)cstr;
        const size_t maxCheck = (std::min)(maxReadable, size_t(MAX_PATH));

        // Find actual string length and validate characters
        size_t actualLen = 0;
        for (size_t i = 0; i < maxCheck; i++)
        {
            char c = cstr[i];
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
        if (actualLen == 0 || actualLen >= maxCheck || actualLen < 3)
            return false;

        // Use actual length, not maxCheck
        std::string sanitized = SanitizeString(
            std::string(cstr, actualLen)
        );

        if (sanitized.size() < 3)
            return false;

        labelName = "String";
        string = sanitized;
        return true;
    }

    ResolveResult ResolveObject(void* object, HANDLE hProcess)
    {
        ResolveResult out;

        if (!object)
            return out;

        std::string label, name, desc;

        _MESSAGE("block 1");
        if (GetStringForClassLabel(object, label, name, desc, hProcess))
        {
            out.kind = ResolveKind::Label;
            out.label = label;
            out.name = name;
            out.description = desc;
            return out;
        }

        _MESSAGE("block 2");
        if (const auto rtti = PDB::GetClassNameFromRTTIorPDB(object, hProcess);
            !rtti.empty())
        {
            out.kind = ResolveKind::RTTI;
            out.label = "RTTI";
            out.name = rtti;
            return out;
        }

        _MESSAGE("block 3");
        if (GetAsString(object, label, desc))
        {
            out.kind = ResolveKind::String;
            out.label = label;
            out.description = desc;
            return out;
        }

        return out;
    }
}