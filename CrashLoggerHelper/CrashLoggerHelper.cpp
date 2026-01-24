#include <windows.h>
#include "../CrashLogger/SharedMem.h"
#include <cstdio>
#include <format>

int WINAPI wWinMain(
    HINSTANCE hInstance,
    HINSTANCE,
    PWSTR lpCmdLine,
    int
)
{

    if (!InitSharedMemory(false))
        return 1;

    FILE* dumpFile = nullptr;

    char path[MAX_PATH];
    GetModuleFileNameA(nullptr, path, MAX_PATH);

    char* slash = strrchr(path, '\\');
    if (!slash)
        return 1;

    slash[1] = 0;
    strcat_s(path, "AllocDump.txt");

    fopen_s(&dumpFile, path, "w");
    if (!dumpFile)
        return 1;

    fprintf(dumpFile,
        "---- CrashLogger Allocation Dump ----\n"
        "Address              Size   Type\n"
        "-----------------------------------\n");
    fflush(dumpFile);

    for (;;)
    {

        uint32_t last = g_shm->header.alive.load();
        Sleep(1000);
        if (g_shm->header.alive.load() == last)
            exit(0);

        uint32_t read =
            g_shm->header.readIdx.load(std::memory_order_acquire);
        uint32_t write =
            g_shm->header.writeIdx.load(std::memory_order_acquire);

        MessageBoxA(nullptr, std::format("writeIdx = {}", g_shm->header.writeIdx.load()).c_str(), "Helper", MB_OK);

        while (read < write)
        {
            const AllocEvent& ev =
                g_shm->events[read % SHM_CAPACITY];

            fprintf(
                dumpFile,
                "0x%016llX %6u %5u\n",
                ev.caller,
                ev.size,
                ev.type
            );

            ++read;
        }

        g_shm->header.readIdx.store(read, std::memory_order_release);
        fflush(dumpFile);
        Sleep(1);
    }

    fclose(dumpFile);
    return 0;
}
