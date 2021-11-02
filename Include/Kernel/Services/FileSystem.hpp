#pragma once
#include <Kernel/Lib/Types.hpp>
#include <Kernel/Core/Service.hpp>

namespace PMOS
{
        namespace VFS
    {
        // file entry type
        enum class EntryType : byte
        {
            Null,
            Directory,
            File,
        };

        // get entry type string
        char* GetEntryTypeString(EntryType type);

        // file system states
        #define FS_STATE_FREE 0
        #define FS_STATE_USED 1

        // sector size
        #define FS_SIZE_SECTOR      512
        #define FS_SIZE_BLOCK_ENTRY 16
        #define FS_SIZE_FILE_ENTRY  64

        // sector positions
        #define FS_SECTOR_SUPER       0
        #define FS_SECTOR_BLOCK_TABLE 1

        // structure for managing various physical table attributes
        typedef struct
        {
            uint StartSector;
            uint EndSector;
            uint SectorCount;
            uint SizeInBytes;
            uint MaxEntries;
        } ATTR_PACK TableHeader;

        // structure for managing disk and entry information
        typedef struct
        {
            uint	SectorCount;
            uint	BytesPerSector;
            TableHeader BlockTable;
            TableHeader EntryTable;
            TableHeader Data;
            byte     Reserved[444];
        } ATTR_PACK SuperBlockHeader;
        

        // structure for managing block entries
        typedef struct
        {
            uint Sector;
            uint Count;
            byte  State;
            byte  Reserved[7];
        } ATTR_PACK BlockEntry;

            // structure for managing directory entries
        typedef struct
        {
            uint  ParentIndex;
            EntryType Type;
            byte   Status;
            char	  Name[58];
        } ATTR_PACK DirectoryEntry;

            // structure for managing file entries
        typedef struct
        {
            uint      ParentIndex;
            EntryType Type;
            byte      Status;
            char      Name[38];
            uint      StartSector;
            uint      SectorCount;
            uint      Size;
            byte*     Data;
        } ATTR_PACK FileEntry;

        class FSHost : public Service
        {
            private:
                // private properties
                SuperBlockHeader SuperBlock;
                DirectoryEntry   RootDirectory;
                byte*            BlockTableData;
                byte*            EntryTableData;
                uint             BlockCount;
                uint             EntryCount;
                uint             DiskSize;
                bool             Mounted;

            public:
                FSHost();
                void Initialize() override;
                void Start() override;
                void Stop() override;

                // mount state
                void Mount();
                void Unmount();
                bool IsMounted();
                // formatting
                void Format(uint size, bool wipe);
                void Wipe();
                void WriteTables();
                // print information
                void PrintDiskInformation(bool debug);
                void PrintBlockTable(bool debug);
                void PrintEntryTable(bool debug);
                void PrintDirectoryContents(char* path);
                // super block
                void CreateSuperBlock(uint disk_size);
                void CopySuperBlock(SuperBlockHeader* dest, SuperBlockHeader* src);
                void ReadSuperBlock();
                void WriteSuperBlock();
                // data blocks
                void CreateBlockTable();
                void ReadBlockTable();
                void WriteBlockTable();
                byte*	ReadDataBlock(uint sector, uint count);
                BlockEntry* AllocateBlock(uint sectors);
                bool        FreeBlock(uint sector, uint count);
                BlockEntry* GetAvailableBlock(uint sectors);
                void		MergeAvailableBlocks();
                BlockEntry* GetNearestBlock(BlockEntry* entry);
                BlockEntry* CreateBlockEntry(uint sector, uint count, byte state);
                bool        DeleteBlockEntry(BlockEntry* entry);
                BlockEntry* GetBlockEntry(uint sector, uint count, byte state);
                bool IsBlockTableValid();
                bool IsEntryTableValid();
                void CalculateBlockCount();
                void CalculateEntryCount();
                // file entrys
                void CreateEntryTable();
                void CreateRootDirectory();
                void ReadEntryTable();
                void WriteEntryTable();
                DirectoryEntry* CreateDirectoryEntry(DirectoryEntry src);
                FileEntry*		CreateFileEntry(FileEntry src);
                bool			CopyFileEntry(void* dest, void* src);
                bool			DeleteFileEntry(void* entry);
                void*		    GetAvailableEntry();
                DirectoryEntry* GetParentFromPath(char* path);
                FileEntry*		GetFileByName(char* path);
                DirectoryEntry* GetDirectoryByName(char* path);
                int			GetFileIndex(void* src);
                void*			GetEntryAtIndex(int index);
                
            public:
                char*                       IOGetParent(char* path, uint depth);
                char*                       IOGetParent(char* path);
                FileEntry                   IOOpenFile(char* path);
                FileEntry					IOCreateFile(char* path, uint size);
                FileEntry					IOCreateFile(char* path, uint size, byte* data);
                FileEntry					IOCreateFile(char* path, uint size, bool write);
                FileEntry					IOCreateFile(char* path, uint size, byte* data, bool write);
                DirectoryEntry				IOCreateDirectory(char* path);
                DirectoryEntry				IOCreateDirectory(char* path, bool write);
                bool						IOFileExists(char* path);
                bool						IODirectoryExists(char* path);
                FileEntry					IOCopyFile(char* dest, char* src);
                DirectoryEntry				IOCopyDirectory(char* dest, char* src);
                FileEntry					IOMoveFile(char* dest, char* src);
                DirectoryEntry				IOMoveDirectory(char* dest, char* src);
                bool						IODeleteFile(char* path);
                bool						IODeleteDirectory(char* path);
                bool						IORenameFile(char* path, char* name);
                bool						IORenameDirectory(char* path, char* name);
                bool						IOWriteAllText(char* path, char* text);
                bool						IOWriteAllBytes(char* path, byte* data, uint size);
                bool						IOWriteAllLines(char* path, char** lines, uint count);
                bool						IOWriteAllText(char* path, char* text, bool write);
                bool						IOWriteAllBytes(char* path, byte* data, uint size, bool write);
                bool						IOWriteAllLines(char* path, char** lines, uint count, bool write);
                char*					    IOReadAllText(char* path);
                byte*				     	IOReadAllBytes(char* path);
                char**                      IOReadAllLines(char* path, uint* count);
                DirectoryEntry**            IOGetDirectories(char* path, uint* count);
                FileEntry**  		        IOGetFiles(char* path, uint* count);

            private:
                void InitTableArrays();
        };
    }
}