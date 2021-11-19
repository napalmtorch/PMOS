#pragma once
#include <Kernel/Lib/Types.hpp>
#include <Kernel/Lib/String.hpp>
#include <Kernel/Lib/Math.hpp>
#include <Kernel/Lib/Map.hpp>
#include <Kernel/Lib/Memory.hpp>
#include <Kernel/HAL/Ports.hpp>
#include <Kernel/HAL/Serial.hpp>
#include <Kernel/HAL/Multiboot.hpp>
#include <Kernel/HAL/PIT.hpp>
#include <Kernel/HAL/RTC.hpp>
#include <Kernel/HAL/CPU.hpp>
#include <Kernel/HAL/PCI.hpp>
#include <Kernel/HAL/Thread.hpp>
#include <Kernel/HAL/RealMode.hpp>
#include <Kernel/HAL/Interrupts/IDT.hpp>
#include <Kernel/HAL/Interrupts/ISR.hpp>
#include <Kernel/HAL/Drivers/Video/VGA.hpp>
#include <Kernel/HAL/Drivers/Video/VESA.hpp>
#include <Kernel/HAL/Drivers/Input/Key.hpp>
#include <Kernel/HAL/Drivers/Input/PS2Keyboard.hpp>
#include <Kernel/HAL/Drivers/Input/PS2Mouse.hpp>
#include <Kernel/HAL/Drivers/Storage/ATA.hpp>
#include <Kernel/Core/Service.hpp>
#include <Kernel/Core/Debug.hpp>
#include <Kernel/Services/MemoryMgr.hpp>
#include <Kernel/Services/ServiceMgr.hpp>
#include <Kernel/Services/ThreadMgr.hpp>
#include <Kernel/Services/Terminal.hpp>
#include <Kernel/Services/CommandLine.hpp>
#include <Kernel/Services/FileSystem.hpp>
#include <Kernel/UI/XServer/XServer.hpp>
#include <Kernel/UI/XServer/WindowMgr.hpp>
#include <Kernel/UI/Control.hpp>
#include <Kernel/UI/Button.hpp>
#include <Kernel/UI/Container.hpp>
#include <Kernel/UI/Window.hpp>
#include <Kernel/VM/BPU.hpp>
#include <Kernel/VM/Runtime.hpp>
#include <Kernel/Graphics/Bitmap.hpp>
#include <Kernel/Graphics/Graphics.hpp>
#include <Kernel/Graphics/Fonts.hpp>

extc { extern uint KernelSize; }

namespace PMOS
{
    namespace Kernel
    {
        // system hardware
        extern HAL::MultibootHeader Multiboot;
        extern HAL::InterruptManager InterruptMgr;
        extern HAL::SerialController Serial;
        extern HAL::PITController PIT;
        extern HAL::RTCController RTC;
        extern HAL::PCIBusController PCI;
        extern HAL::CPUManager CPU;

        // services
        extern Services::ServiceManager ServiceMgr;
        extern Services::MemoryManager MemoryMgr;
        extern Services::TextModeTerminal* Terminal;
        extern Services::CommandLine* CLI;
        extern Threading::ThreadManager ThreadMgr;
        extern VFS::FSHost* FileSys;

        // drivers
        extern HAL::Drivers::VGAController* VGA;
        extern HAL::Drivers::VESAController* VESA;
        extern HAL::Drivers::PS2Keyboard* Keyboard;
        extern HAL::Drivers::PS2Mouse* Mouse;
        extern HAL::Drivers::ATAController* ATA;

        // threads
        extern Threading::Thread* KernelThread;
        extern Threading::Thread* IdleThread;

        // ui
        extern UI::XServer::XServerHost* XServer;
        extern UI::WindowManager* WinMgr;

        // debugging
        extern Debugger Debug;

        // methods
        void BootStage1();
        void BootStage2();
        void Run();
        void PITCallback(uint* regs);
        void ThreadCallback(Threading::Thread* t);
        void IdleThreadCallback(Threading::Thread* t);
        void FetchMultiboot();
        void SpawnKernelThread();
        void SpawnIdleThread();

        // kernel addressing information
        uint GetStartAddress();
        uint GetEndAddress();
        uint GetSize();
    }
}