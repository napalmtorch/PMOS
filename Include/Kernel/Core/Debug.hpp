#pragma once
#include <Kernel/Lib/Types.hpp>
#include <Kernel/Lib/String.hpp>
#include <Kernel/Graphics/Graphics.hpp>

namespace PMOS
{
    enum class DebugMode : byte
    {
        Disabled,
        Serial,
        Terminal,
        All,
    };

    enum class Exception
    {
        Unknown,
        Interrupt,
        OutOfMemory,
    };

    static const int ExceptionMsgCount = 3;
    static const char* ExceptionMsgs[3]
    {
        "Unknown",
        "INT",
        "Out of memory",
    };

    class Debugger
    {
        public:
            DebugMode Mode;

        public:
            void SetMode(DebugMode mode);
            void NewLine();
            void WriteChar(char c);
            void WriteChar(char c, Col4 fg);
            void WriteUnformatted(char* text);
            void WriteUnformatted(char* text, Col4 fg);
            void WriteFormatted(char* text, va_list args);
            void Write(char* fmt, ...);
            void WriteLine(char* fmt, ...);
            void Header(char* text, Col4 fg);
            void Info(char* fmt, ...);
            void OK(char* fmt, ...);
            void Warning(char* fmt, ...);
            void Error(char* fmt, ...);
            void Panic(char* fmt, ...);
            void Panic(int code);
            void DumpMemory(void* ptr, uint len);
    };
}