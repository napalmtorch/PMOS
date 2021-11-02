#include <Kernel/HAL/Drivers/Storage/ATA.hpp>
#include <Kernel/Core/Kernel.hpp>

// ports
#define ATA_PRIMARY_DATA         0x1F0
#define ATA_PRIMARY_ERR          0x1F1
#define ATA_PRIMARY_SECCOUNT     0x1F2
#define ATA_PRIMARY_LBA_LO       0x1F3
#define ATA_PRIMARY_LBA_MID      0x1F4
#define ATA_PRIMARY_LBA_HI       0x1F5
#define ATA_PRIMARY_DRIVE_HEAD   0x1F6
#define ATA_PRIMARY_COMM_REGSTAT 0x1F7
#define ATA_PRIMARY_ALTSTAT_DCR  0x3F6

// status flags
#define ATA_STAT_ERR  (1 << 0) 
#define ATA_STAT_DRQ  (1 << 3)
#define ATA_STAT_SRV  (1 << 4)
#define ATA_STAT_DF   (1 << 5) 
#define ATA_STAT_RDY  (1 << 6)
#define ATA_STAT_BSY  (1 << 7)

namespace PMOS
{
    namespace HAL
    {
        namespace Drivers
        {
            ATAController::ATAController() : Service("atadrv", ServiceType::Driver)
            {

            }

            void ATAController::Initialize()
            {
                Service::Initialize();

                Kernel::ServiceMgr.Register(this);
                Kernel::ServiceMgr.Start(this);
            }

            void ATAController::Start()
            {
                Service::Start();
            }

            void ATAController::Stop()
            {
                Service::Stop();
            }

            byte ATAController::Identify()
            {
                if (!Started) { return 0; }
                Ports::Read8 (ATA_PRIMARY_COMM_REGSTAT);
                Ports::Write8(ATA_PRIMARY_DRIVE_HEAD, 0xA0);
                Ports::Read8 (ATA_PRIMARY_COMM_REGSTAT);
                Ports::Write8(ATA_PRIMARY_SECCOUNT, 0);
                Ports::Read8 (ATA_PRIMARY_COMM_REGSTAT);
                Ports::Write8(ATA_PRIMARY_LBA_LO, 0);
                Ports::Read8 (ATA_PRIMARY_COMM_REGSTAT);
                Ports::Write8(ATA_PRIMARY_LBA_MID, 0);
                Ports::Read8 (ATA_PRIMARY_COMM_REGSTAT);
                Ports::Write8(ATA_PRIMARY_LBA_HI, 0);
                Ports::Read8 (ATA_PRIMARY_COMM_REGSTAT);
                Ports::Write8(ATA_PRIMARY_COMM_REGSTAT, 0xEC);
                Ports::Write8(ATA_PRIMARY_COMM_REGSTAT, 0xE7);

                // Read the status port. If it's zero, the drive does not exist.
                byte status = Ports::Read8(ATA_PRIMARY_COMM_REGSTAT);

                while(status & ATA_STAT_BSY) 
                {
                    uint i = 0;
                    while(1) { i++; }
                    for(i = 0; i < 0x0FFFFFFF; i++) { }
                    status = Ports::Read8(ATA_PRIMARY_COMM_REGSTAT);
                }
                
                if(status == 0) { asm volatile("sti"); return 0; }

                while(status & ATA_STAT_BSY) { status = Ports::Read8(ATA_PRIMARY_COMM_REGSTAT); }

                byte mid = Ports::Read8(ATA_PRIMARY_LBA_MID);
                byte hi = Ports::Read8(ATA_PRIMARY_LBA_HI);
                if(mid || hi) { asm volatile("sti"); return 0; }


                // Wait for ERR or DRQ
                while(!(status & (ATA_STAT_ERR | ATA_STAT_DRQ))) {
                    status = Ports::Read8(ATA_PRIMARY_COMM_REGSTAT);
                }

                if(status & ATA_STAT_ERR) { asm volatile("sti"); return 0; }

                byte buff[256 * 2];
                Ports::ReadString(ATA_PRIMARY_DATA, buff, 256);

                return 1;
            }

            void ATAController::Read(ulong lba, ushort sectors, byte* dest)
            {
                if (!Started) { return; }

                 // HARD CODE MASTER (for now)
                Ports::Write8(ATA_PRIMARY_DRIVE_HEAD, 0x40);                     // Select master
                Ports::Write8(ATA_PRIMARY_SECCOUNT, (sectors >> 8) & 0xFF );     // sectorcount high
                Ports::Write8(ATA_PRIMARY_LBA_LO, (lba >> 24) & 0xFF);           // LBA4
                Ports::Write8(ATA_PRIMARY_LBA_MID, (lba >> 32) & 0xFF);          // LBA5
                Ports::Write8(ATA_PRIMARY_LBA_HI, (lba >> 40) & 0xFF);           // LBA6
                Ports::Write8(ATA_PRIMARY_SECCOUNT, sectors & 0xFF);             // sectorcount low
                Ports::Write8(ATA_PRIMARY_LBA_LO, lba & 0xFF);                   // LBA1
                Ports::Write8(ATA_PRIMARY_LBA_MID, (lba >> 8) & 0xFF);           // LBA2
                Ports::Write8(ATA_PRIMARY_LBA_HI, (lba >> 16) & 0xFF);           // LBA3
                Ports::Write8(ATA_PRIMARY_COMM_REGSTAT, 0x24);                   // READ SECTORS EXT
                //Debug.ThrowMessage("Reading %d sectors from disk", sectors, DebugMessage::System);

                byte i;
                for(i = 0; i < sectors; i++) 
                {
                    // poll
                    while(1) 
                    {
                        byte status = Ports::Read8(ATA_PRIMARY_COMM_REGSTAT);
                        if(status & ATA_STAT_DRQ) { break; }
                        if (status & ATA_STAT_ERR) { Kernel::Debug.Panic("Error reading sector from disk"); break; }
                    }
                    // Transfer the data!
                    Ports::ReadString(ATA_PRIMARY_DATA, (unsigned char *)dest, 256);
                    dest += 256;
                }
            }

            void ATAController::Write(ulong lba, ushort sectors, byte* src)
            {
                if (!Started) { return; }

                // HARD CODE MASTER (for now)
                Ports::Write8(ATA_PRIMARY_DRIVE_HEAD, 0x40);                     // Select master
                Ports::Write8(ATA_PRIMARY_SECCOUNT, (sectors >> 8) & 0xFF );     // sectorcount high
                Ports::Write8(ATA_PRIMARY_LBA_LO, (lba >> 24) & 0xFF);           // LBA4
                Ports::Write8(ATA_PRIMARY_LBA_MID, (lba >> 32) & 0xFF);          // LBA5
                Ports::Write8(ATA_PRIMARY_LBA_HI, (lba >> 40) & 0xFF);           // LBA6
                Ports::Write8(ATA_PRIMARY_SECCOUNT, sectors & 0xFF);             // sectorcount low
                Ports::Write8(ATA_PRIMARY_LBA_LO, lba & 0xFF);                   // LBA1
                Ports::Write8(ATA_PRIMARY_LBA_MID, (lba >> 8) & 0xFF);           // LBA2
                Ports::Write8(ATA_PRIMARY_LBA_HI, (lba >> 16) & 0xFF);           // LBA3
                Ports::Write8(ATA_PRIMARY_COMM_REGSTAT, 0x34);                   // READ SECTORS EXT
                //Debug.ThrowMessage("Writing %d sectors to disk", sectors, DebugMessage::System);

                byte i;
                for(i = 0; i < sectors; i++) 
                {
                    // POLL!
                    while(1) 
                    {
                        byte status = Ports::Read8(ATA_PRIMARY_COMM_REGSTAT);
                        if(status & ATA_STAT_DRQ) { break; }
                        else if(status & ATA_STAT_ERR) { Kernel::Debug.Panic("Error writing sector to disk"); break; }
                    }

                    // Transfer the data!
                    Ports::WriteString(ATA_PRIMARY_DATA, (unsigned char *)src, 256);
                    src += 256;
                }

                // Flush the cache.
                Ports::Write8(ATA_PRIMARY_COMM_REGSTAT, 0xE7);
                // Poll for BSY.
                while(Ports::Read8(ATA_PRIMARY_COMM_REGSTAT) & ATA_STAT_BSY);
            }
        }
    }
}