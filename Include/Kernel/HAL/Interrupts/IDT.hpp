#pragma once
#include <Kernel/Lib/Types.hpp>

extc
{
    // kernel code selector
    #define KERNEL_CS 0x08

    typedef struct
    {
        ushort LowOffset;
        ushort Selector;
        byte   AlwaysZero;
        byte   Flags;
        ushort HighOffset;
    } ATTR_PACK IDTGate;

    typedef struct
    {
        ushort Limit;
        uint   Base;
    } ATTR_PACK IDTRegister;

    void IDTSetGate(int n, uint handler);
    void IDTSet();
}