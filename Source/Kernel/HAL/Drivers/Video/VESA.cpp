#include <Kernel/HAL/Drivers/Video/VESA.hpp>
#include <Kernel/Core/Kernel.hpp>

#define LNG_PTR(seg, off) ((seg << 4) | off)
#define REAL_PTR(arr) LNG_PTR(arr[1], arr[0])
#define SEG(addr) (((uint)addr >> 4) & 0xF000)
#define OFF(addr) ((uint)addr & 0xFFFF)

namespace PMOS
{
    namespace HAL
    {
        namespace Drivers
        {

            VESAController::VESAController() : Service("vesa", ServiceType::Driver)
            {
                
            }

            void VESAController::Initialize()
            {
                Service::Initialize();

                Kernel::ServiceMgr.Register(this);
                Kernel::ServiceMgr.Start(this);
            }

            void VESAController::Start()
            {
                Service::Start();

                // reset buffer
                Buffer = nullptr;
                BufferSize = 0;

                // reset flags
                InfoBlockSet = false;

                // default mode properties
                CurrentMode.Width  = 0;
                CurrentMode.Height = 0;
                CurrentMode.Depth  = 0;
            }

            void VESAController::Stop()
            {
                Service::Stop();

                Kernel::MemoryMgr.Free(Buffer);
            }

            void VESAController::Display()
            {
                uint dest       = ModeInfoBlock.PhysicalBase;
                uint src        = (uint)Buffer;
                uint num_dwords = BufferSize / 4;
                uint num_bytes  = BufferSize % 4;
                uint *dest32    = (uint*)dest;
                uint *src32     = (uint*)src;
                byte *dest8      = ((byte*)dest) + num_dwords * 4;
                byte *src8       = ((byte*)src) + num_dwords * 4;
                uint i;

                for (i = 0; i < num_dwords; i++) { dest32[i] = src32[i]; }
                for (i = 0; i < num_bytes; i++) { dest8[i] = src8[i]; }
            }

            void VESAController::PopulateInfoBlock()
            {
                // skip if already set
                if (InfoBlockSet) { return; }

                // info block pointer
                VESAInfoBlock* info = (VESAInfoBlock*)0x80000;

                // call bios registers
                Registers16 regs;
                regs.AX = 0x4F00;
                regs.ES = 0x8000;
                regs.DI = 0x0000;
                CallInterrupt16(0x10, &regs);

                // finish
                InfoBlock = *info;
                InfoBlockSet = true;

                // get available modes
                GetAvailableModes();
            }

            void VESAController::GetAvailableModes()
            {
                // temporary data
                Registers16  regs;
                VESAModeInfoBlock* info = (VESAModeInfoBlock*)(0x80000 + sizeof(VESAInfoBlock) + 512);
                ushort* modes = (ushort*)REAL_PTR(InfoBlock.VideoMode);
                uint  mode = 0, index = 0, mode_id = 0;
                bool      set = false;
                char      temp[16];

                // loop through modes
                for (size_t i = 0; modes[i] != 0xFFFF; i++)
                {
                    // set up interrupt to get mode info
                    regs.AX = 0x4F01;
                    regs.CX = modes[i];
                    regs.ES = SEG(info);
                    regs.DI = OFF(info);
                    CallInterrupt16(0x10, &regs);

                    // add 32-bit modes only to available modes list
                    if (info->Depth == 32)
                    {
                        // set mode properties and increment
                        AvailableModes[mode_id].Width  = info->Width;
                        AvailableModes[mode_id].Height = info->Height;
                        AvailableModes[mode_id].Depth  = 32;
                        mode_id++;
                    }
                }
            }

            bool VESAController::SetMode(short w, short h)
            {
                if (!Started) { return false; }
                
                // delete old buffer
                if (Buffer != nullptr) { Kernel::MemoryMgr.Free(Buffer); Buffer = nullptr; }

                // populate info block
                PopulateInfoBlock();
                
                // data declarations
                Registers16 regs;
                VESAModeInfoBlock* info = (VESAModeInfoBlock*)(0x80000 + sizeof(VESAInfoBlock) + 512);
                ushort* modes = (ushort*)REAL_PTR(InfoBlock.VideoMode);

                ushort mode;
                for (int i = 0; modes[i] != 0xFFFF; i++)
                {
                    // set up interrupt to get mode info
                    regs.AX = 0x4F01;
                    regs.CX = modes[i];
                    regs.ES = SEG(info);
                    regs.DI = OFF(info);

                    // call interrupt
                    CallInterrupt16(0x10, &regs);

                    // check if mode is valid
                    if (info->Width == w && info->Height == h && info->Depth == 32)
                    {
                        mode = modes[i];
                        
                        size_t size = (info->Height * info->Pitch);
                        Buffer = Kernel::MemoryMgr.Allocate(size * 4, true, AllocationType::FrameBuffer);

                        CurrentMode.Width  = info->Width;
                        CurrentMode.Height = info->Height;
                        CurrentMode.Depth  = 32;

                        regs.AX = 0x4F02;
                        regs.BX = mode | 0x4000;
                        CallInterrupt16(0x10, &regs);

                        ModeInfoBlock = *info;
                        BufferSize = ModeInfoBlock.Height * ModeInfoBlock.Pitch;
                        return true;
                    }
                }
                return false;
            }

            bool VESAController::IsModeAvailable(short w, short h)
            {
                for (size_t i = 0; i < 256; i++)
                {
                    // get current mode
                    VESAMode mode = AvailableModes[i];

                    // check if mode is valid
                    if (mode.Width > 0)
                    {
                        if (mode.Width == w && mode.Height == h) { return true; }
                    }
                }
                return false;
            }

            void VESAController::Clear() { Clear(0x00000000); }

            void VESAController::Clear(uint color)
            {
                uint dest = (uint)Buffer;
                uint num_dwords = BufferSize / 4;
                uint num_bytes = BufferSize % 4;
                uint *dest32 = (uint*)dest;
                byte *dest8 = ((byte*)dest)+num_dwords*4;
                byte val8 = (byte)color;
                uint i;

                for (i=0;i < num_dwords;i++) { dest32[i] = color; }
                for (i=0;i < num_bytes;i++) { dest8[i] = val8; }
            }

            void VESAController::ClearDirect() { ClearDirect(0x00000000); }

            void VESAController::ClearDirect(uint color)
            {
                uint dest = (uint)ModeInfoBlock.PhysicalBase;
                uint num_dwords = BufferSize / 4;
                uint num_bytes = BufferSize % 4;
                uint *dest32 = (uint*)dest;
                byte *dest8 = ((byte*)dest)+num_dwords*4;
                byte val8 = (byte)color;
                uint i;

                for (i=0;i<num_dwords;i++) { dest32[i] = color; }
                for (i=0;i<num_bytes;i++) { dest8[i] = val8; }
            }

            // put pixel at specified position of buffer
            void VESAController::SetPixel(short x, short y, uint color) { ((uint*)Buffer)[x + (y * CurrentMode.Width)] = color; }

            // put pixel at specified index of buffer
            void VESAController::SetPixel(int index, uint color) { ((uint*)Buffer)[index] = color; }

            // put pixel at specified position of screen
            void VESAController::SetPixelDirect(short x, short y, uint color) { ((uint*)ModeInfoBlock.PhysicalBase)[x + (y * CurrentMode.Width)] = color; }

            // put pixel at specified index of screen
            void VESAController::SetPixelDirect(int index, uint color) { ((uint*)ModeInfoBlock.PhysicalBase)[index] = color; }
            
            // get pixel at specified position of buffer
            uint VESAController::GetPixel(short x, short y) { return ((uint*)Buffer)[x + (y * CurrentMode.Width)]; }

            // get pixel at specified index of buffer
            uint VESAController::GetPixel(int index) { return ((uint*)Buffer)[index]; }

            // get pixel at specified position of screen
            uint VESAController::GetPixelDirect(short x, short y) { return ((uint*)ModeInfoBlock.PhysicalBase)[x + (y * CurrentMode.Width)]; }

            // get pixel at specified index of screen
            uint VESAController::GetPixelDirect(int index) { return ((uint*)ModeInfoBlock.PhysicalBase)[index]; }

            // get screen Width
            ushort VESAController::GetWidth()  { return CurrentMode.Width; }

            // get screen Height
            ushort VESAController::GetHeight() { return CurrentMode.Height; }

            // get color depth
            ColDepth VESAController::GetDepth() { return (ColDepth)CurrentMode.Depth; }

            // get buffer
            void* VESAController::GetBuffer() { return Buffer; }
        }
    }
}