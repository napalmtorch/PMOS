#include "Debugger.hpp"
#include "Console.hpp"
#include "StringUtil.hpp"

namespace Debugger
{
	bool Enabled = false;

	bool IsEnabled() { return Enabled; }
	void Enable()    { Enabled = true; }
	void Disable()   { Enabled = false; }

	void ThrowMessage(std::string text, MsgType type)
	{
		if (text.length() == 0) { return; }
		if (!Enabled) { return; }

		std::string header;
		ConsoleColor color;

		switch (type)
		{
			case MsgType::System:  { header = "  >>  "; color = ConsoleColor::Cyan;     break; }
			case MsgType::OK:      { header = "  OK  "; color = ConsoleColor::Green;    break; }
			case MsgType::Warning: { header = "  ??  "; color = ConsoleColor::Yellow;   break; }
			case MsgType::Error:   { header = "  !!  "; color = ConsoleColor::Red;      break; }
			case MsgType::Fatal:   { header = "  XX  "; color = ConsoleColor::DarkRed;  break; }
			default:			   { header = "  >>  "; color = ConsoleColor::DarkGray; break; }
		}

		Console::Write("[");
		Console::Write(header, color);
		Console::Write("] ");
		Console::WriteLine(text);
	}

	void ThrowMessage(std::string text, int num, MsgType type)
	{
		if (text.length() == 0) { return; }
		if (!Enabled) { return; }

		std::string header;
		ConsoleColor color;

		switch (type)
		{
			case MsgType::System:  { header = "  >>  "; color = ConsoleColor::Cyan;     break; }
			case MsgType::OK:      { header = "  OK  "; color = ConsoleColor::Green;    break; }
			case MsgType::Warning: { header = "  ??  "; color = ConsoleColor::Yellow;   break; }
			case MsgType::Error:   { header = "  !!  "; color = ConsoleColor::Red;      break; }
			case MsgType::Fatal:   { header = "  XX  "; color = ConsoleColor::DarkRed;  break; }
			default:			   { header = "  >>  "; color = ConsoleColor::DarkGray; break; }
		}

		std::string num_val;
		int c_index = text.find_first_of('%');
		if (text.c_str()[c_index + 1] == 'd')
		{
			num_val = StringUtil::FromDecimal(num);
			text.replace(c_index, 2, num_val);
		}
		else if (text.c_str()[c_index + 1] == 'X')
		{
			num_val = StringUtil::FromHex(num, false);
			text.replace(c_index, 2, num_val);
		}

		Console::Write("[");
		Console::Write(header, color);
		Console::Write("] ");
		Console::WriteLine(text);
	}

	void ThrowMessage(std::string text1, std::string text2, MsgType type)
	{
		if (text1.length() == 0) { return; }
		if (!Enabled) { return; }

		std::string header;
		ConsoleColor color;

		switch (type)
		{
			case MsgType::System:  { header = "  >>  "; color = ConsoleColor::Cyan;     break; }
			case MsgType::OK:      { header = "  OK  "; color = ConsoleColor::Green;    break; }
			case MsgType::Warning: { header = "  ??  "; color = ConsoleColor::Yellow;   break; }
			case MsgType::Error:   { header = "  !!  "; color = ConsoleColor::Red;      break; }
			case MsgType::Fatal:   { header = "  XX  "; color = ConsoleColor::DarkRed;  break; }
			default:			   { header = "  >>  "; color = ConsoleColor::DarkGray; break; }
		}

		Console::Write("[");
		Console::Write(header, color);
		Console::Write("] ");
		Console::Write(text1);
		Console::WriteLine(text2);
	}

	void ThrowOK(std::string text) { ThrowMessage(text, MsgType::OK); }
	void ThrowOK(std::string text, int num) { ThrowMessage(text, num, MsgType::OK); }
	void ThrowOK(std::string text1, std::string text2) { ThrowMessage(text1, text2, MsgType::OK); }

	void ThrowWarning(std::string text) { ThrowMessage(text, MsgType::Warning); }
	void ThrowWarning(std::string text, int num) { ThrowMessage(text, num, MsgType::Warning); }
	void ThrowWarning(std::string text1, std::string text2) { ThrowMessage(text1, text2, MsgType::Warning); }

	void ThrowError(std::string text) { ThrowMessage(text, MsgType::Error); }
	void ThrowError(std::string text, int num) { ThrowMessage(text, num, MsgType::Error); }
	void ThrowError(std::string text1, std::string text2) { ThrowMessage(text1, text2, MsgType::Error); }

	void DumpMemory(uint8_t* src, uint32_t bytes)
	{
		Console::Write("Dumping memory at: ");
		Console::WriteLine(StringUtil::FromHex((uint64_t)src, true), ConsoleColor::Cyan);

		int xx = 0;
		int bpl = 12;
		uint32_t pos = 0;
		char chars[13];

		for (size_t i = 0; i < (bytes / bpl); i++)
		{
			pos = i * bpl;
			chars[0] = 0;
			Console::Write(StringUtil::FromHex((uint64_t)(src + pos), true), ConsoleColor::Blue);
			Console::Write(":");
			Console::Write(StringUtil::FromHex((uint64_t)(src + pos + bpl), true), ConsoleColor::Blue);
			Console::Write("    ");

			for (size_t j = 0; j < bpl; j++)
			{
				if (src[pos + j] > 0) { Console::Write(StringUtil::FromHex(src[pos + j], false)); }
				else { Console::Write("00", ConsoleColor::Red); }
				Console::Write(" ");

				if (src[pos + j] >= 32 && src[pos + j] <= 126) { chars[j] = src[pos + j]; }
				else { chars[j] = '.'; }
			}

			chars[12] = 0;
			Console::Write("    ");
			Console::WriteLine(chars, ConsoleColor::Yellow);
		}
	}
}