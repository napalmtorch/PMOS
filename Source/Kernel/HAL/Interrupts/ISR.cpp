#include <Kernel/HAL/Interrupts/ISR.hpp>
#include <Kernel/Core/Kernel.hpp>

extc
{
    ISR InterruptHandlers[256];

    // exception messages
    const char* ExceptionMessages[] = 
    {
        "Division By Zero",
        "Debug",
        "Non Maskable Interrupt",
        "Breakpoint",
        "Into Detected Overflow",
        "Out of Bounds",
        "Invalid Opcode",
        "No Coprocessor",

        "Double Fault",
        "Coprocessor Segment Overrun",
        "Bad TSS",
        "Segment Not Present",
        "Stack Fault",
        "General Protection Fault",
        "Page Fault",
        "Unknown Interrupt",

        "Coprocessor Fault",
        "Alignment Check",
        "Machine Check",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",

        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved"
    };

    void ISRInitialize()
    {
        IDTSetGate(0, (uint)isr0);
        IDTSetGate(1, (uint)isr1);
        IDTSetGate(2, (uint)isr2);
        IDTSetGate(3, (uint)isr3);
        IDTSetGate(4, (uint)isr4);
        IDTSetGate(5, (uint)isr5);
        IDTSetGate(6, (uint)isr6);
        IDTSetGate(7, (uint)isr7);
        IDTSetGate(8, (uint)isr8);
        IDTSetGate(9, (uint)isr9);
        IDTSetGate(10, (uint)isr10);
        IDTSetGate(11, (uint)isr11);
        IDTSetGate(12, (uint)isr12);
        IDTSetGate(13, (uint)isr13);
        IDTSetGate(14, (uint)isr14);
        IDTSetGate(15, (uint)isr15);
        IDTSetGate(16, (uint)isr16);
        IDTSetGate(17, (uint)isr17);
        IDTSetGate(18, (uint)isr18);
        IDTSetGate(19, (uint)isr19);
        IDTSetGate(20, (uint)isr20);
        IDTSetGate(21, (uint)isr21);
        IDTSetGate(22, (uint)isr22);
        IDTSetGate(23, (uint)isr23);
        IDTSetGate(24, (uint)isr24);
        IDTSetGate(25, (uint)isr25);
        IDTSetGate(26, (uint)isr26);
        IDTSetGate(27, (uint)isr27);
        IDTSetGate(28, (uint)isr28);
        IDTSetGate(29, (uint)isr29);
        IDTSetGate(30, (uint)isr30);
        IDTSetGate(31, (uint)isr31);

        // Remap the PIC
        PMOS::HAL::Ports::Write8(0x20, 0x11);
        PMOS::HAL::Ports::Write8(0xA0, 0x11);
        PMOS::HAL::Ports::Write8(0x21, 0x20);
        PMOS::HAL::Ports::Write8(0xA1, 0x28);
        PMOS::HAL::Ports::Write8(0x21, 0x04);
        PMOS::HAL::Ports::Write8(0xA1, 0x02);
        PMOS::HAL::Ports::Write8(0x21, 0x01);
        PMOS::HAL::Ports::Write8(0xA1, 0x01);
        PMOS::HAL::Ports::Write8(0x21, 0x0);
        PMOS::HAL::Ports::Write8(0xA1, 0x0); 

        // Install the IRQs
        IDTSetGate(32, (uint)irq0);
        IDTSetGate(33, (uint)irq1);
        IDTSetGate(34, (uint)irq2);
        IDTSetGate(35, (uint)irq3);
        IDTSetGate(36, (uint)irq4);
        IDTSetGate(37, (uint)irq5);
        IDTSetGate(38, (uint)irq6);
        IDTSetGate(39, (uint)irq7);
        IDTSetGate(40, (uint)irq8);
        IDTSetGate(41, (uint)irq9);
        IDTSetGate(42, (uint)irq10);
        IDTSetGate(43, (uint)irq11);
        IDTSetGate(44, (uint)irq12);
        IDTSetGate(45, (uint)irq13);
        IDTSetGate(46, (uint)irq14);
        IDTSetGate(47, (uint)irq15);

        // install the system call irq
        IDTSetGate(128, (uint)syscall);
        
        // load with asm
        IDTSet();
    }


    void ISRRegisterInterrupt(byte irq, ISR handler) { InterruptHandlers[irq] = handler; }

    void ISRUnregisterInterrupt(byte irq) { InterruptHandlers[irq] = nullptr; }

    void ISRHandler(uint* regs)
    {
        ISRRegs* r = (ISRRegs*)regs;

        /* THROW PANIC */
        if (r->Interrupt  < 0 || r->Interrupt >= 32) 
        { 
            char str[128];
            PMOS::StringUtil::FromHex(r->Interrupt, str, true, 4);
            PMOS::StringUtil::Append(str, " : Unknown exception");
            PMOS::Kernel::Debug.Panic(str, r);
        }
        else { PMOS::Kernel::Debug.Panic((char*)ExceptionMessages[r->Interrupt], r); }
    }

    uint IRQHandler(uint regs)
    {
        Registers32* r = (Registers32*)regs;

        if (InterruptHandlers[r->Interrupt] != 0) 
        {
            ISR handler = InterruptHandlers[r->Interrupt];
            handler(&regs);
        }

        if (r->Interrupt >= 40) { PMOS::HAL::Ports::Write8(0xA0, 0x20); }
        PMOS::HAL::Ports::Write8(0x20, 0x20);

        return regs;
    }

    uint SysCallHandler(uint regs)
    {
        return regs;
    }
}

namespace PMOS
{
    namespace HAL
    {
        void InterruptManager::Initialize()
        {
            DisableInterrupts();
            ISRInitialize();
        }

        void InterruptManager::Register(byte irq, ISR handler)
        {
            ISRRegisterInterrupt(irq, handler);
        }

        void InterruptManager::Unregister(byte irq)
        {
            ISRUnregisterInterrupt(irq);
        }

        // toggle interrupts
        void InterruptManager::EnableInterrupts()  { asm volatile("sti"); }
        void InterruptManager::DisableInterrupts() { asm volatile("cli"); }
}
}