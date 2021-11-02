#pragma once
#include <Kernel/Lib/Types.hpp>
#include <Kernel/Graphics/Graphics.hpp>
#include <Kernel/Core/Service.hpp>

namespace PMOS
{
    namespace HAL
    {
        enum class VGAMode : byte
        {
            Text80x25,
            Text80x50,
            Text90x60,
            Pixel320x200,
            Pixel320x200DB,
        };

        namespace Drivers
        {
            class VGAController : public Service
            {
                private:
                    ushort Width, Height;
                    ColDepth Depth;
                    bool TextMode, DoubleBuffered;
                    VGAMode CurrentMode;

                private:
                    byte* Buffer;
                    byte* BackBuffer;

                public:
                    VGAController();
                    void Initialize() override;
                    void Start() override;
                    void Stop() override;
                    void SetMode(VGAMode mode);
                
                private:
                    void ClearText(byte fg, byte bg);

                public:
                    void Clear(byte color);
                    void DrawPixel(ushort x, ushort y, byte color);
                    void DrawChar(ushort x, ushort y, char c, byte fg, byte bg);
                    void Swap();

                public:
                    void EnableCursor(byte start, byte end);
                    void DisableCursor();
                    void SetCursorPos(byte x, byte y);

                public:
                    byte* GetFrameBufferSegment();
                    void WriteRegisters(byte* regs);
                    void SetPlane(byte ap);
                    void SetFont(byte* font, byte height);
                    void SetColorPalette(uint* colors, uint len);
                    void ClearColorPalette(uint len);

                public: 
                    byte* GetBuffer();
                    byte* GetBackBuffer();
                    uint GetWidth();
                    uint GetHeight();
                    ColDepth GetDepth();
            };
        }
    }
}