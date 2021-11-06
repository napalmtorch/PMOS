#include <Kernel/HAL/PCI.hpp>
#include <Kernel/Core/Kernel.hpp>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

namespace PMOS
{
    namespace HAL
    {
        void PCIBusController::Initialize()
        {
            if (Initialized) { return; }

            Probe();
            Kernel::Debug.OK("Finished enumerating PCI devices");

            Initialized = true;
        }

        void PCIBusController::Probe()
        {
            if (Devices != nullptr) 
            { 
                for (int i = 0; i < MaxCount; i++) { if (Devices[i] != nullptr) { MemFree(Devices[i]); } }
                MemFree(Devices); 
            }

            Devices = (PCIDevice**)MemAlloc(MaxCount * sizeof(PCIDevice*), true, AllocationType::System);
            Count = 0;

            for (ushort bus = 0; bus < 256; bus++)
            {
                for (ushort slot = 0; slot < 32; slot++)
                {
                    for (ushort func = 0; func < 8; func++)
                    {
                        ushort vendorID   = ReadWord(bus, slot, func, 0x00);

                        if (vendorID != 0xFFFF)
                        {
                            ushort deviceID   = ReadWord(bus, slot, func, 0x02);
                            ushort command    = ReadWord(bus, slot, func, 0x04);
                            ushort status     = ReadWord(bus, slot, func, 0x06);
                            byte   revID      = ReadWord(bus, slot, func, 0x08);
                            byte   subclassID = (byte)(ReadWord(bus, slot, func, 0x0A) & ~0xFF00);
                            ushort classID    = ReadWord(bus, slot, func, 0x0B) >> 8;
                            byte   clSize     = (byte)(ReadWord(bus, slot, func, 0x0C) >> 8);
                            byte   latTimer   = (byte)(ReadWord(bus, slot, func, 0x0D) >> 8);
                            byte   headerType = (byte)(ReadWord(bus, slot, func, 0x0E) >> 8);
                            byte   bist       = (byte)(ReadWord(bus, slot, func, 0x0F) >> 8);
                            uint   bar0       = (uint)((ReadWord(bus, slot, func, 0x10) << 16) | ReadWord(bus, slot, func, 0x12));

                            PCIDevice* device = (PCIDevice*)MemAlloc(sizeof(PCIDevice), true, AllocationType::PCIDevice);
                            StringUtil::Copy(device->Name, (char*)GetDeviceName(vendorID, deviceID));
                            device->VendorID = vendorID;
                            device->DeviceID = deviceID;
                            device->Command  = command;
                            device->Status = status;
                            device->RevisionID = revID;
                            device->Subclass = subclassID;
                            device->Class = classID;
                            device->CacheLineSize = clSize;
                            device->LatencyTimer = latTimer;
                            device->HeaderType = headerType;
                            device->BIST = bist;
                            device->BAR0 = bar0;
                            Devices[Count] = device;
                            Kernel::Debug.WriteLine("Located device: 0x%4x:0x%4x", Devices[Count]->VendorID, Devices[Count]->DeviceID);
                            Count++;
                        }
                    }
                }
            }
            
            Kernel::Debug.WriteLine("PCI DEVICES: %d", Count);
        }
        
        void PCIBusController::List(DebugMode mode)
        {
            DebugMode oldMode = Kernel::Debug.Mode;
            Kernel::Debug.SetMode(mode);
            Kernel::Debug.WriteUnformatted("-------- ", Col4::DarkGray);
            Kernel::Debug.WriteUnformatted("DEVICES", Col4::Green);
            Kernel::Debug.WriteUnformatted(" -----------------------------------");
            Kernel::Debug.NewLine();
            Kernel::Debug.WriteUnformatted("ID      VEN_ID  DEV_ID  NAME\n", Col4::DarkGray);

            for (size_t i = 0; i < MaxCount; i++)
            {
                if (Devices[i] == nullptr) { continue; }
                
                Kernel::Debug.Write("0x%4x  ", (uint)i);
                Kernel::Debug.Write("0x%4x  ", (uint)Devices[i]->VendorID);
                Kernel::Debug.Write("0x%4x  ", (uint)Devices[i]->DeviceID);
                Kernel::Debug.Write(Devices[i]->Name);
                Kernel::Debug.NewLine();
            }

            Kernel::Debug.NewLine();
            Kernel::Debug.SetMode(oldMode);
        }

        bool PCIBusController::IsInitialized() { return Initialized; }

        void PCIBusController::WriteWord(ushort bus, ushort slot, ushort func, ushort offset, ushort data)
        {
            uint address = (uint)((bus << 16) | (slot << 1) | (func << 8) | (offset & 0xFC) | ((uint)0x80000000));
            Ports::Write32(PCI_CONFIG_ADDRESS, address);
            Ports::Write32(PCI_CONFIG_DATA, data);
        }

        ushort PCIBusController::ReadWord(ushort bus, ushort slot, ushort func, ushort offset)
        {
            uint address;
            uint lbus = (uint)bus;
            uint lslot = (uint)slot;
            uint lfunc = (uint)func;
            ushort temp = 0;
            address = (uint)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xFC) | ((uint)0x80000000));
            Ports::Write32(PCI_CONFIG_ADDRESS, address);
            temp = (ushort)((Ports::Read32(PCI_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF);
            return (temp);
        }

        PCIDevice PCIBusController::GetDevice(int index)
        {
            if (index < 0 || index >= MaxCount) { return PCIDevice {}; }
            PCIDevice* p_device = Devices[index];
            PCIDevice device;
            Memory::Copy((void*)&device, (void*)p_device, sizeof(PCIDevice));
            return device;
        }

        const char* PCIBusController::GetDeviceName(ushort vendor_id, ushort device_id)
        {
            PCIVendor vendor = (PCIVendor)vendor_id;

            // intel devices
            if (vendor == PCIVendor::INTEL)
            {
                switch (device_id)
                {
                    case 0x1237: { return "Intel 440FX Chipset"; }
                    case 0x7000: { return "Intel 82371SB PIIX3 ISA"; }
                    case 0x7010: { return "Intel 82371SB PIIX3 IDE"; }
                    case 0x7020: { return "Intel 82371SB PIIX3 USB"; }
                    case 0x7111: { return "Intel 82371AB/EB/MB PIIX4 IDE"; }
                    case 0x7113: { return "Intel 82371AB/EB/MB PIIX4 ACPI"; }
                    case 0x100E: { return "Intel 82540EM Ethernet Controller"; }
                    case 0x0041: { return "Intel Core PCI Express x16 Root Port"; }
                    case 0x0042: { return "Intel Core IGPU Controller"; }
                    case 0x0044: { return "Intel Core DRAM Controller"; }
                    case 0x0600: { return "Intel RAID Controller"; }
                    case 0x061F: { return "Intel 80303 I/O Processor"; }
                    case 0x2415: { return "Intel 82801AA AC'97 Audio"; }
                    case 0x2668: { return "Intel 82801(ICH6 Family) HD Audio"; }
                    case 0x2770: { return "Intel 82945G Memory Controller Hub"; }
                    case 0x2771: { return "Intel 82945G PCI Express Root Port"; }
                    case 0x2772: { return "Intel 82945G Integrated Graphics"; }
                    case 0x2776: { return "Intel 82945G Integrated Graphics"; }
                    case 0x27D0: { return "Intel NM10/ICH7 PCI Express Port 1"; }
                    case 0x27D2: { return "Intel NM10/ICH7 PCI Express Port 2"; }
                    case 0x27C8: { return "Intel NM10/ICH7 USB UHCI Controller 1"; }
                    case 0x27C9: { return "Intel NM10/ICH7 USB UHCI Controller 2"; }
                    case 0x27CA: { return "Intel NM10/ICH7 USB UHCI Controller 3"; }
                    case 0x27CB: { return "Intel NM10/ICH7 USB UHCI Controller 4"; }
                    case 0x27CC: { return "Intel NM10/ICH7 USB2 EHCI Controller"; }
                    case 0x244E: { return "Intel 82801 PCI Bridge"; }
                    case 0x27DE: { return "Intel 82801G/ICH7 AC'97 Audio"; }
                    case 0x27B8: { return "Intel 82801GB/ICH7 LPC Interface Bridge"; }
                    case 0x27C0: { return "Intel NM10/ICH7 SATA Controller(IDE MODE)"; }
                    case 0x27DA: { return "Intel NM10/ICH7 SMBus Controller"; }
                    case 0x0F00: { return "Intel Atom Z36xxx/Z37xxx SoC Trans-Reg"; }
                    case 0x0F31: { return "Intel Atom Z36xxx/Z37xxx Series Graphics"; }
                    case 0x0F23: { return "Intel Atom E3800 Series SATA AHCI"; }
                    case 0x0F18: { return "Intel Atom Z36xxx/Z37xxx Trusted Exec."; }
                    case 0x0F04: { return "Intel Atom Z36xxx/Z37xxx HD Audio"; }
                    case 0x0F48: { return "Intel Atom E3800 Series PCI Express Port 1"; }
                    case 0x0F4A: { return "Intel Atom E3800 Series PCI Express Port 2"; }
                    case 0x0F34: { return "Intel Atom Z36xxx/Z37xxx USB EHCI"; }
                    case 0x0F1C: { return "Intel Atom Z36xxx/Z37xxx Series PCU"; }
                    case 0x0F12: { return "Intel Atom E3800 Series SMBus Controller"; }
                    default:     { return "Unrecognized Intel Device"; }
                }
            }

            // newer intel devices
            if (vendor == PCIVendor::INTEL_ALT)
            {
                switch (device_id)
                {
                    default: { return "Unrecognized Intel Device"; }
                }
            }

            // vmware
            if (vendor == PCIVendor::VMWARE)
            {
                switch (device_id)
                {
                    case 0x0405: { return "VMWare SVGAII Adapter"; }
                    case 0x0710: { return "VMWare SVGA Adapter"; }
                    case 0x0770: { return "VMWare USB2 EHCI Controller"; }
                    case 0x0720: { return "VMWare VMXNET Ethernet Controller"; }
                    case 0x0740: { return "VMWare VM Communication Interface"; }
                    case 0x0774: { return "VMWare USB1.1 UHCI Controller"; }
                    case 0x0778: { return "VMWare USB3 xHCI 0.96 Controller"; }
                    case 0x0779: { return "VMWare USB3 xHCI 1.00 Controller"; }
                    case 0x0790: { return "VMWare PCI Bridge"; }
                    case 0x07A0: { return "VMWare PCI Express Root Port"; }
                    case 0x07B0: { return "VMWare VMXNET3 Ethernet Controller"; }
                    case 0x07C0: { return "VMWare PVSCSI SCSI Controller"; }
                    case 0x07E0: { return "VMWare SATA AHCI Controller"; }
                    case 0x0801: { return "VMWare VM Interface"; }
                    case 0x0820: { return "VMWare Paravirtual RDMA Controller"; }
                    case 0x0800: { return "VMWare Hypervisor ROM Interface"; }
                    case 0x1977: { return "VMWare HD Audio Controller"; }
                    default: { return "Unrecognized VMWare Device"; }
                }
            }

            // vmware
            if (vendor == PCIVendor::VMWARE_ALT)
            {
                switch (device_id)
                {
                    default: { return "Unrecognized VMWare Device"; }
                }
            }

            // innotek
            if (vendor == PCIVendor::INNOTEK)
            {
                switch (device_id)
                {
                    case 0xBEEF: { return "VirtualBox Graphics Adapter"; }
                    case 0xCAFE: { return "VirtualBox Guest Service"; }
                    default: { return "Unrecognized InnoTek Device"; }
                }
            }

            // amd
            if (vendor == PCIVendor::AMD)
            {
                switch (device_id)
                {
                    case 0x2000: { return "AMD 79C970 Ethernet Controller"; }
                    default: { return "Unrecognized AMD Device"; }
                }
            }

            // ensoniq
            if (vendor == PCIVendor::ENSONIQ)
            {
                switch (device_id)
                {
                    case 0x5000: { return "Ensoniq ES1370 [AudioPCI]"; }
                    default: { return "Unrecognized Ensoniq Device"; }
                }
            }

            // broadcom
            if (vendor == PCIVendor::BROADCOM)
            {
                switch (device_id)
                {
                    case 0x1677: { return "NetXtreme BCM5751 Ethernet PCI-E"; }
                    default: { return "Unrecognized Realtek Device"; }
                }
            }

            // realtek
            if (vendor == PCIVendor::REALTEK)
            {
                switch (device_id)
                {
                    case 0x5289: { return "Realtek RTL8411 PCI Express Card Reader"; }
                    case 0x8168: { return "Realtek RTL8111/8168/8411 Ethernet PCI-E"; }
                    default: { return "Unrecognized Realtek Device"; }
                }
            }

            // atheros
            if (vendor == PCIVendor::ATHEROS)
            {
                switch (device_id)
                {
                    case 0x0036: { return "Atheros AR9485 WiFi Adapter"; }
                    default: { return "Unrecognized Atheros Device"; }
                }
            }

            // qemu graphics controller
            if (vendor_id == 0x1234 && device_id == 0x1111) { return "QEMU VGA Controller"; }

            // unknown
            return "Unrecognized Device";
        }
    }
}