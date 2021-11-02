#pragma once
#include <stddef.h>
#include <string>
#include <string.h>

enum class MsgType
{
	System,
	OK,
	Warning,
	Error,
	Fatal,
};

namespace Debugger
{
	bool IsEnabled();
	void Enable();
	void Disable();

	void ThrowMessage(std::string text, MsgType type);
	void ThrowMessage(std::string text, int num, MsgType type);
	void ThrowMessage(std::string text1, std::string text2, MsgType type);

	void ThrowOK(std::string text);
	void ThrowOK(std::string text, int num);
	void ThrowOK(std::string text1, std::string text2);

	void ThrowWarning(std::string text);
	void ThrowWarning(std::string text, int num);
	void ThrowWarning(std::string text1, std::string text2);

	void ThrowError(std::string text);
	void ThrowError(std::string text, int num);
	void ThrowError(std::string text1, std::string text2);

	void DumpMemory(uint8_t* src, uint32_t bytes);
}