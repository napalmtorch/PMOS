#pragma once
#include <Kernel/Lib/Types.hpp>
#include <Kernel/Core/Service.hpp>
#include <Kernel/Core/Debug.hpp>

#define MM_ALIGN 0x1000

namespace PMOS
{
    enum class AllocationType : byte
    {
        Unused,
        Default,
        String,
        Bitmap,
        System,
        Thread,
        ThreadStack,
        FrameBuffer,
        PCIDevice,
        VMRAM,
        UI,
    };

    typedef struct
    {
        uint Size;
        uint AddressLow;
        uint AddressHigh;
        uint LengthLow;
        uint LengthHigh;
        uint Type;
    } ATTR_PACK MemoryMapEntry;

    typedef struct
    {
        uint Base;
        uint Length;
        uint Size;
        byte Type;
    } ATTR_PACK HeapEntry;

    typedef struct
    {
        uint TableStart;
        uint TableLength;
        uint TablePosition;
        uint TableEntries;
        uint TableMaxEntries;
        uint DataStart;
        uint DataEnd;
        uint DataLength;
        uint DataUsed;
        uint MMapStart;
        uint MMapSize;
        uint MMapCount;
    } ATTR_PACK HeapHeader;

    namespace Services
    {
        class MemoryManager
        {           
            private:
                HeapHeader Header;
                bool MessagesEnabled;
                bool MemoryMapReady;
             
            public:
                void Initialize();
                void ToggleMessages(bool state);
                void ReadMemoryMap();        

            public:
                void PrintTable(DebugMode mode);
                void PrintMemoryMap(DebugMode mode);
                void PrintAllocation(HeapEntry* entry);
                void PrintFree(HeapEntry* entry);

            public:
                void* Allocate(uint size);
                void* Allocate(uint size, bool clear, AllocationType type);
                void  Free(void* ptr);
                void  FreeArray(void** ptr, uint len);
                void MergeFreeEntries();

            public: 
                HeapEntry* GetEntry(int index);
                HeapEntry* GetFreeEntry(uint size);
                int        GetFreeIndex();
                HeapEntry* GetNeighbour(HeapEntry* entry);
                HeapEntry* CreateEntry(HeapEntry entry);
                bool       DeleteEntry(HeapEntry* entry);
                bool       IsAddressValid(uint addr);

            public:
                HeapEntry* GetEntryFromPtr(void* ptr);
                uint       GetSizeFromPtr(void* ptr);
                uint       GetHeapCount();
                uint       GetRAMInstalled();
                uint       GetRAMReserved();
                uint       GetRAMFree();
                uint       GetRAMUsed();
        };
    }
}