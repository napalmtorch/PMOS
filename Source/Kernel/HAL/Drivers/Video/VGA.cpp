#include <Kernel/HAL/Drivers/Video/VGA.hpp>
#include <Kernel/Core/Kernel.hpp>

static byte Text80x25_Data[] = 
{
    /* MISC */
    0x67,
    /* SEQ */
    0x03, 0x00, 0x03, 0x00, 0x02,
    /* CRTC */
    0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F,
    0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x50,
    0x9C, 0x0E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3,
    0xFF,
    /* GC */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
    0xFF,
    /* AC */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x0C, 0x00, 0x0F, 0x08, 0x00
};

static byte Text80x50_Data[] = 
{
    /* MISC */
    0x67,
    /* SEQ */
    0x03, 0x00, 0x03, 0x00, 0x02,
    /* CRTC */
    0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F,
    0x00, 0x47, 0x06, 0x07, 0x00, 0x00, 0x01, 0x40,
    0x9C, 0x8E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3,
    0xFF, 
    /* GC */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
    0xFF, 
    /* AC */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x0C, 0x00, 0x0F, 0x08, 0x00,
};

static byte Text90x60_Data[] = 
{
    /* MISC */
    0xE7,
    /* SEQ */
    0x03, 0x01, 0x03, 0x00, 0x02,
    /* CRTC */
    0x6B, 0x59, 0x5A, 0x82, 0x60, 0x8D, 0x0B, 0x3E,
    0x00, 0x47, 0x06, 0x07, 0x00, 0x00, 0x00, 0x00,
    0xEA, 0x0C, 0xDF, 0x2D, 0x08, 0xE8, 0x05, 0xA3,
    0xFF,
    /* GC */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00,
    0xFF,
    /* AC */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x0C, 0x00, 0x0F, 0x08, 0x00,
};

static byte Pixel320x200_Data[] = 
{
    /* MISC */
    0x63,
    /* SEQ */
    0x03, 0x01, 0x0F, 0x00, 0x0E,
    /* CRTC */
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
    0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
    0xFF,
    /* GC */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
    0xFF,
    /* AC */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x41, 0x00, 0x0F, 0x00, 0x00
};

// data ports
#define PORT_SEQ_DATA 0x3C5
#define PORT_GC_DATA 0x3CF
#define PORT_CRTC_DATA 0x3D5
#define PORT_MASK_DATA 0x3DA
#define PORT_DAC_DATA 0x3C9

// write ports
#define PORT_AC_WRITE 0x3C0
#define PORT_MISC_WRITE 0x3C2
#define PORT_SEQ_WRITE 0x3C4
#define PORT_DAC_WRITE 0x3C8
#define PORT_GC_WRITE 0x3CE
#define PORT_CRTC_WRITE 0x3D4

// read ports
#define PORT_AC_READ 0x3C1
#define PORT_DAC_READ 0x3C7
#define PORT_INSTAT_READ 0x3DA

namespace PMOS
{
    namespace HAL
    {
        namespace Drivers
        {
            const char* VGAModeToString(VGAMode mode)
            {
                if (mode == VGAMode::Text80x25) { return "80x25(Text Mode)"; }
                if (mode == VGAMode::Text80x50) { return "80x50(Text Mode)"; }
                if (mode == VGAMode::Text90x60) { return "90x60(Text Mode)"; }
                if (mode == VGAMode::Pixel320x200) { return "320x200(Graphics Mode)"; }
                if (mode == VGAMode::Pixel320x200DB) { return "320x200(Graphics Mode)"; }
                return "Invalid";
            }

            VGAController::VGAController() : Service("vgadrv", ServiceType::Driver)
            {

            }

            void VGAController::Initialize()
            {
                Service::Initialize();

                Kernel::ServiceMgr.Register(this);
                Kernel::ServiceMgr.Start(this);
            }

            void VGAController::Start()
            {
                Service::Start();
                CurrentMode = VGAMode::Text80x25;
                Width = 80;
                Height = 25;
                Depth = ColDepth::Depth4Bit;
                TextMode = true;
                Buffer = (byte*)0xB8000;
                Clear(0x00);
                EnableCursor(0, 15);
                SetCursorPos(0, 0);
                DoubleBuffered = false;
            }

            void VGAController::Stop()
            {
                Service::Stop();

                CurrentMode = VGAMode::Text80x25;
                Width = 80;
                Height = 25;
                Depth = ColDepth::Depth4Bit;
                TextMode = true;
                DoubleBuffered = false;
            }

            void VGAController::SetMode(VGAMode mode)
            {
                if (!Started) { return; }
                switch (mode)
                {
                    case VGAMode::Text80x25:
                    {
                        Width  = 80;
                        Height = 25;
                        Depth  = ColDepth::Depth4Bit;
                        TextMode = true;
                        DoubleBuffered = false;
                        WriteRegisters(Text80x25_Data);
                        SetFont(Fonts::Serif8x16.Data, 16);
                        break;
                    }

                    case VGAMode::Text80x50:
                    {
                        Width  = 80;
                        Height = 50;
                        Depth  = ColDepth::Depth4Bit;
                        TextMode = true;
                        DoubleBuffered = false;
                        WriteRegisters(Text80x50_Data);
                        SetFont(Fonts::Serif8x8.Data, 8);
                        break;
                    }

                    case VGAMode::Text90x60:
                    {
                        Width  = 90;
                        Height = 60;
                        Depth  = ColDepth::Depth4Bit;
                        TextMode = true;
                        DoubleBuffered = false;
                        WriteRegisters(Text90x60_Data);
                        SetFont(Fonts::Serif8x8.Data, 8);
                        break;
                    }
                    
                    case VGAMode::Pixel320x200:
                    {
                        Width  = 320;
                        Height = 200;
                        Depth  = ColDepth::Depth8Bit;
                        TextMode = false;
                        DoubleBuffered = false;
                        WriteRegisters(Pixel320x200_Data);
                        break;
                    }

                    case VGAMode::Pixel320x200DB:
                    {
                        Width  = 320;
                        Height = 200;
                        Depth  = ColDepth::Depth8Bit;
                        TextMode = false;
                        DoubleBuffered = true;
                        WriteRegisters(Pixel320x200_Data);
                        break;
                    }

                    default: { Stop(); }
                }

                if (Started)
                {
                    Buffer = GetFrameBufferSegment();
                    if (DoubleBuffered) { BackBuffer = (byte*)MemAlloc(Width * Height, true, AllocationType::Bitmap); }
                    Kernel::Debug.Info("Set VGA mode to %s", VGAModeToString(mode));
                    Clear(0x00);
                    EnableCursor(0, 15);
                    SetCursorPos(0, 0);
                    return;
                }
                
                Kernel::Debug.Error("Unable to set VGA mode to %s", VGAModeToString(mode));
            }

            void VGAController::ClearText(byte fg, byte bg)
            {
                byte attr = (byte)((bg << 4) | fg);
                for (uint i = 0; i < Width * Height * 2; i += 2)
                {
                    Buffer[i + 0] = 0x20;
                    Buffer[i + 1] = attr;
                }
            }
                
            void VGAController::Clear(byte color)
            {
                if (TextMode) { ClearText(Buffer[1], color); return; }
                if (DoubleBuffered) { Memory::Set(BackBuffer, color, Width * Height); }
                Memory::Set(Buffer, color, Width * Height);
            }

            void VGAController::DrawPixel(ushort x, ushort y, byte color)
            {
                if (TextMode) { DrawChar(x, y, 0x20, color, color); return; }
                if (DoubleBuffered) { BackBuffer[y * Width + x] = color; return; }
                Buffer[y * Width + x] = color;
            }

            void VGAController::DrawChar(ushort x, ushort y, char c, byte fg, byte bg)
            {
                if (x < 0 || y < 0 || x >= Width || y >= Height) { return; }
                Buffer[2 * (y * Width + x) + 0] = c;
                Buffer[2 * (y * Width + x) + 1] = (byte)((bg << 4) | fg);
            }

            void VGAController::Swap()
            {
                if (!DoubleBuffered) { return; }
                Memory::Copy(Buffer, BackBuffer, Width * Height);
            }
    
            void VGAController::EnableCursor(byte start, byte end)
            {
                Ports::Write8(PORT_CRTC_WRITE, 0x0A);
                Ports::Write8(PORT_CRTC_DATA, (byte)((Ports::Read8(PORT_CRTC_DATA) & 0xC0) | start));
                Ports::Write8(PORT_CRTC_WRITE, 0x0B);
                Ports::Write8(PORT_CRTC_DATA, (byte)((Ports::Read8(PORT_CRTC_DATA) & 0xE0) | end));
            }

            void VGAController::DisableCursor()
            {
                Ports::Write8(PORT_CRTC_WRITE, 0x0A);
                Ports::Write8(PORT_CRTC_DATA, 0x20);
            }

            void VGAController::SetCursorPos(byte x, byte y)
            {
                if (!TextMode) { return; }
                uint offset = (uint)(y * Width + x);
                Ports::Write8(PORT_CRTC_WRITE, 0x0E);
                Ports::Write8(PORT_CRTC_DATA, (byte)((offset & 0xFF00) >> 8));
                Ports::Write8(PORT_CRTC_WRITE, 0x0F);
                Ports::Write8(PORT_CRTC_DATA, (byte)(offset & 0x00FF));
            }
        
            byte* VGAController::GetFrameBufferSegment()
            {
                Ports::Write8(PORT_GC_WRITE, 0x06);
                byte segNum = (byte)(Ports::Read8(PORT_GC_DATA) & (3 << 2));
                switch (segNum)
                {
                    default: { return (byte*)0; }
                    case 1 << 2: { return (byte*)0xA0000; }
                    case 2 << 2: { return (byte*)0xB0000; }
                    case 3 << 2: { return (byte*)0xB8000; }
                }
                return (byte*)0;
            }

            void VGAController::WriteRegisters(byte* regs)
            {
                Ports::Write8(PORT_MISC_WRITE, *(regs++));

                for (byte i = 0; i < 5; i++) { Ports::Write8(PORT_SEQ_WRITE, i); Ports::Write8(PORT_SEQ_DATA, *(regs++)); }

                Ports::Write8(PORT_CRTC_WRITE, 0x03);
                Ports::Write8(PORT_CRTC_DATA, (byte)(Ports::Read8(PORT_CRTC_DATA) | 0x80));
                Ports::Write8(PORT_CRTC_WRITE, 0x11);
                Ports::Write8(PORT_CRTC_DATA, (byte)(Ports::Read8(PORT_CRTC_DATA) | ~0x80));

                regs[0x03] = (byte)(regs[0x03] | 0x80);
                regs[0x11] = (byte)(regs[0x11] & ~0x80);
                for (byte i = 0; i < 25; i++) { Ports::Write8(PORT_CRTC_WRITE, i); Ports::Write8(PORT_CRTC_DATA, *(regs++)); }

                for (byte i = i = 0; i < 9; i++) { Ports::Write8(PORT_GC_WRITE, i); Ports::Write8(PORT_GC_DATA, *(regs++)); }

                byte val = 0;
                for (byte i = 0; i < 21; i++)
                {
                    val = Ports::Read8(PORT_INSTAT_READ);
                    Ports::Write8(PORT_AC_WRITE, i);
                    Ports::Write8(PORT_AC_WRITE, *(regs++));
                }

                val = Ports::Read8(PORT_INSTAT_READ);
                Ports::Write8(PORT_AC_WRITE, 0x20);
            }

            void VGAController::SetPlane(byte ap)
            {
                ap &= 3;
                byte pmask = (byte)(1 << ap);

                Ports::Write8(PORT_GC_WRITE, 4);
                Ports::Write8(PORT_GC_DATA, ap);
                Ports::Write8(PORT_SEQ_WRITE, 2);
                Ports::Write8(PORT_SEQ_DATA, pmask);
            }

            void VGAController::SetFont(byte* font, byte height)
            {
                byte seq2, seq4, gc4, gc5, gc6;

                Ports::Write8(PORT_SEQ_WRITE, 2);
                seq2 = Ports::Read8(PORT_SEQ_DATA);
                Ports::Write8(PORT_SEQ_DATA, 4);
                seq4 =Ports::Read8(PORT_SEQ_DATA);
                Ports::Write8(PORT_SEQ_DATA, (byte)(seq4 | 0x04));

                Ports::Write8(PORT_GC_WRITE, 4);
                gc4 = Ports::Read8(PORT_GC_DATA);
                Ports::Write8(PORT_GC_WRITE, 5);
                gc5 = Ports::Read8(PORT_GC_DATA);
                Ports::Write8(PORT_GC_DATA, gc5 & ~0x10);
                Ports::Write8(PORT_GC_WRITE, 6);
                gc6 = Ports::Read8(PORT_GC_DATA);
                Ports::Write8(PORT_GC_DATA, (byte)(gc6 & ~0x02));

                SetPlane(2);

                byte* seg = GetFrameBufferSegment();
                for (uint i = 0; i < 256; i++)
                { for (uint j = 0; j < height; j++) { seg[(i * 32) + j] = font[(i * height) + j]; } }

                Ports::Write8(PORT_SEQ_WRITE, 2);
                Ports::Write8(PORT_SEQ_DATA, seq2);
                Ports::Write8(PORT_SEQ_WRITE, 4);
                Ports::Write8(PORT_SEQ_DATA, seq4);

                Ports::Write8(PORT_GC_WRITE, 4);
                Ports::Write8(PORT_GC_DATA, gc4);
                Ports::Write8(PORT_GC_WRITE, 5);
                Ports::Write8(PORT_GC_DATA, gc5);
                Ports::Write8(PORT_GC_WRITE, 6);
                Ports::Write8(PORT_GC_DATA, gc6);
            }

            void VGAController::SetColorPalette(uint* colors, uint len)
            {
                for (uint i = 0; i < len; i++)
                {
                    if (!TextMode) { Ports::Write8(PORT_MASK_DATA, 0xFF); } else { Ports::Write8(PORT_MASK_DATA, 0x0F); }
                    Ports::Write8(PORT_DAC_WRITE, i);
                    Ports::Write8(PORT_DAC_DATA, (byte)(colors[i] & 0xFF0000) >> 16);
                    Ports::Write8(PORT_DAC_DATA, (byte)(colors[i] & 0x00FF00) >> 8);
                    Ports::Write8(PORT_DAC_DATA, (byte)(colors[i] & 0x0000FF));
                }
            }

            void VGAController::ClearColorPalette(uint len)
            {
                for (uint i = 0; i < len; i++)
                {
                    if (!TextMode) { Ports::Write8(PORT_MASK_DATA, 0xFF); } else { Ports::Write8(PORT_MASK_DATA, 0x0F); }
                    Ports::Write8(PORT_DAC_WRITE, i);
                    Ports::Write8(PORT_DAC_DATA, 0);
                    Ports::Write8(PORT_DAC_DATA, 0);
                    Ports::Write8(PORT_DAC_DATA, 0);
                }
            }
  
            byte* VGAController::GetBuffer() { return Buffer; }
            
            byte* VGAController::GetBackBuffer() { return BackBuffer; }
            
            uint VGAController::GetWidth() { return Width; }
            
            uint VGAController::GetHeight() { return Height; }

            ColDepth VGAController::GetDepth() { return Depth; }
        }
    }
}