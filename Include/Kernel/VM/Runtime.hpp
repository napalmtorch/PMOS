#pragma once
#include <Kernel/Lib/Types.hpp>
#include <Kernel/HAL/Thread.hpp>
#include <Kernel/VM/BPU.hpp>
#include <Kernel/VM/RAM.hpp>

namespace PMOS
{
    namespace VirtualMachine
    {
        class RuntimeHost
        {
            public:
                BytecodeProcessor BPU;
                Threading::Thread* Thread;
                const char* Name;

            public:
                void Initialize(char* name);
                void LoadTestProgram();
                void LoadProgram(char* filename);
                void LoadProgram(byte* data, uint len);
                void Run();
                static void ThreadMain(Threading::Thread* t);
        };
    }
}