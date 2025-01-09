#include "memory_manager.hpp"
#include <new>

namespace {
    ChessMemoryManager* g_memoryManager = nullptr;
}

void SetGlobalMemoryManager(ChessMemoryManager* manager) {
    g_memoryManager = manager;
}

void* operator new(size_t size) {
    if (g_memoryManager) {
        return g_memoryManager->Allocate(size);
    }
    void* ptr = std::malloc(size);
    if (!ptr) throw std::bad_alloc();
    return ptr;
}

void* operator new[](size_t size) {
    if (g_memoryManager) {
        return g_memoryManager->Allocate(size);
    }
    void* ptr = std::malloc(size);
    if (!ptr) throw std::bad_alloc();
    return ptr;
}

void operator delete(void* ptr) noexcept {
    if (g_memoryManager) {
        g_memoryManager->Deallocate(ptr);
    } else {
        std::free(ptr);
    }
}

void operator delete[](void* ptr) noexcept {
    if (g_memoryManager) {
        g_memoryManager->Deallocate(ptr);
    } else {
        std::free(ptr);
    }
}

void operator delete(void* ptr, size_t) noexcept {
    if (g_memoryManager) {
        g_memoryManager->Deallocate(ptr);
    } else {
        std::free(ptr);
    }
}

void operator delete[](void* ptr, size_t) noexcept {
    if (g_memoryManager) {
        g_memoryManager->Deallocate(ptr);
    } else {
        std::free(ptr);
    }
}