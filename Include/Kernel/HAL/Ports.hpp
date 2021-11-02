#pragma once
#include <Kernel/Lib/Types.hpp>

namespace PMOS
{
    namespace HAL
    {
        namespace Ports
        {
            byte   Read8(ushort port);
            ushort Read16(ushort port);
            uint   Read32(ushort port);
            void   ReadString(ushort port, byte* data, uint size);      

            void Write8(ushort port, byte data);
            void Write16(ushort port, ushort data);
            void Write32(ushort port, uint data);
            void WriteString(ushort port, byte* data, uint size);
        }
    }
}