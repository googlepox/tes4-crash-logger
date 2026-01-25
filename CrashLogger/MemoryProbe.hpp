#pragma once
#include <cstdint>
#include <cstddef>

namespace CrashLogger::Memory
{
    bool IsReadable(const void* ptr, std::size_t size) noexcept;

    bool ReadU32(const void* ptr, std::uint32_t& out) noexcept;
}