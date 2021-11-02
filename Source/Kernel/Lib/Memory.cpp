#include <Kernel/Lib/Memory.hpp>
#include <Kernel/Core/Kernel.hpp>

// memory management overloads
void *operator new(size_t size) { return PMOS::Kernel::MemoryMgr.Allocate((uint)size); }
void *operator new[](size_t size) { return PMOS::Kernel::MemoryMgr.Allocate((uint)size); }
void operator delete(void *p) { PMOS::Kernel::MemoryMgr.Free(p); }
void operator delete(void *p, size_t size) { PMOS::Kernel::MemoryMgr.Free(p); UNUSED(size); }
void operator delete[](void *p) { PMOS::Kernel::MemoryMgr.Free(p); }
void operator delete[](void *p, size_t size) { PMOS::Kernel::MemoryMgr.Free(p); UNUSED(size); }