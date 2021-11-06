#include <Kernel/HAL/RTC.hpp>
#include <Kernel/Core/Kernel.hpp>

#define RTC_PORT_CMD  0x70
#define RTC_PORT_DATA 0x71

namespace PMOS
{
    namespace HAL
    {
        void RTCController::Initialize()
        {
            // allocate strings
            TimeString = (char*)MemAlloc(64, true, AllocationType::String);
            DateString = (char*)MemAlloc(64, true, AllocationType::String);

            // clear strings
            StringUtil::Clear(TimeString);
            StringUtil::Clear(DateString);

            // default properties
            MilitaryTime = false;
            ShowSeconds = false;

            // update
            Read();

            // message
            Kernel::Debug.OK("Initialized RTC controller - %s", TimeString);
        }

        void RTCController::Update()
        {
            // increment tick
            Tick++;

            // half a second has passed
            if (Tick >= Kernel::PIT.GetFrequency() / 2)
            {
                // read data from rtc controller
                Read();

                // reset tick value
                Tick = 0;
            }
        }

        void RTCController::Read()
        {
            // fetch time
            Second = GetRegister(0x00);
            Minute = GetRegister(0x02);
            Hour   = GetRegister(0x04);

            // fetch date
            Day    = GetRegister(0x07);
            Month  = GetRegister(0x08);
            Year   = GetRegister(0x09);

            // check if time is binary coded decimal
            byte bcd = GetRegister(0x0B);

            // convert from binary coded decimal if required
            if (!(bcd & 0x04))
            {
                Second  = (Second & 0x0F) + (Second / 16) * 10;
                Minute  = (Minute & 0x0F) + (Minute / 16) * 10;
                Hour    = ((Hour & 0x0F) + (Hour / 16) * 10) | (Hour & 0x80);
                Day     = (Day & 0x0F) + (Day / 16) * 10;
                Month   = (Month & 0x0F) + (Month / 16) * 10;
                Year    = (Year & 0x0F) + (Year / 16) * 10;
            }

            UpdateStrings();
        }
        
        void RTCController::UpdateStrings()
        {
            StringUtil::Clear(TimeString);
            char num[64];

            // 24-hour
            if (MilitaryTime)
            {
                // hour
                StringUtil::FromDecimal(Hour, num);
                if (Hour < 10) { StringUtil::Append(TimeString, '0'); }
                StringUtil::Append(TimeString, num);
                StringUtil::Append(TimeString, ':');

                // minute
                StringUtil::FromDecimal(Minute, num);
                if (Minute < 10) { StringUtil::Append(TimeString, '0'); }
                StringUtil::Append(TimeString, num);

                // if seconds are visible
                if (ShowSeconds)
                {
                    // second
                    StringUtil::Append(TimeString, ':');
                    StringUtil::FromDecimal(Second, num);
                    if (Second < 10) { StringUtil::Append(TimeString, '0'); }
                    StringUtil::Append(TimeString, num);
                }
            }
            // 12-hour
            else
            {
                // hour
                ushort hr = Hour;

                // determine hour value
                if (Hour > 12) { hr = Hour - 12; }
                else if (Hour > 0 && Hour <= 12) { hr = Hour; }
                else if (Hour == 0) { hr = 12; }

                // append hour
                StringUtil::FromDecimal(hr, num);
                StringUtil::Append(TimeString, num);
                StringUtil::Append(TimeString, ':');

                // minute
                StringUtil::FromDecimal(Minute, num);
                if (Minute < 10) { StringUtil::Append(TimeString, '0'); }
                StringUtil::Append(TimeString, num);

                // if seconds are visible
                if (ShowSeconds)
                {
                    // second
                    StringUtil::Append(TimeString, ':');
                    StringUtil::FromDecimal(Second, num);
                    if (Second < 10) { StringUtil::Append(TimeString, '0'); }
                    StringUtil::Append(TimeString, num);
                }

                // AM or PM
                if (Hour < 12) { StringUtil::Append(TimeString, " AM"); }
                else { StringUtil::Append(TimeString, " PM"); }
            }
        }

        void RTCController::SetRegister(ushort reg, byte data)
        {
            if (!IsUpdating())
            {
                HAL::Ports::Write8(RTC_PORT_CMD, reg);
                HAL::Ports::Write8(RTC_PORT_DATA, data);
            }
        }

        byte RTCController::GetRegister(ushort reg)
        {
            HAL::Ports::Write8(RTC_PORT_CMD, reg);
            return HAL::Ports::Read8(RTC_PORT_DATA);
        }

        bool RTCController::IsUpdating()
        {   
            HAL::Ports::Write8(RTC_PORT_CMD, 0x0A);
            byte status = HAL::Ports::Read8(RTC_PORT_DATA);
            return (bool)(status & 0x80);
        }

        // return numeral values
        byte RTCController::GetSecond() { return Second; }
        byte RTCController::GetMinute() { return Minute; }
        byte RTCController::GetHour() { return Hour; }
        byte RTCController::GetDay() { return Day; }
        byte RTCController::GetMonth() { return Month; }
        byte RTCController::GetYear() { return Year; }

        // return time string
        char* RTCController::GetTimeString(bool military, bool seconds)
        { 
            bool old_mil = MilitaryTime;
            bool old_sec = ShowSeconds;

            MilitaryTime = military;
            ShowSeconds = seconds;

            if (military != old_mil || seconds != old_sec) { UpdateStrings(); }

            UpdateStrings();
            return TimeString; 
        }

        // return date string
        char* RTCController::GetDateString() 
        { 
            UpdateStrings();
            return DateString; 
        }
    }
}