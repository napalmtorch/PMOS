#pragma once
#include <iostream>
#include <string>
#include <string.h>

enum class ConsoleColor
{
	Black,
	DarkBlue,
	DarkGreen,
	DarkCyan,
	DarkRed,
	DarkMagenta,
	DarkYellow,
	Gray,
	DarkGray,
	Blue,
	Green,
	Cyan,
	Red,
	Magenta,
	Yellow,
	White,
};

namespace Console
{
	void Initialize();

	void Clear();

	void Write(std::string text);
	void Write(std::string text, ConsoleColor fg);

	void WriteLine(std::string text);
	void WriteLine(std::string text, ConsoleColor fg);

	void SetForeColor(ConsoleColor color);

	void SetCursorPosition(int x, int y);
	void SetCursorX(int x);
	void SetCursorY(int y);

    void ResetColor();
	ConsoleColor GetForeColor();
    std::string ConvertColor(ConsoleColor color);
}