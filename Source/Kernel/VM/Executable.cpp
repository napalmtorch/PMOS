#include <Kernel/VM/Executable.hpp>
#include <Kernel/Core/Kernel.hpp>

namespace PMOS
{
    namespace VirtualMachine
    {
            bool Executable::LoadFromFile(char* FileName)
            {
                Kernel::CLI->Debug.Error("Executable loading from disk not yet supported");
                return false;
            }

            bool Executable::Create(ExecutableHeader header, byte* data)
            {
                if (data == nullptr) { return false; }
                Memory::Copy((void*)&Header, (void*)&header, sizeof(ExecutableHeader));
                Header.Data = data;
                return true;
            }

            uint Executable::GetCodeAddress() { return Header.CodeAddress; }
            
            uint Executable::GetDataAddress() { return Header.DataAddress; }
            
            uint Executable::GetStackAddress() { return Header.StackAddress; }
            
            uint Executable::GetCodeSize() { return Header.CodeSize; }
            
            uint Executable::GetDataSize() { return Header.DataSize; }
            
            uint Executable::GetStackSize() { return Header.StackSize; }

            uint Executable::GetRAMSize() { return Header.RAMSize; }

            ExecutableHeader Executable::GetHeader()
            {
                ExecutableHeader exec;
                exec.CodeAddress  = Header.CodeAddress;
                exec.DataAddress  = Header.DataAddress;
                exec.StackAddress = Header.StackAddress;
                exec.CodeSize     = Header.CodeSize;
                exec.DataSize     = Header.DataSize;
                exec.StackSize    = Header.StackSize;
                exec.RAMSize      = Header.RAMSize;
                exec.Type         = Header.Type;
                exec.Data         = Header.Data;
                return exec;
            }

            ExecutableType Executable::GetType() { return (ExecutableType)Header.Type; }

            byte* Executable::GetData() { return Header.Data; }
    }
}