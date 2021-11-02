#pragma once
#include <stddef.h>
#include <string>
#include <vector>
#include <algorithm>
#include <string.h>

typedef struct
{
	char* Name;
	char* Help;
	char* Usage;
	void (*Execute)(std::string);
} ShellCommand;

namespace Shell
{
	extern std::vector<ShellCommand> CommandList;

	void Initialize();
	bool RegisterCommand(const char* name, const char* help, const char* usage, void (*exec)(std::string));
	void PrintCaret();
	void ParseCommand(std::string input);
}

namespace Commands
{
    void EXIT(std::string input);
	void CLS(std::string input);
	void INFO(std::string input);
	void SCRIPT(std::string input);
	void HELP(std::string input);
	void MOUNT(std::string input);
	void FORMAT(std::string input);
	void DUMP(std::string input);
	void CREATE(std::string input);
	void LOAD(std::string input);
	void UNLOAD(std::string input);
	void SAVE(std::string input);
	void LBE(std::string input);
	void LFE(std::string input);

	void CD(std::string input);
	void DIR(std::string input);
	void MKDIR(std::string input);
	void ADD(std::string input);
	void REMOVE(std::string input);
	void COPY(std::string input);
	void MOVE(std::string input);
}