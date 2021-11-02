#include "FileSystem.hpp"
#include "StringUtil.hpp"
#include "Console.hpp"
#include "Debugger.hpp"
#include "Shell.hpp"
#include "ATA.hpp"

namespace FileSystem
{
	// get entry type string
	std::string GetEntryTypeString(EntryType type)
	{
		switch (type)
		{
			case EntryType::Directory: { return "DIR "; }
			case EntryType::File: { return "FILE"; }
			default: { return "NULL"; }
		}
		return "NULL";
	}

	// file system instance
	FSHost MasterFS;

	// initialize table arrays
	void FSHost::InitTableArrays()
	{
		// create new block table data array
		if (SuperBlock.BlockTable.SizeInBytes == 0) { Debugger::ThrowError("Unable to load block table"); return; }
		BlockTableData = (uint8_t*)malloc(SuperBlock.BlockTable.SizeInBytes);
		if (BlockTableData == nullptr) { Debugger::ThrowError("Unable to allocate memory while creating block table data array"); return; }
		memset(BlockTableData, 0, SuperBlock.BlockTable.SizeInBytes);
		Debugger::ThrowOK("Created new block table data array");

		// create new entry table data array
		if (SuperBlock.EntryTable.SizeInBytes == 0) { Debugger::ThrowError("Unable to load entry table"); return; }
		EntryTableData = (uint8_t*)malloc(SuperBlock.EntryTable.SizeInBytes);
		if (EntryTableData == nullptr) { Debugger::ThrowError("Unable to allocate memory while creating entry table data array"); return; }
		memset(EntryTableData, 0, SuperBlock.EntryTable.SizeInBytes);
		Debugger::ThrowOK("Created new entry table data array");
	}

	// print full disk information
	void FSHost::PrintDiskInformation()
	{
		Console::Write("-- ", ConsoleColor::DarkGray);
		Console::Write(" DISK INFORMATION ", ConsoleColor::Green);
		Console::WriteLine("------------------------------", ConsoleColor::DarkGray);

		Console::Write("SECTOR COUNT                ", ConsoleColor::Yellow);
		Console::WriteLine(StringUtil::FromDecimal(SuperBlock.SectorCount));
		
		Console::Write("BYTES/SECTOR                ", ConsoleColor::Yellow);
		Console::WriteLine(StringUtil::FromDecimal(SuperBlock.BytesPerSector));

		Console::Write("-- ", ConsoleColor::DarkGray);
		Console::Write(" BLOCK TABLE INFORMATION ", ConsoleColor::Cyan);
		Console::WriteLine("-----------------------", ConsoleColor::DarkGray);

		Console::Write("BT START                    ", ConsoleColor::Yellow);
		Console::WriteLine(StringUtil::FromDecimal(SuperBlock.BlockTable.StartSector));

		Console::Write("BT END                      ", ConsoleColor::Yellow);
		Console::WriteLine(StringUtil::FromDecimal(SuperBlock.BlockTable.EndSector));

		Console::Write("BT COUNT                    ", ConsoleColor::Yellow);
		Console::WriteLine(StringUtil::FromDecimal(SuperBlock.BlockTable.SectorCount));

		Console::Write("BT SIZE                     ", ConsoleColor::Yellow);
		Console::Write(StringUtil::FromDecimal(SuperBlock.BlockTable.SizeInBytes / 1024) + " KB ");
		Console::WriteLine("(" + StringUtil::FromDecimal(SuperBlock.BlockTable.SizeInBytes) + " bytes)");

		Console::Write("BT MAX ENTRIES              ", ConsoleColor::Yellow);
		Console::WriteLine(StringUtil::FromDecimal(SuperBlock.BlockTable.MaxEntries));

		Console::Write("-- ", ConsoleColor::DarkGray);
		Console::Write(" ENTRY TABLE INFORMATION ", ConsoleColor::Cyan);
		Console::WriteLine("-----------------------", ConsoleColor::DarkGray);

		Console::Write("ET START                    ", ConsoleColor::Yellow);
		Console::WriteLine(StringUtil::FromDecimal(SuperBlock.EntryTable.StartSector));

		Console::Write("ET END                      ", ConsoleColor::Yellow);
		Console::WriteLine(StringUtil::FromDecimal(SuperBlock.EntryTable.EndSector));

		Console::Write("ET COUNT                    ", ConsoleColor::Yellow);
		Console::WriteLine(StringUtil::FromDecimal(SuperBlock.EntryTable.SectorCount));

		Console::Write("ET SIZE                     ", ConsoleColor::Yellow);
		Console::Write(StringUtil::FromDecimal(SuperBlock.EntryTable.SizeInBytes / 1024) + " KB ");
		Console::WriteLine("(" + StringUtil::FromDecimal(SuperBlock.EntryTable.SizeInBytes) + " bytes)");

		Console::Write("ET MAX ENTRIES              ", ConsoleColor::Yellow);
		Console::WriteLine(StringUtil::FromDecimal(SuperBlock.EntryTable.MaxEntries));

		Console::WriteLine("--------------------------------------------------", ConsoleColor::DarkGray);
		Console::Write("\n");
	}

	// print block table entries
	void FSHost::PrintBlockTable()
	{
		// header message
		Console::Write("-- ", ConsoleColor::DarkGray);
		Console::Write(" BLOCK TABLE ENTRIES ", ConsoleColor::Green);
		Console::WriteLine("---------------------------", ConsoleColor::DarkGray);

		// loop through entries
		for (uint64_t i = 0; i < SuperBlock.BlockTable.MaxEntries; i++)
		{
			// read entry from table
			BlockEntry* entry = (BlockEntry*)(BlockTableData + (i * (uint64_t)FS_SIZE_BLOCK_ENTRY));

			// validate entry
			if (entry->Sector > 0)
			{
				// print id
				Console::Write("ID: ", ConsoleColor::Yellow);
				Console::Write(StringUtil::FromDecimal(i));
                Console::Write("    ");
				// print sector
				Console::Write("SECTOR", ConsoleColor::Cyan);
				Console::Write(" = ");
				Console::Write(StringUtil::FromHex(entry->Sector, true));
				Console::Write("    ");
				// print state
				Console::Write("STATE", ConsoleColor::Green);
				Console::Write(" = ");
				if (entry->State == FS_STATE_USED) { Console::Write("USED"); } else { Console::Write("FREE"); }
				Console::Write("    ");
				// print size
				Console::Write("SIZE", ConsoleColor::DarkGray);
				Console::Write(" = ");
				Console::WriteLine(StringUtil::FromDecimal(entry->Count) + " sectors");
			}
		}

		// footer message
		Console::WriteLine("--------------------------------------------------", ConsoleColor::DarkGray);
		Console::Write("\n");
	}

	// print file entry table entries
	void FSHost::PrintEntryTable()
	{
		// header message
		Console::Write("-- ", ConsoleColor::DarkGray);
		Console::Write(" ENTRY TABLE ENTRIES ", ConsoleColor::Green);
		Console::WriteLine("---------------------------", ConsoleColor::DarkGray);

		// loop through entries
		for (uint64_t i = 0; i < SuperBlock.EntryTable.MaxEntries; i++)
		{
			// read entry from table
			FileEntry* entry = (FileEntry*)(EntryTableData + (i * (uint64_t)FS_SIZE_FILE_ENTRY));

			// validate entry
			if (entry->Type == EntryType::File || entry->Type == EntryType::Directory)
			{
				// print index
				Console::Write("ID: ", ConsoleColor::Yellow);
				Console::Write(StringUtil::FromDecimal(i));
				Console::Write("    ");
				// print type
				Console::Write("TYPE", ConsoleColor::Green);
				Console::Write(" = ");
				Console::Write(GetEntryTypeString(entry->Type) + " ");
				// print status
				Console::Write("STATUS", ConsoleColor::Green);
				Console::Write(" = ");
				if (entry->Status == 1) { Console::Write("ROOT"); } else { Console::Write(StringUtil::FromDecimal(entry->Status)); }
				Console::Write("    ");
				// print parent
				Console::Write("PARENT = ", ConsoleColor::Green);
				Console::Write(" = ");
				Console::Write(StringUtil::FromDecimal(entry->ParentIndex));
				Console::Write("    ");
				// print name
				Console::Write("NAME", ConsoleColor::DarkGray);
				Console::Write(" = ");
				if (strlen(entry->Name) > 0) { Console::Write(std::string(entry->Name)); }
				else { Console::Write("ERROR", ConsoleColor::Red); }
				Console::Write("    ");

				// file specific properties
				if (entry->Type == EntryType::File)
				{
					Console::Write("SIZE", ConsoleColor::Blue);
					Console::Write(" = ");
					Console::Write(StringUtil::FromDecimal(entry->Size));
					Console::Write("    ");

					Console::Write("SECTOR", ConsoleColor::Blue);
					Console::Write(" = ");
					Console::Write(StringUtil::FromDecimal(entry->StartSector));
				}

				// newline
				Console::Write("\n");
			}
		}

		// footer message
		Console::WriteLine("--------------------------------------------------", ConsoleColor::DarkGray);
		Console::Write("\n");
	}

	// print directories and files contained in specified directory
	void FSHost::PrintDirectoryContents(std::string path)
	{
		if (!IODirectoryExists(path)) { return; }

		std::vector<DirectoryEntry> dirs = IOGetDirectories(path);
		std::vector<FileEntry> files = IOGetFiles(path);

		if (dirs.size() > 0)
		{
			for (size_t i = 0; i < dirs.size(); i++)
			{
				Console::Write("-- ");
				if (strlen(dirs.at(i).Name) > 0) { Console::WriteLine(dirs.at(i).Name, ConsoleColor::Green); }
			} 
		}

		if (files.size() > 0)
		{
			for (size_t i = 0; i < files.size(); i++)
			{
				if (strlen(files.at(i).Name) > 0) { Console::Write(files.at(i).Name, ConsoleColor::White); }
				Console::Write("    ");
				Console::Write("SIZE = ");
				Console::Write(StringUtil::FromDecimal(files.at(i).Size), ConsoleColor::DarkGray);
				Console::Write("    ");
				Console::WriteLine("bytes", ConsoleColor::DarkGray);
			}
		}

		if (dirs.size() == 0 && files.size() == 0) { Console::WriteLine("No files found."); return; }
	}

	// mount the file system
	void FSHost::Mount()
	{
		// read super block from disk
		ReadSuperBlock();

		// initialize table data arrays
		InitTableArrays();

		// read block table from disk
		ReadBlockTable();
		
		// read entry table
		ReadEntryTable();

		// set flag and print message
		Mounted = true;
		Debugger::ThrowOK("Mounted disk image");
	}

	// unmount the file system
	void FSHost::Unmount()
	{
		uint32_t block_table_size = SuperBlock.BlockTable.SizeInBytes;
		uint32_t entry_table_size = SuperBlock.EntryTable.SizeInBytes;

		if (BlockTableData != nullptr && block_table_size > 0) { memset(BlockTableData, 0, block_table_size); }
		if (EntryTableData != nullptr && entry_table_size > 0) { memset(EntryTableData, 0, entry_table_size); }

		memset(&SuperBlock, 0, sizeof(SuperBlockHeader));

		// set flag and print message
		Mounted = false;
		Debugger::ThrowOK("Un-mounted disk image");
	}
	
	// check if file system is mounted
	bool FSHost::IsMounted() { return Mounted; }

	// format the disk with manually specified size in bytes
	void FSHost::Format(uint32_t size)
	{
		// starting message
		Debugger::ThrowMessage("Started formatting %d MB disk", size / 1024 / 1024, MsgType::System);

		// make sure drive is unmounted before formatting
		Unmount();

		// wipe data from disk
		Wipe();

		// create new super block header and write to disk
		CreateSuperBlock(size);
		WriteSuperBlock();

		// initialize table data arrys
		InitTableArrays();

		// create new block table
		CreateBlockTable();

		// create new entry table
		CreateEntryTable();

		// create new root directory
		CreateRootDirectory();

		// write updated data to disk
		WriteTables();

		// finished message
		Debugger::ThrowOK("Successfully formatted %d MB disk image", size / 1024 / 1024);

		PrintDiskInformation();
		PrintBlockTable();

		// re-mount disk
		Mount();
	}

	// fill entire disk image with zeros
	void FSHost::Wipe()
	{
		// clear disk
		Console::Write("[");
		Console::Write("  >>  ", ConsoleColor::Cyan);
		Console::Write("] ");
		Console::Write("Wiping disk");
		uint8_t* data = (uint8_t*)malloc(FS_SIZE_SECTOR);
		if (data != nullptr)
		{
			memset(data, 0, FS_SIZE_SECTOR);
			for (uint32_t i = 0; i < ATADisk::GetSize() / FS_SIZE_SECTOR; i++)
			{
				ATADisk::Write(data, i);
				if (i % ((ATADisk::GetSize() / FS_SIZE_SECTOR) / 16) == 0) { Console::Write("."); }
			}
			Console::Write("\n");
			Debugger::ThrowOK("Finished wiping disk");
		}
		else { Debugger::ThrowError("Unable to wipe disk"); return; }
	}

	// write block and entry table to disk
	void FSHost::WriteTables()
	{
		WriteSuperBlock();
		WriteBlockTable();
		WriteEntryTable();
	}

	// create new super block header
	void FSHost::CreateSuperBlock(uint32_t disk_size)
	{
		// starting message
		Debugger::ThrowMessage("Creating new super block", MsgType::System);

		// set disk properties
		SuperBlock.SectorCount = disk_size / FS_SIZE_SECTOR;
		SuperBlock.BytesPerSector = FS_SIZE_SECTOR;

		// set block table properties
		SuperBlock.BlockTable.MaxEntries  = 16384;
		SuperBlock.BlockTable.StartSector = FS_SECTOR_BLOCK_TABLE;
		SuperBlock.BlockTable.SizeInBytes = (SuperBlock.BlockTable.MaxEntries * FS_SIZE_BLOCK_ENTRY);
		SuperBlock.BlockTable.SectorCount = SuperBlock.BlockTable.SizeInBytes / FS_SIZE_SECTOR;
		SuperBlock.BlockTable.EndSector = SuperBlock.BlockTable.StartSector + SuperBlock.BlockTable.SectorCount;

		// set file entry table properties
		SuperBlock.EntryTable.MaxEntries = 16384;
		SuperBlock.EntryTable.StartSector = SuperBlock.BlockTable.EndSector;
		SuperBlock.EntryTable.SizeInBytes = (SuperBlock.EntryTable.MaxEntries * FS_SIZE_FILE_ENTRY);
		SuperBlock.EntryTable.SectorCount = SuperBlock.EntryTable.SizeInBytes / FS_SIZE_SECTOR;
		SuperBlock.EntryTable.EndSector = SuperBlock.EntryTable.StartSector + SuperBlock.EntryTable.SectorCount;

		// set physical data properties
		SuperBlock.Data.MaxEntries = 0;
		SuperBlock.Data.StartSector = SuperBlock.EntryTable.EndSector;
		SuperBlock.Data.SizeInBytes = (SuperBlock.SectorCount - SuperBlock.Data.StartSector) * FS_SIZE_SECTOR;
		SuperBlock.Data.SectorCount = SuperBlock.Data.SizeInBytes / FS_SIZE_SECTOR;
		SuperBlock.Data.EndSector = SuperBlock.Data.StartSector + SuperBlock.Data.SectorCount;

		// clear reserved region
		memset(&SuperBlock.Reserved, 0, 444);

		// finished message
		Debugger::ThrowOK("Finished creating super block");
	}

	void FSHost::CopySuperBlock(SuperBlockHeader* dest, SuperBlockHeader* src)
	{
		dest->SectorCount = src->SectorCount;
		dest->BytesPerSector = src->BytesPerSector;
		dest->BlockTable.EndSector = src->BlockTable.EndSector;
		dest->BlockTable.MaxEntries = src->BlockTable.MaxEntries;
		dest->BlockTable.SectorCount = src->BlockTable.SectorCount;
		dest->BlockTable.SizeInBytes = src->BlockTable.SizeInBytes;
		dest->BlockTable.StartSector = src->BlockTable.StartSector;
		dest->EntryTable.EndSector = src->EntryTable.EndSector;
		dest->EntryTable.MaxEntries = src->EntryTable.MaxEntries;
		dest->EntryTable.SectorCount = src->EntryTable.SectorCount;
		dest->EntryTable.SizeInBytes = src->EntryTable.SizeInBytes;
		dest->EntryTable.StartSector = src->EntryTable.StartSector;
		dest->Data.EndSector = src->Data.EndSector;
		dest->Data.MaxEntries = src->Data.MaxEntries;
		dest->Data.SectorCount = src->Data.SectorCount;
		dest->Data.SizeInBytes = src->Data.SizeInBytes;
		dest->Data.StartSector = src->Data.StartSector;
		memcpy(dest->Reserved, src->Reserved, 444);
	}
	
	// read super block header from disk
	void FSHost::ReadSuperBlock()
	{
		// starting message
		Debugger::ThrowMessage("Reading super block from disk", MsgType::System);

		// create temporary data array for storing sector data
		uint8_t* data = ATADisk::Read(FS_SECTOR_SUPER);

		// successfully allocated memory
		if (data != nullptr)
		{
			// copy super block from data array
			SuperBlockHeader* super = (SuperBlockHeader*)data;
			CopySuperBlock(&SuperBlock, super);

			// free memory and message
			free(data);
			Debugger::ThrowOK("Finished reading super block from disk");
		}
		// unable to allocate memory
		else { Debugger::ThrowError("Error allocating memory while reading super block"); }
	}

	// write super block header to disk
	void FSHost::WriteSuperBlock()
	{
		// starting message
		Debugger::ThrowMessage("Writing super block to disk", MsgType::System);

		// create temporary data array for storing sector data
		uint8_t* data = (uint8_t*)malloc(FS_SIZE_SECTOR);

		// successfully allocated memory
		if (data != nullptr)
		{
			// copy super block to data array
			SuperBlockHeader* super = (SuperBlockHeader*)data;
			memset(data, 0, FS_SIZE_SECTOR);
			CopySuperBlock(super, &SuperBlock);

			// write super block sector to disk
			ATADisk::Write(data, FS_SECTOR_SUPER);
			free(data);

			// finished message
			Debugger::ThrowOK("Finished writing super block to disk");
		}
		// unable to allocate memory
		else { Debugger::ThrowError("Error allocating memory while writing super block"); }
	}

	// create new block table - must be done after super block is created
	void FSHost::CreateBlockTable()
	{
		// validate data properties
		if (SuperBlock.Data.StartSector == 0) { Debugger::ThrowError("Invalid start sector while reading block table"); return; }
		if (SuperBlock.Data.SectorCount == 0) { Debugger::ThrowError("Invalid sector count while reading block table"); return; }

		// starting message
		Debugger::ThrowMessage("Creating new block table", MsgType::System);

		// get data properties from super block
		uint32_t start = SuperBlock.Data.StartSector;
		uint32_t size  = SuperBlock.Data.SectorCount;
		uint8_t  state = FS_STATE_FREE;

		// create new mass data block entry
		BlockEntry* entry = CreateBlockEntry(start, size, state);
	}

	// read block table from disk
	void FSHost::ReadBlockTable()
	{
		// validate block table
		if (!IsBlockTableValid()) { Debugger::ThrowError("Unable to validate block table before reading from disk"); return; }

		// starting message
		Debugger::ThrowMessage("Reading block table from disk", MsgType::System);

		// loop through block table sectors
		uint8_t* data = nullptr;
		uint64_t table_offset = 0;
		for (uint32_t sec = 0; sec < SuperBlock.BlockTable.SectorCount; sec++)
		{
			// read sector from disk
			data = ATADisk::Read(SuperBlock.BlockTable.StartSector + sec);
			if (data == nullptr) { Debugger::ThrowError("Unable to allocate memory while reading block table from disk"); return; }

			// copy data from sector to table array
			memcpy((void*)(BlockTableData + table_offset), data, FS_SIZE_SECTOR);
			table_offset += FS_SIZE_SECTOR;
		}

		// free data
		if (data != nullptr) { free(data); }

		// finished message
		Debugger::ThrowOK("Finished reading block table from disk");
	}

	// write block table to disk
	void FSHost::WriteBlockTable()
	{
		// validate block table
		if (!IsBlockTableValid()) { Debugger::ThrowError("Unable to validate block table before writing to disk"); return; }

		Debugger::ThrowMessage("Writing block table to disk", MsgType::System);

		// loop through block table sectors
		uint8_t* data = nullptr;
		uint64_t table_offset = 0;
		for (uint32_t sec = 0; sec < SuperBlock.BlockTable.SectorCount; sec++)
		{
			// create temporary array
			data = (uint8_t*)malloc(FS_SIZE_SECTOR);
			if (data == nullptr) { Debugger::ThrowError("Unable to allocate memory while writing block table to disk"); return; }

			// copy data to temporary sector
			memset(data, 0, FS_SIZE_SECTOR);
			memcpy(data, (void*)(BlockTableData + table_offset), FS_SIZE_SECTOR);
			table_offset += FS_SIZE_SECTOR;
			
			// write sector to disk
			ATADisk::Write(data, SuperBlock.BlockTable.StartSector + sec);

			// free data
			free(data);
		}

		Debugger::ThrowOK("Finished writing block table to disk");
	}

	// read data from data block
	uint8_t* FSHost::ReadDataBlock(uint32_t sector, uint32_t count)
	{
		if (!IsBlockTableValid()) { Debugger::ThrowError("Error validating block table while reading block entry"); return nullptr; }
		bool located = false;

		// locate through blocks
		for (uint64_t i = 0; i < SuperBlock.BlockTable.MaxEntries; i++)
		{
			// read entry from table
			BlockEntry* entry = (BlockEntry*)(BlockTableData + (i * (uint64_t)FS_SIZE_BLOCK_ENTRY));

			// found match
			if (entry->Sector > 0 && entry->Sector == sector && entry->Count == count && entry->State == FS_STATE_USED)
			{
				located = true;
				break;
			}
		}

		// if located, load block to array
		if (located)
		{
			uint8_t* output = (uint8_t*)malloc(count * (uint64_t)FS_SIZE_SECTOR);
			if (output == nullptr) { Debugger::ThrowError("Unable to allocate memory while reading data block"); return nullptr; }

			uint32_t index = 0;
			for (uint32_t sec = sector; sec < sector + count; sec++)
			{
				uint8_t* sec_data = ATADisk::Read(sec);
				if (sec_data != nullptr && output != nullptr)
				{
					for (size_t i = 0; i < FS_SIZE_SECTOR; i++)
					{
						output[index] = (uint8_t)sec_data[i];
						index++;
					}
					free(sec_data);
				}
				else { Debugger::ThrowError("Unable to allocate memory while reading sector during data block iteration"); return nullptr; }
			}

			return output;
		}
		// unable to locate data block
		else { Debugger::ThrowError("Unable to locate data block"); return nullptr; }
	}

	// allocate new block and return result
	BlockEntry* FSHost::AllocateBlock(uint32_t sectors)
	{
		// validate block table
		if (!IsBlockTableValid()) { return nullptr; }

		// validate size
		if (sectors == 0) { Debugger::ThrowError("Allocation of 0 sectors not permitted"); return nullptr; }

		// get next available entry for allocation
		BlockEntry* entry = GetAvailableBlock(sectors);
		
		// validate entry
		if (entry == nullptr)			   { Debugger::ThrowError("Unable to allocate %d sectors of disk", sectors); return nullptr; }
		if (entry->Sector == 0)			   { Debugger::ThrowError("Unexpected sector value while reading next available block"); return nullptr; }
		if (entry->Count == 0)			   { Debugger::ThrowError("Unexpected sector count while reading next available block"); return nullptr; }

		// print message and return success
		Debugger::ThrowOK("Allocated %d sectors of disk", sectors);
		return entry;
	}

	// free already allocated block and return result
	bool FSHost::FreeBlock(uint32_t sector, uint32_t count)
	{
		// validate block table
		if (!IsBlockTableValid()) { return false; }

		// validate block properties
		if (sector == 0 || count == 0) { return false; }

		// locate perfectly sized block to use
		for (uint64_t i = 0; i < SuperBlock.BlockTable.MaxEntries; i++)
		{
			// read entry from table
			BlockEntry* entry = (BlockEntry*)(BlockTableData + (i * (uint64_t)FS_SIZE_BLOCK_ENTRY));

			// found matching entry
			if (entry->Sector > 0 && entry->Sector == sector && entry->Count == count && entry->State == FS_STATE_USED)
			{
				// set entry state
				entry->State = FS_STATE_FREE;
				uint8_t* empty = (uint8_t*)malloc(FS_SIZE_SECTOR);
				// clear data from block
				if (empty != nullptr)
				{
					memset(empty, 0, FS_SIZE_SECTOR);
					for (size_t i = 0; i < count; i++) { ATADisk::Write(empty, entry->Sector + i); }
				}
				// merge available blocks
				MergeAvailableBlocks();
				// print message and return success
				Debugger::ThrowOK("Unallocated %d sectors of disk", count);
				return true;
			}
		}

		// unable to free block
		Debugger::ThrowError("Unable to free block (sec: " + StringUtil::FromDecimal(sector) + ", count: " + StringUtil::FromDecimal(count) + ")");
		return false;
	}

	// get next available block for allocation
	BlockEntry* FSHost::GetAvailableBlock(uint32_t sectors)
	{
		// validate block table
		if (!IsBlockTableValid()) { return nullptr; }

		// validate sector count
		if (sectors == 0) { return nullptr; }

		// locate perfectly sized block to use
		for (uint64_t i = 0; i < SuperBlock.BlockTable.MaxEntries; i++)
		{
			// read entry from table
			BlockEntry* entry = (BlockEntry*)(BlockTableData + (i * (uint64_t)FS_SIZE_BLOCK_ENTRY));

			// found usable block
			if (entry->Sector > 0 && entry->Count == sectors && entry->State == FS_STATE_FREE)
			{
				// set state and return block
				entry->State = FS_STATE_USED;
				return entry;
			}
		}

		// locate larger block and split it up
		for (uint64_t i = 0; i < SuperBlock.BlockTable.MaxEntries; i++)
		{
			// read entry from table
			BlockEntry* entry = (BlockEntry*)(BlockTableData + (i * (uint64_t)FS_SIZE_BLOCK_ENTRY));

			// found usable block
			if (entry->Sector > 0 && entry->Count >= sectors && entry->State == FS_STATE_FREE)
			{
				// create new entry
				BlockEntry* new_entry = CreateBlockEntry(entry->Sector, sectors, FS_STATE_USED);
				// split block by amount needed
				entry->Sector += sectors;
				entry->Count  -= sectors;
				entry->State   = FS_STATE_FREE;
				// return block
				return new_entry;
			}
		}

		// unable to locate available block
		Debugger::ThrowError("Unable to locate available data block");
		return nullptr;
	}

	// merge all available block
	void FSHost::MergeAvailableBlocks()
	{
		// loop through entries and combine neighbours
		for (uint64_t i = 0; i < SuperBlock.BlockTable.MaxEntries; i++)
		{
			// read entries from table
			BlockEntry* entry = (BlockEntry*)(BlockTableData + (i * (uint64_t)FS_SIZE_BLOCK_ENTRY));

			// found available block
			if (entry->Sector > 0 && entry->Count > 0 && entry->State == FS_STATE_FREE)
			{
				// get neighbouring block
				BlockEntry* next  = GetNearestBlock(entry);

				// check if neighbouring block is valid
				if (next != nullptr)
				{
					// add neighbouring block
					entry->Sector -= next->Count;
					entry->Count += next->Count;
					entry->State = FS_STATE_FREE;
					DeleteBlockEntry(next);
				}
			}
		}

		// loop through entires
		uint32_t free_sector = 0;
		uint32_t free_count  = 0;
		for (uint64_t i = 0; i < SuperBlock.BlockTable.MaxEntries; i++)
		{
			// read entries from table
			BlockEntry* entry = (BlockEntry*)(BlockTableData + (i * (uint64_t)FS_SIZE_BLOCK_ENTRY));

			// found empty block
			if (entry->Sector > 0 && entry->Count > 0 && entry->State == FS_STATE_FREE)
			{
				// add properties to total
				free_count += entry->Count;
				// delete old entry
				if (entry->Sector > free_sector) { free_sector = entry->Sector; }
				DeleteBlockEntry(entry);
			}
		}

		// create new free block
		BlockEntry* free_block = CreateBlockEntry(free_sector, free_count, FS_STATE_FREE);
		if (free_block == nullptr) { Debugger::ThrowError("Unexpected error while merging available blocks"); return; }

		// finished message
		Debugger::ThrowOK("Merged all available data blocks");
	}

	// get nearest block based on sector
	BlockEntry* FSHost::GetNearestBlock(BlockEntry* entry)
	{
		// validate entry
		if (entry == nullptr) { return nullptr; }
		if (entry->Sector == 0 || entry->Count == 0) { return nullptr; }

		// loop through block entries
		for (uint64_t i = 0; i < SuperBlock.BlockTable.MaxEntries; i++)
		{
			// read entry from table
			BlockEntry* temp = (BlockEntry*)(BlockTableData + (i * (uint64_t)FS_SIZE_BLOCK_ENTRY));

			// located neighbouring block
			if (temp != entry && entry->Sector + entry->Count == temp->Sector && temp->State == FS_STATE_FREE) { return temp; }
			if (temp != entry && temp->Sector + temp->Count == entry->Sector && temp->State == FS_STATE_FREE) { return temp; }

		}

		// unable to locate nearest block
		return nullptr;
	}

	// create new data block entry
	BlockEntry* FSHost::CreateBlockEntry(uint32_t sector, uint32_t count, uint8_t state)
	{
		// validate block table
		if (!IsBlockTableValid()) { return nullptr; }

		// loop through block table
		for (uint64_t i = 0; i < SuperBlock.BlockTable.MaxEntries; i++)
		{
			// read entry from table
			BlockEntry* entry = (BlockEntry*)(BlockTableData + (i * (uint64_t)FS_SIZE_BLOCK_ENTRY));

			// check if entry is unused
			if (entry->Sector == 0 && entry->Count == 0 && entry->State == FS_STATE_FREE)
			{
				entry->Sector = sector;
				entry->Count  = count;
				entry->State  = state;
				//Debugger::ThrowOK("Created new block entry(sec: " + StringUtil::FromDecimal(sector) + ", count: " + StringUtil::FromDecimal(entry->Count) + ", state = " + (entry->State == 1 ? "USED)" : "FREE)"));
				return entry;
			}
		}
		return nullptr;
	}

	// delete existing block entry
	bool FSHost::DeleteBlockEntry(BlockEntry* entry)
	{
		// validate block table
		if (!IsBlockTableValid()) { return false; }

		// validate entry
		if (entry == nullptr) { return false; }

		// loop through entries
		for (uint64_t i = 0; i < SuperBlock.BlockTable.MaxEntries; i++)
		{
			// read entry from table
			BlockEntry* temp = (BlockEntry*)(BlockTableData + (i * (uint64_t)FS_SIZE_BLOCK_ENTRY));

			if (temp == entry && temp->Sector == entry->Sector && temp->Count == entry->Count && temp->State == entry->State)
			{
				// reset block state
				temp->Sector = 0;
				temp->Count = 0;
				temp->State == FS_STATE_FREE;
				// return success
				//Debugger::ThrowOK("Deleted block entry , (uint32_t)entry);
				return true;
			}
		}

		// unable to locate block 
		Debugger::ThrowError("Unable to locate block entry %XX for deletion", (uint64_t)entry);
		return false;
	}

	// get block entry based on properties
	BlockEntry* FSHost::GetBlockEntry(uint32_t sector, uint32_t count, uint8_t state)
	{
		// validate properties
		if (sector == 0 || count == 0) { return nullptr; }

		// loop through block table
		for (uint64_t i = 0; i < SuperBlock.BlockTable.MaxEntries; i++)
		{
			// read entry from table
			BlockEntry* entry = (BlockEntry*)(BlockTableData + (i * (uint64_t)FS_SIZE_BLOCK_ENTRY));
			if (entry->Sector > 0 && entry->Sector == sector && entry->Count == count && entry->State == state) { return entry; }
		}

		// unable to locate entry
		return nullptr;
	}

	// check if block table is loaded and valid
	bool FSHost::IsBlockTableValid()
	{
		if (SuperBlock.BlockTable.StartSector == 0) { return false; }
		if (SuperBlock.BlockTable.EndSector == 0)   { return false; }
		if (SuperBlock.BlockTable.SectorCount == 0) { return false; }
		if (SuperBlock.BlockTable.SizeInBytes == 0) { return false; }
		if (SuperBlock.BlockTable.MaxEntries == 0)  { return false; }
		return true;
	}

	// check if entry table is loaded and valid
	bool FSHost::IsEntryTableValid()
	{
		if (SuperBlock.EntryTable.StartSector == 0) { return false; }
		if (SuperBlock.EntryTable.EndSector == 0)   { return false; }
		if (SuperBlock.EntryTable.SectorCount == 0) { return false; }
		if (SuperBlock.EntryTable.SizeInBytes == 0) { return false; }
		if (SuperBlock.EntryTable.MaxEntries == 0)  { return false; }
		return true;
	}

	// calculate total amount of block entries
	void FSHost::CalculateBlockCount()
	{
		// validate block table
		if (!IsBlockTableValid()) { BlockCount = 0; return; }

		// loop through block table
		uint32_t count = 0;
		for (uint64_t i = 0; i < SuperBlock.BlockTable.MaxEntries; i++)
		{
			// read entry from table
			BlockEntry* entry = (BlockEntry*)(BlockTableData + (i * (uint64_t)FS_SIZE_BLOCK_ENTRY));

			// located valid entry, increment count
			if (entry->Sector > 0) { count++; }
		}
		BlockCount = count;
	}

	// calulate total amount of file entries
	void FSHost::CalculateEntryCount()
	{
		// validate entry table
		if (!IsEntryTableValid()) { EntryCount = 0; return; }

		// loop through entry table
		uint32_t count = 0;
		for (uint64_t i = 0; i < SuperBlock.EntryTable.MaxEntries; i++)
		{
			// read entry from table
			DirectoryEntry* entry = (DirectoryEntry*)(EntryTableData + (i * (uint64_t)FS_SIZE_FILE_ENTRY));

			// located valid entry, increment count
			if (entry->Type != EntryType::Null) { count++; }
		}
		EntryCount = count;
	}

	// create new entry table
	void FSHost::CreateEntryTable()
	{
		// validate entry table size
		if (SuperBlock.EntryTable.SizeInBytes == 0) { Debugger::ThrowError("Invalid size while creating new entry table"); return; }

		// starting message
		Debugger::ThrowMessage("Creating new entry table", MsgType::System);

		// clear entry table
		memset(EntryTableData, 0, SuperBlock.EntryTable.SizeInBytes);

		// finished message
		Debugger::ThrowOK("Created new entry table");
	}

	// create new root directory entry
	void FSHost::CreateRootDirectory()
	{
		// set properties
		memset(RootDirectory.Name, 0, 58);
		memcpy(RootDirectory.Name, (void*)"DISK\0", 5);
		RootDirectory.Type = EntryType::Directory;
		RootDirectory.Status = 1;
		RootDirectory.ParentIndex = 0;

		// create directory
		DirectoryEntry* root = CreateDirectoryEntry(RootDirectory);

		// finished message depending on result
		if (root != nullptr) { Debugger::ThrowOK("Finished creating new root directory"); }
		else { Debugger::ThrowError("Unable to create new directory"); }
	}

	// read entry table from disk
	void FSHost::ReadEntryTable()
	{
		// validate entry table
		if (!IsEntryTableValid()) { Debugger::ThrowError("Unable to validate entry table before reading from disk"); return; }

		// starting message
		Debugger::ThrowMessage("Reading entry table from disk", MsgType::System);

		// loop through entry table sectors
		uint8_t* data = nullptr;
		uint32_t table_offset = 0;
		for (uint32_t sec = 0; sec < SuperBlock.EntryTable.SectorCount; sec++)
		{
			// read sector from disk
			if (data != nullptr) { free(data); }
			data = ATADisk::Read(SuperBlock.EntryTable.StartSector + sec);
			if (data == nullptr) { Debugger::ThrowError("Unable to allocate memory while reading entry table from disk"); return; }

			// copy data from sector to table array
			if (table_offset < SuperBlock.EntryTable.SizeInBytes) { memcpy((void*)(EntryTableData + table_offset), data, FS_SIZE_SECTOR); }
			else { Debugger::ThrowError("Memory write violation while reading entry table"); return; }
			table_offset += FS_SIZE_SECTOR;
		}

		// free data
		if (data != nullptr) { free(data); }

		// finished message
		Debugger::ThrowOK("Finished reading entry table from disk");
	}

	// write entry table to disk
	void FSHost::WriteEntryTable()
	{
		// validate entry table
		if (!IsEntryTableValid()) { Debugger::ThrowError("Unable to validate entry table before writing to disk"); return; }

		Debugger::ThrowMessage("Writing entry table to disk", MsgType::System);

		// loop through entry table sectors
		uint8_t* data = nullptr;
		uint32_t table_offset = 0;
		for (uint32_t sec = 0; sec < SuperBlock.EntryTable.SectorCount; sec++)
		{
			// create temporary array
			data = (uint8_t*)malloc(FS_SIZE_SECTOR);
			if (data == nullptr) { Debugger::ThrowError("Unable to allocate memory while writing entry table to disk"); return; }

			// copy data to temporary sector
			memset(data, 0, FS_SIZE_SECTOR);
			memcpy(data, (void*)(EntryTableData + table_offset), FS_SIZE_SECTOR);
			table_offset += FS_SIZE_SECTOR;

			// write sector to disk
			ATADisk::Write(data, SuperBlock.EntryTable.StartSector + sec);

			// free data
			free(data);
		}

		Debugger::ThrowOK("Finished writing entry table to disk");
	}

	// create new directory entry
	DirectoryEntry* FSHost::CreateDirectoryEntry(DirectoryEntry src)
	{
		// validate source directory
		if (src.Type != EntryType::Directory) { return nullptr; }

		// get available entry in table
		DirectoryEntry* dest = (DirectoryEntry*)GetAvailableEntry();

		// validate entry in table
		if (dest == nullptr) { Debugger::ThrowError("Unable to locate available entry while creating directory"); return nullptr; }

		// copy from source to destination
		memset(dest, 0, FS_SIZE_FILE_ENTRY);
		CopyFileEntry(dest, &src);

		// print message and return entry
		Debugger::ThrowOK("Created new directory: " + std::string(dest->Name));
		return dest;
	}

	// create new file entry
	FileEntry* FSHost::CreateFileEntry(FileEntry src)
	{
		// validate source directory
		if (src.Type != EntryType::File) { return nullptr; }

		// get available entry in table
		FileEntry* dest = (FileEntry*)GetAvailableEntry();

		// validate entry in table
		if (dest == nullptr) { Debugger::ThrowError("Unable to locate available entry while creating file"); return nullptr; }

		// get sector amount required for file
		uint32_t sectors = src.Size / FS_SIZE_SECTOR;
		if (sectors < 1) { sectors = 1; }
		sectors++;

		// allocate data block to file
		BlockEntry* block = AllocateBlock(sectors);

		// validate data block
		if (block == nullptr) { Debugger::ThrowError("Unable to allocate required data block while creating file"); return nullptr; }

		// set file properties from block properties
		src.StartSector = block->Sector;
		src.SectorCount = block->Count;

		// copy from source to destination
		CopyFileEntry(dest, &src);

		// print message and return entry
		Debugger::ThrowOK("Created new file: " + std::string(dest->Name));
		return dest;
	}

	// copy file or directory from specified locations;
	bool FSHost::CopyFileEntry(void* dest, void* src)
	{
		// validate destination and source
		if (dest == nullptr) { Debugger::ThrowError("Null destination while copying file entry"); return false; }
		if (src == nullptr)  { Debugger::ThrowError("Null source while copying file entry"); return false; }

		// copy data
		memcpy(dest, src, FS_SIZE_FILE_ENTRY);
        return true;
	}

	// delete file entry
	bool FSHost::DeleteFileEntry(void* entry)
	{
		FileEntry* centry = (FileEntry*)entry;

		// validate block table
		if (!IsBlockTableValid()) { return false; }

		// validate entry
		if (centry == nullptr) { return false; }

		// loop through entries
		for (uint64_t i = 0; i < SuperBlock.EntryTable.MaxEntries; i++)
		{
			// read entry from table
			FileEntry* temp = (FileEntry*)(EntryTableData + (i * (uint64_t)FS_SIZE_FILE_ENTRY));

			if (temp == entry && !strcmp(temp->Name, centry->Name) && temp->Type == EntryType::File)
			{
				// clear data and return success
				memset(temp, 0, FS_SIZE_FILE_ENTRY);
				return true;
			}
		}

		// unable to locate block 
		Debugger::ThrowError("Unable to delete file entry");
		return false;
	}

	// get next available file entry
	void* FSHost::GetAvailableEntry()
	{
		// loop through entries
		for (size_t i = 0; i < SuperBlock.EntryTable.SizeInBytes; i += FS_SIZE_FILE_ENTRY)
		{
			// get entry
			FileEntry* entry = (FileEntry*)(EntryTableData + i);

			// entry is unused
			if (entry->Type == EntryType::Null) { return entry; }
		}
		
		// unable to locate available entry
		return nullptr;
	}

	// get parent directory from specified path
	DirectoryEntry* FSHost::GetParentFromPath(std::string path)
	{	
		// split path into peices
		std::vector<std::string> args = StringUtil::Split(path, '/');

		// check if parent is root
		if (args.size() == 1 && path.at(0) == '/') { return (DirectoryEntry*)EntryTableData; }

		// loop through parts of directory
		if (args.size() > 1)
		{
			int32_t index = 0;
			int32_t p = 0;
			void* output = nullptr;
				
			for (uint32_t arg = 0; arg < args.size() - 1; arg++)
			{
				output = nullptr;
				index = 0;
				for (size_t i = 0; i < SuperBlock.EntryTable.SizeInBytes; i += FS_SIZE_FILE_ENTRY)
				{
					// get entry
					FileEntry* entry = (FileEntry*)(EntryTableData + i);

					// found match for current piece of path
					if (entry->Type != EntryType::Null && entry->ParentIndex == p && !strcmp(entry->Name, args.at(arg).c_str()))
					{
						p = index;
						output = entry;
					}

					// increment index
					index++;
				}
			}

			return (DirectoryEntry*)output;
		}	

		 // unable to locate parent directory
		return nullptr;
	}

	// attempt to get file entry by path
	FileEntry* FSHost::GetFileByName(std::string path)
	{
		// validate path
		if (path.c_str() == nullptr || path.length() == 0) { return nullptr; }

		// attempt to fetch parent of file
		DirectoryEntry* parent = GetParentFromPath(path);

		// validate parent
		if (parent == nullptr) { Debugger::ThrowError("Unable to get file by name"); return nullptr; }

		// fetch file name
		std::vector<std::string> args = StringUtil::Split(path, '/');
		if (args.size() == 0) { Debugger::ThrowError("Unable to parse path while getting file by name"); return nullptr; }
		std::string filename = args.at(args.size() - 1);

		// get parent index
		uint32_t parent_index = GetFileIndex(parent);

		// locate file in entry table
		for (size_t i = 0; i < SuperBlock.EntryTable.SizeInBytes; i += FS_SIZE_FILE_ENTRY)
		{
			// get entry
			FileEntry* entry = (FileEntry*)(EntryTableData + i);

			// found file in entry table
			if (entry->ParentIndex == parent_index && StringUtil::Equals(filename, entry->Name) && entry->Type == EntryType::File)
			{
				return entry;
			}
		}

		// unable to locate file in entry table
		return nullptr;
	}

	// attempt to get directory entry by path
	DirectoryEntry* FSHost::GetDirectoryByName(std::string path)
	{
		// validate path
		if (path.c_str() == nullptr || path.length() == 0) { return nullptr; }

		// attempt to fetch parent of directory
		DirectoryEntry* parent = GetParentFromPath(path);

		// validate parent
		if (StringUtil::Equals(path, "/")) { return (DirectoryEntry*)EntryTableData; }
		if (parent == nullptr) { Debugger::ThrowError("Unable to get directory by name"); return nullptr; }

		// remove ending slash if exists
		if (path.length() > 1 && path.at(path.size() - 1) == '/') { path = path.substr(0, path.size() - 1); }

		// fetch file name
		std::vector<std::string> args = StringUtil::Split(path, '/');
		if (args.size() == 0) { Debugger::ThrowError("Unable to parse path while getting directory by name"); return nullptr; }
		std::string dirname = args.at(args.size() - 1);

		// get parent index
		uint32_t parent_index = GetFileIndex(parent);

		// locate file in entry table
		for (size_t i = 0; i < SuperBlock.EntryTable.SizeInBytes; i += FS_SIZE_FILE_ENTRY)
		{
			// get entry
			DirectoryEntry* entry = (DirectoryEntry*)(EntryTableData + i);

			// found file in entry table
			if (entry->ParentIndex == parent_index && StringUtil::Equals(dirname, entry->Name) && entry->Type == EntryType::Directory)
			{
				return entry;
			}
		}

		// unable to locate directory in entry table
		return nullptr;
	}

	// get entry index from entry table
	int32_t FSHost::GetFileIndex(void* src)
	{
		// validate source
		if (src == nullptr) { return -1; }

		// loop through entries
		int32_t index = 0;
		for (size_t i = 0; i < SuperBlock.EntryTable.SizeInBytes; i += FS_SIZE_FILE_ENTRY)
		{
			// get entry
			FileEntry* entry = (FileEntry*)(EntryTableData + i);

			if (entry == src) { return index; }
			index++;
		}

		// unable to get index of entry
		return -1;
	}

	// get index of entry in entry table
	void* FSHost::GetEntryAtIndex(int32_t index)
	{
		// loop through entries
		int32_t x  = 0;
		for (size_t i = 0; i < SuperBlock.EntryTable.SizeInBytes; i += FS_SIZE_FILE_ENTRY)
		{
			// get entry
			FileEntry* entry = (FileEntry*)(EntryTableData + i);

			if (x == index) { return entry; }
			x++;
		}

		// unable to locate entry at index
		return nullptr;
	}

	// -----------------------------------------------------------------------------------

	// open a file for reading
	FileEntry FSHost::IOOpenFile(std::string path)
	{
		if (!IOFileExists(path)) { Debugger::ThrowError("Unable to locate file " + path); return FileEntry {}; }

		FileEntry* fileptr = GetFileByName(path);
		if (fileptr == nullptr) { Debugger::ThrowError("Unable to get file by name while reading"); return FileEntry {}; }
		if (fileptr->Type != EntryType::File) { Debugger::ThrowError("Invalid type while reading file"); return FileEntry {}; }

		uint8_t* block_data = ReadDataBlock(fileptr->StartSector, fileptr->SectorCount);
		if (block_data == nullptr) { Debugger::ThrowError("Unable to read data from file"); return FileEntry {}; }
		
		FileEntry file;
		memcpy(file.Name, fileptr->Name, 38);
		file.Type = fileptr->Type;
		file.Status = fileptr->Status;
		file.ParentIndex = fileptr->ParentIndex;
		file.Size = fileptr->Size;
		file.StartSector = fileptr->StartSector;
		file.SectorCount = fileptr->SectorCount;
		file.Data = block_data;
		return file;
	}

	// import a file from host machine onto disk image
	FileEntry FSHost::IOImportFile(std::string dest, std::string src)
	{
		// check if desination file already exists
		if (IOFileExists(dest)) { Debugger::ThrowError("File already exists"); return FileEntry {}; }

		// attempt to load file
		std::ifstream file(src.c_str());
		if (file.good())
		{
			// load file
			FILE* fileptr = fopen(src.c_str(), "rb");
			if (fileptr != nullptr)
			{
				// read data from host machine
				fseek(fileptr, 0, SEEK_END);
				long len = ftell(fileptr);
				rewind(fileptr);
				uint8_t* data = (uint8_t*)malloc(len);
				if (data == nullptr) { Debugger::ThrowError("Unexpected error while importing file from host"); return FileEntry{}; }
				fread(data, len, 1, fileptr);
				fclose(fileptr);	

				// create new file with data from host
				FileEntry file = IOCreateFile(dest, len, data);
				if (file.Type == EntryType::Null) { Debugger::ThrowError("Unable to create new file while importing file from host"); return FileEntry{}; }
				Debugger::ThrowOK("Sucessfully imported file " + dest + ", size = " + StringUtil::FromDecimal(len) + " bytes");
				return file;
			}
		}
		return FileEntry {};
	}

	// create a new blank file with at location with specified size
	FileEntry FSHost::IOCreateFile(std::string path, uint32_t size)
	{
		// validate arguments
		if (path.length() == 0 || size == 0) { return FileEntry {}; }

		// get parent directory
		DirectoryEntry* parent = GetParentFromPath(path);

		// validate parent
		if (parent == nullptr) { Debugger::ThrowError("Unable to locate parent directory while creating file"); return FileEntry {}; }

		std::vector<std::string> args = StringUtil::Split(path, '/');
		std::string filename;
		if (args.size() >= 1) { filename = args.at(args.size() - 1); }
		if (args.size() == 0 || filename.length() == 0) { Debugger::ThrowError("Invalid name while creating file"); return FileEntry{}; }
		if (args.size() == 1) { filename = path.substr(1, path.length() - 1); }
		while (filename.length() > 38) { filename = filename.substr(0, filename.length() - 1); }

		// set properties
		FileEntry file;
		memset(file.Name, 0, 38);
		memcpy(file.Name, filename.c_str(), filename.length());
		file.Type = EntryType::File;
		file.Size = size;
		file.ParentIndex = GetFileIndex(parent);
		file.Status = 0;

		// create file entry
		FileEntry* fileptr = CreateFileEntry(file);
		if (fileptr == nullptr) { Debugger::ThrowError("Unable to create file"); return FileEntry{}; }
		file.StartSector = fileptr->StartSector;
		file.SectorCount = fileptr->SectorCount;

		// return file entry copy
		return file;
	}

	// create a new file at location with specified size and data
	FileEntry FSHost::IOCreateFile(std::string path, uint32_t size, uint8_t* data)
	{
		// validate arguments
		if (path.length() == 0 || size == 0) { return FileEntry{}; }

		// get parent directory
		DirectoryEntry* parent = GetParentFromPath(path);

		// validate parent
		if (parent == nullptr) { Debugger::ThrowError("Unable to locate parent directory while creating file"); return FileEntry{}; }

		std::vector<std::string> args = StringUtil::Split(path, '/');
		std::string filename;
		if (args.size() >= 1) { filename = args.at(args.size() - 1); }
		if (args.size() == 0 || filename.length() == 0) { Debugger::ThrowError("Invalid name while creating file"); return FileEntry{}; }
		if (args.size() == 1) { filename = path.substr(1, path.length() - 1); }
		while (filename.length() > 38) { filename = filename.substr(0, filename.length() - 1); }

		// set properties
		FileEntry file;
		memset(file.Name, 0, 38);
		memcpy(file.Name, filename.c_str(), filename.length());
		file.Type = EntryType::File;
		file.Size = size;
		file.ParentIndex = GetFileIndex(parent);
		file.Status = 0;

		// create file entry
		FileEntry* fileptr = CreateFileEntry(file);
		if (fileptr == nullptr) { Debugger::ThrowError("Unable to create file"); return FileEntry {}; }
		file.StartSector = fileptr->StartSector;
		file.SectorCount = fileptr->SectorCount;

		// get data block
		BlockEntry* block = GetBlockEntry(fileptr->StartSector, fileptr->SectorCount, FS_STATE_USED);

		// copy data to data block
		uint8_t* temp = (uint8_t*)malloc(FS_SIZE_SECTOR);
		if (temp == nullptr || data == nullptr) { Debugger::ThrowError("Unable to allocate memory while creating file"); return FileEntry{}; }
		uint32_t total  = 0;

		// write data
		for (uint64_t sec = 0; sec < fileptr->SectorCount; sec++)
		{
			for (uint32_t i = 0; i < FS_SIZE_SECTOR; i++)
			{
				if (total < file.Size) { temp[i] = data[total]; }
				total++;
			}

			// write data to sector
			ATADisk::Write(temp, file.StartSector + sec);
		}

		return file;
	}

	// create a new directory at location
	DirectoryEntry FSHost::IOCreateDirectory(std::string path)
	{
		// validate arguments
		if (path.length() == 0) { return DirectoryEntry {}; }

		// get parent directory
		DirectoryEntry* parent = GetParentFromPath(path);

		// validate parent
		if (parent == nullptr) { Debugger::ThrowError("Unable to locate parent directory while creating file"); return DirectoryEntry {}; }

		// get directory name
		std::vector<std::string> args = StringUtil::Split(path, '/');
		std::string dirname;
		if (args.size() > 1) { dirname = args.at(args.size() - 1); }
		if (args.size() == 0) { Debugger::ThrowError("Invalid name while creating directory"); return DirectoryEntry {}; }
		if (path.length() > 0 && args.size() <= 1) { dirname = path.substr(1, path.length() - 1); }
		while (dirname.length() > 58) { dirname = dirname.substr(0, dirname.length() - 1); }

		// set properties
		DirectoryEntry dir;
		memset(dir.Name, 0, 58);
		memcpy(dir.Name, dirname.c_str(), dirname.length());
		dir.ParentIndex = GetFileIndex(parent);
		dir.Type = EntryType::Directory;
		dir.Status = 0;

		// create directory entry
		DirectoryEntry* dirptr = CreateDirectoryEntry(dir);
		if (dirptr == nullptr) { Debugger::ThrowError("Unable to create directory"); return DirectoryEntry{}; }
		
		// return directory entry copy
		return dir;
	}

	// check if file exists 
	bool FSHost::IOFileExists(std::string path)
	{
		// validate arguments
		if (path.length() == 0) { return false; }

		// get parent directory
		DirectoryEntry* parent = GetParentFromPath(path);
		uint32_t parent_index = GetFileIndex(parent);

		// validate parent
		if (parent == nullptr) { return false; }

		// get file name
		std::vector<std::string> args = StringUtil::Split(path, '/');
		std::string filename;
		if (args.size() >= 1) { filename = args.at(args.size() - 1); }
		if (args.size() == 0 || filename.length() == 0) { return false; }
		while (filename.length() > 38) { filename = filename.substr(0, filename.length() - 1); }

		// loop through entries and locate
		for (uint64_t i = 0; i < SuperBlock.EntryTable.SizeInBytes; i += FS_SIZE_FILE_ENTRY)
		{
			FileEntry* entry = (FileEntry*)(EntryTableData + i);

			// located entry
			if (entry->Type == EntryType::File && strlen(entry->Name) > 0 && StringUtil::Equals(filename, entry->Name) && entry->ParentIndex == parent_index && entry->Size > 0)
			{ return true; }
		}
		return false;
	}

	// check if directory exists
	bool FSHost::IODirectoryExists(std::string path)
	{
		// validate arguments
		if (path.length() == 0) { return false; }
		if (StringUtil::Equals(path, "/")) { return true; }

		// get parent directory
		DirectoryEntry* parent = GetParentFromPath(path);
		uint32_t parent_index = GetFileIndex(parent);

		// validate parent
		if (parent == nullptr) { return false; }

		// get directory name
		std::vector<std::string> args = StringUtil::Split(path, '/');
		std::string dirname;
		if (args.size() >= 1) { dirname = args.at(args.size() - 1); }
		if (args.size() == 0) { return false; }
		if (path.length() > 0 && args.size() <= 1) { dirname = path.substr(1, path.length() - 1); }
		while (dirname.length() > 58) { dirname = dirname.substr(0, dirname.length() - 1); }

		// loop through entries and locate
		for (uint64_t i = 0; i < SuperBlock.EntryTable.SizeInBytes; i += FS_SIZE_FILE_ENTRY)
		{
			DirectoryEntry* entry = (DirectoryEntry*)(EntryTableData + i);

			// located entry
			if (entry->Type == EntryType::Directory && strlen(entry->Name) > 0 && StringUtil::Equals(dirname, entry->Name) && entry->ParentIndex == parent_index)
			{
				return true;
			}
		}
		return false;
	}

	// copy file from source to destination
	FileEntry FSHost::IOCopyFile(std::string dest, std::string src)
	{
		// validate files
		if (!IOFileExists(src)) { Debugger::ThrowError("Unable to locate source file before copying"); return FileEntry {}; }
		if (IOFileExists(dest)) { Debugger::ThrowError("Destination file already exists"); return FileEntry {}; }

		/* NOT YET IMPLEMENTED */

		// error
		return FileEntry{};
	}

	// copy directory from source to destination
	DirectoryEntry FSHost::IOCopyDirectory(std::string dest, std::string src)
	{
		// validate files
		if (!IODirectoryExists(src)) { Debugger::ThrowError("Unable to locate source directory before copying"); return DirectoryEntry {}; }
		if (IODirectoryExists(dest)) { Debugger::ThrowError("Destination directory already exists"); return DirectoryEntry {}; }

		/* NOT YET IMPLEMENTED */

		// error
		return DirectoryEntry {};
	}

	// move file from source to destination
	FileEntry FSHost::IOMoveFile(std::string dest, std::string src)
	{
		return FileEntry{};
	}

	// move directory from source to destination
	DirectoryEntry FSHost::IOMoveDirectory(std::string dest, std::string src)
	{
		return DirectoryEntry{};
	}

	// attempt to delete existing file
	bool FSHost::IODeleteFile(std::string path)
	{
		// validate file
		if (!IOFileExists(path)) { Debugger::ThrowError("Unable to locate file for deletion"); return false; }

		// get file entry
		FileEntry* fileptr = GetFileByName(path);
		if (fileptr == nullptr) { Debugger::ThrowError("Unexpected error while deleting file"); return false; }

		// free data
		bool freed = FreeBlock(fileptr->StartSector, fileptr->SectorCount);
		if (!freed) { Debugger::ThrowError("Unable to free data while deleting file"); return false; }

		// delete file entry
		if (!DeleteFileEntry(fileptr)) { Debugger::ThrowError("Unable to delete entry while deleting file"); return false; }

		// successfully delete
		Debugger::ThrowOK("Successfully delete file " + path);
		return true;
	}

	// attempt to delete existing directory
	bool FSHost::IODeleteDirectory(std::string path)
	{
		if (!IODirectoryExists(path)) { Debugger::ThrowError("Unable to locate directory for deletion"); return false; }
		return false;
	}

	// rename file with specified name
	bool FSHost::IORenameFile(std::string path, std::string name)
	{
		return false;
	}

	// rename directory with specified name
	bool FSHost::IORenameDirectory(std::string path, std::string name)
	{
		return false;
	}

	// write text to file
	bool FSHost::IOWriteAllText(std::string path, std::string text)
	{
		Debugger::ThrowOK("ALLOCATION");

		// check if file already exists
		if (IOFileExists(path)) 
		{ 
			// get file
			FileEntry* fileptr = GetFileByName(path);
			if (fileptr == nullptr) { Debugger::ThrowError("Unexpected error while writing text to file"); return false; }

			// get amount of sectors required
			uint32_t sectors = text.length() / FS_SIZE_SECTOR;
			if (sectors < 1) { sectors = 1; }

			// create new data block
			BlockEntry* new_block = AllocateBlock(sectors);
			if (new_block == nullptr) { Debugger::ThrowError("Unable to allocate memory while writing text to file"); return false; }

			// sector data array
			uint8_t* temp = (uint8_t*)malloc(FS_SIZE_SECTOR);
			if (temp == nullptr) { Debugger::ThrowError("Unable to allocate memory while creating file"); return false; }
			uint32_t total = 0;

			// write data
			for (uint64_t sec = 0; sec < sectors; sec++)
			{
				for (uint32_t i = 0; i < FS_SIZE_SECTOR; i++)
				{
					if (total < text.length()) { temp[i] = text[total]; }
					total++;
				}

				// write data to sector
				ATADisk::Write(temp, new_block->Sector + sec);
			}

			// free old data
			bool freed = FreeBlock(fileptr->StartSector, fileptr->SectorCount);
			if (!freed) { Debugger::ThrowError("Unable to free data while writing text to file"); return false; }

			// override block properties in file
			fileptr->StartSector = new_block->Sector;
			fileptr->SectorCount = new_block->Count;

			// success
			Debugger::ThrowOK("Finished writing text to " + path);
			return true;
		}
		// create new file
		else
		{
			// create new file
			FileEntry file = IOCreateFile(path, text.length() + 1);
			if (file.Type == EntryType::Null) { Debugger::ThrowError("Unexpected error while creating file for writing"); return false; }

			// sector data array
			uint8_t* temp = (uint8_t*)malloc(FS_SIZE_SECTOR);
			if (temp == nullptr) { Debugger::ThrowError("Unable to allocate memory while creating file"); return false; }
			uint32_t total = 0;

			// write data
			for (uint64_t sec = 0; sec < file.SectorCount; sec++)
			{
				for (uint32_t i = 0; i < FS_SIZE_SECTOR; i++)
				{
					if (total < text.length()) { temp[i] = text[total]; }
					total++;
				}

				// write data to sector
				ATADisk::Write(temp, file.StartSector + sec);
			}
		}

		// error
		return false;
	}

	// write binary data to file
	bool FSHost::IOWriteAllBytes(std::string path, uint8_t* data, uint32_t size)
	{
		return false;
	}

	// write lines of text to file
	bool FSHost::IOWriteAllLines(std::string path, std::vector<std::string> lines)
	{
		return false;
	}

	// read all text from file
	std::string FSHost::IOReadAllText(std::string path)
	{
		// validate file
		if (!IOFileExists(path)) { Debugger::ThrowError("Unable to locate file for reading"); return std::string(""); }

		// read file
		FileEntry* fileptr = GetFileByName(path);
		if (fileptr == nullptr) { Debugger::ThrowError("Unexpected error while reading file"); return std::string(""); }

		// get data from file
		uint8_t* data = ReadDataBlock(fileptr->StartSector, fileptr->SectorCount);
		if (data == nullptr) { Debugger::ThrowError("Unexpected error reading data block during file read"); return std::string(""); }

		// copy data to string
		std::string output;
		for (size_t i = 0; i < fileptr->Size; i++) { output += (char)data[i]; }

		// free unused data and return string
		if (data != nullptr) { free(data); }
		return output;
	}

	// read all binary data from file
	uint8_t* FSHost::IOReadAllBytes(std::string path)
	{
		// validate file
		if (!IOFileExists(path)) { Debugger::ThrowError("Unable to locate file for reading"); return nullptr; }

		// read file
		FileEntry* fileptr = GetFileByName(path);
		if (fileptr == nullptr) { Debugger::ThrowError("Unexpected error while reading file"); return nullptr; }

		// get data from file
		uint8_t* data = ReadDataBlock(fileptr->StartSector, fileptr->SectorCount);
		if (data == nullptr) { Debugger::ThrowError("Unexpected error reading data block during file read"); nullptr; }

		// return file data
		return data;
	}

	// read all lines of text from file
	std::vector<std::string> FSHost::IOReadAllLines(std::string path)
	{
		// attempt to load text from file
		std::string text = IOReadAllText(path);
		std::vector<std::string> output;
		
		// validate text
		if (text.c_str() == nullptr) { return output; }

		// if data is null return null list
		if (text.length() == 0) { return output; }

		// get lines from text
		std::string line;
		for (size_t i = 0; i < text.length(); i++)
		{
			// create new line
			if (text.c_str()[i] == '\n')
			{
				output.push_back(line);
				line.clear();
			}
			// add character to current line
			else { line += text.c_str()[i]; }
		}
		if (line.length() > 0) { output.push_back(line); }

		// return lines
		return output;
	}

	// get list of directories contained in directory
	std::vector<DirectoryEntry> FSHost::IOGetDirectories(std::string path)
	{
		// create list of directories
		std::vector<DirectoryEntry> output;

		// validate path
		if (!IODirectoryExists(path)) { return output; }

		// get parent directory
		DirectoryEntry* parent = GetDirectoryByName(path);
		uint32_t parent_index = GetFileIndex(parent);
		if (StringUtil::Equals(path, "/")) { parent_index = 0; }
		if (parent == nullptr && !StringUtil::Equals(path, "/")) { Debugger::ThrowError("Unable to locate parent while getting list of directories"); return output; }

		// loop through entries
		for (uint64_t i = 0; i < SuperBlock.EntryTable.SizeInBytes; i += FS_SIZE_FILE_ENTRY)
		{
			DirectoryEntry* entry = (DirectoryEntry*)(EntryTableData + i);

			// found entry from parent directory
			if (entry->Type == EntryType::Directory && strlen(entry->Name) > 0 && entry->ParentIndex == parent_index)
			{
				// copy properties
				DirectoryEntry dir;
				memcpy(dir.Name, entry->Name, 58);
				dir.ParentIndex = entry->ParentIndex;
				dir.Status = entry->Status;
				dir.Type = entry->Type;

				// add to list unless root directory
				if (dir.Status != 1) { output.push_back(dir); }
			}
		}

		// return list of directories
		return output;
	}

	// get list of files contained in directory
	std::vector<FileEntry> FSHost::IOGetFiles(std::string path)
	{
		// create list of directories
		std::vector<FileEntry> output;

		// validate path
		if (!IODirectoryExists(path)) { return output; }

		// get parent directory
		DirectoryEntry* parent = GetDirectoryByName(path);
		uint32_t parent_index = GetFileIndex(parent);
		if (StringUtil::Equals(path, "/")) { parent_index = 0; }
		if (parent == nullptr && !StringUtil::Equals(path, "/")) { Debugger::ThrowError("Unable to locate parent while getting list of files"); return output; }

		// loop through entries
		for (uint64_t i = 0; i < SuperBlock.EntryTable.SizeInBytes; i += FS_SIZE_FILE_ENTRY)
		{
			FileEntry* entry = (FileEntry*)(EntryTableData + i);

			// found entry from parent directory
			if (entry->Type == EntryType::File && strlen(entry->Name) > 0 && entry->ParentIndex == parent_index)
			{
				// copy properties
				FileEntry file;
				memcpy(file.Name, entry->Name, 38);
				file.ParentIndex = entry->ParentIndex;
				file.Status = entry->Status;
				file.Type = entry->Type;
				file.Size = entry->Size;
				file.StartSector = entry->StartSector;
				file.SectorCount = entry->SectorCount;

				// add to list
				output.push_back(file);
			}
		}

		// return list of directories
		return output;
	}
}