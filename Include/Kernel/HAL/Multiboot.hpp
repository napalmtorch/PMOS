#pragma once
#include <Kernel/Lib/Types.hpp>

extern "C" 
{
    extern uint MultibootPointer;
}

namespace PMOS
{
    namespace HAL
    {
        typedef struct
        {
            uint Flags;
            uint MemoryLower;
            uint MemoryUpper;
            uint BootDevice;
            char* CommandLine;
            uint ModulesCount;
            uint ModulesAddress;
            uint Syms[4];
            uint MemoryMapLength;
            uint MemoryMapAddress;
            uint DrivesCount;
            uint DrivesAddress;
            uint ConfigTable;
            char* BootLoaderName;
            uint APMTable;
            uint VBEControlInfo;
            uint VBEModeInfo;
            uint VBEMode;
            uint VBEInterfaceSegment;
            uint VBEInterfaceOffset;
            uint VBEInterfaceLength;
        } ATTR_PACK MultibootHeader;

        typedef struct
        {
            
        } __attribute__((packed)) poop_t;
    }
}