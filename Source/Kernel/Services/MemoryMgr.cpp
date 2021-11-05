#include <Kernel/Services/MemoryMgr.hpp>
#include <Kernel/Core/Kernel.hpp>

namespace PMOS
{
    namespace Services
    {

        void MemoryManager::Initialize()
        {
            MessagesEnabled = true;
            uint start = Kernel::GetEndAddress() & 0xFFFFF000;
            start += 0x1000;
            Header.MMapStart = start;

            ReadMemoryMap();

            uint tableStart = ((Header.MMapStart + Header.MMapSize) & 0xFFFFF000) + MM_ALIGN;
            
            Header.TableMaxEntries = 65536;
            Header.TableStart = tableStart;
            Header.TableLength = Header.TableMaxEntries * sizeof(HeapEntry);
            Header.TablePosition = 0;
            Header.TableEntries = 0;
            
            uint dataStart = ((Header.TableStart + Header.TableLength) & 0xFFFFF000) + MM_ALIGN;
            Header.DataStart = dataStart;
            Header.DataLength = Header.DataEnd - Header.DataStart;
            Header.DataUsed = 0;

            Memory::Set((void*)Header.TableStart, 0, Header.TableLength);
            Memory::Set((void*)Header.DataStart, 0, Header.DataLength);

            CreateEntry({ Header.DataStart, Header.DataLength, (byte)AllocationType::Unused });

            Kernel::Debug.Info("TABLE START      0x%8x", Header.TableStart);
            Kernel::Debug.Info("TABLE SIZE       %d KB", Header.TableLength / 1024);
            Kernel::Debug.Info("DATA START       0x%8x", Header.DataStart);
            Kernel::Debug.Info("DATA END         0x%8x", Header.DataEnd);
            Kernel::Debug.Info("DATA SIZE        %d MB", Header.DataLength / 1024 / 1024);
        }

        void MemoryManager::ToggleMessages(bool state) { MessagesEnabled = state; }
        
        void MemoryManager::ReadMemoryMap()
        {
            if (!MemoryMapReady)
            {
                Header.MMapSize = Kernel::Multiboot.MemoryMapLength;
                Header.MMapCount = 0;
                int pos = 0;

                for (uint i = 0; i < Header.MMapSize; i += sizeof(MemoryMapEntry))
                {
                    MemoryMapEntry* entry = (MemoryMapEntry*)(Kernel::Multiboot.MemoryMapAddress + i);

                    if (entry->Type > 0)
                    {
                        Memory::Copy((void*)(Header.MMapStart + pos), entry, sizeof(MemoryMapEntry));

                        if (entry->Type == 0x01 && i > 0)
                        {
                            Header.DataEnd = entry->AddressLow + entry->LengthLow;
                            KernelSize = Kernel::GetEndAddress() - entry->AddressLow;
                        }

                        Header.MMapCount++;
                        pos += sizeof(MemoryMapEntry);
                    }
                }

                MemoryMapReady = true;
            }
        }

        void MemoryManager::PrintTable(DebugMode mode)
        {
            DebugMode oldMode = Kernel::Debug.Mode;
            Kernel::Debug.SetMode(mode);
            Kernel::Debug.WriteUnformatted("-------- ", Col4::DarkGray);
            Kernel::Debug.WriteUnformatted("HEAP TABLE", Col4::Green);
            Kernel::Debug.WriteUnformatted(" --------------------------------");
            Kernel::Debug.NewLine();
            Kernel::Debug.WriteUnformatted("ID        ADDR          TYPE    SIZE\n", Col4::DarkGray);

            for (uint i = 0; i < Header.TableMaxEntries; i++)
            {
                HeapEntry* entry = GetEntry(i);

                if (entry->Base > 0)
                {
                    Col4 old = Kernel::Terminal->GetForeColor();
                    Kernel::Terminal->SetForeColor(Col4::Gray);
                    Kernel::Debug.Write("0x%4x    ", i);
                    Kernel::Terminal->SetForeColor(Col4::White);
                    Kernel::Debug.Write("0x%8x    ", entry->Base);
                    Kernel::Terminal->SetForeColor(Col4::Gray);
                    Kernel::Debug.Write("0x%2x    ", entry->Type);
                    Kernel::Terminal->SetForeColor(Col4::Yellow);

                    char sizemsg[8];
                    String::Clear(sizemsg);
                    uint size = 0;

                    if (entry->Size < 1024) { String::Copy(sizemsg, "BYTES"); size = entry->Size; }
                    if (entry->Size >= 1024 && entry->Size < 1024 * 1024) { String::Copy(sizemsg, "KB    "); size = entry->Size / 1024; }
                    else if (entry->Size >= 1024 * 1024 && entry->Size < 1024 * 1024 * 1024) { String::Copy(sizemsg, "MB    "); size = entry->Size / 1024 / 1024; }
                    else if (entry->Size >= 1024 * 1024 * 1024) { String::Copy(sizemsg, "GB    "); size = entry->Size / 1024 / 1024 / 1024; }

                    Kernel::Debug.Write("%d ", size);
                    Kernel::Terminal->SetForeColor(old);
                    Kernel::Terminal->SetCursorX(37);
                    Kernel::Debug.WriteLine(sizemsg);
                }
            }

            Kernel::Debug.NewLine();
            Kernel::Debug.SetMode(oldMode);
        }

        void MemoryManager::PrintMemoryMap(DebugMode mode)
        {
            DebugMode oldMode = Kernel::Debug.Mode;
            Kernel::Debug.SetMode(mode);
            Kernel::Debug.WriteUnformatted("-------- ", Col4::DarkGray);
            Kernel::Debug.WriteUnformatted("MMAP TABLE", Col4::Green);
            Kernel::Debug.WriteUnformatted(" --------------------------------");
            Kernel::Debug.NewLine();
            Kernel::Debug.WriteUnformatted("INDEX     ADDR          TYPE    SIZE\n", Col4::DarkGray);

            for (uint i = 0; i < Header.MMapCount; i++)
            {
                MemoryMapEntry* entry = (MemoryMapEntry*)(Header.MMapStart + (i * sizeof(MemoryMapEntry)));
                Col4 old = Kernel::Terminal->GetForeColor();
                Kernel::Terminal->SetForeColor(Col4::Gray);
                Kernel::Debug.Write("0x%4x    ", i);
                Kernel::Terminal->SetForeColor(Col4::White);
                Kernel::Debug.Write("0x%8x    ", entry->AddressLow);
                Kernel::Terminal->SetForeColor(Col4::Gray);
                Kernel::Debug.Write("0x%2x    ", entry->Type);
                Kernel::Terminal->SetForeColor(Col4::Yellow);

                char sizemsg[8];
                String::Clear(sizemsg);
                uint size = 0;

                if (entry->LengthLow < 1024) { String::Copy(sizemsg, "BYTES"); size = entry->LengthLow; }
                if (entry->LengthLow >= 1024 && entry->LengthLow < 1024 * 1024) { String::Copy(sizemsg, "KB    "); size = entry->LengthLow / 1024; }
                else if (entry->LengthLow >= 1024 * 1024 && entry->LengthLow < 1024 * 1024 * 1024) { String::Copy(sizemsg, "MB    "); size = entry->LengthLow / 1024 / 1024; }
                else if (entry->LengthLow >= 1024 * 1024 * 1024) { String::Copy(sizemsg, "GB    "); size = entry->LengthLow / 1024 / 1024 / 1024; }

                Kernel::Debug.Write("%d", size);
                Kernel::Terminal->SetForeColor(old);
                Kernel::Terminal->SetCursorX(38);
                Kernel::Debug.WriteLine(" %s", sizemsg);       
            }
            Kernel::Debug.NewLine();
            Kernel::Debug.SetMode(oldMode);
        }

        void MemoryManager::PrintAllocation(HeapEntry* entry)
        {
            char temp[16];
            Kernel::Debug.WriteUnformatted("[");
            Kernel::Debug.WriteUnformatted("MALLOC", Col4::Cyan);
            Kernel::Debug.WriteUnformatted("] ");
            Kernel::Debug.WriteUnformatted("ADDR", Col4::Green);
            Kernel::Debug.WriteUnformatted(" = ");
            Kernel::Debug.Write("0x%8x", entry->Base);
            Kernel::Debug.WriteUnformatted("  TYPE", Col4::Green);
            Kernel::Debug.WriteUnformatted(" = ");
            Kernel::Debug.Write("0x%2x", (uint)entry->Type);
            Kernel::Debug.WriteUnformatted("  SIZE", Col4::Green);
            Kernel::Debug.WriteUnformatted(" = ");
            Kernel::Debug.WriteLine("%d", entry->Size);
        }

        void MemoryManager::PrintFree(HeapEntry* entry)
        {
            char temp[16];
            Kernel::Debug.WriteUnformatted("[");
            Kernel::Debug.WriteUnformatted(" FREE ", Col4::Yellow);
            Kernel::Debug.WriteUnformatted("] ");
            Kernel::Debug.WriteUnformatted("ADDR", Col4::Green);
            Kernel::Debug.WriteUnformatted(" = ");
            Kernel::Debug.Write("0x%8x", entry->Base);
            Kernel::Debug.WriteUnformatted("  TYPE", Col4::Green);
            Kernel::Debug.WriteUnformatted(" = ");
            Kernel::Debug.Write("0x%2x", (uint)entry->Type);
            Kernel::Debug.WriteUnformatted("  SIZE", Col4::Green);
            Kernel::Debug.WriteUnformatted(" = ");
            Kernel::Debug.WriteLine("%d", entry->Size);
        }

        uint Align(uint addr)
        {
            uint out = addr;
            out &= 0xFFFFF000;
            if (out < addr) { out += MM_ALIGN; }
            return out;
        }

        void* MemoryManager::Allocate(uint size) { Allocate(size, false, AllocationType::Default); }

        void* MemoryManager::Allocate(uint size, bool clear, AllocationType type)
        {
            uint real_size = size;
            if (size == 0) { return nullptr; }
            size = Align(size);
            
            HeapEntry* entry = GetFreeEntry(size);
            if (entry == nullptr) { Kernel::Debug.Panic((int)Exception::OutOfMemory); return nullptr; }

            if (clear) { Memory::Set((void*)entry->Base, 0, entry->Length); }

            if (type == AllocationType::Unused) { type = AllocationType::Default; }
            entry->Type = (byte)type;
            entry->Size = real_size;

            Header.DataUsed += size;
            GetEntry(0)->Size = GetEntry(0)->Length;
            if (!IsAddressValid(entry->Base)) { Kernel::Debug.Panic("Invalid pointer after allocation"); return nullptr; }
            if (MessagesEnabled) { PrintAllocation(entry); }
            return (void*)entry->Base;
        }

        void MemoryManager::Free(void* ptr)
        {
            if (!IsAddressValid((uint)ptr)) { return; }   

            for (uint i = 0; i < Header.TableMaxEntries; i++)
            {
                HeapEntry* temp = GetEntry(i);

                if (temp->Base == (uint)ptr)
                {
                    if (MessagesEnabled) { PrintFree(temp); }
                    Header.DataUsed -= temp->Length;
                    Memory::Set((void*)temp->Base, 0, temp->Length);
                    temp->Type = (byte)AllocationType::Unused;
                    MergeFreeEntries();
                    GetEntry(0)->Size = GetEntry(0)->Length;
                    return;
                }
            }

            Kernel::Debug.Warning("Unable to free memory at 0x%8x", (uint)ptr);
        }

        void MemoryManager::FreeArray(void** ptr, uint len)
        {
            if (ptr == nullptr) { return; }
            for (size_t i = 0; i < len; i++)
            {
                if (ptr[i] != nullptr) { Free(ptr[i]); }
            }
            Free(ptr);
        }

        void MemoryManager::MergeFreeEntries()
        {
            HeapEntry* mass = GetEntry(0);
            uint free_total = 0;
            uint free_addr = 0xFFFFFFFF;

            // loop through entries
            for (uint i = 0; i < Header.TableMaxEntries; i++)
            {
                // get entry
                HeapEntry* entry = GetEntry(i);
                
                // validate entry
                if (IsAddressValid(entry->Base) && entry->Type == (byte)AllocationType::Unused)
                {
                    // get nearest
                    HeapEntry* nearest = GetNeighbour(entry);

                    // validate nearest entry
                    if (nearest != nullptr && nearest != entry && nearest != mass && entry != mass)
                    {
                        if (entry->Base > nearest->Base) { entry->Base = nearest->Base; }
                        entry->Length += nearest->Length;
                        DeleteEntry(nearest);
                    }
                }
            }

            // attempt to free mass neighbour
            HeapEntry* nearest = GetNeighbour(mass);

            // loop through entries
            for (uint i = 0; i < Header.TableMaxEntries; i++)
            {
                // get entry
                HeapEntry* entry = GetEntry(i);

                if (entry->Base != mass->Base)
                {
                    if (entry->Base + entry->Length == mass->Base && entry->Type == (byte)AllocationType::Unused)
                    {
                        mass->Base = entry->Base;
                        mass->Length += entry->Length;
                        mass->Type = (byte)AllocationType::Unused;
                        DeleteEntry(entry);
                        break;
                    }
                }
            }
        }

        HeapEntry* MemoryManager::GetEntry(int index)
        {
            if (index < 0 || index >= Header.TableMaxEntries) { return nullptr; }
            return (HeapEntry*)(Header.TableStart + (index * sizeof(HeapEntry)));
        }

        HeapEntry* MemoryManager::GetFreeEntry(uint size)
        {
            if (size == 0) { return nullptr; }

            for (uint i = 0; i < Header.TableMaxEntries; i++)
            {
                HeapEntry* entry = GetEntry(i);

                if (IsAddressValid(entry->Base) && size == entry->Length && entry->Type == (byte)AllocationType::Unused)
                {
                    entry->Type = (byte)AllocationType::Default;
                    return entry;
                }
            }

            HeapEntry* mass = GetEntry(0);
            uint addr = mass->Base;

            HeapEntry* entry = CreateEntry({ addr, size, (byte)AllocationType::Default });
            mass->Base += size;
            mass->Length -= size;
            mass->Type = (byte)AllocationType::Unused;
            return entry;
        }

        int MemoryManager::GetFreeIndex()
        {
            for (int i = 0; i < Header.TableMaxEntries; i++)
            {
                HeapEntry* entry = GetEntry(i);
                if (entry == nullptr) { continue; }
                if (entry->Base == 0) { return i; }
            }
            return -1;
        }

        HeapEntry* MemoryManager::GetNeighbour(HeapEntry* entry)
        {
            if (entry == nullptr) { return nullptr; }
            if (!IsAddressValid(entry->Base)) { return nullptr; }
            
            for (uint i = 1; i < Header.TableMaxEntries; i++)
            {
                HeapEntry* temp = GetEntry(i);

                if (temp != nullptr && temp != entry && temp->Base + temp->Length == entry->Base && temp->Type == (byte)AllocationType::Unused) { return temp; }
                if (temp != nullptr && temp != entry && entry->Base - entry->Length == temp->Base && temp->Type == (byte)AllocationType::Unused) { return temp; }
            }
            return nullptr;
        }

        HeapEntry* MemoryManager::CreateEntry(HeapEntry entry)
        {
            if (!IsAddressValid(entry.Base)) { return nullptr; }   
            if (entry.Length == 0) { return nullptr; }

            HeapEntry* new_entry = GetEntry(GetFreeIndex());
            if (new_entry == nullptr) { Kernel::Debug.Panic((int)Exception::OutOfMemory); return nullptr; }

            new_entry->Base = entry.Base;
            new_entry->Length = entry.Length;
            new_entry->Type = entry.Type;

            Header.TablePosition++;
            Header.TableEntries++;
            return new_entry;
        }

        bool MemoryManager::DeleteEntry(HeapEntry* entry)
        {
            if (entry == nullptr) { return false; }
            if (entry->Base == 0) { return false; }

            for (uint i = 0; i < Header.TableMaxEntries; i++)
            {
                HeapEntry* temp = GetEntry(i);
                if (temp->Base == entry->Base && temp->Length == entry->Length && temp->Type == entry->Type)
                {
                    temp->Base   = 0;
                    temp->Length = 0;
                    temp->Type   = 0;
                    Header.TableEntries--;
                    Header.TablePosition--;
                    return true;
                }
            }

            Kernel::Debug.Warning("Unable to delete heap entry { BASE:0x%8x LEN:0x%8x TYPE:0x%2x", entry->Base, entry->Length, entry->Type);
            return false;
        }

        bool MemoryManager::IsAddressValid(uint addr)
        {
            if (addr < Header.DataStart || addr >= Header.DataEnd) { return false; }
            return true;
        }

        HeapEntry* MemoryManager::GetEntryFromPtr(void* ptr)
        {
            if (ptr == nullptr) { return nullptr;}
            for (uint i = 0; i < Header.TableMaxEntries; i++)
            {
                HeapEntry* entry = GetEntry(i);
                if (entry->Base == (uint)ptr) { return entry; }
            }
            return nullptr;
        }

        uint MemoryManager::GetSizeFromPtr(void* ptr)
        {
            if (ptr == nullptr) { return 0;}
            for (uint i = 0; i < Header.TableMaxEntries; i++)
            {
                HeapEntry* entry = GetEntry(i);
                if (entry->Base == (uint)ptr) { return entry->Length; }
            }
            return 0;
        }
        
        uint MemoryManager::GetHeapCount() { return Header.TableEntries; }

        uint MemoryManager::GetUsedHeapCount() 
        { 
            Header.TableEntriesUsed = 0;
            for (uint i = 0; i < Header.TableMaxEntries; i++)
            {
                HeapEntry* entry = GetEntry(i);
                if (IsAddressValid(entry->Base) && entry->Type != (byte)AllocationType::Unused) { Header.TableEntriesUsed++; }
            }
            return Header.TableEntriesUsed;
        }

        uint MemoryManager::GetRAMInstalled()
        {
            register ulong* mem;
            ulong mem_count, a;
            uint memkb;
            byte	irq1, irq2;

            irq1 = HAL::Ports::Read8(0x21);
            irq2 = HAL::Ports::Read8(0xA1);

            HAL::Ports::Write8(0x21, 0xFF);
            HAL::Ports::Write8(0xA1, 0xFF);

            mem_count = 0;
            memkb = 0;
            __asm__ __volatile__("wbinvd");

            do 
            {
                memkb++;
                mem_count += 1024 * 1024;
                mem = (ulong*)mem_count;

                a = *mem;
                *mem = 0x55AA55AA;

                asm("":::"memory");
                if (*mem != 0x55AA55AA) { mem_count = 0; }
                else 
                {
                    *mem = 0xAA55AA55;
                    asm("":::"memory");
                    if (*mem != 0xAA55AA55)
                        mem_count = 0;
                }

                asm("":::"memory");
                *mem = a;

            } while (memkb < 4096 && mem_count != 0);

            mem = (ulong*)0x413;

            HAL::Ports::Write8(0x21, irq1);
            HAL::Ports::Write8(0xA1, irq2);

            return memkb * 1024 * 1024;
        }

        uint MemoryManager::GetRAMReserved() { return Header.DataLength; }
        
        uint MemoryManager::GetRAMFree() { return Header.DataLength - Header.DataUsed; }
        
        uint MemoryManager::GetRAMUsed() { return Header.DataUsed; }
    }
}