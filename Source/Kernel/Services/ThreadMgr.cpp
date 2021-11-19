#include <Kernel/Services/ThreadMgr.hpp>
#include <Kernel/Core/Kernel.hpp>

namespace PMOS
{
    namespace Threading
    {
        // init flag
        bool  ThreadSwitchInit = false;

        ThreadManager::ThreadManager() : Service("threadmgr", PMOS::ServiceType::KernelComponent)
        {

        }

        // initialize thread manager
        void ThreadManager::Initialize()
        {
            Service::Initialize();

            // set maximum thread count
            MaxCount = 256;

            // create thread list
            Threads = (Thread**)MemAlloc(MaxCount * sizeof(Thread*), true, AllocationType::System);

            // reset other properties
            CurrentThread = nullptr;
            CurrentIndex  = 0;
            Count         = 0;

            Unloading = nullptr;

            CPUUsage = 100.0f;

            // message
            Kernel::Debug.OK("Initialized thread manager");
        }

        void ThreadManager::Start() { Service::Start(); }

        void ThreadManager::Stop() { Service::Stop(); }

        // load thread into list
        void ThreadManager::Load(Thread* t)
        {
            // validate thread
            if (t == nullptr) { return; }

            // get next free index
            uint i = GetFreeIndex();

            // validate next free index
            if (i >= MaxCount) { Kernel::Debug.Error("Maximum amount of running threads has been reached"); return; }

            // add thread to list
            Threads[i] = t;
            
            // increment thread count
            Count++;

            Kernel::Debug.Info("Loaded thread %s", t->GetName());
        }

        // unload thread from list
        void ThreadManager::Unload(Thread* t)
        {
            // validate thread
            if (t == nullptr) { return; }

            Unloading = t;

            // loop through threads
            for (uint i = 0; i < MaxCount; i++)
            {
                // thread match
                if (Threads[i] != nullptr && Threads[i] == t)
                {
                    // stop execution thread if it is active
                    if (CurrentIndex == (int)i)  { CurrentIndex = -1; CurrentThread = nullptr; }
                    if (CurrentThread == t) { CurrentIndex = -1; CurrentThread = nullptr; }
                    
                    // dispose thread
                    Threads[i]->Dispose();

                    // decrement count
                    Count--;

                    // clear value in list and return
                    Threads[i] = nullptr;
                    Unloading = nullptr;
                    return;
                }
            }
        }

        // create new thread
        Thread* ThreadManager::Create(char* name, ThreadPriority priority, void protocol(Thread*))
        {
            Thread* t = new Thread(name, priority, protocol);
            HeapEntry* e = Kernel::MemoryMgr.GetEntryFromPtr(t);
            if (e != nullptr) { e->Type = (byte)AllocationType::Thread; }
            return t;
        }

        // create new thread with specified stack size
        Thread* ThreadManager::Create(char* name, uint stack, ThreadPriority priority, void protocol(Thread*))
        {
            Thread* t = new Thread(name, stack, priority, protocol);
            HeapEntry* e = Kernel::MemoryMgr.GetEntryFromPtr(t);
            if (e != nullptr) { e->Type = (byte)AllocationType::Thread; }
            return t;
        }

        void ThreadManager::CalculateCPUUsage()
        {
            ulong total_tps;

            // calculate usage of each thread
            for (uint i = 0; i < MaxCount; i++)
            {
                if (Threads[i] != nullptr)
                {
                    if (Threads[i] != Kernel::IdleThread) { total_tps += Threads[i]->GetTPS(); }
                    Threads[i]->CPUUsage = 0.0f;
                }
            }

            // calculate total usage
            CPUUsage = 100.0f - (((float)total_tps / (float)Kernel::IdleThread->GetTPS()) * 100);
            if (CPUUsage < 0.0f)   { CPUUsage = 0.0f; }
            if (CPUUsage > 100.0f) { CPUUsage = 100.0f; }
        }

        // terminate thread by pointer
        bool ThreadManager::Terminate(Thread* thread)
        {
            // validate thread
            if (thread == nullptr) { return false; }

            // loop through list
            for (uint i = 0; i < MaxCount; i++)
            {
                // located thread - terminate and return success
                if (Threads[i] == thread)
                {
                    Threads[i]->Stop();
                    return true;
                }
            }
            
            // unable to locate thread in list
            return false;
        }

        // terminate thread by index
        bool ThreadManager::Terminate(int index)
        {
            // validate index
            if (index < 0 || index >= MaxCount) { return false; }

            // loop through list
            for (uint i = 0; i < MaxCount; i++)
            {
                // located thread - terminate and return success
                if (Threads[i] != nullptr && i == index)
                {
                    Threads[i]->Stop();
                    return true;
                }
            }

            // unable to locate thread in list
            return false;
        }

        // terminate first thread with specified name
        bool ThreadManager::Terminate(char* name)
        {
            // validate name
            if (name == nullptr) { return false; }
            if (StringUtil::Length(name) == 0) { return false; }

            // loop through list
            for (uint i = 0; i < MaxCount; i++)
            {
                // located thread - terminate and return success
                if (Threads[i] != nullptr && StringUtil::Equals(Threads[i]->Properties.Name, name))
                {
                    Threads[i]->Stop();
                    return true;
                }
            }

            // unable to locate thread in list
            return false;
        }

        // terminate all threads with specified name
        bool ThreadManager::TerminateAll(char* name)
        {
            // validate name
            if (name == nullptr) { return false; }
            if (StringUtil::Length(name) == 0) { return false; }

            Kernel::InterruptMgr.DisableInterrupts();

            // loop through list
            ushort del = 0;
            for (uint i = 0; i < MaxCount; i++)
            {
                // located thread - terminate and continue to next
                if (Threads[i] != nullptr && StringUtil::Equals(Threads[i]->Properties.Name, name))
                {
                    Threads[i]->Stop();
                    del++;
                }
            }

            Kernel::InterruptMgr.EnableInterrupts();

            // unable to locate thread in list
            if (del > 0) { return true; }
            return false;
        }

        // print list of threads
        void ThreadManager::Print(DebugMode mode)
        {
            DebugMode oldMode = Kernel::Debug.Mode;
            Kernel::Debug.SetMode(mode);
            Kernel::Debug.WriteUnformatted("-------- ", Col4::DarkGray);
            Kernel::Debug.WriteUnformatted("THREADS", Col4::Green);
            Kernel::Debug.WriteUnformatted(" -----------------------------------");
            Kernel::Debug.NewLine();
            Kernel::Debug.WriteUnformatted("ID          PRIORITY      STATE      STACK       NAME\n", Col4::DarkGray);

            for (size_t i = 0; i < MaxCount; i++)
            {
                if (Threads[i] == nullptr) { continue; }
                
                Kernel::Debug.Write("0x%8x  ", (uint)Threads[i]->GetID());
                Kernel::Debug.Write("0x%2x          ", (uint)Threads[i]->GetPriority());
                Kernel::Debug.Write("0x%2x       ", (uint)Threads[i]->GetState());
                Kernel::Debug.Write("0x%8x  ", (uint)Threads[i]->StackSize);
                Kernel::Debug.Write("%s", Threads[i]->GetName());
                Kernel::Debug.NewLine();
            }

            Kernel::Debug.NewLine();
            Kernel::Debug.SetMode(oldMode);
        }

        // get total cpu usage
        float ThreadManager::GetCPUUsage() { return CPUUsage; }

        // handle thread switching
        int PriorityTick;
        void ThreadManager::Schedule(uint* regs)
        {
            // call primary pit callback method
            Kernel::PITCallback(regs);
            
            // get registers from argument
            ISRRegs* r = (ISRRegs*)*regs;

            // validate thread list
            if (Kernel::ThreadMgr.Count == 0) { return; }

            // save registers
            if (ThreadSwitchInit) 
            { 
                Kernel::ThreadMgr.Threads[Kernel::ThreadMgr.CurrentIndex]->Registers = r; 
            }

            Kernel::ThreadMgr.CurrentIndex++;
            if (Kernel::ThreadMgr.CurrentIndex >= Kernel::ThreadMgr.Count) { Kernel::ThreadMgr.CurrentIndex = 0; }

            // get next thread
            Thread* next = Kernel::ThreadMgr.Threads[Kernel::ThreadMgr.CurrentIndex];
            if (next == nullptr) { return; }

            // check if thread has halted
            if (next->GetState() == ThreadState::Halted)
            {
                Kernel::ThreadMgr.Unload(next);
                Schedule(regs);
                return;
            }
            // check if thread has paused
            else if (next->GetState() == ThreadState::Paused)
            {
                Schedule(regs);
                return;
            }

            Kernel::ThreadMgr.CurrentThread = next;
            *regs = (uint)Kernel::ThreadMgr.CurrentThread->Registers;
            ThreadSwitchInit = true;
        }

        // get next available spot in thread array
        uint ThreadManager::GetFreeIndex()
        {
            // loop through threads
            for (uint i = 0; i < MaxCount; i++)
            {
                if (Threads[i] == nullptr) { return i; }
            }

            // return invalid index
            return 0xFFFFFFFF;
        }
    }
}