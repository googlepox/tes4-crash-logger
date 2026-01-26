#pragma once
#include <string>
#include <cstdint>
#include <windows.h>

namespace CrashLogger
{

    enum class ResolveKind : uint8_t
    {
        None,
        Label,
        RTTI,
        String
    };

    struct ResolveResult
    {
        ResolveKind kind = ResolveKind::None;
        std::string label;
        std::string name;
        std::string description;

        bool Valid() const { return kind != ResolveKind::None; }
    };

    bool GetStringForClassLabel(
        void* object,
        std::string& labelName,
        std::string& objectName,
        std::string& description,
        HANDLE hProcess);
    bool GetAsString(
        HANDLE process,
        const void* remotePtr,
        std::string& labelName,
        std::string& outString);
    ResolveResult ResolveObject(void* object, HANDLE hProcess);
}