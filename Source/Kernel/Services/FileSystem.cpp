#include <Kernel/Services/FileSystem.hpp>
#include <Kernel/Core/Kernel.hpp>

namespace PMOS
{
    namespace VFS
    {
        static const char TypeStringNull[] = "NULL";
        static const char TypeStringFile[] = "FILE";
        static const char TypeStringDir[]  = "DIR ";

        // null entries
        const FileEntry NullFile = FileEntry {};
        const DirectoryEntry NullDir = DirectoryEntry {};

        // get entry type string
        char* GetEntryTypeString(EntryType type)
        {
            switch (type)
            {
                case EntryType::Directory: { return (char*)TypeStringDir; }
                case EntryType::File:      { return (char*)TypeStringFile; }
                default:                   { return (char*)TypeStringNull; }
            }
        }

        FSHost::FSHost() : Service("fshost", ServiceType::Utility)
        {

        }

        void FSHost::Initialize()
        {
            Service::Initialize();

            Kernel::ServiceMgr.Register(this);
            Kernel::ServiceMgr.Start(this);
        }

        void FSHost::Start()
        {
            Service::Start();
            
            Mount();
        }

        void FSHost::Stop()
        {
            Service::Stop();
        }

        // initialize table arrays
        void FSHost::InitTableArrays()
        {
            // create new block table data array
            if (SuperBlock.BlockTable.SizeInBytes == 0) { return; }
            if (BlockTableData != nullptr) { MemFree(BlockTableData); }
            BlockTableData = (byte*)MemAlloc(SuperBlock.BlockTable.SizeInBytes, true, AllocationType::System);
            Memory::Set(BlockTableData, 0, SuperBlock.BlockTable.SizeInBytes);

            // create new entry table data array
            if (SuperBlock.EntryTable.SizeInBytes == 0) { return; }
            if (EntryTableData != nullptr) { MemFree(EntryTableData); }
            EntryTableData = (byte*)MemAlloc(SuperBlock.EntryTable.SizeInBytes, true, AllocationType::System);
            Memory::Set(EntryTableData, 0, SuperBlock.EntryTable.SizeInBytes);
        }

        // print full disk information
        void FSHost::PrintDiskInformation(bool debug)
        {
            
        }

        // print block table entries
        void FSHost::PrintBlockTable(bool debug)
        {
           
        }

        // print file entry table entries
        void FSHost::PrintEntryTable(bool debug)
        {
           
        }

        // print contents of specified directory
        void FSHost::PrintDirectoryContents(char* path)
        {
            if (path == nullptr) { return; }
            if (StringUtil::Length(path) == 0) { return; }

            // validate path
            if (!IODirectoryExists(path)) { return; }

            // get entries in path
            uint dirs_len = 0, files_len = 0;
            DirectoryEntry** dirs = IOGetDirectories(path, &dirs_len);
            FileEntry** files = IOGetFiles(path, &files_len);
            if (dirs_len == 0 && files_len == 0) { if (dirs != nullptr) { MemFree(dirs); } if (files != nullptr) { MemFree(files); } Kernel::CLI->Debug.WriteLine("No files found."); return; }

            // print directories
            for (uint i = 0; i < dirs_len; i++)
            {
                // print
                Kernel::CLI->Debug.WriteUnformatted("> ", Col4::Yellow);
                Kernel::CLI->Debug.WriteUnformatted(dirs[i]->Name, Col4::Yellow);
                Kernel::CLI->Debug.NewLine();
                MemFree(dirs[i]);
            }

            // print files
            for (uint i = 0; i < files_len; i++)
            {
                // print
                Kernel::CLI->Debug.WriteLine(files[i]->Name);
                MemFree(files[i]);
            }

            MemFree(dirs);
            MemFree(files);
        }

        // mount the file system
        void FSHost::Mount()
        {
            if (!Kernel::ATA->Identify()) { return; }

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
        }

        // unmount the file system
        void FSHost::Unmount()
        {
            uint block_table_size = SuperBlock.BlockTable.SizeInBytes;
            uint entry_table_size = SuperBlock.EntryTable.SizeInBytes;

            if (BlockTableData != nullptr && block_table_size > 0) { Memory::Set(BlockTableData, 0, block_table_size); }
            if (EntryTableData != nullptr && entry_table_size > 0) { Memory::Set(EntryTableData, 0, entry_table_size); }

            Memory::Set(&SuperBlock, 0, sizeof(SuperBlockHeader));

            // set flag and print message
            Mounted = false;
        }

        // check if file system is mounted
        bool FSHost::IsMounted() { return Mounted; }

        // format the disk with manually specified size in bytes
        void FSHost::Format(uint size, bool wipe)
        {
            DiskSize = size;

            // make sure drive is unmounted before formatting
            Unmount();

            // wipe data from disk
            if (wipe) { Wipe(); }

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

            IOCreateDirectory("/sys");
            IOCreateDirectory("/sys/apps");
            IOCreateDirectory("/sys/resources");
            IOCreateDirectory("/sys/doc");

            // write updated data to disk
            WriteTables();

            PrintDiskInformation(false);
            PrintBlockTable(false);
            PrintEntryTable(false);

            // re-mount disk
            Mount();
        }

        // fill entire disk image with zeros
        void FSHost::Wipe()
        {
            // clear disk
            Kernel::Debug.WriteLine("DISK SIZE: %d", DiskSize);
            byte* data = (byte*)MemAlloc(FS_SIZE_SECTOR);
            if (data != nullptr)
            {
                Memory::Set(data, 0, FS_SIZE_SECTOR);
                Kernel::Debug.Write("Wiping drive");
                int tick = 0;
                for (uint i = 0; i < DiskSize / FS_SIZE_SECTOR; i++)
                {
                    tick++;
                    Kernel::ATA->Write(i, 1, data);
                    if (tick >= 512) { Kernel::Debug.Write("."); tick = 0; }
                }
            }
            else { return; }
            MemFree(data);
            Kernel::Debug.NewLine();
        }

        // write block and entry table to disk
        void FSHost::WriteTables()
        {
            WriteSuperBlock();
            WriteBlockTable();
            WriteEntryTable();
        }

        // create new super block header
        void FSHost::CreateSuperBlock(uint disk_size)
        {
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
            Memory::Set(&SuperBlock.Reserved, 0, 444);
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
            Memory::Copy(dest->Reserved, src->Reserved, 444);
        }

        // read super block header from disk
        void FSHost::ReadSuperBlock()
        {
            // create temporary data array for storing sector data
            byte* data = (byte*)MemAlloc(FS_SIZE_SECTOR);
            Kernel::ATA->Read(FS_SECTOR_SUPER, 1, data);

            // successfully allocated memory
            if (data != nullptr)
            {
                // copy super block from data array
                SuperBlockHeader* super = (SuperBlockHeader*)data;
                CopySuperBlock(&SuperBlock, super);
                DiskSize = super->BytesPerSector * super->SectorCount;

                // free memory and message
                MemFree(data);
            }
        }
        
        // write super block header to disk
        void FSHost::WriteSuperBlock()
        {
            // create temporary data array for storing sector data
            byte* data = (byte*)MemAlloc(FS_SIZE_SECTOR);

            // successfully allocated memory
            if (data != nullptr)
            {
                // copy super block to data array
                SuperBlockHeader* super = (SuperBlockHeader*)data;
                Memory::Set(data, 0, FS_SIZE_SECTOR);
                CopySuperBlock(super, &SuperBlock);

                // write super block sector to disk
                Kernel::ATA->Write(FS_SECTOR_SUPER, 1, data);

                // finished message
                MemFree(data);
            }
        }
        
        // create new block table - must be done after super block is created
        void FSHost::CreateBlockTable()
        {
            // validate data properties
            if (SuperBlock.Data.StartSector == 0) { Kernel::Debug.Error("Invalid start sector while reading block table"); return; }
            if (SuperBlock.Data.SectorCount == 0) { Kernel::Debug.Error("Invalid sector count while reading block table"); return; }

            // get data properties from super block
            uint start = SuperBlock.Data.StartSector;
            uint size  = SuperBlock.Data.SectorCount;
            byte  state = FS_STATE_FREE;

            // create new mass data block entry
            BlockEntry* entry = CreateBlockEntry(start, size, state);
        }

        // read block table from disk
        void FSHost::ReadBlockTable()
        {
            // validate block table
            if (!IsBlockTableValid()) { Kernel::Debug.Error("Unable to validate block table before reading from disk"); return; }

            // create data array
            byte* data = (byte*)MemAlloc(FS_SIZE_SECTOR);
            if (data == nullptr) { Kernel::Debug.Error("Unable to allocate memory while reading block table from disk"); return; }

            // loop through block table sectors
            uint table_offset = 0;
            for (uint sec = SuperBlock.BlockTable.StartSector; sec < SuperBlock.BlockTable.EndSector; sec++)
            {
                // read sector from disk
                Kernel::ATA->Read(sec, 1, data);

                // copy data from sector to table array
                Memory::Copy((void*)(BlockTableData + table_offset), data, FS_SIZE_SECTOR);
                table_offset += FS_SIZE_SECTOR;
            }

            // free data
            if (data != nullptr) { MemFree(data); }
        }

        // write block table to disk
        void FSHost::WriteBlockTable()
        {
            // validate block table
            if (!IsBlockTableValid()) { Kernel::Debug.Error("Unable to validate block table before writing to disk"); return; }

            // loop through block table sectors
            byte* data = nullptr;
            uint table_offset = 0;
            for (uint sec = SuperBlock.BlockTable.StartSector; sec < SuperBlock.BlockTable.EndSector; sec++)
            {
                // create temporary array
                data = (byte*)MemAlloc(FS_SIZE_SECTOR);
                if (data == nullptr) { Kernel::Debug.Error("Unable to allocate memory while writing block table to disk"); return; }

                // copy data to temporary sector
                Memory::Set(data, 0, FS_SIZE_SECTOR);
                Memory::Copy(data, (void*)(BlockTableData + table_offset), FS_SIZE_SECTOR);
                table_offset += FS_SIZE_SECTOR;
                
                // write sector to disk
                Kernel::ATA->Write(sec, 1, data);

                // free data
                MemFree(data);
            }

            if (data != nullptr) { MemFree(data); }
        }

        // read data from data block
        byte* FSHost::ReadDataBlock(uint sector, uint count)
        {
            if (!IsBlockTableValid()) { Kernel::Debug.Error("Unable to validate block table before writing data block to disk"); return nullptr; }
            bool located = false;

            // locate through blocks
            for (ulong i = 0; i < SuperBlock.BlockTable.MaxEntries; i++)
            {
                // read entry from table
                BlockEntry* entry = (BlockEntry*)(BlockTableData + (i * (ulong)FS_SIZE_BLOCK_ENTRY));

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
                byte* output = (byte*)MemAlloc(count * (ulong)FS_SIZE_SECTOR);
                if (output == nullptr) { Kernel::Debug.Error("Unable to allocate memory while reading data block"); return nullptr; }

                uint index = 0;
                byte* sec_data = (byte*)MemAlloc(FS_SIZE_SECTOR);
                for (uint sec = sector; sec < sector + count; sec++)
                {
                    Kernel::ATA->Read(sec, 1, sec_data);
                    if (sec_data != nullptr && output != nullptr)
                    {
                        for (size_t i = 0; i < FS_SIZE_SECTOR; i++)
                        {
                            output[index] = sec_data[i];
                            index++;
                        }
                    }
                    else 
                    { 
                        Kernel::Debug.Error("Unable to allocate memory while reading sector during data block iteration"); 
                        if (sec_data != nullptr) { MemFree(sec_data); }
                        if (output != nullptr) { MemFree(output); }
                        return nullptr; }
                }

                MemFree(sec_data);
                return output;
            }
            // unable to locate data block
            else { Kernel::Debug.Error("Unable to locate data block"); return nullptr; }
        }

        // allocate new block and return result
        BlockEntry* FSHost::AllocateBlock(uint sectors)
        {
            // validate block table
            if (!IsBlockTableValid()) { return nullptr; }

            // validate size
            if (sectors == 0) { Kernel::Debug.Error("Allocation of 0 sectors not permitted"); return nullptr; }

            // get next available entry for allocation
            BlockEntry* entry = GetAvailableBlock(sectors);
            
            // validate entry
            if (entry == nullptr)			   { Kernel::Debug.Error("Unable to allocate %d sectors of disk", sectors); return nullptr; }
            if (entry->Sector == 0)			   { Kernel::Debug.Error("Unexpected sector value while reading next available block"); return nullptr; }
            if (entry->Count == 0)			   { Kernel::Debug.Error("Unexpected sector count while reading next available block"); return nullptr; }

            return entry;
        }

        // free already allocated block and return result
        bool FSHost::FreeBlock(uint sector, uint count)
        {
            char temp[64];

            // validate block table
            if (!IsBlockTableValid()) { return false; }

            // validate block properties
            if (sector == 0 || count == 0) { return false; }

            // locate perfectly sized block to use
            for (ulong i = 0; i < SuperBlock.BlockTable.MaxEntries; i++)
            {
                // read entry from table
                BlockEntry* entry = (BlockEntry*)(BlockTableData + (i * (ulong)FS_SIZE_BLOCK_ENTRY));

                // found matching entry
                if (entry->Sector > 0 && entry->Sector == sector && entry->Count == count && entry->State == FS_STATE_USED)
                {
                    // set entry state
                    entry->State = FS_STATE_FREE;
                    byte* empty = (byte*)MemAlloc(FS_SIZE_SECTOR);
                    // clear data from block
                    if (empty != nullptr)
                    {
                        Memory::Set(empty, 0, FS_SIZE_SECTOR);
                        for (size_t i = 0; i < count; i++) { Kernel::ATA->Write(entry->Sector + i, 1, empty); }
                        MemFree(empty);
                    }
                    // merge available blocks
                    MergeAvailableBlocks();
                    Kernel::Debug.Info("Unallocated %d sectors of disk", count);
                    return true;
                }
            }

            // unable to free block
            Kernel::Debug.Error("Unable to free data block");
            return false;
        }

        // get next available block for allocation
        BlockEntry* FSHost::GetAvailableBlock(uint sectors)
        {
            // validate block table
            if (!IsBlockTableValid()) { return nullptr; }

            // validate sector count
            if (sectors == 0) { return nullptr; }

            // locate perfectly sized block to use
            for (ulong i = 0; i < SuperBlock.BlockTable.MaxEntries; i++)
            {
                // read entry from table
                BlockEntry* entry = (BlockEntry*)(BlockTableData + (i * (ulong)FS_SIZE_BLOCK_ENTRY));

                // found usable block
                if (entry->Sector > 0 && entry->Count == sectors && entry->State == FS_STATE_FREE)
                {
                    // set state and return block
                    entry->State = FS_STATE_USED;
                    return entry;
                }
            }

            // locate larger block and split it up
            for (ulong i = 0; i < SuperBlock.BlockTable.MaxEntries; i++)
            {
                // read entry from table
                BlockEntry* entry = (BlockEntry*)(BlockTableData + (i * (ulong)FS_SIZE_BLOCK_ENTRY));

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
            Kernel::Debug.Error("Unable to locate available data block");
            return nullptr;
        }

        // merge all available block
        void FSHost::MergeAvailableBlocks()
        {
            // loop through entries and combine neighbours
            for (ulong i = 0; i < SuperBlock.BlockTable.MaxEntries; i++)
            {
                // read entries from table
                BlockEntry* entry = (BlockEntry*)(BlockTableData + (i * (ulong)FS_SIZE_BLOCK_ENTRY));

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
            uint free_sector = 0;
            uint free_count  = 0;
            for (ulong i = 0; i < SuperBlock.BlockTable.MaxEntries; i++)
            {
                // read entries from table
                BlockEntry* entry = (BlockEntry*)(BlockTableData + (i * (ulong)FS_SIZE_BLOCK_ENTRY));

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
            if (free_block == nullptr) { Kernel::Debug.Error("Unexpected error while merging available blocks"); return; }
        }

        // get nearest block based on sector
        BlockEntry* FSHost::GetNearestBlock(BlockEntry* entry)
        {
            // validate entry
            if (entry == nullptr) { return nullptr; }
            if (entry->Sector == 0 || entry->Count == 0) { return nullptr; }

            // loop through block entries
            for (ulong i = 0; i < SuperBlock.BlockTable.MaxEntries; i++)
            {
                // read entry from table
                BlockEntry* temp = (BlockEntry*)(BlockTableData + (i * (ulong)FS_SIZE_BLOCK_ENTRY));

                // located neighbouring block
                if (temp != entry && entry->Sector + entry->Count == temp->Sector && temp->State == FS_STATE_FREE) { return temp; }
                if (temp != entry && temp->Sector + temp->Count == entry->Sector && temp->State == FS_STATE_FREE) { return temp; }

            }

            // unable to locate nearest block
            return nullptr;
        }
        
        // create new data block entry
        BlockEntry* FSHost::CreateBlockEntry(uint sector, uint count, byte state)
        {
            // validate block table
            if (!IsBlockTableValid()) { return nullptr; }

            // loop through block table
            for (ulong i = 0; i < SuperBlock.BlockTable.MaxEntries; i++)
            {
                // read entry from table
                BlockEntry* entry = (BlockEntry*)(BlockTableData + (i * (ulong)FS_SIZE_BLOCK_ENTRY));

                // check if entry is unused
                if (entry->Sector == 0 && entry->Count == 0 && entry->State == FS_STATE_FREE)
                {
                    entry->Sector = sector;
                    entry->Count  = count;
                    entry->State  = state;
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
            for (ulong i = 0; i < SuperBlock.BlockTable.MaxEntries; i++)
            {
                // read entry from table
                BlockEntry* temp = (BlockEntry*)(BlockTableData + (i * (ulong)FS_SIZE_BLOCK_ENTRY));

                if (temp == entry && temp->Sector == entry->Sector && temp->Count == entry->Count && temp->State == entry->State)
                {
                    // reset block state
                    temp->Sector = 0;
                    temp->Count = 0;
                    temp->State = FS_STATE_FREE;
                    // return success
                    return true;
                }
            }

            // unable to locate block 
            Kernel::Debug.Error("Unable to locate block entry 0x%8x for deletion", (uint)entry);
            return false;
        }

        // get block entry based on properties
        BlockEntry* FSHost::GetBlockEntry(uint sector, uint count, byte state)
        {
            // validate properties
            if (sector == 0 || count == 0) { return nullptr; }

            // loop through block table
            for (ulong i = 0; i < SuperBlock.BlockTable.MaxEntries; i++)
            {
                // read entry from table
                BlockEntry* entry = (BlockEntry*)(BlockTableData + (i * (ulong)FS_SIZE_BLOCK_ENTRY));
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
            uint count = 0;
            for (ulong i = 0; i < SuperBlock.BlockTable.MaxEntries; i++)
            {
                // read entry from table
                BlockEntry* entry = (BlockEntry*)(BlockTableData + (i * (ulong)FS_SIZE_BLOCK_ENTRY));

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
            uint count = 0;
            for (ulong i = 0; i < SuperBlock.EntryTable.MaxEntries; i++)
            {
                // read entry from table
                DirectoryEntry* entry = (DirectoryEntry*)(EntryTableData + (i * (ulong)FS_SIZE_FILE_ENTRY));

                // located valid entry, increment count
                if (entry->Type != EntryType::Null) { count++; }
            }
            EntryCount = count;
        }

        // create new entry table
        void FSHost::CreateEntryTable()
        {
            // validate entry table size
            if (SuperBlock.EntryTable.SizeInBytes == 0) { Kernel::Debug.Error("Invalid size while creating new entry table"); return; }

            // clear entry table
            Memory::Set(EntryTableData, 0, SuperBlock.EntryTable.SizeInBytes);

            // finished message
            Kernel::Debug.OK("Created new entry table");
        }

        // create new root directory entry
        void FSHost::CreateRootDirectory()
        {
            // set properties
            Memory::Set(RootDirectory.Name, 0, 58);
            Memory::Copy(RootDirectory.Name, (void*)"DISK\0", 5);
            RootDirectory.Type = EntryType::Directory;
            RootDirectory.Status = 1;
            RootDirectory.ParentIndex = 0;

            // create directory
            DirectoryEntry* root = CreateDirectoryEntry(RootDirectory);

            // finished message depending on result
            if (root != nullptr) { Kernel::Debug.OK("Finished creating new root directory"); }
            else { Kernel::Debug.Error("Unable to create new directory"); }
        }

        // read entry table from disk
        void FSHost::ReadEntryTable()
        {
            // validate entry table
            if (!IsEntryTableValid()) { Kernel::Debug.Error("Unable to validate entry table before reading from disk"); return; }

            // loop through entry table sectors
            byte* data = (byte*)MemAlloc(FS_SIZE_SECTOR);
            uint table_offset = 0;
            for (uint sec = 0; sec < SuperBlock.EntryTable.SectorCount; sec++)
            {
                // read sector from disk
                Kernel::ATA->Read(SuperBlock.EntryTable.StartSector + sec, 1, data);
                if (data == nullptr) { Kernel::Debug.Error("Unable to allocate memory while reading entry table from disk"); return; }

                // copy data from sector to table array
                if (table_offset < SuperBlock.EntryTable.SizeInBytes) { Memory::Copy((void*)(EntryTableData + table_offset), data, FS_SIZE_SECTOR); }
                else { if (data != nullptr) { MemFree(data); } Kernel::Debug.Error("Memory write violation while reading entry table"); return; }
                table_offset += FS_SIZE_SECTOR;
            }

            // free data
            if (data != nullptr) { MemFree(data); }
        }

        // write entry table to disk
        void FSHost::WriteEntryTable()
        {
            // validate entry table
            if (!IsEntryTableValid()) { Kernel::Debug.Error("Unable to validate entry table before writing to disk"); return; }

            // loop through entry table sectors
            byte* data = nullptr;
            uint table_offset = 0;
            for (uint sec = SuperBlock.EntryTable.StartSector; sec < SuperBlock.EntryTable.EndSector; sec++)
            {
                // create temporary array
                data = (byte*)MemAlloc(FS_SIZE_SECTOR);
                if (data == nullptr) { Kernel::Debug.Error("Unable to allocate memory while writing entry table to disk"); return; }

                // copy data to temporary sector
                Memory::Set(data, 0, FS_SIZE_SECTOR);
                Memory::Copy(data, (void*)(EntryTableData + table_offset), FS_SIZE_SECTOR);
                table_offset += FS_SIZE_SECTOR;

                // write sector to disk
                Kernel::ATA->Write(sec, 1, data);

                // free data
                MemFree(data);
            }

            Kernel::Debug.OK("Finished writing entry table to disk");
        }

        // create new directory entry
        DirectoryEntry* FSHost::CreateDirectoryEntry(DirectoryEntry src)
        {
            // validate source directory
            if (src.Type != EntryType::Directory) { return nullptr; }

            // get available entry in table
            DirectoryEntry* dest = (DirectoryEntry*)GetAvailableEntry();

            // validate entry in table
            if (dest == nullptr) { Kernel::Debug.Error("Unable to locate available entry while creating directory"); return nullptr; }

            // copy from source to destination
            Memory::Set(dest, 0, FS_SIZE_FILE_ENTRY);
            CopyFileEntry(dest, &src);

            // print message and return entry
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
            if (dest == nullptr) { Kernel::Debug.Error("Unable to locate available entry while creating file"); return nullptr; }

            // get sector amount required for file
            uint sectors = src.Size / FS_SIZE_SECTOR;
            if (sectors < 1) { sectors = 1; }
            sectors++;

            // allocate data block to file
            BlockEntry* block = AllocateBlock(sectors);

            // validate data block
            if (block == nullptr) { Kernel::Debug.Error("Unable to allocate required data block while creating file"); return nullptr; }

            // set file properties from block properties
            src.StartSector = block->Sector;
            src.SectorCount = block->Count;

            // copy from source to destination
            CopyFileEntry(dest, &src);

            // print message and return entry
            return dest;
        }

        // copy file or directory from specified locations;
        bool FSHost::CopyFileEntry(void* dest, void* src)
        {
            // validate destination and source
            if (dest == nullptr) { Kernel::Debug.Error("Null destination while copying file entry"); return false; }
            if (src == nullptr)  { Kernel::Debug.Error("Null source while copying file entry"); return false; }

            // copy data
            Memory::Copy(dest, src, FS_SIZE_FILE_ENTRY);
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
            for (ulong i = 0; i < SuperBlock.EntryTable.MaxEntries; i++)
            {
                // read entry from table
                FileEntry* temp = (FileEntry*)(EntryTableData + (i * (ulong)FS_SIZE_FILE_ENTRY));

                if (temp == entry && StringUtil::Equals(temp->Name, centry->Name) && temp->Type == EntryType::File)
                {
                    // clear data and return success
                    Memory::Set(temp, 0, FS_SIZE_FILE_ENTRY);
                    return true;
                }
            }

            // unable to locate block 
            Kernel::Debug.Error("Unable to delete file entry");
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

        // easily free a character array
        void FreeCharArray(char** args, uint* len)
        {
            for (uint i = 0; i < *len; i++) { MemFree(args[i]); }
            MemFree(args);
        }

        // delete blank entries from list
        char** DeleteBlankEntries(char** args, uint len, uint* out_len)
        {
            char**   output = nullptr;
            uint output_len = 0;

            // count valid entries
            for (uint i = 0; i < len; i++)
            {
                if (StringUtil::Length(args[i]) > 0) { output_len++; }
            }

            // create new array
            output = (char**)MemAlloc(output_len * sizeof(char*));

            // add new entries
            int index = 0;
            for (uint i = 0; i < len; i++)
            {
                if (StringUtil::Length(args[i]) > 0)
                {
                    char* str = (char*)MemAlloc(StringUtil::Length(args[i]));
                    StringUtil::Copy(str, args[i]);
                    output[index] = str;
                    index++;
                }
            }

            FreeCharArray(args, &len);
            *out_len = output_len;
            return output;
        }

        DirectoryEntry* FSHost::GetParentFromPath(char* path)
        {
            // validate path string
            if (path == nullptr) { return nullptr; }
            if (StringUtil::Length(path) == 0) { return nullptr; }

            // split path into peices
            uint args_len = 0;
            char** args = StringUtil::Split(path, '/', &args_len);

            // check if parent is root
            if (args_len <= 2 && path[0] == '/') { FreeCharArray(args, &args_len); return (DirectoryEntry*)EntryTableData; }

            if (args_len > 1)
            {
                int   index = 0;
                int   p = 0;
                void* output = nullptr;

                // loop through parts of path
                for (uint arg = 0; arg < args_len - 1; arg++)
                {
                    output = nullptr;
                    index = 0;

                    // validate path part
                    if (args[arg] != nullptr && StringUtil::Length(args[arg]) > 0)
                    {
                        // loop through entries in table
                        for (size_t i = 0; i < SuperBlock.EntryTable.SizeInBytes; i += FS_SIZE_FILE_ENTRY)
                        {
                            // get entry
                            FileEntry* entry = (FileEntry*)(EntryTableData + i);

                            // found match for current piece of path
                            if (entry->Type != EntryType::Null && entry->ParentIndex == (uint)p && StringUtil::Equals(entry->Name, args[arg]))
                            {
                                p = index;
                                output = entry;
                            }

                            // increment index
                            index++;
                        }
                    }
                }

                // free args
                FreeCharArray(args, &args_len);

                // return directory
                return (DirectoryEntry*)output;
            }

            // unable to locate parent directory
            Kernel::Debug.Error("Unable to locate parent directory of %s", path);
            FreeCharArray(args, &args_len);
            return nullptr;
        }

        FileEntry* FSHost::GetFileByName(char* path)
        {
            // validate path string
            if (path == nullptr) { return nullptr; }
            if (StringUtil::Length(path) == 0) { return nullptr; }

            // atytempt to fetch parent of file
            DirectoryEntry* parent = GetParentFromPath(path);

            // validate parent
            if (parent == nullptr) { Kernel::Debug.Error("Unable to locate parent while getting file by name"); return nullptr; }

            // split path into peices
            uint args_len = 0;
            char** args = StringUtil::Split(path, '/', &args_len);
            if (args_len == 0) { FreeCharArray(args, &args_len); Kernel::Debug.Error("Unable to split path while getting file by name"); return nullptr; }
            char* filename;
            int xx = args_len - 1;
            while (args[xx] != nullptr)
            {
                if (args[xx] != nullptr && StringUtil::Length(args[xx]) > 0) { filename = args[xx]; break; }
                if (xx == 0) { break; } xx--;
            }
            if (StringUtil::Length(filename) == 0 || filename == nullptr) { FreeCharArray(args, &args_len); Kernel::Debug.Error("Invalid file name while getting file by name"); return nullptr; }

            // get parent index
            uint parent_index = GetFileIndex(parent);

            // locate file in entry table
            for (size_t i = 0; i < SuperBlock.EntryTable.SizeInBytes; i += FS_SIZE_FILE_ENTRY)
            {
                // get entry
                FileEntry* entry = (FileEntry*)(EntryTableData + i);

                // found file in entry table
                if (entry->ParentIndex == parent_index && StringUtil::Equals(filename, entry->Name) && entry->Type == EntryType::File) 
                { 
                    FreeCharArray(args, &args_len);
                    return entry; 
                }
            }

            // unable to locate file in entry table
            Kernel::Debug.Error("Unable to get file by name: %s", path);
            FreeCharArray(args, &args_len);
            return nullptr;
        }

        DirectoryEntry* FSHost::GetDirectoryByName(char* path)
        {
            // validate path string
            if (path == nullptr) { return nullptr; }
            if (StringUtil::Length(path) == 0) { return nullptr; }

            // atytempt to fetch parent of file
            DirectoryEntry* parent = GetParentFromPath(path);

            // check if path is root
            if (StringUtil::Equals(path, "/")) { return (DirectoryEntry*)EntryTableData; }

            // validate parent
            if (parent == nullptr) { Kernel::Debug.Error("Unable to locate parent while getting directory by name"); return nullptr; }

            // split path into peices
            uint args_len = 0;
            char** args = StringUtil::Split(path, '/', &args_len);
            if (args_len == 0) { FreeCharArray(args, &args_len); Kernel::Debug.Error("Unable to split path while getting directory by name"); return nullptr; }
            char* dirname;
            int xx = args_len - 1;
            while (args[xx] != nullptr)
            {
                if (args[xx] != nullptr && StringUtil::Length(args[xx]) > 0) { dirname = args[xx]; break; }
                if (xx == 0) { break; } xx--;
            }
            if (StringUtil::Length(dirname) == 0 || dirname == nullptr) { FreeCharArray(args, &args_len); Kernel::Debug.Error("Invalid directory name while getting directory by name"); return nullptr; }

            // get parent index
            uint parent_index = GetFileIndex(parent);

            // locate file in entry table
            for (size_t i = 0; i < SuperBlock.EntryTable.SizeInBytes; i += FS_SIZE_FILE_ENTRY)
            {
                // get entry
                DirectoryEntry* entry = (DirectoryEntry*)(EntryTableData + i);

                // found file in entry table
                if (entry->ParentIndex == parent_index && StringUtil::Equals(dirname, entry->Name) && entry->Type == EntryType::Directory) 
                { 
                    FreeCharArray(args, &args_len);
                    return entry; 
                }
            }

            // unable to locate file in entry table
            Kernel::Debug.Error("Unable to get directory by name: %s", path);
            FreeCharArray(args, &args_len);
            return nullptr;
        }

        int FSHost::GetFileIndex(void* src)
        {
            // loop through entries
            int index = 0;
            for (uint i = 0; i < SuperBlock.EntryTable.SizeInBytes; i += FS_SIZE_FILE_ENTRY)
            {
                // get entry
                FileEntry* entry = (FileEntry*)(EntryTableData + i);

                // check if entry is valid
                if (entry == src && entry->Type != EntryType::Null) { return index; }

                index++;
            }

            return -1;
        }

        void* FSHost::GetEntryAtIndex(int index)
        {
            // loop through entries
            int x = 0;
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

        // -------------------------------------------------------------------------------------------------------------------
        
        // get path's parent
        char* FSHost::IOGetParent(char* path)
        {
            // get length of input string
            uint len = StringUtil::Length(path);
            
            // loop through input string reverse
            for (int i = len - 1; i >= 0; i--)
            {
                // located ending character - return string
                if (path[i] == '/') 
                { 
                    // delete slash
                    StringUtil::Delete(path); 
                    // terminate string
                    StringUtil::Append(path, '\0');
                    // add slash to end incase all characters have been removed
                    if (StringUtil::Length(path) == 0) { StringUtil::Append(path, "/"); }
                    return path; 
                }
                // delete character before final slash
                else { StringUtil::Delete(path); }
            }
            
            // terminate string
            StringUtil::Append(path, '\0');
            // add slash to end incase all characters have been removed
            if (StringUtil::Length(path) == 0) { StringUtil::Append(path, "/"); }
            
            // return modified input path
            return path;
        }
        
        // get path's parent - depth indicates how many parents to go through
        char* FSHost::IOGetParent(char* path, uint depth)
        {
            for (uint i = 0; i < depth; i++)
            {
                path = IOGetParent(path);	
            }
            if (StringUtil::Length(path) == 0) { StringUtil::Append(path, "/"); }
            return path;
        }

        FileEntry FSHost::IOOpenFile(char* path)
        {
            if(!IOFileExists(path)) { Kernel::Debug.Error("Unable to locate file %s", path); return NullFile; }

            FileEntry* fileptr = GetFileByName(path);
            if (fileptr == nullptr) { Kernel::Debug.Error("Unable to get file by name while opening file"); return NullFile; }
            if (fileptr->Type != EntryType::File) { Kernel::Debug.Error("Invalid type while opening file"); return NullFile; }
            
            byte* block_data = ReadDataBlock(fileptr->StartSector, fileptr->SectorCount);
            if (block_data == nullptr) { Kernel::Debug.Error("Unable to read data from file"); return NullFile; }

            FileEntry file;
            Memory::Copy(file.Name, fileptr->Name, 38);
            file.Type = fileptr->Type;
            file.Status = fileptr->Status;
            file.ParentIndex = fileptr->ParentIndex;
            file.Size = fileptr->Size;
            file.StartSector = fileptr->StartSector;
            file.SectorCount = fileptr->SectorCount;
            file.Data = block_data;
            return file;
        }

        FileEntry FSHost::IOCreateFile(char* path, uint size) { return IOCreateFile(path, size, true); }

        FileEntry FSHost::IOCreateFile(char* path, uint size, byte* data) { return IOCreateFile(path, size, data, true); }

        FileEntry FSHost::IOCreateFile(char* path, uint size, bool write)
        {
            // validate arguments
            if (StringUtil::Length(path) == 0 || size == 0) { return NullFile; }

            // get parent directory
            DirectoryEntry* parent = GetParentFromPath(path);

            // validate parent
            if (parent == nullptr) { Kernel::Debug.Error("Unable to locate parent directory while creating blank file"); return NullFile; }

            // split path into peices
        
            // split path into peices
            uint args_len = 0;
            char** args = StringUtil::Split(path, '/', &args_len);
            if (args_len == 0) { FreeCharArray(args, &args_len); Kernel::Debug.Error("Unable to split path while creating blank file"); return NullFile; }
            char* filename;
            int xx = args_len - 1;
            while (args[xx] != nullptr)
            {
                if (args[xx] != nullptr && StringUtil::Length(args[xx]) > 0) { filename = args[xx]; break; }
                if (xx == 0) { break; } xx--;
            }
            if (StringUtil::Length(filename) == 0 || filename == nullptr) { FreeCharArray(args, &args_len); Kernel::Debug.Error("Invalid file name while creating blank file"); return NullFile; }
            while (StringUtil::Length(filename) > 38) { StringUtil::Delete(filename); }


            // set properties
            FileEntry file;
            Memory::Set(file.Name, 0, 38);
            Memory::Copy(file.Name, filename, StringUtil::Length(filename));
            file.Type = EntryType::File;
            file.Size = size;
            file.ParentIndex = GetFileIndex(parent);
            file.Status = 0;

            // create file entry
            FileEntry* fileptr = CreateFileEntry(file);
            if (fileptr == nullptr) { Kernel::Debug.Error("Unable to create file"); FreeCharArray(args, &args_len); return NullFile; }
            file.StartSector = fileptr->StartSector;
            file.SectorCount = fileptr->SectorCount;

            // return file entry copy
            if (write) { WriteTables(); }
            FreeCharArray(args, &args_len);
            return file;
        }
        
        FileEntry FSHost::IOCreateFile(char* path, uint size, byte* data, bool write)
        {
            // validate arguments
            if (StringUtil::Length(path) == 0 || size == 0) { return NullFile; }

            // get parent directory
            DirectoryEntry* parent = GetParentFromPath(path);

            // validate parent
            if (parent == nullptr) { Kernel::Debug.Error("Unable to locate parent directory while creating blank file"); return NullFile; }

            // split path into peices
            uint args_len = 0;
            char** args = StringUtil::Split(path, '/', &args_len);
            if (args_len == 0) { FreeCharArray(args, &args_len); Kernel::Debug.Error("Unable to split path while creating blank file"); return NullFile; }
            char* filename;
            int xx = args_len - 1;
            while (args[xx] != nullptr)
            {
                if (args[xx] != nullptr && StringUtil::Length(args[xx]) > 0) { filename = args[xx]; break; }
                if (xx == 0) { break; } xx--;
            }
            if (StringUtil::Length(filename) == 0 || filename == nullptr) { FreeCharArray(args, &args_len); Kernel::Debug.Error("Invalid file name while creating blank file"); return NullFile; }
            while (StringUtil::Length(filename) > 38) { StringUtil::Delete(filename); }


            // set properties
            FileEntry file;
            Memory::Set(file.Name, 0, 38);
            Memory::Copy(file.Name, filename, StringUtil::Length(filename));
            file.Type = EntryType::File;
            file.Size = size;
            file.ParentIndex = GetFileIndex(parent);
            file.Status = 0;

            // create file entry
            FileEntry* fileptr = CreateFileEntry(file);
            if (fileptr == nullptr) { Kernel::Debug.Error("Unable to create file"); FreeCharArray(args, &args_len); return NullFile; }
            file.StartSector = fileptr->StartSector;
            file.SectorCount = fileptr->SectorCount;

            // get block data
            BlockEntry* block = GetBlockEntry(fileptr->StartSector, fileptr->SectorCount, FS_STATE_USED);

            // copy data to data block
            byte* temp = (byte*)MemAlloc(FS_SIZE_SECTOR);
            if (temp == nullptr || data == nullptr) { Kernel::Debug.Error("Unable to allocate memory while creating file"); FreeCharArray(args, &args_len); return NullFile; }
            uint total  = 0;

            // write data
            for (ulong sec = 0; sec < fileptr->SectorCount; sec++)
            {
                for (uint i = 0; i < FS_SIZE_SECTOR; i++)
                {
                    if (total < file.Size) { temp[i] = data[total]; }
                    total++;
                }

                // write data to sector
                Kernel::ATA->Write(file.StartSector + sec, 1, temp);
            }

            if (write) { WriteTables(); }
            MemFree(temp);
            FreeCharArray(args, &args_len);
            return file;
        }

        DirectoryEntry FSHost::IOCreateDirectory(char* path) { return IOCreateDirectory(path, true); }

        DirectoryEntry FSHost::IOCreateDirectory(char* path, bool write)
        {
            // validate arguments
            if (StringUtil::Length(path) == 0) { return NullDir; }

            // get parent directory
            DirectoryEntry* parent = GetParentFromPath(path);

            // validate parent
            if (parent == nullptr) { Kernel::Debug.Error("Unable to locate parent directory while creating directory"); return NullDir; }

            // split path into peices
            uint args_len = 0;
            char** args = StringUtil::Split(path, '/', &args_len);
            if (args_len == 0) { FreeCharArray(args, &args_len); Kernel::Debug.Error("Unable to split path while creating directory"); return NullDir; }
            char* dirname;
            int xx = args_len - 1;
            while (args[xx] != nullptr)
            {
                if (args[xx] != nullptr && StringUtil::Length(args[xx]) > 0) { dirname = args[xx]; break; }
                if (xx == 0) { break; } xx--;
            }
            if (StringUtil::Length(dirname) == 0 || dirname == nullptr) { FreeCharArray(args, &args_len); Kernel::Debug.Error("Invalid directory name while creating directory"); return NullDir; }
            while (StringUtil::Length(dirname) >= 48) { StringUtil::Delete(dirname); }

            // set properties
            DirectoryEntry dir;
            Memory::Set(dir.Name, 0, 58);
            Memory::Copy(dir.Name, dirname, StringUtil::Length(dirname));
            dir.ParentIndex = GetFileIndex(parent);
            dir.Type = EntryType::Directory;
            dir.Status = 0;

            // create directory entry
            DirectoryEntry* dirptr = CreateDirectoryEntry(dir);
            if (dirptr == nullptr) { Kernel::Debug.Error("Unable to create directory"); return NullDir; }
            
            // return directory entry copy
            if (write) { WriteTables(); }
            FreeCharArray(args, &args_len);
            return dir;
        }

        bool FSHost::IOFileExists(char* path)
        {
            // validate path
            if (path == nullptr) { Kernel::Debug.Error("Null path while searching for file"); return false; }
            if (StringUtil::Length(path) == 0) { Kernel::Debug.Error("Blank path while searching for file"); return false; }

            // get parent directory
            DirectoryEntry* parent = GetParentFromPath(path);
            uint parent_index = GetFileIndex(parent);

            // validate parent
            if (parent == nullptr) { Kernel::Debug.Error("Parent was null while searching for file"); return false; }

            // split path into peices
            uint args_len = 0;
            char** args = StringUtil::Split(path, '/', &args_len);
            if (args_len == 0) { FreeCharArray(args, &args_len); Kernel::Debug.Error("Unable to split path while searching for file"); return false; }
            char* filename;
            int xx = args_len - 1;
            while (args[xx] != nullptr)
            {
                if (args[xx] != nullptr && StringUtil::Length(args[xx]) > 0) { filename = args[xx]; break; }
                if (xx <= 0) { break; } xx--;
            }
            if (StringUtil::Length(filename) == 0 || filename == nullptr) { FreeCharArray(args, &args_len); Kernel::Debug.Error("Invalid file name while searching for file"); return false; }
            while (StringUtil::Length(filename) > 38) { StringUtil::Delete(filename); }
            Kernel::Debug.Info("FILENAME: %s", filename);

            // loop through entries and locate
            for (ulong i = 0; i < SuperBlock.EntryTable.SizeInBytes; i += FS_SIZE_FILE_ENTRY)
            {
                FileEntry* entry = (FileEntry*)(EntryTableData + i);

                // located entry
                if (entry->Type == EntryType::File && StringUtil::Length(entry->Name) > 0 && StringUtil::Equals(filename, entry->Name) && entry->ParentIndex == parent_index)
                { 
                    if (entry->Size == 0) { Kernel::Debug.Warning("File is empty."); }
                    FreeCharArray(args, &args_len); 
                    return true; 
                }
            }

            // unable to locate file
            Kernel::Debug.Error("Unable to locate file while searching for file: %s", path);
            FreeCharArray(args, &args_len);
            return false;
        }

        bool FSHost::IODirectoryExists(char* path)
        {
            
            if (path == nullptr) { return false; }

            // validate path
            if (StringUtil::Length(path) == 0) { Kernel::Debug.Error("Path was null while searching for directory"); return false; }

            // check if root
            if (StringUtil::Equals(path, "/")) { return true; }

            // get parent directory
            DirectoryEntry* parent = GetParentFromPath(path);
            uint parent_index = GetFileIndex(parent);

            // validate parent
            if (parent == nullptr) { Kernel::Debug.Error("Parent was null while searching for directory"); return false; }

            // split path into peices
            uint args_len = 0;
            char** args = StringUtil::Split(path, '/', &args_len);
            if (args_len == 0) { FreeCharArray(args, &args_len); Kernel::Debug.Error("Unable to split path while searching for directory"); return false; }
            char* dirname;
            int xx = args_len - 1;

            while (args[xx] != nullptr)
            {
                if (args[xx] != nullptr) 
                {
                    if (StringUtil::Length(args[xx]) > 0) { dirname = args[xx]; break; }
                }
                if (xx == 0) { break; } xx--;
            }
            if (StringUtil::Length(dirname) == 0 || dirname == nullptr) { FreeCharArray(args, &args_len); Kernel::Debug.Error("Invalid directory name while searching for directory"); return false; }

            // loop through entries and locate
            for (ulong i = 0; i < SuperBlock.EntryTable.SizeInBytes; i += FS_SIZE_FILE_ENTRY)
            {
                DirectoryEntry* entry = (DirectoryEntry*)(EntryTableData + i);

                // located entry
                if (entry->Type == EntryType::Directory && StringUtil::Length(entry->Name) > 0 && StringUtil::Equals(dirname, entry->Name) && entry->ParentIndex == parent_index)
                {
                    FreeCharArray(args, &args_len);
                    return true;
                }
            }		

            Kernel::Debug.Error("Unable to locate directory while searching for directory: %s", path);
            FreeCharArray(args, &args_len);
            return false;
        }

        FileEntry FSHost::IOCopyFile(char* dest, char* src)
        {
            Kernel::Debug.Info("IOCopyFile - NOT YET IMPLEMENTED");
            return NullFile;
        }

        DirectoryEntry FSHost::IOCopyDirectory(char* dest, char* src)
        {
            Kernel::Debug.Info("IOCopyDirectory - NOT YET IMPLEMENTED");
            return NullDir;
        }

        FileEntry FSHost::IOMoveFile(char* dest, char* src)
        {
            Kernel::Debug.Info("IOMoveFile - NOT YET IMPLEMENTED");
            return NullFile;
        }

        DirectoryEntry FSHost::IOMoveDirectory(char* dest, char* src)
        {
            Kernel::Debug.Info("IOMoveDirectory - NOT YET IMPLEMENTED");
            return NullDir;
        }

        bool FSHost::IODeleteFile(char* path)
        {
            Kernel::Debug.Info("IODeleteFile - NOT YET IMPLEMENTED");
            return false;
        }

        bool FSHost::IODeleteDirectory(char* path)
        {
            Kernel::Debug.Info("IODeleteDirectory - NOT YET IMPLEMENTED");
            return false;
        }

        bool FSHost::IORenameFile(char* path, char* name)
        {
            Kernel::Debug.Info("IORenameFile - NOT YET IMPLEMENTED");
            return false;
        }

        bool FSHost::IORenameDirectory(char* path, char* name)
        {
            Kernel::Debug.Info("IORenameDirectory - NOT YET IMPLEMENTED");
            return false;
        }

        bool FSHost::IOWriteAllText(char* path, char* text) { return IOWriteAllText(path, text, true); }
        bool FSHost::IOWriteAllBytes(char* path, byte* data, uint size) { return IOWriteAllBytes(path, data, size, true); }
        bool FSHost::IOWriteAllLines(char* path, char** lines, uint count) { return IOWriteAllLines(path, lines, count, true); }

        bool FSHost::IOWriteAllText(char* path, char* text, bool write)
        {
            // file already exists - override
            if (IOFileExists(path))
            {
                // get file
                FileEntry* fileptr = GetFileByName(path);
                if (fileptr == nullptr) { Kernel::Debug.Error("Unexpected error while writing text to file"); return false; }

                // get amount of sectors required
                uint sectors = StringUtil::Length(text) / FS_SIZE_SECTOR;
                if (sectors < 1) { sectors = 1; }

                // create new data block
                BlockEntry* new_block = AllocateBlock(sectors);
                if (new_block == nullptr) { Kernel::Debug.Error("Unable to allocate memory while writing text to file"); return false; }

                // sector data array
                byte* temp = (byte*)MemAlloc(FS_SIZE_SECTOR);
                if (temp == nullptr) { Kernel::Debug.Error("Unable to allocate memory while writing text to file"); return false; }
                uint total = 0;
                uint size = 0;

                // write data
                for (ulong sec = 0; sec < sectors; sec++)
                {
                    for (uint i = 0; i < FS_SIZE_SECTOR; i++)
                    {
                        if (total < StringUtil::Length(text)) { temp[i] = text[total]; size++; }
                        total++;
                    }

                    // write data to sector
                    Kernel::ATA->Write(new_block->Sector + sec, 1, temp);
                }

                // free old data
                bool freed = FreeBlock(fileptr->StartSector, fileptr->SectorCount);
                if (!freed) { Kernel::Debug.Error("Unable to free data while writing text to file"); return false; }

                // override block properties in file
                fileptr->StartSector = new_block->Sector;
                fileptr->SectorCount = new_block->Count;
                fileptr->Size = size;
                if (write) { WriteTables(); }
                MemFree(temp);

                // success
                Kernel::Debug.OK("Finished writing text to %s", path);
                return true;
            }
            // create new file
            else
            {
                // create new file
                FileEntry file = IOCreateFile(path, StringUtil::Length(text) + 1, false);
                if (file.Type == EntryType::Null) { Kernel::Debug.Error("Unexpected error while creating file for writing"); return false; }

                // sector data array
                byte* temp = (byte*)MemAlloc(FS_SIZE_SECTOR);
                if (temp == nullptr) { Kernel::Debug.Error("Unable to allocate memory while creating file"); return false; }
                uint total = 0;

                // write data
                for (ulong sec = 0; sec < file.SectorCount; sec++)
                {
                    for (uint i = 0; i < FS_SIZE_SECTOR; i++)
                    {
                        if (total < StringUtil::Length(text)) { temp[i] = text[total]; }
                        total++;
                    }

                    // write data to sector
                    Kernel::ATA->Write(file.StartSector + sec, 1, temp);
                }

                // success
                if (write) { WriteTables(); }
                MemFree(temp);
                Kernel::Debug.OK("Finished writing text to %s", path);
                return true;
            }

            // error
            Kernel::Debug.WriteLine("Unexpected error while writing all text");
            return false;
        }

        bool FSHost::IOWriteAllBytes(char* path, byte* data, uint size, bool write)
        {
            // file already exists - override
            if (IOFileExists(path))
            {
                // get file
                FileEntry* fileptr = GetFileByName(path);
                if (fileptr == nullptr) { Kernel::Debug.Error("Unexpected error while writing data to file"); return false; }

                // get amount of sectors required
                uint sectors = size / FS_SIZE_SECTOR;
                if (sectors < 1) { sectors = 1; }

                // create new data block
                BlockEntry* new_block = AllocateBlock(sectors);
                if (new_block == nullptr) { Kernel::Debug.Error("Unable to allocate memory while writing data to file"); return false; }

                // sector data array
                byte* temp = (byte*)MemAlloc(FS_SIZE_SECTOR);
                if (temp == nullptr) { Kernel::Debug.Error("Unable to allocate memory while writing data to file"); return false; }
                uint total = 0;
                uint n = 0;

                // write data
                for (ulong sec = 0; sec < sectors; sec++)
                {
                    for (uint i = 0; i < FS_SIZE_SECTOR; i++)
                    {
                        if (total < size) { temp[i] = data[total]; n++; }
                        total++;
                    }

                    // write data to sector
                    Kernel::ATA->Write(new_block->Sector + sec, 1, temp);
                }

                // free old data
                bool freed = FreeBlock(fileptr->StartSector, fileptr->SectorCount);
                if (!freed) { Kernel::Debug.Error("Unable to free data while writing data to file"); return false; }

                // override block properties in file
                fileptr->StartSector = new_block->Sector;
                fileptr->SectorCount = new_block->Count;
                fileptr->Size = n;
                if (write) { WriteTables(); }
                MemFree(temp);

                // success
                Kernel::Debug.OK("Finished writing data to %s", path);
                return true;
            }
            // create new file
            else
            {
                // create new file
                FileEntry file = IOCreateFile(path, size + 1, false);
                if (file.Type == EntryType::Null) { Kernel::Debug.Error("Unexpected error while creating file for writing"); return false; }

                // sector data array
                byte* temp = (byte*)MemAlloc(FS_SIZE_SECTOR);
                if (temp == nullptr) { Kernel::Debug.Error("Unable to allocate memory while creating file"); return false; }
                uint total = 0;

                // write data
                for (ulong sec = 0; sec < file.SectorCount; sec++)
                {
                    for (uint i = 0; i < FS_SIZE_SECTOR; i++)
                    {
                        if (total < size) { temp[i] = data[total]; }
                        total++;
                    }

                    // write data to sector
                    Kernel::ATA->Write(file.StartSector + sec, 1, temp);
                }

                // success
                if (write) { WriteTables(); }
                MemFree(temp);
                Kernel::Debug.OK("Finished writing data to %s", path);
                return true;
            }

            // error
            Kernel::Debug.WriteLine("Unexpected error while writing all data");
            return false;
        }

        bool FSHost::IOWriteAllLines(char* path, char** lines, uint count, bool write)
        {
            Kernel::Debug.Info("IOWriteAllLines - NOT YET IMPLEMENTED");
            return false;
        }
        
        char* FSHost::IOReadAllText(char* path)
        {
            // validate file
            if (!IOFileExists(path)) { Kernel::Debug.Error("Unable to locate file for reading"); return nullptr; }

            // read file
            FileEntry* fileptr = GetFileByName(path);
            if (fileptr == nullptr) { Kernel::Debug.Error("Unexpected error while reading file"); return nullptr; }

            // get data from file
            byte* data = ReadDataBlock(fileptr->StartSector, fileptr->SectorCount);
            if (data == nullptr) { Kernel::Debug.Error("Unexpected error reading data block during file read"); return nullptr; }

            // copy data to string
            char* output = (char*)MemAlloc(fileptr->Size + 2);
            for (size_t i = 0; i < fileptr->Size; i++) { output[i] = data[i]; }

            // free unused data and return string
            if (data != nullptr) { MemFree(data); }
            return output;
        }

        byte* FSHost::IOReadAllBytes(char* path)
        {
            // validate file
            if (!IOFileExists(path)) { Kernel::Debug.Error("Unable to locate file for reading"); return nullptr; }

            // read file
            FileEntry* fileptr = GetFileByName(path);
            if (fileptr == nullptr) { Kernel::Debug.Error("Unexpected error while reading file"); return nullptr; }

            // get data from file
            byte* data = ReadDataBlock(fileptr->StartSector, fileptr->SectorCount);
            if (data == nullptr) { Kernel::Debug.Error("Unexpected error reading data block during file read"); return nullptr; }

            // return file data
            return data;
        }

        char** FSHost::IOReadAllLines(char* path, uint* count)
        {
            // validate file
            if (!IOFileExists(path)) { Kernel::Debug.Error("Unable to locate file for reading all lines"); return nullptr; }

            // read data from file
            char* file_text = IOReadAllText(path);

            // split by newline
            uint lines_count = 1;

            // count lines
            for (uint i = 0; i < StringUtil::Length(file_text); i++)
            {
                if (file_text[i] == '\n') { lines_count++; }
            }

            // allocate new array
            char** output = (char**)MemAlloc(sizeof(char*) * lines_count);
            char* temp = (char*)MemAlloc(16384);

            // add lines
            int index = 0;
            int len = 0;
            for (uint i = 0; i < StringUtil::Length(file_text); i++)
            {
                if (file_text[i] == '\n' || i == StringUtil::Length(file_text) - 1)
                {
                    output[index] = (char*)MemAlloc(len + 1, true, AllocationType::String);
                    StringUtil::Copy(output[index], temp);
                    index++;
                    len = 0;
                    StringUtil::Clear(temp);
                }
                else
                {
                    StringUtil::Append(temp, file_text[i]);
                    len++;
                }
            }

            // return list
            MemFree(temp);
            MemFree(file_text);
            *count = lines_count;
            return output;
        }

        DirectoryEntry** FSHost::IOGetDirectories(char* path, uint* count)
        {
            // output array
            uint output_len = 0;
            DirectoryEntry** output = nullptr;

            // get parent directory
            DirectoryEntry* parent = GetDirectoryByName(path);
            uint parent_index = GetFileIndex(parent);
            if (parent == nullptr && !StringUtil::Equals(path, "/")) { Kernel::Debug.Error("Unable to locate parent while getting list of directories"); return nullptr; }
            if (StringUtil::Equals(path, "/")) { parent_index = 0; }

            // count amount of entries in directory
            for (uint i = 0; i < SuperBlock.EntryTable.SizeInBytes; i += FS_SIZE_FILE_ENTRY)
            {
                // get entry
                DirectoryEntry* entry = (DirectoryEntry*)(EntryTableData + i);

                // found valid entry
                if (entry->Type == EntryType::Directory && StringUtil::Length(entry->Name) > 0 && entry->ParentIndex == parent_index && entry->Status != 1) { output_len++; }
            }

            output = (DirectoryEntry**)MemAlloc(output_len * sizeof(DirectoryEntry*));

            // add directories to output
            int index = 0;
            for (uint i = 0; i < SuperBlock.EntryTable.SizeInBytes; i += FS_SIZE_FILE_ENTRY)
            {
                // get entry
                DirectoryEntry* entry = (DirectoryEntry*)(EntryTableData + i);

                // found valid entry
                if (entry->Type == EntryType::Directory && StringUtil::Length(entry->Name) > 0 && entry->ParentIndex == parent_index && entry->Status != 1)
                {
                    // create new entry
                    DirectoryEntry* dir = (DirectoryEntry*)MemAlloc(FS_SIZE_FILE_ENTRY);

                    // copy properties
                    Memory::Copy(dir->Name, entry->Name, 58);
                    dir->ParentIndex = entry->ParentIndex;
                    dir->Status = entry->Status;
                    dir->Type = entry->Type;

                    // add to list
                    output[index] = dir;

                    index++;
                }
            }

            // set length and return output
            *count = output_len;
            return output;
        }

        FileEntry** FSHost::IOGetFiles(char* path, uint* count)
        {
            // output array
            uint output_len = 0;
            FileEntry** output = nullptr;

            // get parent directory
            DirectoryEntry* parent = GetDirectoryByName(path);
            uint parent_index = GetFileIndex(parent);
            if (parent == nullptr && !StringUtil::Equals(path, "/")) { Kernel::Debug.Error("Unable to locate parent while getting list of files"); return nullptr; }
            if (StringUtil::Equals(path, "/")) { parent_index = 0; }

            // count amount of entries in directory
            for (uint i = 0; i < SuperBlock.EntryTable.SizeInBytes; i += FS_SIZE_FILE_ENTRY)
            {
                // get entry
                FileEntry* entry = (FileEntry*)(EntryTableData + i);

                // found valid entry
                if (entry->Type == EntryType::File && StringUtil::Length(entry->Name) > 0 && entry->ParentIndex == parent_index) { output_len++; }
            }

            output = (FileEntry**)MemAlloc(output_len * sizeof(FileEntry*));

            // add directories to output
            int index = 0;
            for (uint i = 0; i < SuperBlock.EntryTable.SizeInBytes; i += FS_SIZE_FILE_ENTRY)
            {
                // get entry
                FileEntry* entry = (FileEntry*)(EntryTableData + i);

                // found valid entry
                if (entry->Type == EntryType::File && StringUtil::Length(entry->Name) > 0 && entry->ParentIndex == parent_index)
                {
                    // create new entry
                    FileEntry* file = (FileEntry*)MemAlloc(FS_SIZE_FILE_ENTRY);

                    // copy properties
                    Memory::Copy(file->Name, entry->Name, 58);
                    file->ParentIndex = entry->ParentIndex;
                    file->Status = entry->Status;
                    file->Type = entry->Type;
                    file->StartSector = entry->StartSector;
                    file->SectorCount = entry->SectorCount;
                    file->Size = entry->Size;

                    // add to list
                    output[index] = file;
                    index++;
                }
            }

            // set length and return output
            *count = output_len;
            return output;
        }
    }
}