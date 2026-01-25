#pragma once
#include <cstdint>
#include <atomic>

constexpr uint32_t SHM_CAPACITY = 1 << 20; // number of events
constexpr uint32_t SHM_MAGIC = 0x4D454D43; // 'CMEM'

extern HMODULE g_hThisDLL;

struct AllocEvent
{
    void* ptr;
    uint32_t size;
    uint32_t caller;
    uint8_t type;
    uint8_t _pad[3];
    char objectType[64];
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