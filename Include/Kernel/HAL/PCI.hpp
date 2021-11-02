#pragma once
#include <Kernel/Lib/Types.hpp>
#include <Kernel/Core/Debug.hpp>

namespace PMOS
{
    namespace HAL
    {
        enum class PCIVendor
        {
            INTEL                = 0x8086,
            INTEL_ALT            = 0x8087,
            VMWARE               = 0x15AD,
            VMWARE_ALT           = 0x0E0F,
            DELL                 = 0x1028,
            DELL_ALT             = 0x413C,
            ATI                  = 0x1002,
            AMD                  = 0x1022,
            VIA                  = 0x2109,
            BENQ                 = 0x04A5,
            ACER                 = 0x5986,
            HP                   = 0x03F0,
            HP_ALT               = 0x103C,
            HP_ENT               = 0x1590,
            ASUS                 = 0x1043,
            ASUS_ALT             = 0x0B05,
            INNOTEK              = 0x80EE,
            REALTEK              = 0x10EC,
            REAKLTEK_ALT         = 0x0BDA,
            ENSONIQ              = 0x1274,
            BROADCOM             = 0x14E4,
            ATHEROS              = 0x168C,
        };

        enum class DeviceType
        {
            UNCLASSIFIED        = 0x00,
            STORAGE             = 0x01,
            NETWORK             = 0x02,
            DISPLAY             = 0x03,
            MULTIMEDIA          = 0x04,
            MEMORY              = 0x05,
            BRIDGE              = 0x06,
            SIMPLE_COMM         = 0x07,
            SYS_PERIPHERAL      = 0x08,
            INPUT               = 0x09,
            DOCKING_STATION     = 0x0A,
            PROCESSOR           = 0x0B,
            SERIAL_BUS          = 0x0C,
            WIRELESS            = 0x0D,
            INTELLIGENT         = 0x0E,
            SATELLITE_COMM      = 0x0F,
            ENCRYPTION          = 0x10,
            SIGNAL_PROCESSOR    = 0x11,
            PROCESSING_ACCEL    = 0x12,
            NON_ESSENTIAL_INSTR = 0x13,
            CO_PROCESSOR        = 0x40,
            UNASSIGNED          = 0xFF,
        };

        typedef struct
        {
            ushort DeviceID;
            ushort VendorID;
            ushort Status;
            ushort Command;
            ushort Class;
            byte   Subclass;
            byte   RevisionID;
            byte   BIST;
            byte   HeaderType;
            byte   LatencyTimer;
            byte   CacheLineSize;
            uint   BAR0;
            uint   BAR1;
            uint   BAR2;
            uint   BAR3;
            uint   BAR4;
            uint   BAR5;
            uint   CardbusPtr;
            ushort SubsystemID;
            ushort SubsystemVendorID;
            uint   ROMExpansionBAR;
            ushort Reserved0;
            byte   CapabilitiesPtr;
            uint   Reserved;
            byte   MaxLatency;
            byte   MinGrant;
            byte   InterruptPin;
            byte   InterruptLine;
            char   Name[72];
        } ATTR_PACK PCIDevice;

        class PCIBusController
        {
            private:
                static const size_t MaxCount = 256;
                PCIDevice** Devices;
                bool Initialized;
                size_t Count = 0;

            public:
                void Initialize();
                void Probe();
                void List(DebugMode mode);
                bool IsInitialized();

            public:
                void WriteWord(ushort bus, ushort slot, ushort func, ushort offset, ushort data);
                ushort ReadWord(ushort bus, ushort slot, ushort func, ushort offset);

            public:
                PCIDevice GetDevice(int index);

            public:
                static const char* GetDeviceName(ushort vendor_id, ushort device_id);
        };
    }
}