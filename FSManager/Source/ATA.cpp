#include "ATA.hpp"
#include "StringUtil.hpp"
#include "Console.hpp"
#include "Debugger.hpp"

namespace ATADisk
{
	// disk image file data
	uint8_t* ImageData;
	uint32_t ImageSize;

	std::string CurrentFile;

	// load disk image file
	bool LoadImageFile(std::string filename)
	{
		std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
		if (file.is_open())
		{
			char* data = 0;
			int size = 0;
			file.seekg(0, std::ios::end);
			size = file.tellg();
			file.seekg(0, std::ios::beg);

			ImageData = (uint8_t*)malloc(size);
			ImageSize = size;
			CurrentFile = filename;
			file.read((char*)ImageData, size);
			Debugger::ThrowOK("Loaded " + StringUtil::FromDecimal(size / 1024 / 1024) + "MB disk image file");
			return true;
		}
		return false;
	}

	// unload current disk image file 
	bool UnloadImageFile()
	{
		if (ImageData == nullptr) { return false; }
		if (ImageSize == 0) { return false; }

		free(ImageData);
		ImageData = nullptr;
		ImageSize = 0;
		CurrentFile = "";

		Debugger::ThrowOK("Finished unloading disk image");
		return true;
	}

	// save currently opened image file
	bool SaveImageFile(std::string filename)
	{
		if (ImageData == nullptr) { return false; }
		if (ImageSize == 0)		  { return false; }

		FILE* file = fopen(filename.c_str(), "wb");
		if (file != nullptr)
		{
			Debugger::ThrowMessage("Saving disk image", MsgType::System);
			fwrite(ImageData, ImageSize, 1, file);
			fclose(file);
		}
		else { Debugger::ThrowError("Unable to save disk image"); return false; }
		Debugger::ThrowOK("Done.");
		return true;
	}

	// create new image file and save
	bool CreateImageFile(std::string filename, uint32_t bytes)
	{
		// read file from disk
		std::ofstream file(filename);

		// delete old data
		if (ImageData != nullptr) { free(ImageData); Debugger::ThrowMessage("Unloaded previous disk", MsgType::System); }
		ImageSize = bytes;
		ImageData = (uint8_t*)malloc(bytes);
		if (ImageData != nullptr) { memset(ImageData, 0, ImageSize); }
		Debugger::ThrowMessage("Created new disk image block", MsgType::System);
		if (ImageData != nullptr)
		{
			Debugger::ThrowMessage("Wiping image file", MsgType::System);
			memset((void*)ImageData, 0, (size_t)bytes);
			Debugger::ThrowMessage("Done.", MsgType::System);
			file.write((char*)ImageData, bytes);
			Debugger::ThrowMessage("Saved file data", MsgType::System);
		}
		CurrentFile = filename;
		file.close();

		// return success
		std::string msg;
		msg.append("Successfully create ");
		msg.append(StringUtil::FromDecimal(ImageSize / 1024 / 1024));
		msg.append("MB image file at ");
		Debugger::ThrowOK(msg, filename);
		return true;
	}

	// read sector from disk image
	uint8_t* Read(int sector)
	{
		// validate disk image
		if (ImageData == nullptr || ImageSize == 0) { return nullptr; }

		// calculate offset
		uint32_t offset = sector * SectorSize;

		// validate offset
		if (offset < 0 || offset >= ImageSize) { Debugger::ThrowError("Sector out of bounds violation while reading sector %d", sector); return nullptr; }
		
		// create output array
		uint8_t* output = (uint8_t*)malloc(SectorSize);
		if (output != nullptr) { memset(output, 0, SectorSize); }

		// validate output
		if (output == nullptr) { Debugger::ThrowError("Memory allocation voliation while reading sector %d", sector); return nullptr; }

		// copy data and return
		memcpy(output, (void*)(ImageData + offset), SectorSize);
		return output;
	}

	// write sector from array to disk image
	bool Write(uint8_t* src, int sector)
	{
		// validate disk image
		if (ImageData == nullptr || ImageSize == 0) { return false; }

		// calculate offset
		uint32_t offset = sector * SectorSize;

		// validate offset
		if (offset < 0 || offset >= ImageSize) { Debugger::ThrowError("Sector out of bounds violation while writing sector %d", sector); return false; }

		// validate sarc
		if (src == nullptr) { Debugger::ThrowError("Null exception while writing sector %d", sector); return false; }

		// copy data to disk image
		memcpy((void*)(ImageData + offset), src, SectorSize);
		return true;
	}

	// get disk image file data
	uint8_t* GetImage() { return ImageData; }

	uint32_t GetSize() { return ImageSize; }
}