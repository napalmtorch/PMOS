#include <Kernel/HAL/Ports.hpp>

extc byte inb(ushort port)
{
    byte result;
    asm volatile("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

extc ushort inw(ushort port)
{
    ushort result;
    asm volatile("in %%dx, %%ax" : "=a" (result) : "d" (port));
    return result;
}

extc uint inl(ushort port)
{
    uint rv;
    asm volatile("inl %%dx, %%eax" : "=a" (rv) : "dN" (port));
    return rv;
}

extc void insw(ushort port, byte* data, uint size)
{
    asm volatile("rep insw" : "+D" (data), "+c" (size) : "d" (port) : "memory");
}

extc void outb(ushort port, byte data)
{
    asm volatile("out %%al, %%dx" : : "a" (data), "d" (port));
}

extc void outw(ushort port, ushort data)
{
    asm volatile("out %%ax, %%dx" : : "a" (data), "d" (port)); 
}

extc void outl(ushort port, uint data)
{
    asm volatile("outl %%eax, %%dx" : : "dN" (port), "a" (data));
}

extc void outsw(ushort port, byte* data, uint size)
{
    asm volatile("rep outsw" : "+S" (data), "+c" (size) : "d" (port));
}

namespace PMOS
{
    namespace HAL
    {
        namespace Ports
        {
            byte Read8(ushort port) { return inb(port); }
            ushort Read16(ushort port) { return inw(port); }
            uint Read32(ushort port) { return inl(port); }
            void ReadString(ushort port, byte* data, uint size) { insw(port, data, size); }

            void Write8(ushort port, byte data) { outb(port, data); }
            void Write16(ushort port, ushort data) { outw(port, data); }
            void Write32(ushort port, uint data) { outl(port, data); }
            void WriteString(ushort port, byte* data, uint size) { outsw(port, data, size); }
        }
    }
}