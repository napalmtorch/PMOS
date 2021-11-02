#pragma once
#include <string>
#include <string.h>
#include <vector>
#include <stddef.h>
#include <stdio.h>

namespace StringUtil
{
	std::string FromDecimal(int64_t num);
	std::string FromHex(uint64_t num, bool prefix);

	int64_t ToDecimal(std::string val);

	std::vector<std::string> Split(std::string text, char delim);

	bool Equals(std::string str1, std::string str2);
}