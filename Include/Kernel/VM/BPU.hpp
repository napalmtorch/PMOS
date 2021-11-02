#pragma once
#include <Kernel/Lib/Types.hpp>
#include <Kernel/VM/Executable.hpp>
#include <Kernel/VM/RAM.hpp>

namespace PMOS
{
    namespace VirtualMachine
    {
        enum class RegisterID : byte
        {
            R0, R1, R2, R3, R4, R5, R6, R7,
            SP, PC, COND,
        };

        enum class ConditionFlags
        {
            Positive = 1 << 0,
            Zero     = 1 << 1, 
            Negative = 1 << 2, 
        };

        class BPURegisters
        {
            private:
                uint Registers[11];
                
            public:
                void Reset();
                void Write(RegisterID reg, uint data);
                void Write(byte reg, uint data);
                uint Read(RegisterID reg);
                uint Read(byte reg);

            public:
                void Add(RegisterID reg, uint data);
                void Add(byte reg, uint data);
                void Subtract(RegisterID reg, uint data);
                void Subtract(byte reg, uint data);
                void Multiply(RegisterID reg, uint data);
                void Multiply(byte reg, uint data);
                void Divide(RegisterID reg, uint data);
                void Divide(byte reg, uint data);
        };

        class BytecodeProcessor
        {
            public:
                BPURegisters Registers;
                ExecutableHeader ExecHeader;
                RAMController RAM;
                
            private:
                bool Halted;

            public:
                void Initialize();
                void Load(ExecutableHeader header);

            public:
                void Reset();
                void Halt();
                void Continue();

            public:
                void Step();
                void Execute(byte op);

            public:
                void IncrementPC(uint amount);
                void DecrementPC(uint amount);

            public:
                void StackReset();
                void StackPush(uint val);
                uint StackPop();
                uint StackPeek();
        };
    }
}