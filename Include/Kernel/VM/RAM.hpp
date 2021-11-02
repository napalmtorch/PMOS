#pragma once
#include <Kernel/Lib/Types.hpp>

namespace PMOS
{
    namespace VirtualMachine
    {
        class RAMController
        {
            public:
                byte* Data;
                uint  Size;

            public:
                void Initialize(uint size);
                void Write8(uint addr, byte data);
                void Write16(uint addr, ushort data);
                void Write32(uint addr, uint data);
                void WriteString(uint addr, char* str);

            public:
                byte   Read8(uint addr);
                ushort Read16(uint addr);
                uint   Read32(uint addr);
                char*  ReadString(uint addr);
        };
    }
}