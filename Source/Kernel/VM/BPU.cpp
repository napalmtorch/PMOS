#include <Kernel/VM/BPU.hpp>
#include <Kernel/Core/Kernel.hpp>

namespace PMOS
{
    namespace VirtualMachine
    {
        void BPURegisters::Reset()
        {
            Memory::Set(Registers, 0, sizeof(Registers));
        }

        void BPURegisters::Write(RegisterID reg, uint data) { Write((byte)reg, data); }

        void BPURegisters::Write(byte reg, uint data)
        {   
            if (reg >= 11) { Kernel::Debug.Error("VM: Register write out of bounds"); return; }
            Registers[reg] = data;
        }

        uint BPURegisters::Read(RegisterID reg) { return Read((byte)reg); }

        uint BPURegisters::Read(byte reg)
        {
            if (reg >= 11) { Kernel::Debug.Error("VM: Register read out of bounds"); return 0; }
            return Registers[reg];
        }

        void BPURegisters::Add(RegisterID reg, uint data)
        {
            if ((byte)reg >= 11) { Kernel::Debug.Error("VM: Register add out of bounds"); return; }
            Registers[(byte)reg] += data;
        }

        void BPURegisters::Add(byte reg, uint data)
        {
            if (reg >= 11) { Kernel::Debug.Error("VM: Register read out of bounds"); return; }
            Registers[reg] += data;
        }

        void BPURegisters::Subtract(RegisterID reg, uint data)
        {
            if ((byte)reg >= 11) { Kernel::Debug.Error("VM: Register subtract out of bounds"); return; }
            Registers[(byte)reg] -= data;
        }

        void BPURegisters::Subtract(byte reg, uint data)
        {
            if (reg >= 11) { Kernel::Debug.Error("VM: Register read out of bounds"); return; }
            Registers[reg] -= data;
        }

        void BPURegisters::Multiply(RegisterID reg, uint data)
        {
            if ((byte)reg >= 11) { Kernel::Debug.Error("VM: Register multiply out of bounds"); return; }
            Registers[(byte)reg] *= data;
        }

        void BPURegisters::Multiply(byte reg, uint data)
        {
            if (reg >= 11) { Kernel::Debug.Error("VM: Register read out of bounds"); return; }
            Registers[reg] *= data;
        }

        void BPURegisters::Divide(RegisterID reg, uint data)
        {
            if ((byte)reg >= 11) { Kernel::Debug.Error("VM: Register divide out of bounds"); return; }
            Registers[(byte)reg] /= data;
        }
    
        void BPURegisters::Divide(byte reg, uint data)
        {
            if (reg >= 11) { Kernel::Debug.Error("VM: Register read out of bounds"); return; }
            Registers[reg] /= data;
        }

        // -------------------------------------------------------------------------------------------------------------------

        void BytecodeProcessor::Initialize()
        {
            Registers.Reset();
            Halted  = true;
            Kernel::CLI->Debug.Info("BPU INIT");
        }

        void BytecodeProcessor::Reset()
        {
            Registers.Reset();
            Registers.Write(RegisterID::PC, 0);
            
            StackReset();
            Halted = true;
        }

        void BytecodeProcessor::Halt() { Halted = true; }

        void BytecodeProcessor::Continue() { Halted = false; }

        bool BytecodeProcessor::IsHalted() { return Halted; }

        void BytecodeProcessor::Step()
        {
            if (!Halted)
            {
                if (Registers.Read(RegisterID::PC) >= RAM.Size) { Halt(); while (true); }
                byte op = RAM.Read8(Registers.Read(RegisterID::PC));
                Execute(op);
            }
        }

        void BytecodeProcessor::Execute(byte op)
        {

        }

        void BytecodeProcessor::IncrementPC(uint amount) { Registers.Add(RegisterID::PC, amount); }

        void BytecodeProcessor::DecrementPC(uint amount) { Registers.Subtract(RegisterID::PC, amount); }

        void BytecodeProcessor::StackReset()
        {
           
        }

        void BytecodeProcessor::StackPush(uint val)
        {
            
        }

        uint BytecodeProcessor::StackPop()
        {
            return 0;
        }

        uint BytecodeProcessor::StackPeek()
        {
            return 0;
        }
    }
}