#include <Kernel/Core/Kernel.hpp>

extc
{
    extern uint KernelStart;
    extern uint KernelEnd;
    uint KernelSize;
}

namespace PMOS
{
    namespace Kernel
    {
        HAL::MultibootHeader Multiboot;
        HAL::InterruptManager InterruptMgr;
        HAL::SerialController Serial;
        HAL::PITController PIT;
        HAL::RTCController RTC;
        HAL::PCIBusController PCI;
        HAL::CPUManager CPU;

        Services::ServiceManager ServiceMgr;
        Services::MemoryManager MemoryMgr;
        Services::TextModeTerminal* Terminal;
        Services::CommandLine* CLI;
        Threading::ThreadManager ThreadMgr;

        VFS::FSHost* FileSys;

        HAL::Drivers::VGAController* VGA;
        HAL::Drivers::VESAController* VESA;
        HAL::Drivers::PS2Keyboard* Keyboard;
        HAL::Drivers::PS2Mouse* Mouse;
        HAL::Drivers::ATAController* ATA;

        Threading::Thread* KernelThread;
        Threading::Thread* IdleThread;

        UI::XServer::XServerHost* XServer;
        UI::WindowManager* WinMgr;

        Debugger Debug;

        int UpdateTimer;

        void BootStage1()
        {
            Debug.SetMode(DebugMode::Serial);

            Serial = HAL::SerialController();
            Serial.SetPort(HAL::SerialPort::COM1);

            InterruptMgr = HAL::InterruptManager();
            InterruptMgr.Initialize();

            Multiboot = HAL::MultibootHeader();
            FetchMultiboot();

            MemoryMgr = Services::MemoryManager();
            MemoryMgr.Initialize();

            ServiceMgr = Services::ServiceManager();
            ServiceMgr.Initialize();

            //VGA = new HAL::Drivers::VGAController();
            //VGA->Initialize();
            //VGA->SetMode(HAL::VGAMode::Text80x25);
            VESA = new HAL::Drivers::VESAController();
            VESA->Initialize();
            VESA->SetMode(800, 600);

            Terminal = new Services::TextModeTerminal();
            Terminal->Initialize();
            Terminal->Clear();

            //Debug.SetMode(DebugMode::All);

            ThreadMgr = Threading::ThreadManager();
            ThreadMgr.Initialize();

            PIT = HAL::PITController();
            PIT.Initialize(5000, ThreadMgr.Schedule);

            RTC = HAL::RTCController();
            RTC.Initialize();

            SpawnKernelThread();
        }

        void BootStage2()
        {
            SpawnIdleThread();

            CPU = HAL::CPUManager();
            CPU.Detect();
            
            PCI.Initialize();
         
            Keyboard = new HAL::Drivers::PS2Keyboard();
            Keyboard->Initialize();

            Mouse = new HAL::Drivers::PS2Mouse();
            Mouse->Initialize();

            InterruptMgr.EnableInterrupts();
            Debug.Info("Enabled interrupts");

            CLI = new Services::CommandLine();
            CLI->Initialize();

            ATA = new HAL::Drivers::ATAController();
            ATA->Initialize();

            FileSys = new VFS::FSHost();
            FileSys->Initialize();

            Keyboard->Start();

            Terminal->Write("PurpleMoon", Col4::Magenta);
            Terminal->WriteLine(" OS");
            Terminal->WriteLine("Version 0.2", Col4::DarkGray);
        }

        void Run()
        {
            if (XServer != nullptr)
            {
                if (XServer->IsStarted()) { XServer->Update(); return; }
            }

            if (CLI != nullptr) { CLI->Execute(); }
        }

        void PITCallback(uint* regs)
        {
            PIT.CalculateMilliseconds();
            RTC.Update();
        }

        void ThreadCallback(Threading::Thread* t)
        {
            BootStage2();

            CLI->PrintCaret();

            while (true) 
            { 
                t->CalculateTPS();
                Run(); 
            }
        }

        void IdleThreadCallback(Threading::Thread* t)
        {
            while (true)
            {
                t->CalculateTPS();
            }
        }

        void FetchMultiboot()
        {
            HAL::MultibootHeader* mboot = (HAL::MultibootHeader*)0x10000;
            for (int i = 0; i < sizeof(HAL::MultibootHeader); i++) { ((byte*)&Multiboot)[i] = ((byte*)mboot)[i]; }
        }

        void SpawnKernelThread()
        {
            if (KernelThread != nullptr) { return; }
            KernelThread = ThreadMgr.Create("kernel", 2 * 1024 * 1024, ThreadPriority::Protected, ThreadCallback);
            KernelThread->Start();
        }

        void SpawnIdleThread()
        {
            if (IdleThread != nullptr) { return; }
            IdleThread = ThreadMgr.Create("idle", 8192, ThreadPriority::Protected, IdleThreadCallback);
            IdleThread->Start();
        }

        uint GetStartAddress() { return (uint)&KernelStart; }

        uint GetEndAddress() { return (uint)&KernelEnd; }

        uint GetSize() { return KernelSize; }
    }
}