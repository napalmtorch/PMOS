#include <Kernel/Core/Debug.hpp>
#include <Kernel/Core/Kernel.hpp>

namespace PMOS
{
    void Debugger::Debugger::SetMode(DebugMode mode) { Mode = mode; }

    void Debugger::NewLine()
    {
        if (Mode == DebugMode::Serial || Mode == DebugMode::All) { Kernel::Serial.WriteChar('\n'); }
        if (Mode == DebugMode::Terminal || Mode == DebugMode::All) { Kernel::Terminal->NewLine(); }
    }

    void Debugger::WriteChar(char c)
    {
        if (Mode == DebugMode::Serial || Mode == DebugMode::All) { Kernel::Serial.WriteChar(c); }
        if (Mode == DebugMode::Terminal || Mode == DebugMode::All) { Kernel::Terminal->WriteChar(c); }
    }

    void Debugger::WriteChar(char c, Col4 fg)
    {
        if (Mode == DebugMode::Serial || Mode == DebugMode::All) { Kernel::Serial.WriteChar(c, fg); }
        if (Mode == DebugMode::Terminal || Mode == DebugMode::All) { Kernel::Terminal->WriteChar(c, fg); }       
    }

    void Debugger::WriteUnformatted(char* text)
    {
        int i = 0;
        while (text[i] != 0)
        {
            WriteChar(text[i]);
            i++;
        }
    }

    void Debugger::WriteUnformatted(char* text, Col4 fg)
    {
        int i = 0;
        while (text[i] != 0)
        {
            WriteChar(text[i], fg);
            i++;
        }
    }

    void Debugger::WriteFormatted(char* text, va_list args)
    {
        while (*text != 0)
        {
            if (*text == '%')
            {
                text++;
                if (*text == 'c')
                {
                    int c = va_arg(args, int);
                    WriteChar((char)c);
                    text++;
                }
                else if (*text == 'd' || *text == 'i')
                {
                    int dec = va_arg(args, int);
                    char str[16];
                    WriteUnformatted(String::FromDecimal(dec, str));
                    text++;
                }
                else if (*text == 'u')
                {
                    uint dec = va_arg(args, uint);
                    char str[16];
                    WriteUnformatted(String::FromDecimal(dec, str));
                    text++;
                }
                else if (*text == 'x')
                {
                    uint num = va_arg(args, uint);
                    char str[16];
                    WriteUnformatted(String::FromHex(num, str, false));
                    text++;
                }
                else if (*text == '2')
                {
                    text++;
                    if (*text != 'x' && *text != 'X') { WriteChar(*text); text++; continue; }
                    uint num = va_arg(args, uint);
                    char str[16];
                    WriteUnformatted(String::FromHex(num, str, false, 1));
                    text++;
                }
                else if (*text == '4')
                {
                    text++;
                    if (*text != 'x' && *text != 'X') { WriteChar(*text); text++; continue; }
                    uint num = va_arg(args, uint);
                    char str[16];
                    WriteUnformatted(String::FromHex(num, str, false, 2));
                    text++;
                }
                else if (*text == '8')
                {
                    text++;
                    if (*text != 'x' && *text != 'X') { WriteChar(*text); text++; continue; }
                    uint num = va_arg(args, uint);
                    char str[16];
                    WriteUnformatted(String::FromHex(num, str, false, 4));
                    text++;
                }
                else if (*text == 'f')
                {
                    double f = va_arg(args, double);
                    char str[32];
                    WriteUnformatted(String::FromFloat((float)f, str, 4));
                    text++;
                }
                else if (*text == 's')
                {
                    char* str = va_arg(args, char*);
                    WriteUnformatted(str);
                    text++;
                }
                else { WriteChar(*text); text++; }
            }
            else { WriteChar(*text); text++; }
        }
        if (Mode == DebugMode::Terminal || Mode == DebugMode::All) { Kernel::Terminal->UpdateCursor(); }       
    }

    void Debugger::Write(char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        WriteFormatted(fmt, args);
        va_end(args);
    }

    void Debugger::WriteLine(char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        WriteFormatted(fmt, args);
        va_end(args);
        NewLine();
    }

    void Debugger::Header(char* text, Col4 fg)
    {
        WriteUnformatted("[", Col4::White);
        WriteUnformatted(text, fg);
        WriteUnformatted("] ", Col4::White);
    }

    void Debugger::Info(char* fmt, ...)
    {
        Header("  >>  ", Col4::Cyan);
        va_list args;
        va_start(args, fmt);
        WriteFormatted(fmt, args);
        va_end(args);
        NewLine();
        if (Mode == DebugMode::Terminal || Mode == DebugMode::All) { Kernel::Terminal->UpdateCursor(); }    
    }

    void Debugger::OK(char* fmt, ...)
    {
        Header("  OK  ", Col4::Green);
        va_list args;
        va_start(args, fmt);
        WriteFormatted(fmt, args);
        va_end(args);
        NewLine();
        if (Mode == DebugMode::Terminal || Mode == DebugMode::All) { Kernel::Terminal->UpdateCursor(); }    

    }

    void Debugger::Warning(char* fmt, ...)
    {
        Header("  ??  ", Col4::Yellow);
        va_list args;
        va_start(args, fmt);
        WriteFormatted(fmt, args);
        va_end(args);
        NewLine();
        if (Mode == DebugMode::Terminal || Mode == DebugMode::All) { Kernel::Terminal->UpdateCursor(); }    
    }

    void Debugger::Error(char* fmt, ...)
    {
        Header("  !!  ", Col4::Red);
        va_list args;
        va_start(args, fmt);
        WriteFormatted(fmt, args);
        va_end(args);
        NewLine();
        if (Mode == DebugMode::Terminal || Mode == DebugMode::All) { Kernel::Terminal->UpdateCursor(); }    
    }

    void Debugger::Panic(char* fmt, ...)
    {
        Header("  !!  ", Col4::Red);
        va_list args;
        va_start(args, fmt);
        WriteFormatted(fmt, args);
        va_end(args);
        NewLine();

        asm volatile("cli");
        asm volatile("hlt");
    }
    
    void Debugger::Panic(int code)
    {
        if (code >= ExceptionMsgCount) { code = 0; }
        Error((char*)ExceptionMsgs[code]);

        asm volatile("cli");
        asm volatile("hlt");
    }

    void Debugger::DumpMemory(void* ptr, uint len)
    {
        int bytes_per_line = 16;
        char temp[16];
        char chars[bytes_per_line];
        byte* src = (byte*)ptr;

        WriteUnformatted("Dumping memory at: ");
        String::FromHex((uint)src, temp, false, 4);
        WriteUnformatted(temp, Col4::Cyan);
        NewLine();

        int xx = 0;
        uint pos = 0;
        for (size_t i = 0; i < (len / bytes_per_line); i++)
        {              
            // address range
            pos = i * bytes_per_line;
            chars[0] = '\0';
            String::FromHex((uint)(src + pos), temp, false, 4);
            WriteUnformatted(temp, Col4::Cyan); WriteUnformatted(":");
            String::FromHex((uint)(src + pos + (bytes_per_line - 1)), temp, false, 4);
            WriteUnformatted(temp, Col4::Cyan);
            WriteUnformatted("    ");

            // bytes
            for (size_t j = 0; j < bytes_per_line; j++)
            {
                String::FromHex(src[pos + j], temp, false, 1);
                if (src[pos + j] > 0) { Write(temp); }
                else { WriteUnformatted(temp, Col4::Red); }
                Write(" ");

                if (src[pos + j] >= 32 && src[pos + j] <= 126) { String::Append(chars, src[pos + j]); }
                else { String::Append(chars, '.'); }
            }

            WriteUnformatted("    ");
            WriteUnformatted(chars, Col4::Yellow);
            NewLine();
        }
    }
}