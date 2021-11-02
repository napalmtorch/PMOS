#pragma once
#include <string>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <string.h>

namespace ATADisk
{
	const uint32_t SectorSize = 512;

	extern std::string CurrentFile;

	bool     LoadImageFile(std::string filename);
	bool     UnloadImageFile();
	bool     SaveImageFile(std::string filename);
	bool     CreateImageFile(std::string filename, uint32_t bytes);
	uint8_t* Read(int sector);
	bool     Write(uint8_t* src, int sector);
	uint8_t* GetImage();
	uint32_t GetSize();
}