#pragma once
#include <Kernel/Lib/Types.hpp>

namespace PMOS
{
    namespace Memory
    {
        static inline void* Set(void* dest, int data, uint size)
        {
            uint num_dwords = size / 4;
            uint num_bytes = size % 4;
            uint *dest32 = (uint*)dest;
            byte *dest8 = ((byte*)dest) + num_dwords * 4;
            byte val8 = (byte)data;
            uint val32 = data | (data << 8) | (data << 16) | (data << 24);
            uint i;

            for (i = 0; i < num_dwords; i++) { dest32[i] = val32; }
            for (i = 0; i < num_bytes;  i++) { dest8[i] = val8; }
            return dest;
        }

        static inline void* Copy(void* dest, void* src, uint size)
        {
            uint buffer_size = size;
            uint destt       = (uint)dest;
            uint srcc        = (uint)src;
            uint num_dwords = buffer_size / 4;
            uint num_bytes  = buffer_size % 4;
            uint *dest32    = (uint*)destt;
            uint *src32     = (uint*)srcc;
            byte *dest8      = ((byte*)destt) + num_dwords * 4;
            byte *src8       = ((byte*)srcc) + num_dwords * 4;
            uint i;

            for (i = 0; i < num_dwords; i++) { dest32[i] = src32[i]; }
            for (i = 0; i < num_bytes; i++) { dest8[i] = src8[i]; }
            return dest;
        }

        static inline void* Move(void* dest, void* src, uint size)
        {
            uint buffer_size = size;
            uint destt       = (uint)dest;
            uint srcc        = (uint)src;
            uint num_dwords = buffer_size / 4;
            uint num_bytes  = buffer_size % 4;
            uint *dest32    = (uint*)destt;
            uint *src32     = (uint*)srcc;
            byte *dest8      = ((byte*)destt) + num_dwords * 4;
            byte *src8       = ((byte*)srcc) + num_dwords * 4;
            uint i;

            for (i = 0; i < num_dwords; i++) { dest32[i] = src32[i]; }
            for (i = 0; i < num_bytes; i++) { dest8[i] = src8[i]; }
            Set(src, 0, size);
            return dest;
        }

        static inline int Compare(void* p1, void* p2, uint size)
        {
            byte *p = (byte*)p1;
            byte *q = (byte*)p2;
            int charCompareStatus = 0;

            if (p1 == p2) { return charCompareStatus; }
            while (size > 0)
            {
                if (*p != *q) { charCompareStatus = (*p > *q) ? 1 : -1; break; }
                size--;
                p++;
                q++;
            }
            return charCompareStatus;
        } 
    }
}

// allocate overloads
extern void *operator new(size_t size);
extern void *operator new[](size_t size);

// delete overloads
extern void operator delete(void *p);
extern void operator delete(void *p, size_t size);
extern void operator delete[](void *p);
extern void operator delete[](void *p, size_t size);