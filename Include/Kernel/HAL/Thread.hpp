#pragma once
#include <Kernel/Lib/Types.hpp>
#include <Kernel/HAL/Interrupts/ISR.hpp>

namespace PMOS
{
    enum class ThreadState
    {
        Initialized     = 0x00,
        Running         = 0x01,
        Halted          = 0x02,
        Completed       = 0x03,
        Sleeping        = 0x04,
        Paused          = 0x05,
    };

    enum class ThreadPriority
    {
        Low,
        Medium,
        High,
        Protected,
    };

    namespace Threading
    {   
        typedef struct
        {
            char           Name[64];
            ulong          ID;
            ThreadState    State;
            ThreadPriority Priority;
        } ATTR_PACK ThreadProperties;

        class Thread;
        class ThreadManager;

        void ThreadEntry();

        extern const uint STACK_SIZE;

        class Thread
        {
            friend class ThreadManager;

            private:
                ThreadProperties Properties;
                uint TPS, TPSTick, Time, LastTime;
                float CPUUsage;

            public:
                Registers32* Registers;
                byte*        Stack;
                uint         StackSize;

            public:
                void         (*Protocol)(Thread* sender);

            public:
                Thread();
                Thread(char* name, ThreadPriority priority, void protocol(Thread*));
                Thread(char* name, uint stack, ThreadPriority priority, void protocol(Thread*));
                void Dispose();
                void CalculateTPS();

            public:
                bool Start();
                bool Stop();
                void Sleep(uint ms);
                void OnUnhandledException(char* msg);

            public:
                void SetPriority(ThreadPriority priority);
                void SetState(ThreadState state);

            public: 
                void ClearStack();
                void SetRegisters(Registers32* regs);

            public:
                char* GetName();
                ulong GetID();
                ThreadState    GetState();
                ThreadPriority GetPriority();
                uint GetTPS();
        };
    }
}