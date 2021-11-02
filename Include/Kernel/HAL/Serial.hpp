#pragma once
#include <Kernel/Lib/Types.hpp>
#include <Kernel/Lib/String.hpp>
#include <Kernel/Graphics/Graphics.hpp>

namespace PMOS
{
    namespace HAL
    {
        enum class SerialPort : ushort
        {
            Disabled = 0x0000,
            COM1     = 0x03F8,
            COM2     = 0x02F8,
            COM3     = 0x03E8,
            COM4     = 0x02E8,
        };

        class SerialController
        {
            private:
                SerialPort CurrentPort;

            public:
                void SetPort(SerialPort port);
                SerialPort GetPort();

            public:
                static const char* SerialPortToString(SerialPort port);
            
            public:
                char Read();
                void WriteChar(char c);
                void WriteChar(char c, Col4 fg);
                void Write(char* text);
                void Write(char* text, Col4 fg);
                void WriteLine(char* text);
                void WriteLine(char* text, Col4 fg);
                void SetColor(Col4 color);
                byte HasRecieved();
                byte CanSend();
        };
    }
}