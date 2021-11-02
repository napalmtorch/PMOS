#pragma once
#include <Kernel/Lib/Types.hpp>

namespace PMOS
{
    namespace HAL
    {
        typedef struct
        {
            bool PSE, PAE, APIC, MTRR;
        } ATTR_PACK CPUFeatures;

        typedef struct
        {
            bool MMX, TSC, MSR, AVX, F16C, RDRAND;
            bool SSE, SSE2, SSE3, SSSE3, SSE41, SSE42;
        } ATTR_PACK CPUInstructions;

        class CPUManager
        {
            public:
                char* Vendor;
                char* Name;
                CPUFeatures Features;
                CPUInstructions Instructions;
                bool X64Compatible;

            public:
                void Detect();

            private:
                void GetCPUInfo(uint reg, uint* eax, uint* ebx, uint* ecx, uint* edx);
        };
    }
}