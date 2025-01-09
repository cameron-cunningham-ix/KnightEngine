#pragma once

#include <benchmark/benchmark.h>
#include <atomic>
#include <cstdlib>
#include <array>
#include <cstring>

class ChessMemoryManager : public benchmark::MemoryManager {
private:
    
    std::atomic<bool> isActive{false};
    std::atomic<int64_t> numAllocs{0};
    std::atomic<int64_t> currBytesUsed{0};
    std::atomic<int64_t> maxBytesUsed{0};
    std::atomic<int64_t> totalAllocatedBytes{0};
    std::atomic<int64_t> totalDeallocatedBytes{0};
    std::atomic<int64_t> netHeapGrowth{0};

public:
    ChessMemoryManager() = default;
    ~ChessMemoryManager() override = default;

    ChessMemoryManager(const ChessMemoryManager&) = delete;
    ChessMemoryManager& operator=(const ChessMemoryManager&) = delete;

    int64_t Reset() {
        if (!isActive) return 0;
        numAllocs = 0;
        currBytesUsed = 0;
        maxBytesUsed = 0;
        totalAllocatedBytes = 0;
        totalDeallocatedBytes = 0;
        netHeapGrowth = 0;
        return 0;
    }

    void Start() override {
        Reset();
        isActive = true;
    }

    void Stop(MemoryManager::Result& result) override {
        if (!isActive) return;
        result.num_allocs = numAllocs;
        result.max_bytes_used = maxBytesUsed;
        result.total_allocated_bytes = totalAllocatedBytes;
        result.net_heap_growth = totalAllocatedBytes - totalDeallocatedBytes;
        Reset();
        isActive = false;
    }

    void* Allocate(size_t size) {
        void* ptr = std::malloc(size);
        if (!ptr) throw std::bad_alloc();

        if (isActive) {
            numAllocs++;
            totalAllocatedBytes += size;
            currBytesUsed += size;
            if (currBytesUsed > maxBytesUsed) {
                maxBytesUsed = currBytesUsed.load();
            }
        }
        return ptr;
    }

    void Deallocate(void* ptr) {
        if (!ptr) return;

        if (isActive) {
            int64_t size = sizeof(ptr);
            currBytesUsed -= size;
            totalDeallocatedBytes += size;
        }
        std::free(ptr);
    }

    bool IsActive() const { return isActive; }
    int64_t getmaxBytesUsed() const { return maxBytesUsed; }
    int64_t gettotalAllocatedBytes() const { return totalAllocatedBytes; }
    int64_t getnumAllocs() const { return numAllocs; }
    int64_t getnetHeapGrowth() const { return netHeapGrowth; }
};