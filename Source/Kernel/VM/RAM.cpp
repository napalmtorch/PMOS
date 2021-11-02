#include <Kernel/VM/RAM.hpp>
#include <Kernel/Core/Kernel.hpp>

namespace PMOS
{
    namespace VirtualMachine
    {
        void RAMController::Initialize(uint size)
        {
            if (Data != nullptr) { Kernel::MemoryMgr.Free(Data); }
            Data = (byte*)Kernel::MemoryMgr.Allocate(size, true, AllocationType::VMRAM);
            Size = size;
        }

        void RAMController::Write8(uint addr, byte data)
        {
            if (addr >= Size) { Kernel::Debug.Error("VM: Memory write(8-bit) out of bounds"); return; }
            Data[addr] = data;
        }

        void RAMController::Write16(uint addr, ushort data)
        {
            if (addr + 1 >= Size) { Kernel::Debug.Error("VM: Memory write(16-bit) out of bounds"); return; }
            Data[addr + 0] = (byte)((data & 0xFF00) >> 8);
            Data[addr + 1] = (byte)((data & 0x00FF));
        }

        void RAMController::Write32(uint addr, uint data)
        {
            if (addr + 3 >= Size) { Kernel::Debug.Error("VM: Memory write(32-bit) out of bounds"); return; }
            Data[addr + 0] = (byte)((data & 0xFF000000) >> 24);
            Data[addr + 1] = (byte)((data & 0x00FF0000) >> 16);
            Data[addr + 2] = (byte)((data & 0x0000FF00) >> 8);
            Data[addr + 3] = (byte)((data & 0x000000FF));
        }

        void RAMController::WriteString(uint addr, char* str)
        {
            int pos = 0;
            while (str[pos] != 0)
            {
                Write8(addr + pos, str[pos]);
                pos++;
            }
        }

        byte RAMController::Read8(uint addr)
        {
            if (addr >= Size) { Kernel::Debug.Error("VM: Memory read(8-bit) out of bounds"); return 0; }
            return Data[addr];
        }

        ushort RAMController::Read16(uint addr)
        {
            if (addr + 1 >= Size) { Kernel::Debug.Error("VM: Memory read(16-bit) out of bounds"); return 0; }
            return (ushort)(((Data[addr] & 0xFF00) >> 8) | Data[addr + 1]);
        }

        uint RAMController::Read32(uint addr)
        {
            if (addr + 3 >= Size) { Kernel::Debug.Error("VM: Memory read(32-bit) out of bounds"); return 0; }
            return (uint)(((Data[addr] & 0xFF000000) >> 24) | ((Data[addr + 1] & 0x00FF0000) >> 16) | ((Data[addr + 2] & 0x0000FF00) >> 8) | (Data[addr + 3]));
        }

        char* RAMController::ReadString(uint addr)
        {
            int pos = 0;
            while (Data[addr + pos] != 0) { pos++; }
            char* output = (char*)Kernel::MemoryMgr.Allocate(pos + 1);
            pos = 0;
            while (Data[addr + pos] != 0) { output[pos] = Data[addr + pos]; pos++; }
            return output;
        }
    }
}