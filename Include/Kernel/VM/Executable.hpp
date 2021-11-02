#pragma once
#include <Kernel/Lib/Types.hpp>

namespace PMOS
{
    namespace VirtualMachine
    {
        enum class ExecutableType
        {
            Raw,
            TerminalExecutable,
            UIExecutable,
        };

        typedef struct
        {
            uint CodeAddress;
            uint DataAddress;
            uint StackAddress;
            uint CodeSize;
            uint DataSize;
            uint StackSize;
            uint RAMSize;
            byte Type;
            byte* Data;
        } ATTR_PACK ExecutableHeader;

        class Executable
        {
            private:
                char* FileName;
                ExecutableHeader Header;

            public:
                bool LoadFromFile(char* FileName);
                bool Create(ExecutableHeader header, byte* data);

            public:
                uint GetCodeAddress();
                uint GetDataAddress();
                uint GetStackAddress();
                uint GetCodeSize();
                uint GetDataSize();
                uint GetStackSize();
                uint GetRAMSize();
                ExecutableHeader GetHeader();
                ExecutableType GetType();
                byte* GetData();
        };
    }
}