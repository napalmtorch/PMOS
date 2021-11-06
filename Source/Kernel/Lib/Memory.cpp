#include <Kernel/Lib/Memory.hpp>
#include <Kernel/Core/Kernel.hpp>

// memory management overloads
void* operator new(size_t size) { return PMOS::Kernel::MemoryMgr.Allocate((uint)size); }
void* operator new[](size_t size) { return PMOS::Kernel::MemoryMgr.Allocate((uint)size); }
void operator delete(void *p) { PMOS::Kernel::MemoryMgr.Free(p); }
void operator delete(void *p, size_t size) { PMOS::Kernel::MemoryMgr.Free(p); UNUSED(size); }
void operator delete[](void *p) { PMOS::Kernel::MemoryMgr.Free(p); }
void operator delete[](void *p, size_t size) { PMOS::Kernel::MemoryMgr.Free(p); UNUSED(size); }

void* MemAlloc(size_t size)
{
    return PMOS::Kernel::MemoryMgr.Allocate(size);
}

void* MemAlloc(size_t size, bool clear, PMOS::AllocationType type)
{
    return PMOS::Kernel::MemoryMgr.Allocate(size, clear, type);
}

void MemFree(void* ptr)
{
    PMOS::Kernel::MemoryMgr.Free(ptr);
}

void MemFreeArray(void** arr, size_t len)
{
    if (arr == nullptr) { return; }
    for (size_t i = 0; i < len; i++) { if (arr[i] != nullptr) { PMOS::Kernel::MemoryMgr.Free(arr[i]); } }
    PMOS::Kernel::MemoryMgr.Free(arr);
}