#include <windows.h>
#include "SharedMem.h"
#include <cstdio>

static constexpr size_t SHM_SIZE = sizeof(SharedMemBuffer);
static const char* SHM_NAME = "CrashLogger_MemEvents";

SharedMemBuffer* g_shm = nullptr;
bool g_shmReady = false;

static HANDLE g_hMap = nullptr;

HMODULE g_hThisDLL = nullptr;


FILE* logFile = nullptr;

bool InitSharedMemory(bool create)
{
    if (create)
    {
        g_hMap = CreateFileMappingA(
            INVALID_HANDLE_VALUE,
            nullptr,
            PAGE_READWRITE,
            0,
            SHM_SIZE,
            SHM_NAME
        );
    }
    else
    {
        g_hMap = OpenFileMappingA(
            FILE_MAP_ALL_ACCESS,
            FALSE,
            SHM_NAME
        );
    }

    if (!g_hMap)
        return false;

    g_shm = static_cast<SharedMemBuffer*>(
        MapViewOfFile(g_hMap, FILE_MAP_ALL_ACCESS, 0, 0, SHM_SIZE)
        );

    if (!g_shm)
    {
        CloseHandle(g_hMap);
        g_hMap = nullptr;
        return false;
    }

    if (create)
    {
        g_shm->header.magic = SHM_MAGIC;
        g_shm->header.writeIdx.store(0, std::memory_order_relaxed);
        g_shm->header.readIdx.store(0, std::memory_order_relaxed);
    }

    g_shmReady = true;
    return true;
}

void ShutdownSharedMemory()
{
    g_shmReady = false;

    if (g_shm)
    {
        UnmapViewOfFile(g_shm);
        g_shm = nullptr;
    }

    if (g_hMap)
    {
        CloseHandle(g_hMap);
        g_hMap = nullptr;
    }
}
