#pragma once
#include <Kernel/Lib/Types.hpp>
#include <Kernel/HAL/Thread.hpp>
#include <Kernel/HAL/Interrupts/ISR.hpp>
#include <Kernel/Core/Service.hpp>
#include <Kernel/Core/Debug.hpp>

namespace PMOS
{
    namespace Threading
    {
        class ThreadManager : public Service
        {
            friend class Thread;

            public:
                Thread** Threads;
                Thread*  CurrentThread;
                int      CurrentIndex;
                uint     Count;
                uint     MaxCount;
                Thread*  Unloading;

            private:
                float CPUUsage;

            public:
                ThreadManager();
                void Initialize() override;
                void Start() override;
                void Stop() override;

            public:
                void Load(Thread* t);
                void Unload(Thread* t);
                Thread* Create(char* name, ThreadPriority priority, void protocol(Thread*));
                Thread* Create(char* name, uint stack, ThreadPriority priority, void protocol(Thread*));

            public:
                static void Schedule(uint* regs);

            public:
                void CalculateCPUUsage();
                float GetCPUUsage();

            public:
                bool Terminate(Thread* thread);
                bool Terminate(int index);
                bool Terminate(char* name);
                bool TerminateAll(char* name);

            public:
                void Print(DebugMode mode);

            private:
                uint GetFreeIndex();
        };
    }
}