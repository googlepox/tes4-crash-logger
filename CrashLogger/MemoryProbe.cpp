#include <windows.h>
#include "MemoryProbe.hpp"

namespace CrashLogger::Memory
{
    bool IsReadable(const void* ptr, std::size_t size) noexcept
    {
        MEMORY_BASIC_INFORMATION mbi{};
        if (!VirtualQuery(ptr, &mbi, sizeof(mbi)))
            return false;

        if (mbi.State != MEM_COMMIT)
            return false;

        if (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD))
            return false;

        auto start = reinterpret_cast<std::uintptr_t>(ptr);
        auto end = start + size;
        auto regionEnd =
            reinterpret_cast<std::uintptr_t>(mbi.BaseAddress) + mbi.RegionSize;

        return end <= regionEnd;
    }

    bool ReadU32(const void* ptr, std::uint32_t& out) noexcept
    {
        __try
        {
            if (!IsReadable(ptr, sizeof(std::uint32_t)))
                return false;

            out = *reinterpret_cast<const std::uint32_t*>(ptr);
            return true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            return false;
        }
    }
}