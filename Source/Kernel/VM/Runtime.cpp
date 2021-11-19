#include <Kernel/VM/Runtime.hpp>
#include <Kernel/Core/Kernel.hpp>

namespace PMOS
{
    namespace VirtualMachine
    {
        RuntimeHost* CurrentRuntime;

        void RuntimeHost::Initialize(char* name)
        {
            Name = name;
            BPU.Initialize();
            BPU.RAM.Initialize(512 * 1024);
            Thread = Kernel::ThreadMgr.Create(name, 512 * 1024, ThreadPriority::Protected, ThreadMain);
        }

        void RuntimeHost::LoadTestProgram()
        {
            byte prog[] = 
            {
                0x01, 0x00, 0xDE, 0xAD, 0xCA, 0xFE,
                0x00,
                0x00,
                0xFF,
            };
            LoadProgram(prog, sizeof(prog));
        }
        
        void RuntimeHost::LoadProgram(char* filename)
        {
            if (!Kernel::FileSys->IOFileExists(filename)) { Kernel::CLI->Debug.Error("Unable to locate file '%s'", filename); return; }
            VFS::FileEntry file = Kernel::FileSys->IOOpenFile(filename);
            LoadProgram(file.Data, file.Size);
            Kernel::CLI->Debug.Info("Loaded program '%s'", filename);
            MemFree(file.Data);
        }

        void RuntimeHost::LoadProgram(byte* data, uint len)
        {
            Memory::Copy(BPU.RAM.Data, data, len);
        }
        
        void RuntimeHost::Run()
        {
            CurrentRuntime = this;
            Thread->Start();
        }

        void RuntimeHost::ThreadMain(Threading::Thread* t)
        {
            RuntimeHost* runtime = CurrentRuntime;
            if (runtime == nullptr) { return; }
            runtime->BPU.Continue();

            int time, last_time, tick;
            while (true)
            {
                t->CalculateTPS();

                time = Kernel::PIT.GetTotalMilliseconds();
                if (time != last_time)
                {
                    tick++;
                    last_time = time;
                }

                if (tick >= 500)
                {
                    runtime->BPU.Step();
                    tick = 0;
                }

                if (!runtime->BPU.IsHalted()) { return; }
            }
        }
    }
}