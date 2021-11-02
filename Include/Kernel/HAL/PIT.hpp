#pragma once
#include <Kernel/Lib/Types.hpp>
#include <Kernel/HAL/Interrupts/ISR.hpp>

namespace PMOS
{
    namespace HAL
    {
        class PITController
        {
            private:
                uint Frequency;
                uint Milliseconds;
                ulong TotalMilliseconds;
                uint MillisTick;

            public:
                ISR Callback;

            public:
                void Initialize(uint freq, ISR callback);
                void Disable();
                void CalculateMilliseconds();

            public:
                uint GetFrequency();
                uint GetMilliseconds();
                ulong GetTotalMilliseconds();
        };
    }
}