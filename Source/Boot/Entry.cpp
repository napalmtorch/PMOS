#include <Boot/Entry.hpp>
#include <Kernel/Core/Kernel.hpp>

extc
{
    void KernelEntry()
    {
        // kernel boot sequence
        PMOS::Kernel::BootStage1();

        while (true);
        asm volatile("cli");
        asm volatile("hlt");
    }
}