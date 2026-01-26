

#include <windows.h>
#include <Psapi.h>
#include <DbgHelp.h>

#include <cstdio>
#include <format>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <fstream>
#include <chrono>

#include "../CrashLogger/SharedMem.h"
#include "../CrashLogger/PDB.h"
#include "../CrashLogger/CrashLoggerAPI.h"
#include "../CrashLogger/MemoryProbe.hpp"
#include "../CrashLogger/StackAPI.hpp"

#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "DbgHelp.lib")

//#pragma comment(linker, "/SUBSYSTEM:WINDOWS")

HANDLE hOblivionProcess = nullptr;

uint32_t GetDepthForCaller(const std::string& callerSymbol)
{
	if (callerSymbol.find("Buffer") != std::string::npos ||
		callerSymbol.find("Array") != std::string::npos ||
		callerSymbol.find("Insert") != std::string::npos)
	{
		return 0;
	}

	if (callerSymbol.find("NiGeometryData_ReadBinary") != std::string::npos)
	{
		return 0;
	}

	if (callerSymbol.find("new") != std::string::npos ||
		callerSymbol.find("CreateCopy") != std::string::npos ||
		callerSymbol.find("ReadBinary") != std::string::npos)
	{
		return 1;
	}

	if (callerSymbol.find("Extra") != std::string::npos)
	{
		return 2;
	}

	return 2;
}

std::string GetModuleFromAddress(uint32_t address, HANDLE hProcess)
{
    HMODULE hMods[1024];
    DWORD cbNeeded;

    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
    {
        for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
        {
            MODULEINFO modInfo;
            if (GetModuleInformation(hProcess, hMods[i], &modInfo, sizeof(modInfo)))
            {
                uintptr_t base = reinterpret_cast<uintptr_t>(modInfo.lpBaseOfDll);
                uintptr_t end = base + modInfo.SizeOfImage;

                if (address >= base && address < end)
                {
                    char modName[MAX_PATH];
                    if (GetModuleBaseNameA(hProcess, hMods[i], modName, sizeof(modName)))
                    {
                        return std::format("{}+0x{:X}", modName, address - base);
                    }
                }
            }
        }
    }
    return std::format("0x{:08X}", address);
}

std::string SafeGetSymbol(uint32_t addr, HANDLE hProcess, bool tryFullSymbols)
{
    std::string moduleInfo = GetModuleFromAddress(addr, hProcess);

    if (tryFullSymbols)
    {
        std::string sym = CrashLogger::PDB::GetSymbol(addr, hProcess);
        if (!sym.empty())
        {
            return sym;
        }
    }

    return moduleInfo;
}




CrashLogger::ResolveResult ResolveObjectSafe(void* object)
{
    CrashLogger::ResolveResult out;

    if (!object)
        return out;

    if (!CrashLogger::Memory::IsReadable(object, sizeof(void*)))
        return out;

    return CrashLogger::ResolveObject(object, hOblivionProcess);
}

// Inner implementation – keep your real logic here.
static std::string SafeGetLineForObjectRemoteImpl(void* ptr, HANDLE hProcess)
{
    if (!ptr) return "<null>";

    MEMORY_BASIC_INFORMATION mbi{};
    if (!VirtualQueryEx(hProcess, ptr, &mbi, sizeof(mbi)))
    {
        DWORD err = GetLastError();
        return std::format("<VQEx fail: {}>", err);
    }

    if (mbi.State != MEM_COMMIT)
        return std::format("<not commit: {}>", mbi.State);

    unsigned char buffer[64] = {};
    SIZE_T bytesRead = 0;
    if (!ReadProcessMemory(hProcess, ptr, buffer, sizeof(buffer), &bytesRead))
        return "<RPM fail>";

    if (bytesRead < sizeof(void*))
        return "<too small>";

    void* vtablePtr = *reinterpret_cast<void**>(buffer);

    if (!vtablePtr)
        return "<zeroed>";

    // Try to treat first dword as pointer / vtable
    if (!VirtualQueryEx(hProcess, vtablePtr, &mbi, sizeof(mbi)))
    {
        // --- non-vtable heuristics ---

        // String-ish?
        bool isPrintable = true;
        int printableCount = 0;
        for (size_t i = 0; i < bytesRead && buffer[i] != 0; ++i)
        {
            if (buffer[i] >= 32 && buffer[i] <= 126)
                printableCount++;
            else if (buffer[i] != 0 && buffer[i] != '\r' && buffer[i] != '\n' && buffer[i] != '\t')
            {
                isPrintable = false;
                break;
            }
        }

        if (isPrintable && printableCount > 3)
            return "<string data>";

        // All zeros?
        bool allZeros = true;
        for (size_t i = 0; i < bytesRead; ++i)
        {
            if (buffer[i] != 0)
            {
                allZeros = false;
                break;
            }
        }
        if (allZeros)
            return "<zero buffer>";

        // Mostly small numbers?
        int smallValueCount = 0;
        for (size_t i = 0; i < bytesRead; ++i)
        {
            if (buffer[i] < 16)
                smallValueCount++;
        }
        if (smallValueCount > bytesRead * 0.8)
            return "<numeric array>";

        return "<raw buffer>";
    }

    std::string targetModule = GetModuleFromAddress((uint32_t)vtablePtr, hProcess);
    bool isExecutable = (mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE));
    bool isReadOnly = (mbi.Protect & PAGE_READONLY);

    if (!isExecutable && !isReadOnly)
    {
        if (targetModule.find("Oblivion.exe") != std::string::npos)
            return std::format("<ptr->heap @{}>", targetModule);
        return "<struct/pointer>";
    }

    std::string sym = SafeGetSymbol((uint32_t)vtablePtr, hProcess, true);

    size_t vftablePos = sym.find("::`vftable");
    if (vftablePos != std::string::npos)
        return sym.substr(0, vftablePos);

    size_t rttiPos = sym.find("::`RTTI");
    if (rttiPos != std::string::npos)
        return sym.substr(0, rttiPos);

    if (sym.find("string") != std::string::npos || sym.find("str") != std::string::npos)
        return std::format("<const str @{}>", targetModule);

    if (!sym.empty() && sym.rfind("0x", 0) != 0) // doesn't start with "0x"
    {
        if (sym.length() > 30)
            sym = sym.substr(0, 27) + "...";
        return std::format("<ro->{}", sym);
    }

    if (targetModule.empty())
        return "<no module>";

    if (targetModule.find("Oblivion.exe") == std::string::npos)
    {
        auto plusPos = targetModule.find('+');
        std::string base = (plusPos == std::string::npos)
            ? targetModule
            : targetModule.substr(0, plusPos);
        return std::format("<external @{}>", base);
    }

    return std::format("<data @{}>", targetModule);
}

std::string SafeGetLineForObjectRemote(void* ptr, HANDLE hProcess)
{
    std::string res = SafeGetLineForObjectRemoteImpl(ptr, hProcess);

    if (res.empty())
    {
        res = "<unclassified>";
    }

    return res;
}

LONG WINAPI HelperUnhandledExceptionFilter(EXCEPTION_POINTERS* exceptionInfo)
{
    //LOG("\n\n=== UNHANDLED EXCEPTION ===\n");
    //LOG("Exception Code: 0x%08X\n", exceptionInfo->ExceptionRecord->ExceptionCode);
    //LOG("Exception Address: 0x%p\n", exceptionInfo->ExceptionRecord->ExceptionAddress);
    //LOG("Press Enter to exit...\n");
    return EXCEPTION_EXECUTE_HANDLER;
}

void MonitorLoop(std::ofstream& out, HANDLE hProcess, bool symbolsAvailable)
{
    // Track active allocations and stats
    struct AllocInfo
    {
        uint32_t caller;
        uint32_t size;
        std::string objectType;
    };

    struct LiveAlloc
    {
        void* ptr;
        uint32_t size;
        uint32_t caller;
        std::string objectType;
    };

    std::unordered_map<void*, AllocInfo> activeAllocs;
    std::unordered_map<uint32_t, uint32_t> churnCounts;
    std::vector<LiveAlloc> liveAllocs;
    liveAllocs.reserve(activeAllocs.size());

    for (auto& [ptr, info] : activeAllocs)
    {
        liveAllocs.push_back({ ptr, info.size, info.caller });
    }

    std::sort(liveAllocs.begin(), liveAllocs.end(),
        [](const LiveAlloc& a, const LiveAlloc& b) {
            return a.size > b.size;
        });

    uint64_t totalActiveBytes = 0;
    uint32_t lastDumpTime = GetTickCount64();
    uint64_t eventsProcessed = 0;

    //LOG("\nMonitoring loop started...\n");

    for (;;)
    {

        bool sawExit = false;
        DWORD exitSeenAt = 0;

        DWORD exitCode = 0;
        if (!GetExitCodeProcess(hProcess, &exitCode) || exitCode != STILL_ACTIVE)
        {
            sawExit = true;
        }

        uint32_t read = g_shm->header.readIdx.load(std::memory_order_acquire);
        uint32_t write = g_shm->header.writeIdx.load(std::memory_order_acquire);

        if (sawExit)
        {
            g_shm->header.readIdx.store(read, std::memory_order_release);
            break;
        }

        if (read < write)
        {
            LOG("Processing events: %u -> %u (%u events)\n", read, write, write - read);
        }

        int resolved = 0;
        for (auto& [ptr, info] : activeAllocs)
        {
            if (!info.objectType.empty())
                continue;

            LOG("attempting to resolve");
            auto res = CrashLogger::ResolveObject(ptr, hProcess);
            LOG("resolve returned");
            if (!res.name.empty())
            {
                info.objectType = res.name;
                LOG("Resolved %s", info.objectType.c_str());
                ++resolved;
            }

            if (resolved >= 1000) // safety cap if you want one
                break;
        }

        if (resolved > 0)
        {
            LOG("Resolved %d object types before final dump", resolved);
        }

        while (read < write)
        {

            uint32_t slot = read % SHM_CAPACITY;
            const AllocEvent& ev = g_shm->events[slot];

            eventsProcessed++;

            if ((ev.sizeAndType & AE_TYPE_FREE) == 0) // Allocation
            {
                if (ev.ptr != 0)
                {
                    uint32_t size = (ev.sizeAndType & AE_SIZE_MASK);
                    activeAllocs[(void*)(uintptr_t)ev.ptr] = { ev.caller, size };
                    totalActiveBytes += size;
                    churnCounts[ev.caller]++;
                }
            }
            else // Free
            {
                if (ev.ptr != 0)
                {
                    auto it = activeAllocs.find((void*)(uintptr_t)ev.ptr);
                    if (it != activeAllocs.end())
                    {
                        totalActiveBytes -= it->second.size;
                        activeAllocs.erase(it);
                    }
                }
            }

            ++read;
        }

        g_shm->header.readIdx.store(read, std::memory_order_release);

        // Dump summary every 5 seconds
        uint32_t now = GetTickCount64();
        if (now - lastDumpTime >= 5000 && eventsProcessed > 0)
        {
            //LOG("Writing periodic dump... (events processed: %llu)\n", eventsProcessed);

            out.seekp(0);
            out.clear();

            out << "===== OBLIVION MEMORY PROFILE DUMP =====\n";
            out << std::format("Events Processed: {}\n", eventsProcessed);
            out << std::format("Active Allocations: {}\n", activeAllocs.size());
            out << std::format("Total Bytes: {} ({:.2f} MB)\n",
                totalActiveBytes, totalActiveBytes / (1024.0 * 1024.0));

            out.flush();
            lastDumpTime = now;
            //LOG("Periodic dump written.\n");
        }

        Sleep(10);
    }

    // FINAL DUMP AFTER LOOP EXITS
    //LOG("Writing final dump... (events processed: %llu)\n", eventsProcessed);

    out.seekp(0);
    out.clear();

    // Aggregate stats by caller
    std::unordered_map<uint32_t, uint64_t> callerBytes;
    std::unordered_map<uint32_t, uint32_t> callerCounts;
    std::unordered_map<uint32_t, uint32_t> callerMaxSize;

    for (auto& [ptr, info] : activeAllocs)
    {
        callerBytes[info.caller] += info.size;
        callerCounts[info.caller] += 1;

        auto& maxSize = callerMaxSize[info.caller];
        if (info.size > maxSize)
            maxSize = info.size;
    }

    // Sort by total bytes
    struct CallerStat
    {
        uint32_t caller;
        uint64_t bytes;
        uint32_t count;
        uint32_t maxSize;
        void* samplePtr;
        std::string type;
    };

    std::vector<CallerStat> stats;
    stats.reserve(callerBytes.size());
    for (auto& [caller, bytes] : callerBytes)
    {
        void* samplePtr = nullptr;
        std::string type;
        for (auto& [ptr, info] : activeAllocs)
        {
            if (info.caller == caller)
            {
                samplePtr = ptr;
                type = info.objectType;
                break;
            }
        }
        stats.push_back({
            caller,
            bytes,
            callerCounts[caller],
            callerMaxSize[caller],
            samplePtr,
            type 
        });
    }

    std::sort(stats.begin(), stats.end(),
        [](const CallerStat& a, const CallerStat& b) {
            if (a.maxSize != b.maxSize)
                return a.maxSize > b.maxSize;
            return a.bytes > b.bytes;
        });
    // Write full dump
    out << "===== OBLIVION MEMORY PROFILE DUMP (FINAL) =====\n";
    out << std::format("Timestamp: {:%Y-%m-%d %H:%M:%S}\n",
        std::chrono::system_clock::now());
    out << std::format("Events Processed: {}\n", eventsProcessed);
    out << std::format("Active Allocations: {}\n", activeAllocs.size());
    out << "Profiling Active: YES\n\n";

    // Active allocations by caller (top 20)
    out << "===== Active Allocations by Caller (Top 20) =====\n";
    out << std::format("{:<50} {:>12} {:>15} {:>10} {}\n",
        "Function", "Count", "Bytes", "MaxSize", "Object");
    out << "--------------------------------------------------------------------------------\n";
    size_t displayCount = (std::min)(stats.size(), size_t(20));
    for (size_t i = 0; i < displayCount; ++i)
    {
        LOG("active alloc resolve");
        std::string sym = SafeGetSymbol(stats[i].caller, hOblivionProcess, symbolsAvailable);
        std::string sampleObject = stats[i].type;
        if (sampleObject.empty())
            sampleObject = SafeGetLineForObjectRemote(stats[i].samplePtr, hOblivionProcess);

        out << std::format("{:<50} {:>12} {:>15} {:>10} {}\n",
            sym,
            stats[i].count,
            stats[i].bytes,
            stats[i].maxSize,
            sampleObject);
    }
    if (stats.size() > 20)
    {
        out << std::format("... and {} more callers\n", stats.size() - 20);
    }

    out << "--------------------------------------------------------------------------------\n";
    out << "\n===== Largest Live Allocations (Top 50) =====\n";
    out << std::format("{:<10} {:>10} {:<50} {}\n",
        "Size", "Ptr", "Caller", "Object");
    out << "--------------------------------------------------------------------------------\n";

    liveAllocs.reserve(activeAllocs.size());

    for (auto& [ptr, info] : activeAllocs)
    {
        liveAllocs.push_back({ ptr, info.size, info.caller, info.objectType });
    }

    std::sort(liveAllocs.begin(), liveAllocs.end(),
        [](const LiveAlloc& a, const LiveAlloc& b) {
            return a.size > b.size;
        });

    size_t max = std::min<size_t>(50, liveAllocs.size());
    for (size_t i = 0; i < max; ++i)
    {
        auto& a = liveAllocs[i];
        std::string sym = SafeGetSymbol(a.caller, hProcess, symbolsAvailable);
        std::string obj = liveAllocs[i].objectType;

        out << std::format("{:<10} 0x{:08X} {:<50} {}\n",
            a.size,
            (uint32_t)(uintptr_t)a.ptr,
            sym,
            obj);
    }
    out << "--------------------------------------------------------------------------------\n";
    out << std::format("Total Active Allocations: {:>15} bytes ({:.2f} MB)\n",
        totalActiveBytes,
        totalActiveBytes / (1024.0 * 1024.0));
    // High-churn callers
    out << "\n===== High-Churn Callers - Frequent Allocators (Top 20) =====\n";
    {
        std::vector<std::pair<uint32_t, uint32_t>> churnVec;
        for (auto& [caller, count] : churnCounts)
        {
            if (count > 100)
            {
                churnVec.push_back({ caller, count });
            }
        }

        std::sort(churnVec.begin(), churnVec.end(),
            [](const auto& a, const auto& b) {
                return a.second > b.second;
            });

        if (!churnVec.empty())
        {
            size_t displayCount = (std::min)(churnVec.size(), size_t(20));
            for (size_t i = 0; i < displayCount; ++i)
            {
                auto& [caller, count] = churnVec[i];
                std::string sym = SafeGetSymbol(caller, hProcess, symbolsAvailable);
                out << std::format("{:<50} allocated {:>10} times\n",
                    sym, count);
            }
        }
    }

    // Allocations by module
    out << "\n===== Allocations by Module =====\n";
    {
        std::unordered_map<std::string, uint64_t> moduleBytes;
        std::unordered_map<std::string, uint32_t> moduleCounts;

        for (auto& [ptr, info] : activeAllocs)
        {
            std::string module = GetModuleFromAddress(info.caller, hProcess);
            size_t plusPos = module.find('+');
            if (plusPos != std::string::npos)
            {
                module = module.substr(0, plusPos);
            }

            moduleBytes[module] += info.size;
            moduleCounts[module] += 1;
        }

        std::vector<std::pair<std::string, uint64_t>> moduleVec;
        for (auto& [mod, bytes] : moduleBytes)
        {
            moduleVec.push_back({ mod, bytes });
        }
        std::sort(moduleVec.begin(), moduleVec.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });

        out << std::format("{:<40} {:>12} {:>15}\n", "Module", "Count", "Bytes");
        out << "--------------------------------------------------------------------\n";

        for (auto& [mod, bytes] : moduleVec)
        {
            out << std::format("{:<40} {:>12} {:>15}\n",
                mod, moduleCounts[mod], bytes);
        }
    }

    out << "\n\nGame process exited.\n";
    out.flush();

    //LOG("Final dump complete.\n");
}

void HideConsole()
{
    HWND hwnd = GetConsoleWindow();
    if (hwnd)
        ShowWindow(hwnd, SW_HIDE);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR lpCmdLine, int)
{

    SetUnhandledExceptionFilter(HelperUnhandledExceptionFilter);

    AllocConsole();
    //HideConsole();
    FILE* consoleOut;
    freopen_s(&consoleOut, "CONOUT$", "w", stdout);
    setvbuf(stdout, NULL, _IONBF, 0);

    char logPath[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, logPath);
    strcat_s(logPath, "\\CrashLoggerHelper.log");
    fopen_s(&logFile, logPath, "w");
    if (!logFile)
    {
        printf("ERROR: Failed to open log file!\n");
        return 1;
    }

    //LOG("Starting allocation dump monitor...\n");

    if (!InitSharedMemory(false))
    {
        //LOG("ERROR: Failed to initialize shared memory!\n");
        //LOG("Press Enter to exit...\n");
        return 1;
    }

    //LOG("After InitSharedMemory (returned=%d)\n", MB_OK);

    //LOG("Shared memory initialized successfully.\n");

    DWORD processes[1024], cbNeeded;
    if (EnumProcesses(processes, sizeof(processes), &cbNeeded))
    {
        DWORD numProcesses = cbNeeded / sizeof(DWORD);
        for (DWORD i = 0; i < numProcesses; i++)
        {
            if (processes[i] == 0) continue;

            HANDLE hProc = OpenProcess(
                PROCESS_VM_READ | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_QUERY_LIMITED_INFORMATION,
                FALSE,
                processes[i]
            );
            if (hProc)
            {
                char processName[MAX_PATH];
                if (GetModuleBaseNameA(hProc, nullptr, processName, sizeof(processName)))
                {
                    if (_stricmp(processName, "Oblivion.exe") == 0)
                    {
                        //LOG("Found Oblivion.exe with PID: %u\n", processes[i]);
                        hOblivionProcess = hProc;
                        break;
                    }
                }
                CloseHandle(hProc);
            }
        }
    }

    if (!hOblivionProcess)
    {
        //LOG("ERROR: Could not find Oblivion.exe process!\n");
        //LOG("Press Enter to exit...\n");
        return 1;
    }

    //LOG("Oblivion process handle: %p\n", hOblivionProcess);

    char workingDirectory[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, workingDirectory);
    //LOG("Working directory: %s\n", workingDirectory);

    bool symbolsAvailable = false;
    char symbolPath[MAX_PATH] = {};
    char altSymbolPath[MAX_PATH] = {};
    GetEnvironmentVariableA("_NT_SYMBOL_PATH", symbolPath, MAX_PATH);
    GetEnvironmentVariableA("_NT_ALTERNATE_SYMBOL_PATH", altSymbolPath, MAX_PATH);

    std::string lookPath = std::format("{};{}\\Data\\OBSE\\plugins;{};{}",
        workingDirectory, workingDirectory, symbolPath, altSymbolPath);

    symbolsAvailable = SymInitialize(hOblivionProcess, lookPath.c_str(), TRUE);
    //LOG("Symbol initialization: %s\n", symbolsAvailable ? "SUCCESS" : "FAILED");

    char filePath[MAX_PATH];
    sprintf_s(filePath, "%s\\AllocDump.txt", workingDirectory);
    //LOG("Output file: %s\n", filePath);

    std::ofstream out(filePath);
    if (!out.is_open())
    {
        //LOG("ERROR: Failed to open output file!\n");
        //LOG("Press Enter to exit...\n");
        CloseHandle(hOblivionProcess);
        return 1;
    }

    //LOG("Output file opened successfully.\n");

    out << "Waiting for allocations...\n";
    out.flush();

    MonitorLoop(out, hOblivionProcess, symbolsAvailable);

    //LOG("Exiting...\n");
    out.close();
    SymCleanup(hOblivionProcess);
    CloseHandle(hOblivionProcess);

    //LOG("Press Enter to exit...\n");
    return 0;
}