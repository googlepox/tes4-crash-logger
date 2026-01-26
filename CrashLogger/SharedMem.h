#pragma once
#include <cstdint>
#include <atomic>
#include <stdio.h>

constexpr uint32_t SHM_CAPACITY = 1 << 20; // number of events
constexpr uint32_t SHM_MAGIC = 0x4D454D43; // 'CMEM'

extern HMODULE g_hThisDLL;

constexpr uint32_t AE_TYPE_FREE = 0x80000000u;
constexpr uint32_t AE_SIZE_MASK = 0x7FFFFFFFu;

constexpr float HEAP_WARN = 0.90f;
constexpr float HEAP_CRITICAL = 0.94f;
constexpr float HEAP_PANIC = 0.97f;

extern FILE* logFile;

#define LOG(fmt, ...) do { \
    fprintf(logFile, fmt "\n", ##__VA_ARGS__); \
    fflush(logFile); \
    printf(fmt "\n", ##__VA_ARGS__); \
    fflush(stdout); \
} while(0)

struct AllocEvent
{
    void* ptr;
    uint32_t sizeAndType;
    uint32_t caller;
};

struct SharedMemHeader
{
    uint32_t magic;
    std::atomic<uint32_t> writeIdx;
    std::atomic<uint32_t> readIdx;
};

struct SharedMemBuffer
{
    SharedMemHeader header;
    AllocEvent events[SHM_CAPACITY];
};

extern SharedMemBuffer* g_shm;
extern bool g_shmReady;

// mode:
//  - true  = create (writer / game)
//  - false = open   (reader / helper)
bool InitSharedMemory(bool create);
void ShutdownSharedMemory();

// non-blocking, safe for hooks
inline void TryWriteShared(const AllocEvent& ev)
{
    if (!g_shmReady)
        return;

    uint32_t write = g_shm->header.writeIdx.load(std::memory_order_relaxed);
    uint32_t read = g_shm->header.readIdx.load(std::memory_order_acquire);

    if ((write - read) >= SHM_CAPACITY)
        return;

    g_shm->events[write % SHM_CAPACITY] = ev;
    g_shm->header.writeIdx.store(write + 1, std::memory_order_release);
}