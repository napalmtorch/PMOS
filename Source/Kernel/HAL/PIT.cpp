#include <Kernel/HAL/PIT.hpp>
#include <Kernel/Core/Kernel.hpp>

namespace PMOS
{
    namespace HAL
    {
        // initialize pit controller
        void PITController::Initialize(uint freq, ISR callback)
        {
            // validate frequency
            if (freq == 0) { Disable(); return; }

            // register interrupt
            Callback = callback;
            Kernel::InterruptMgr.Register(IRQ0, Callback);

            // set frequency value
            if (freq > 5000) { Frequency = 5000; }
            else { Frequency = freq; }

            // pack frequency
            uint f = 1193180 / Frequency;
            byte high = (byte)((f & 0xFF00) >> 8);
            byte low  = (byte)((f & 0x00FF));

            Milliseconds = 0;
            TotalMilliseconds = 0;
            MillisTick = 0;

            // send frequency data to pit
            Ports::Write8(0x43, 0x36);
            Ports::Write8(0x40, low);
            Ports::Write8(0x40, high);

            Kernel::Debug.Info("Initialized PIT(freq = %d, callback = 0x%8x)", freq, (uint)callback);
        }

        // disable pit controller
        void PITController::Disable()
        {
            // unregister interrupt
            Kernel::InterruptMgr.Unregister(IRQ0);
            
            // reset properties
            Frequency         = 0;
            Milliseconds      = 0;
            TotalMilliseconds = 0;
        }

        // calculate pit times
        void PITController::CalculateMilliseconds()
        {
            // increment millis tick
            MillisTick++;

            // millisecond has passed
            if (MillisTick >= Frequency / 1000)
            {
                Milliseconds++;
                TotalMilliseconds++;
                MillisTick = 0;
            }

            // reset current millisecond timer
            if (Milliseconds >= 1000) { Milliseconds = 0; MillisTick = 0; }
        }

        // get currently set pit frequency
        uint PITController::GetFrequency() { return Frequency; }

        // get current millisecond within pit second
        uint PITController::GetMilliseconds() { return Milliseconds; }

        // get total amount of passed milliseconds
        ulong PITController::GetTotalMilliseconds() { return TotalMilliseconds; }
    }
}