#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <stddef.h>

namespace FileSystem
{
	// file entry types
	enum class EntryType : uint8_t
	{
		Null,
		Directory,
		File,
	};

	// get entry type string
	std::string GetEntryTypeString(EntryType type);

	// file system states
	#define FS_STATE_FREE 0
	#define FS_STATE_USED 1

	// sector size
	#define FS_SIZE_SECTOR 512
	#define FS_SIZE_BLOCK_ENTRY 16
	#define FS_SIZE_FILE_ENTRY 64

	// sector positions
	#define FS_SECTOR_SUPER 0
	#define FS_SECTOR_BLOCK_TABLE 1

	// pack following structures
	#pragma pack(push, 1)

	// structure for managing various physical table attributes
	typedef struct
	{
		uint32_t StartSector;
		uint32_t EndSector;
		uint32_t SectorCount;
		uint32_t SizeInBytes;
		uint32_t MaxEntries;
	} TableHeader;
		
	// structure for managing disk and entry information
	typedef struct
	{
		uint32_t	SectorCount;
		uint32_t	BytesPerSector;
		TableHeader BlockTable;
		TableHeader EntryTable;
		TableHeader Data;
		uint8_t     Reserved[444];
	} SuperBlockHeader;

	// structure for managing block entries
	typedef struct
	{
		uint32_t Sector;
		uint32_t Count;
		uint8_t  State;
		uint8_t  Reserved[7];
	} BlockEntry;

	// structure for managing directory entries
	typedef struct
	{
		uint32_t  ParentIndex;
		EntryType Type;
		uint8_t   Status;
		char	  Name[58];
	} DirectoryEntry;

	// structure for managing directory entries
	typedef struct
	{
		uint32_t  ParentIndex;
		EntryType Type;
		uint8_t   Status;
		char      Name[38];
		uint32_t  StartSector;
		uint32_t  SectorCount;
		uint32_t  Size;
		uint8_t*  Data;
	} FileEntry;

	// finish packing structures
	#pragma pack(pop)

	// file system controller
	class FSHost
	{
		private:
			// private properties
			SuperBlockHeader SuperBlock;
			DirectoryEntry   RootDirectory;
			uint8_t*         BlockTableData;
			uint8_t*		 EntryTableData;
			uint32_t		 BlockCount;
			uint32_t		 EntryCount;
			bool			 Mounted;

		public:
			// control
			void Mount();
			void Unmount();
			bool IsMounted();
			void Format(uint32_t size);
			void Wipe();
			void WriteTables();
			void PrintDiskInformation();
			void PrintBlockTable();
			void PrintEntryTable();
			void PrintDirectoryContents(std::string path);
			// super block
			void CreateSuperBlock(uint32_t disk_size);
			void CopySuperBlock(SuperBlockHeader* dest, SuperBlockHeader* src);
			void ReadSuperBlock();
			void WriteSuperBlock();
			// data blocks
			void CreateBlockTable();
			void ReadBlockTable();
			void WriteBlockTable();
			uint8_t*	ReadDataBlock(uint32_t sector, uint32_t count);
			BlockEntry* AllocateBlock(uint32_t sectors);
			bool        FreeBlock(uint32_t sector, uint32_t count);
			BlockEntry* GetAvailableBlock(uint32_t sectors);
			void		MergeAvailableBlocks();
			BlockEntry* GetNearestBlock(BlockEntry* entry);
			BlockEntry* CreateBlockEntry(uint32_t sector, uint32_t count, uint8_t state);
			bool        DeleteBlockEntry(BlockEntry* entry);
			BlockEntry* GetBlockEntry(uint32_t sector, uint32_t count, uint8_t state);
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
			DirectoryEntry* GetParentFromPath(std::string path);
			FileEntry*		GetFileByName(std::string path);
			DirectoryEntry* GetDirectoryByName(std::string path);
			int32_t			GetFileIndex(void* src);
			void*			GetEntryAtIndex(int32_t index);
			
		public:
			FileEntry					IOOpenFile(std::string path);
			FileEntry					IOImportFile(std::string dest, std::string src);
			FileEntry					IOCreateFile(std::string path, uint32_t size);
			FileEntry					IOCreateFile(std::string path, uint32_t size, uint8_t* data);
			DirectoryEntry				IOCreateDirectory(std::string path);
			bool						IOFileExists(std::string path);
			bool						IODirectoryExists(std::string path);
			FileEntry					IOCopyFile(std::string dest, std::string src);
			DirectoryEntry				IOCopyDirectory(std::string dest, std::string src);
			FileEntry					IOMoveFile(std::string dest, std::string src);
			DirectoryEntry				IOMoveDirectory(std::string dest, std::string src);
			bool						IODeleteFile(std::string path);
			bool						IODeleteDirectory(std::string path);
			bool						IORenameFile(std::string path, std::string name);
			bool						IORenameDirectory(std::string path, std::string name);
			bool						IOWriteAllText(std::string path, std::string text);
			bool						IOWriteAllBytes(std::string path, uint8_t* data, uint32_t size);
			bool						IOWriteAllLines(std::string path, std::vector<std::string> lines);
			std::string					IOReadAllText(std::string path);
			uint8_t*					IOReadAllBytes(std::string path);
			std::vector<std::string>	IOReadAllLines(std::string path);
			std::vector<DirectoryEntry> IOGetDirectories(std::string path);
			std::vector<FileEntry>		IOGetFiles(std::string path);

		private:
			void InitTableArrays();

	};

	// file system instance
	extern FSHost MasterFS;
}