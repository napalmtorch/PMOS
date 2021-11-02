#include "Console.hpp"
#include "StringUtil.hpp"

namespace Console
{
    ConsoleColor ForeColor;
    uint32_t CursorY = 0;

    void Initialize()
    {
        ForeColor = ConsoleColor::White;
        CursorY = 0;
        Clear();
    }

	void Clear()
    {
        std::cout << "\033[41]";
        std::system("clear");
        CursorY = 0;
    }

	void Write(std::string text) { std::cout << ConvertColor(ForeColor); std::cout << text; }
	void Write(std::string text, ConsoleColor fg) 
    { 
        ConsoleColor old_fg = ForeColor;
        SetForeColor(fg);
        std::cout << text;
        SetForeColor(old_fg);
    }

	void WriteLine(std::string text) { WriteLine(text, ForeColor); CursorY++; }
	void WriteLine(std::string text, ConsoleColor fg) { Write(text + "\n", fg); CursorY++; }

	void SetForeColor(ConsoleColor color) { ForeColor = color; std::cout << ConvertColor(color); }

	void SetCursorPosition(int x, int y) 
    { 
        printf("\033[%d;%dH", x + 1, y + 1);
    }

	void SetCursorX(int x)
    {
        SetCursorPosition(x, CursorY);
    }

	void SetCursorY(int y)
    {
        
    }

	ConsoleColor GetForeColor() { return ForeColor; }

    void ResetColor() { std::cout << ConvertColor(ConsoleColor::White); }

    std::string ConvertColor(ConsoleColor color)
    {
        switch (color)
        {
            case ConsoleColor::Black:       { return std::string("\033[;30m"); }
            case ConsoleColor::DarkBlue:    { return std::string("\033[;34m"); }
            case ConsoleColor::DarkGreen:   { return std::string("\033[;32m"); }
            case ConsoleColor::DarkCyan:    { return std::string("\033[;36m"); }
            case ConsoleColor::DarkRed:     { return std::string("\033[;31m"); }
            case ConsoleColor::DarkMagenta: { return std::string("\033[;35m"); }
            case ConsoleColor::DarkYellow:  { return std::string("\033[;33m"); }
            case ConsoleColor::Gray:        { return std::string("\033[;37m"); }
            case ConsoleColor::DarkGray:    { return std::string("\033[;37m"); }
            case ConsoleColor::Blue:        { return std::string("\033[;34m"); }
            case ConsoleColor::Green:       { return std::string("\033[;32m"); }
            case ConsoleColor::Cyan:        { return std::string("\033[;36m"); }
            case ConsoleColor::Red:         { return std::string("\033[;31m"); }
            case ConsoleColor::Magenta:     { return std::string("\033[;35m"); }
            case ConsoleColor::Yellow:      { return std::string("\033[;33m"); }
            case ConsoleColor::White:       { return std::string("\033[;37m"); }
        }
        return std::string("\033[;30m");
    }
}