#pragma once
#include <Kernel/Lib/Types.hpp>

namespace PMOS
{
    namespace HAL
    {
        class RTCController
        {
            private:
                byte Second;
                byte Minute;
                byte Hour;
                byte Day;
                byte Month;
                byte Year;
                char*   TimeString;
                char*   DateString;
                bool    MilitaryTime;
                bool    ShowSeconds;

            private:
                uint Tick;

            public:
                void Initialize();
                void Update();
                void Read();

            private:
                void UpdateStrings();
                void SetRegister(ushort reg, byte data);
                byte GetRegister(ushort reg);
                bool IsUpdating();

            public:
                byte GetSecond();
                byte GetMinute();
                byte GetHour();
                byte GetDay();
                byte GetMonth();
                byte GetYear();
                char* GetTimeString(bool military, bool seconds);
                char* GetDateString();
        };
    }
}