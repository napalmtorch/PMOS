#pragma once
#include <Kernel/Lib/Types.hpp>
#include <Kernel/Core/Service.hpp>

namespace PMOS
{
    namespace HAL
    {
        namespace Drivers
        {
            class ATAController : public Service
            {
                public:
                    ATAController();
                    void Initialize() override;
                    void Start() override;
                    void Stop() override;

                public:
                    byte Identify();
                    void Read(ulong lba, ushort sectors, byte* dest);
                    void Write(ulong lba, ushort sectors, byte* src);
            };
        }
    }
}