#include <Kernel/Graphics/Graphics.hpp>
#include <Kernel/Core/Kernel.hpp>

namespace PMOS
{
    namespace Graphics
    {
        void VESADirectCanvas::Clear(Col32 color)
        {
            Kernel::VESA->ClearDirect(color.ToInteger());
        }

        void VESADirectCanvas::DrawPixel(int x, int y, Col32 color)
        {
            if (x < 0 || y < 0 || x >= Kernel::VESA->GetWidth() || y >= Kernel::VESA->GetHeight()) { return; }
            Kernel::VESA->SetPixelDirect(x, y, color.ToInteger());
        }

        void VESADirectCanvas::DrawFilledRect(int x, int y, int w, int h, Col32 color)
        {
            for (int i = 0; i < w * h; i++) { DrawPixel(x + (i % w), y + (i / w), color); }
        }

        void VESADirectCanvas::DrawChar(int x, int y, char c, Col32 fg, Font font)
        {
            uint p = font.Height * c;
            for (size_t cy = 0; cy < font.Height; cy++)
            {
                for (size_t cx = 0; cx < font.Width; cx++)
                {
                    uint xx = x + (font.Width - cx);
                    uint yy = y + cy;
                    if (GetBitAddressFromByte(font.Data[p + cy], cx + 1)) { DrawPixel(xx, yy, fg); }
                }
            }
        }

        void VESADirectCanvas::DrawChar(int x, int y, char c, Col32 fg, Col32 bg, Font font)
        {
            uint p = font.Height * c;
            for (size_t cy = 0; cy < font.Height; cy++)
            {
                for (size_t cx = 0; cx < font.Width; cx++)
                {
                    uint xx = x + (font.Width - cx);
                    uint yy = y + cy;
                    if (GetBitAddressFromByte(font.Data[p + cy], cx + 1)) { DrawPixel(xx, yy, fg); }
                    else { DrawPixel(xx, yy, bg); }
                }
            }
        }

        Col32 VESADirectCanvas::ConvertColor(Col4 color)
        {
            switch (color)
            {
                case Col4::Black: { return Colors::Black; }
                case Col4::DarkBlue: { return Colors::DarkBlue; }
                case Col4::DarkGreen: { return Colors::DarkGreen; }
                case Col4::DarkCyan: { return Colors::DarkCyan; }
                case Col4::DarkRed: { return Colors::DarkRed; }
                case Col4::DarkMagenta: { return Colors::DarkMagenta; }
                case Col4::DarkYellow: { return Colors::DarkYellow; }
                case Col4::Gray: { return Colors::Gray; }
                case Col4::DarkGray: { return Colors::DarkGray; }
                case Col4::Blue: { return Colors::Blue; }
                case Col4::Green: { return Colors::Green; }
                case Col4::Cyan: { return Colors::Cyan; }
                case Col4::Red: { return Colors::Red; }
                case Col4::Magenta: { return Colors::Magenta; }
                case Col4::Yellow: { return Colors::Yellow; }
                case Col4::White: { return Colors::White; }
                default: { return Colors::Black; }
            }
        }

        // ------------------------------------------------------------------------------------------------------------

        void VESACanvas::Initialize()
        {
            BufferSize = Kernel::VESA->GetWidth() * Kernel::VESA->GetHeight() * 4;
            Buffer = (byte*)Kernel::MemoryMgr.Allocate(BufferSize, true, AllocationType::FrameBuffer);
        }

        void VESACanvas::Clear(Col32 color)
        {
            uint c = color.ToInteger();
            uint dest = (uint)Buffer;
            uint num_dwords = BufferSize / 4;
            uint num_bytes = BufferSize % 4;
            uint *dest32 = (uint*)dest;
            byte *dest8 = ((byte*)dest)+num_dwords*4;
            byte val8 = (byte)c;
            uint i;

            for (i=0;i < num_dwords;i++) { dest32[i] = c; }
            for (i=0;i < num_bytes;i++) { dest8[i] = val8; }
        }

        void VESACanvas::DrawPixel(int x, int y, Col32 color)
        {
            if ((uint)x >= Kernel::VESA->GetWidth() || y >= Kernel::VESA->GetHeight()) { return; }
            ((uint*)Buffer)[(y * Kernel::VESA->GetWidth() + x)] = color.ToInteger();
        }

        void VESACanvas::DrawPixel(int x, int y, uint color)
        {
            if ((uint)x >= Kernel::VESA->GetWidth() || y >= Kernel::VESA->GetHeight()) { return; }
            ((uint*)Buffer)[(y * Kernel::VESA->GetWidth() + x)] = color;
        }

        void VESACanvas::DrawFilledRect(int x, int y, int w, int h, Col32 color)
        {
            for (int yy = 0; yy < h; yy++)
            {
                for (int xx = 0; xx < w; xx++) { DrawPixel(x + xx, y + yy, color); }
            }
        }

        void VESACanvas::DrawFilledRect(int x, int y, int w, int h, uint color)
        {
            for (int yy = 0; yy < h; yy++)
            {
                for (int xx = 0; xx < w; xx++) { DrawPixel(x + xx, y + yy, color); }
            }
        }

        void VESACanvas::DrawFilledRect(Rectangle bounds, Col32 color) { DrawFilledRect(bounds.X, bounds.Y, bounds.Width, bounds.Height, color); }
        void VESACanvas::DrawFilledRect(Rectangle bounds, uint color) { DrawFilledRect(bounds.X, bounds.Y, bounds.Width, bounds.Height, color); }

        void VESACanvas::DrawRect(int x, int y, int w, int h, int thickness, Col32 color)
        {
            DrawFilledRect(x, y, w, thickness, color);
            DrawFilledRect(x, y + h - thickness, w, thickness, color);
            DrawFilledRect(x, y + thickness, thickness, h - (thickness * 2), color);
            DrawFilledRect(x + w - thickness, y + thickness, thickness, h - (thickness * 2), color);
        }

        void VESACanvas::DrawRect(int x, int y, int w, int h, int thickness, uint color)
        {
            DrawFilledRect(x, y, w, thickness, color);
            DrawFilledRect(x, y + h - thickness, w, thickness, color);
            DrawFilledRect(x, y + thickness, thickness, h - (thickness * 2), color);
            DrawFilledRect(x + w - thickness, y + thickness, thickness, h - (thickness * 2), color);
        }

        void VESACanvas::DrawRect(Rectangle bounds, int thickness, Col32 color) { DrawRect(bounds.X, bounds.Y, bounds.Width, bounds.Height, thickness, color); }
        void VESACanvas::DrawRect(Rectangle bounds, int thickness, uint color) { DrawRect(bounds.X, bounds.Y, bounds.Width, bounds.Height, thickness, color); }

        void VESACanvas::DrawRect3D(int x, int y, int w, int h, bool invert, Col32 ca, Col32 cb, Col32 cc)
        {
            if (invert)
            {
                DrawFilledRect(x, y, w - 1, 1, cc);
                DrawFilledRect(x, y, 1, h - 1, cc);
                DrawFilledRect(x + 1, y + 1, w - 3, 1, cb);
                DrawFilledRect(x + 1, y + 1, 1, h - 3, cb);
                DrawFilledRect(x, y + h - 1, w, 1, ca);
                DrawFilledRect(x + w - 1, y, 1, h, ca);
            }
            else
            {
                DrawFilledRect(x, y, w, 1, ca);
                DrawFilledRect(x, y, 1, h, ca);
                DrawFilledRect(x + 1, y + h - 2, w - 2, 1, cb);
                DrawFilledRect(x + w - 2, y + 1, 1, h - 2, cb);
                DrawFilledRect(x, y + h - 1, w, 1, cc);
                DrawFilledRect(x + w - 1, y, 1, h, cc);
            }
        }

        void VESACanvas::DrawRect3D(int x, int y, int w, int h, bool invert, uint ca, uint cb, uint cc)
        {
            if (invert)
            {
                DrawFilledRect(x, y, w - 1, 1, cc);
                DrawFilledRect(x, y, 1, h - 1, cc);
                DrawFilledRect(x + 1, y + 1, w - 3, 1, cb);
                DrawFilledRect(x + 1, y + 1, 1, h - 3, cb);
                DrawFilledRect(x, y + h - 1, w, 1, ca);
                DrawFilledRect(x + w - 1, y, 1, h, ca);
            }
            else
            {
                DrawFilledRect(x, y, w, 1, ca);
                DrawFilledRect(x, y, 1, h, ca);
                DrawFilledRect(x + 1, y + h - 2, w - 2, 1, cb);
                DrawFilledRect(x + w - 2, y + 1, 1, h - 2, cb);
                DrawFilledRect(x, y + h - 1, w, 1, cc);
                DrawFilledRect(x + w - 1, y, 1, h, cc);
            }
        }

        void VESACanvas::DrawRect3D(Rectangle bounds, bool invert, Col32 ca, Col32 cb, Col32 cc) { DrawRect3D(bounds.X, bounds.Y, bounds.Width, bounds.Height, invert, ca, cb, cc); }
        void VESACanvas::DrawRect3D(Rectangle bounds, bool invert, uint ca, uint cb, uint cc) { DrawRect3D(bounds.X, bounds.Y, bounds.Width, bounds.Height, invert, ca, cb, cc); }

        void VESACanvas::DrawChar(int x, int y, char c, Col32 fg, Font font)
        {
            if (c == 0) { return; }
            uint p = font.Height * c;
            for (size_t cy = 0; cy < font.Height; cy++)
            {
                for (size_t cx = 0; cx < font.Width; cx++)
                {
                    if (GetBitAddressFromByte(font.Data[p + cy], cx + 1))
                    { DrawPixel(x + (font.Width - cx), y + cy, fg); }
                }
            }
        }

        void VESACanvas::DrawChar(int x, int y, char c, Col32 fg, Col32 bg, Font font)
        {
            uint p = font.Height * c;
            for (size_t cy = 0; cy < font.Height; cy++)
            {
                for (size_t cx = 0; cx < font.Width; cx++)
                {
                    uint xx = x + (font.Width - cx);
                    uint yy = y + cy;
                    if (GetBitAddressFromByte(font.Data[p + cy], cx + 1)) { DrawPixel(xx, yy, fg); }
                    else { DrawPixel(xx, yy, bg); }
                }
            }
        }

        void VESACanvas::DrawChar(int x, int y, char c, uint fg, Font font)
        {
            if (c == 0) { return; }
            uint p = font.Height * c;
            for (size_t cy = 0; cy < font.Height; cy++)
            {
                for (size_t cx = 0; cx < font.Width; cx++)
                {
                    if (GetBitAddressFromByte(font.Data[p + cy], cx + 1))
                    { DrawPixel(x + (font.Width - cx), y + cy, fg); }
                }
            }
        }

        void VESACanvas::DrawChar(int x, int y, char c, uint fg, uint bg, Font font)
        {
            uint p = font.Height * c;
            for (size_t cy = 0; cy < font.Height; cy++)
            {
                for (size_t cx = 0; cx < font.Width; cx++)
                {
                    uint xx = x + (font.Width - cx);
                    uint yy = y + cy;
                    if (GetBitAddressFromByte(font.Data[p + cy], cx + 1)) { DrawPixel(xx, yy, fg); }
                    else { DrawPixel(xx, yy, bg); }
                }
            }
        }

        void VESACanvas::DrawString(int x, int y, char* text, Col32 fg, Font font)
        {
            if (text == nullptr) { return; }
            int dx = x, dy = y;
            for (size_t i = 0; i < String::Length(text); i++)
            {
                if (text[i] == '\n') { dx = x; dy += font.Height + font.SpacingY; }
                else if (text[i] >= 32 && text[i] <= 126)
                {
                    DrawChar(dx, dy, text[i], fg, font);
                    dx += font.Width + font.SpacingX;
                }
            }
        }

        void VESACanvas::DrawString(int x, int y, char* text, Col32 fg, Col32 bg, Font font)
        {
            if (text == nullptr) { return; }
            int dx = x, dy = y;
            for (size_t i = 0; i < String::Length(text); i++)
            {
                if (text[i] == '\n') { dx = x; dy += font.Height + font.SpacingY; }
                else if (text[i] >= 32 && text[i] <= 126)
                {
                    DrawChar(dx, dy, text[i], fg, bg, font);
                    dx += font.Width + font.SpacingX;
                }
            }
        }

        void VESACanvas::DrawString(int x, int y, char* text, uint fg, Font font)
        {
            if (text == nullptr) { return; }
            int dx = x, dy = y;
            for (size_t i = 0; i < String::Length(text); i++)
            {
                if (text[i] == '\n') { dx = x; dy += font.Height + font.SpacingY; }
                else if (text[i] >= 32 && text[i] <= 126)
                {
                    DrawChar(dx, dy, text[i], fg, font);
                    dx += font.Width + font.SpacingX;
                }
            }
        }

        void VESACanvas::DrawString(int x, int y, char* text, uint fg, uint bg, Font font)
        {
            if (text == nullptr) { return; }
            int dx = x, dy = y;
            for (size_t i = 0; i < String::Length(text); i++)
            {
                if (text[i] == '\n') { dx = x; dy += font.Height + font.SpacingY; }
                else if (text[i] >= 32 && text[i] <= 126)
                {
                    DrawChar(dx, dy, text[i], fg, bg, font);
                    dx += font.Width + font.SpacingX;
                }
            }
        }

        void VESACanvas::DrawArray(int x, int y, int w, int h, uint* data)
        {
            if (data == nullptr) { return; }
            if (x < 0) { x = 0; }
            if (y < 0) { y = 0; }
            while (x + w >= Kernel::VESA->GetWidth()) { w--; }
            while (y + h >= Kernel::VESA->GetHeight()) { y--; }

            for (int yy = 0; yy < h; yy++)
            {
                byte* src = (byte*)(data + (yy * w));
                uint real_offset = (x + ((y + yy) * Kernel::VESA->GetWidth())) * 4;
                byte* dest = (byte*)((uint)Kernel::XServer->Canvas.Buffer + real_offset);
                Memory::Copy(dest, src, w * 4);
            }
        }

        void VESACanvas::DrawArray(int x, int y, int w, int h, Col32 trans, uint* data)
        {
            uint offset = 0; 
            for (int yy = 0; yy < h; yy++)
            {
                for (int xx = 0; xx < w; xx++)
                {
                    offset = xx + (yy * w);
                    if (data[offset] != trans.ToInteger())
                    { DrawPixel(x + xx, y + yy, data[offset]); }
                }
            }
        }

        void VESACanvas::DrawFlatArray(int x, int y, int w, int h, uint color, byte* data)
        {
            if (data == nullptr) { return; }
            uint offset = 0;
            for (int yy = 0; yy < h; yy++)
            {
                for (int xx = 0; xx < w; xx++)
                {
                    offset = xx + (yy * w);
                    if (data[offset] > 0) { DrawPixel(x + xx, y + yy, color); }
                }
            }
        }

        void VESACanvas::DrawBitmap(int x, int y, void* bitmap)
        {
            if (bitmap == nullptr) { return; }

            int w = ((Bitmap*)bitmap)->Width;
            int h = ((Bitmap*)bitmap)->Height;

            uint* data = (uint*)((Bitmap*)bitmap)->ImageData;
            for (int yy = 0; yy < h; yy++)
            {
                for (int xx = 0; xx < w; xx++)
                {
                    uint color = data[(xx + (yy * w))];
                    DrawPixel(x + xx, y + yy, color);
                }
            }
        }

        void VESACanvas::DrawBitmap(int x, int y, Col32 trans, void* bitmap)
        {
            if (bitmap == nullptr) { return; }

            int w = ((Bitmap*)bitmap)->Width;
            int h = ((Bitmap*)bitmap)->Height;

            uint* data = (uint*)((Bitmap*)bitmap)->ImageData;
            for (int yy = 0; yy < h; yy++)
            {
                for (int xx = 0; xx < w; xx++)
                {
                    uint color = data[(xx + (yy * w))];
                    if (color != trans.ToInteger()) { DrawPixel(x + xx, y + yy, color); }
                }
            }
        }

        void VESACanvas::DrawBitmapFast(int x, int y, void* bitmap)
        {
            if (bitmap == nullptr) { return; }

            int   w = ((Bitmap*)bitmap)->Width;
            int   h = ((Bitmap*)bitmap)->Height;
            uint* d = (uint*)((Bitmap*)bitmap)->ImageData;
            while (x + w > Kernel::VESA->GetWidth()) { w--; }
            while (y + h > Kernel::VESA->GetHeight()) { h--; }

            for (int yy = 0; yy < (int)((Bitmap*)bitmap)->Height; yy++)
            {
                byte* src = (byte*)(d + (yy * ((Bitmap*)bitmap)->Width));
                int xx = x;
                if (xx < 0) { xx = 0; }
                while (xx + w >= Kernel::VESA->GetWidth()) { xx--; }
                uint real_offset = (xx + ((y + yy) * Kernel::VESA->GetWidth())) * 4;
                byte* dest = (byte*)((uint)Buffer + real_offset);
                if (y + yy >= Kernel::VESA->GetHeight()) { return; }
                if (y + yy >= 0 && dest >= Buffer && dest < (byte*)((uint)Buffer + (Kernel::VESA->GetWidth() * Kernel::VESA->GetHeight() * 4)))
                {
                    if (x >= 0) { Memory::Copy(dest, src, w * 4); }
                    else { Memory::Copy(dest, src - (x * 4), (w + x) * 4); }
                }
            }
        }

        void VESACanvas::Display()
        {
            uint dest       = Kernel::VESA->ModeInfoBlock.PhysicalBase;
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

        // --------------------------------------------------------------------------------------------------------------------------------

        BitmapCanvas::BitmapCanvas()
        {
            Image = nullptr;
        }

        BitmapCanvas::BitmapCanvas(Bitmap* bmp)
        {
            Image = bmp;
        }

        void BitmapCanvas::SetBitmap(Bitmap* bmp)
        {
            Image = bmp;
        }

        void BitmapCanvas::Clear() { Clear(Colors::Black); }
        void BitmapCanvas::Clear(Col32 color) { Image->DrawFilledRect(0, 0, Image->Width, Image->Height, color.ToInteger()); }
        void BitmapCanvas::Clear(uint color) { Image->DrawFilledRect(0, 0, Image->Width, Image->Height, color); }

        void BitmapCanvas::DrawPixel(int x, int y, Col32 color)
        {
            if (Image == nullptr) { return; }
            Image->SetPixel(x, y, color.ToInteger());
        }

        void BitmapCanvas::DrawPixel(int x, int y, uint color)
        {
            if (Image == nullptr) { return; }
            Image->SetPixel(x, y, color);
        }

        void BitmapCanvas::DrawFilledRect(int x, int y, int w, int h, Col32 color)
        {
            if (w == 0 || h == 0) { return; }
            Image->DrawFilledRect(x, y, w, h, color.ToInteger());
        }

        void BitmapCanvas::DrawFilledRect(int x, int y, int w, int h, uint color)
        {
            if (w == 0 || h == 0) { return; }
            Image->DrawFilledRect(x, y, w, h, color);
        }

        void BitmapCanvas::DrawFilledRect(Rectangle bounds, Col32 color) { DrawFilledRect(bounds.X, bounds.Y, bounds.Width, bounds.Height, color); }
        void BitmapCanvas::DrawFilledRect(Rectangle bounds, uint color) { DrawFilledRect(bounds.X, bounds.Y, bounds.Width, bounds.Height, color); }

        void BitmapCanvas::DrawRect(int x, int y, int w, int h, int thickness, Col32 color)
        {
            if (w == 0 || h == 0) { return; }
            DrawFilledRect(x, y, w, thickness, color);
            DrawFilledRect(x, y + h - thickness, w, thickness, color);
            DrawFilledRect(x, y + thickness, thickness, h - (thickness * 2), color);
            DrawFilledRect(x + w - thickness, y + thickness, thickness, h - (thickness * 2), color);
        }

        void BitmapCanvas::DrawRect(int x, int y, int w, int h, int thickness, uint color)
        {
            if (w == 0 || h == 0) { return; }
            DrawFilledRect(x, y, w, thickness, color);
            DrawFilledRect(x, y + h - thickness, w, thickness, color);
            DrawFilledRect(x, y + thickness, thickness, h - (thickness * 2), color);
            DrawFilledRect(x + w - thickness, y + thickness, thickness, h - (thickness * 2), color);
        }

        void BitmapCanvas::DrawRect(Rectangle bounds, int thickness, Col32 color) { DrawRect(bounds.X, bounds.Y, bounds.Width, bounds.Height, thickness, color); }
        void BitmapCanvas::DrawRect(Rectangle bounds, int thickness, uint color) { DrawRect(bounds.X, bounds.Y, bounds.Width, bounds.Height, thickness, color); }

        void BitmapCanvas::DrawRect3D(int x, int y, int w, int h, bool invert, Col32 ca, Col32 cb, Col32 cc)
        {
            if (invert)
            {
                DrawFilledRect(x, y, w - 1, 1, cc);
                DrawFilledRect(x, y, 1, h - 1, cc);
                DrawFilledRect(x + 1, y + 1, w - 3, 1, cb);
                DrawFilledRect(x + 1, y + 1, 1, h - 3, cb);
                DrawFilledRect(x, y + h - 1, w, 1, ca);
                DrawFilledRect(x + w - 1, y, 1, h, ca);
            }
            else
            {
                DrawFilledRect(x, y, w, 1, ca);
                DrawFilledRect(x, y, 1, h, ca);
                DrawFilledRect(x + 1, y + h - 2, w - 2, 1, cb);
                DrawFilledRect(x + w - 2, y + 1, 1, h - 2, cb);
                DrawFilledRect(x, y + h - 1, w, 1, cc);
                DrawFilledRect(x + w - 1, y, 1, h, cc);
            }
        }

        void BitmapCanvas::DrawRect3D(int x, int y, int w, int h, bool invert, uint ca, uint cb, uint cc)
        {
            if (invert)
            {
                DrawFilledRect(x, y, w - 1, 1, cc);
                DrawFilledRect(x, y, 1, h - 1, cc);
                DrawFilledRect(x + 1, y + 1, w - 3, 1, cb);
                DrawFilledRect(x + 1, y + 1, 1, h - 3, cb);
                DrawFilledRect(x, y + h - 1, w, 1, ca);
                DrawFilledRect(x + w - 1, y, 1, h, ca);
            }
            else
            {
                DrawFilledRect(x, y, w, 1, ca);
                DrawFilledRect(x, y, 1, h, ca);
                DrawFilledRect(x + 1, y + h - 2, w - 2, 1, cb);
                DrawFilledRect(x + w - 2, y + 1, 1, h - 2, cb);
                DrawFilledRect(x, y + h - 1, w, 1, cc);
                DrawFilledRect(x + w - 1, y, 1, h, cc);
            }
        }

        void BitmapCanvas::DrawRect3D(Rectangle bounds, bool invert, Col32 ca, Col32 cb, Col32 cc) { DrawRect3D(bounds.X, bounds.Y, bounds.Width, bounds.Height, invert, ca, cb, cc); }
        void BitmapCanvas::DrawRect3D(Rectangle bounds, bool invert, uint ca, uint cb, uint cc) { DrawRect3D(bounds.X, bounds.Y, bounds.Width, bounds.Height, invert, ca, cb, cc); }

        void BitmapCanvas::DrawChar(int x, int y, char c, Col32 fg, Font font)
        {
            if (c == 0) { return; }
            uint p = font.Height * c;
            for (size_t cy = 0; cy < font.Height; cy++)
            {
                for (size_t cx = 0; cx < font.Width; cx++)
                {
                    if (GetBitAddressFromByte(font.Data[p + cy], cx + 1))
                    { DrawPixel(x + (font.Width - cx), y + cy, fg); }
                }
            }
        }

        void BitmapCanvas::DrawChar(int x, int y, char c, Col32 fg, Col32 bg, Font font)
        {
            uint p = font.Height * c;
            for (size_t cy = 0; cy < font.Height; cy++)
            {
                for (size_t cx = 0; cx < font.Width; cx++)
                {
                    uint xx = x + (font.Width - cx);
                    uint yy = y + cy;
                    if (GetBitAddressFromByte(font.Data[p + cy], cx + 1)) { DrawPixel(xx, yy, fg); }
                    else { DrawPixel(xx, yy, bg); }
                }
            }
        }

        void BitmapCanvas::DrawChar(int x, int y, char c, uint fg, Font font)
        {
            if (c == 0) { return; }
            uint p = font.Height * c;
            for (size_t cy = 0; cy < font.Height; cy++)
            {
                for (size_t cx = 0; cx < font.Width; cx++)
                {
                    if (GetBitAddressFromByte(font.Data[p + cy], cx + 1))
                    { DrawPixel(x + (font.Width - cx), y + cy, fg); }
                }
            }
        }

        void BitmapCanvas::DrawChar(int x, int y, char c, uint fg, uint bg, Font font)
        {
            uint p = font.Height * c;
            for (size_t cy = 0; cy < font.Height; cy++)
            {
                for (size_t cx = 0; cx < font.Width; cx++)
                {
                    uint xx = x + (font.Width - cx);
                    uint yy = y + cy;
                    if (GetBitAddressFromByte(font.Data[p + cy], cx + 1)) { DrawPixel(xx, yy, fg); }
                    else { DrawPixel(xx, yy, bg); }
                }
            }
        }

        void BitmapCanvas::DrawString(int x, int y, char* text, Col32 fg, Font font)
        {
            if (text == nullptr) { return; }
            int dx = x, dy = y;
            for (size_t i = 0; i < String::Length(text); i++)
            {
                if (text[i] == '\n') { dx = x; dy += font.Height + font.SpacingY; }
                else if (text[i] >= 32 && text[i] <= 126)
                {
                    DrawChar(dx, dy, text[i], fg, font);
                    dx += font.Width + font.SpacingX;
                }
            }
        }

        void BitmapCanvas::DrawString(int x, int y, char* text, Col32 fg, Col32 bg, Font font)
        {
            if (text == nullptr) { return; }
            int dx = x, dy = y;
            for (size_t i = 0; i < String::Length(text); i++)
            {
                if (text[i] == '\n') { dx = x; dy += font.Height + font.SpacingY; }
                else if (text[i] >= 32 && text[i] <= 126)
                {
                    DrawChar(dx, dy, text[i], fg, bg, font);
                    dx += font.Width + font.SpacingX;
                }
            }
        }

        void BitmapCanvas::DrawString(int x, int y, char* text, uint fg, Font font)
        {
            Kernel::Debug.WriteLine("DRAWING CHAR: X = %d, Y = %d, text = %s, fg = %8x, font = %8x", x, y, text, fg, (Font*)&font);
            if (text == nullptr) { return; }
            int dx = x, dy = y;
            for (size_t i = 0; i < String::Length(text); i++)
            {
                if (text[i] == '\n') { dx = x; dy += font.Height + font.SpacingY; }
                else if (text[i] >= 32 && text[i] <= 126)
                {
                    DrawChar(dx, dy, text[i], fg, font);
                    dx += font.Width + font.SpacingX;
                }
            }
        }

        void BitmapCanvas::DrawString(int x, int y, char* text, uint fg, uint bg, Font font)
        {
            if (text == nullptr) { return; }
            int dx = x, dy = y;
            for (size_t i = 0; i < String::Length(text); i++)
            {
                if (text[i] == '\n') { dx = x; dy += font.Height + font.SpacingY; }
                else if (text[i] >= 32 && text[i] <= 126)
                {
                    DrawChar(dx, dy, text[i], fg, bg, font);
                    dx += font.Width + font.SpacingX;
                }
            }
        }

        void BitmapCanvas::DrawArray(int x, int y, int w, int h, uint* data)
        {
            if (data == nullptr) { return; }
            if (x < 0) { x = 0; }
            if (y < 0) { y = 0; }
            while (x + w >= Kernel::VESA->GetWidth()) { w--; }
            while (y + h >= Kernel::VESA->GetHeight()) { y--; }

            for (int yy = 0; yy < h; yy++)
            {
                byte* src = (byte*)(data + (yy * w));
                uint real_offset = (x + ((y + yy) * Kernel::VESA->GetWidth())) * 4;
                byte* dest = (byte*)((uint)Kernel::XServer->Canvas.Buffer + real_offset);
                Memory::Copy(dest, src, w * 4);
            }
        }

        void BitmapCanvas::DrawArray(int x, int y, int w, int h, Col32 trans, uint* data)
        {
            uint offset = 0; 
            for (int yy = 0; yy < h; yy++)
            {
                for (int xx = 0; xx < w; xx++)
                {
                    offset = xx + (yy * w);
                    if (data[offset] != trans.ToInteger())
                    { DrawPixel(x + xx, y + yy, data[offset]); }
                }
            }
        }

        void BitmapCanvas::DrawFlatArray(int x, int y, int w, int h, uint color, byte* data)
        {
            if (data == nullptr) { return; }
            uint offset = 0;
            for (int yy = 0; yy < h; yy++)
            {
                for (int xx = 0; xx < w; xx++)
                {
                    offset = xx + (yy * w);
                    if (data[offset] > 0) { DrawPixel(x + xx, y + yy, color); }
                }
            }
        }

        void BitmapCanvas::DrawBitmap(int x, int y, void* bitmap)
        {
            if (bitmap == nullptr) { return; }

            int w = ((Bitmap*)bitmap)->Width;
            int h = ((Bitmap*)bitmap)->Height;

            uint* data = (uint*)((Bitmap*)bitmap)->ImageData;
            for (int yy = 0; yy < h; yy++)
            {
                for (int xx = 0; xx < w; xx++)
                {
                    uint color = data[(xx + (yy * w))];
                    DrawPixel(x + xx, y + yy, color);
                }
            }
        }

        void BitmapCanvas::DrawBitmap(int x, int y, Col32 trans, void* bitmap)
        {
            if (bitmap == nullptr) { return; }

            int w = ((Bitmap*)bitmap)->Width;
            int h = ((Bitmap*)bitmap)->Height;

            uint* data = (uint*)((Bitmap*)bitmap)->ImageData;
            for (int yy = 0; yy < h; yy++)
            {
                for (int xx = 0; xx < w; xx++)
                {
                    uint color = data[(xx + (yy * w))];
                    if (color != trans.ToInteger()) { DrawPixel(x + xx, y + yy, color); }
                }
            }
        }

        void BitmapCanvas::DrawBitmapFast(int x, int y, void* bitmap)
        {
            if (bitmap == nullptr) { return; }

            int   w = ((Bitmap*)bitmap)->Width;
            int   h = ((Bitmap*)bitmap)->Height;
            uint* d = (uint*)((Bitmap*)bitmap)->ImageData;
            while (x + w > Kernel::VESA->GetWidth()) { w--; }
            while (y + h > Kernel::VESA->GetHeight()) { h--; }

            for (int yy = 0; yy < (int)((Bitmap*)bitmap)->Height; yy++)
            {
                byte* src = (byte*)(d + (yy * ((Bitmap*)bitmap)->Width));
                int xx = x;
                if (xx < 0) { xx = 0; }
                while (xx + w >= Kernel::VESA->GetWidth()) { xx--; }
                uint real_offset = (xx + ((y + yy) * Kernel::VESA->GetWidth())) * 4;
                byte* dest = (byte*)((uint)Kernel::XServer->Canvas.Buffer + real_offset);
                if (y + yy >= Kernel::VESA->GetHeight()) { return; }
                if (y + yy >= 0 && dest >= Kernel::XServer->Canvas.Buffer && dest < (byte*)(uint)Kernel::XServer->Canvas.Buffer + (Kernel::VESA->GetWidth() * Kernel::VESA->GetHeight() * 4))
                {
                    if (x >= 0) { Memory::Copy(dest, src, w * 4); }
                    else { Memory::Copy(dest, src - (x * 4), (w + x) * 4); }
                }
            }
        }
    }
}