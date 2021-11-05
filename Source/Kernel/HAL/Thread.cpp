#include <Kernel/HAL/Thread.hpp>
#include <Kernel/Core/Kernel.hpp>

namespace PMOS
{
    namespace Threading
    {
        // default stack size
        const uint STACK_SIZE = 512 * 1024;

        // current thread id
        ulong ThreadID = 0x0000000000000000;

        // handle thread
        void ThreadEntry()
        {
            Kernel::ThreadMgr.CurrentThread->Protocol(Kernel::ThreadMgr.CurrentThread);
            Kernel::ThreadMgr.CurrentThread->SetState(ThreadState::Halted);
            Kernel::ThreadMgr.Unload(Kernel::ThreadMgr.CurrentThread);
            while (true);
        }

        // --------------------------------------------------------------------------------------------------

        // blank constructor
        Thread::Thread() { }

        // create thread with specified name, priority and protocol method
        Thread::Thread(char* name, ThreadPriority priority, void protocol(Thread*))
        {
            // set stack size
            StackSize = STACK_SIZE;

            // create stack
            Stack = (byte*)Kernel::MemoryMgr.Allocate(StackSize, true, AllocationType::ThreadStack);

            // set registers pointer
            Registers = (ISRRegs*)(((uint)Stack + StackSize) - sizeof(ISRRegs));

            // copy name
            for (uint i = 0; i < 64; i++) { if (i < String::Length(name)) { Properties.Name[i] = name[i]; } }
            
            // set properties
            Properties.Priority = priority;
            Properties.ID       = ThreadID++;
            Properties.State    = ThreadState::Initialized;

            // set protocol
            Protocol = protocol;

            // clear stack
            ClearStack();

            // set registers
            Registers->EIP    = (uint)ThreadEntry;
            Registers->CS     = KERNEL_CS;
            Registers->EFlags = 0x202;

            Kernel::Debug.Info("Created thread %s", Properties.Name);
        }

        // create thread with specified name, stack size, priority and protocol method
        Thread::Thread(char* name, uint stack, ThreadPriority priority, void protocol(Thread*))
        {
            // set stack size
            StackSize = stack;

            // create stack
            Stack = (byte*)Kernel::MemoryMgr.Allocate(StackSize, true, AllocationType::ThreadStack);

            // set registers pointer
            Registers = (ISRRegs*)(((uint)Stack + StackSize) - sizeof(ISRRegs));

            // copy name
            for (uint i = 0; i < 64; i++) { if (i < String::Length(name)) { Properties.Name[i] = name[i]; } }
            
            // set properties
            Properties.Priority = priority;
            Properties.ID       = ThreadID++;
            Properties.State    = ThreadState::Initialized;

            // set protocol
            Protocol = protocol;

            // clear stack
            ClearStack();

            // set registers
            Registers->EIP    = (uint)ThreadEntry;
            Registers->CS     = KERNEL_CS;
            Registers->EFlags = 0x202;
        }

        // dispose thread and contents
        void Thread::Dispose()
        {
            // free stack memory
            Kernel::MemoryMgr.Free(Stack);

            // free thread
            Kernel::MemoryMgr.Free(this);
        }

        // calculate ticks per second of thread
        void Thread::CalculateTPS()
        {
            TPSTick++;
            Time = Kernel::RTC.GetSecond();
            if (LastTime != Time)
            {
                TPS = TPSTick;
                TPSTick = 0;
                LastTime = Time;
            }
        }

        // start thread
        bool Thread::Start()
        {
            // validate state
            if (Properties.State != ThreadState::Initialized) { return false; }

            // disable interrupts
            asm volatile("cli");

            // load thread
            Kernel::ThreadMgr.Load(this);

            // set state
            Properties.State = ThreadState::Running;

            // message
            Kernel::Debug.Info("Started thread: NAME = %s ID = %d, STACK_SIZE = %d PRIORITY = 0x%2x", Properties.Name, Properties.ID, StackSize, (uint)Properties.Priority);

            // re-enable interrupts and return success
            asm volatile("sti");
            return true;
        }

        // stop thread
        bool Thread::Stop()
        {
            // validate state
            if (Properties.State != ThreadState::Running) { return false; }
        
            // disable interrtups
            asm volatile("cli");

            // message
            Kernel::Debug.Info("Stopped thread: NAME = %s ID = %d, STACK_SIZE = %d PRIORITY = 0x%2x", Properties.Name, Properties.ID, StackSize, (uint)Properties.Priority);

            // set state
            Properties.State = ThreadState::Halted;

            // enable interrupts and return
            asm volatile("sti");

            return true;
        }

        void Thread::Sleep(uint ms)
        {
            return;
        }

        // on unhandled exception within thread execution
        void Thread::OnUnhandledException(char* msg)
        {
            Kernel::Debug.Error("Thread exception: %s", msg);
            Kernel::ThreadMgr.Unload(this);
        }

        // set thread priority
        void Thread::SetPriority(ThreadPriority priority) { Properties.Priority = priority; }

        // set thread state
        void Thread::SetState(ThreadState state) { Properties.State = state; }

        // clear thread stack
        void Thread::ClearStack() { Memory::Set(Stack, 0, StackSize); }

        // set thread registers
        void Thread::SetRegisters(ISRRegs* regs) { Registers = regs; }

        // get thread name
        char* Thread::GetName() { return Properties.Name; }

        // get thrread id
        ulong Thread::GetID() { return Properties.ID; }

        // get thread state
        ThreadState Thread::GetState() { return Properties.State; }

        // get thread priority
        ThreadPriority Thread::GetPriority() { return Properties.Priority; }

        // get current ticks per second
        uint Thread::GetTPS() { return TPS; }
    }
}