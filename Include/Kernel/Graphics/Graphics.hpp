#pragma once
#include <Kernel/Lib/Types.hpp>
#include <Kernel/Lib/Math.hpp>
#include <Kernel/Graphics/Fonts.hpp>
#include <Kernel/Graphics/Bitmap.hpp>

namespace PMOS
{
    enum class Col4 : byte
    {
        Black,
        DarkBlue,
        DarkGreen,
        DarkCyan,
        DarkRed,
        DarkMagenta,
        DarkYellow,
        Gray,
        DarkGray,
        Blue,
        Green,
        Cyan,
        Red,
        Magenta,
        Yellow,
        White,
    };

    struct Col32
    {
        public:
            byte R, G, B, A;

        public:
            uint ToInteger() { return (uint)((A << 24) | (R << 16) | (G << 8) | B); }
    };

    enum class ColDepth
    {
        Depth4Bit       = 4,
        Depth8Bit       = 8,
        Depth16Bit      = 16,
        Depth24Bit      = 24,
        Depth32Bit      = 32,
    };

    namespace Graphics
    {
        static inline uint RGBToPackedValue(Col32 rgb) { return (uint)((rgb.A << 24) | (rgb.R << 16) | (rgb.G << 8) | rgb.B); }

        static inline uint RGBToPackedValue(byte r, byte g, byte b) { return (uint)((0xFF << 24) | (r << 16) | (g << 8) | b); }

        static inline uint RGBToPackedValue(byte a, byte r, byte g, byte b) { return (uint)((a << 24) | (r << 16) | (g << 8) | b); }


        class VESADirectCanvas
        {
            public:
                void Clear(Col32 color);
                void DrawPixel(int x, int y, Col32 color);
                void DrawFilledRect(int x, int y, int w, int h, Col32 color);
                void DrawChar(int x, int y, char c, Col32 fg, Font font);
                void DrawChar(int x, int y, char c, Col32 fg, Col32 bg, Font font);
                void DrawString(int x, int y, char* text, Col32 fg, Font font);
                void DrawString(int x, int y, char* text, Col32 fg, Col32 bg, Font font);
                Col32 ConvertColor(Col4 color);
        };

        class VESACanvas
        {
            public:
                byte* Buffer;
                uint  BufferSize;

            public:
                void Initialize();
                void Display();
                void Clear();
                void Clear(Col32 color);
                void Clear(uint color);

                void DrawPixel(int x, int y, Col32 color);
                void DrawPixel(int x, int y, uint color);

                void DrawFilledRect(int x, int y, int w, int h, Col32 color);
                void DrawFilledRect(int x, int y, int w, int h, uint color);
                void DrawFilledRect(Rectangle bounds, Col32 color);
                void DrawFilledRect(Rectangle bounds, uint color);

                void DrawRect(int x, int y, int w, int h, int thickness, Col32 color);
                void DrawRect(int x, int y, int w, int h, int thickness, uint color);
                void DrawRect(Rectangle bounds, int thickness, Col32 color);
                void DrawRect(Rectangle bounds, int thickness, uint color);

                void DrawRect3D(int x, int y, int w, int h, bool invert, Col32 ca, Col32 cb, Col32 cc);
                void DrawRect3D(int x, int y, int w, int h, bool invert, uint ca, uint cb, uint cc);
                void DrawRect3D(Rectangle bounds, bool invert, Col32 ca, Col32 cb, Col32 cc);
                void DrawRect3D(Rectangle bounds, bool invert, uint ca, uint cb, uint cc);

                void DrawChar(int x, int y, char c, Col32 fg, Font font);
                void DrawChar(int x, int y, char c, Col32 fg, Col32 bg, Font font);
                void DrawChar(int x, int y, char c, uint fg, Font font);
                void DrawChar(int x, int y, char c, uint fg, uint bg, Font font);

                void DrawString(int x, int y, char* text, Col32 fg, Font font);
                void DrawString(int x, int y, char* text, Col32 fg, Col32 bg, Font font);
                void DrawString(int x, int y, char* text, uint fg, Font font);
                void DrawString(int x, int y, char* text, uint fg, uint bg, Font font);

                void DrawArray(int x, int y, int w, int h, uint* data);
                void DrawArray(int x, int y, int w, int h, Col32 trans, uint* data);

                void DrawFlatArray(int x, int y, int w, int h, uint color, byte* data);

                void DrawBitmap(int x, int y, void* bitmap);
                void DrawBitmap(int x, int y, Col32 trans, void* bitmap);
                void DrawBitmapFast(int x, int y, void* bitmap);
        };

        class BitmapCanvas
        {
            private:
                Bitmap* Image;

            public:
                BitmapCanvas();
                BitmapCanvas(Bitmap* bmp);
                void SetBitmap(Bitmap* bmp);

            public:
                void Display();

                void Clear();
                void Clear(Col32 color);
                void Clear(uint color);

                void DrawPixel(int x, int y, Col32 color);
                void DrawPixel(int x, int y, uint color);

                void DrawFilledRect(int x, int y, int w, int h, Col32 color);
                void DrawFilledRect(int x, int y, int w, int h, uint color);
                void DrawFilledRect(Rectangle bounds, Col32 color);
                void DrawFilledRect(Rectangle bounds, uint color);

                void DrawRect(int x, int y, int w, int h, int thickness, Col32 color);
                void DrawRect(int x, int y, int w, int h, int thickness, uint color);
                void DrawRect(Rectangle bounds, int thickness, Col32 color);
                void DrawRect(Rectangle bounds, int thickness, uint color);

                void DrawRect3D(int x, int y, int w, int h, bool invert, Col32 ca, Col32 cb, Col32 cc);
                void DrawRect3D(int x, int y, int w, int h, bool invert, uint ca, uint cb, uint cc);
                void DrawRect3D(Rectangle bounds, bool invert, Col32 ca, Col32 cb, Col32 cc);
                void DrawRect3D(Rectangle bounds, bool invert, uint ca, uint cb, uint cc);

                void DrawChar(int x, int y, char c, Col32 fg, Font font);
                void DrawChar(int x, int y, char c, Col32 fg, Col32 bg, Font font);
                void DrawChar(int x, int y, char c, uint fg, Font font);
                void DrawChar(int x, int y, char c, uint fg, uint bg, Font font);

                void DrawString(int x, int y, char* text, Col32 fg, Font font);
                void DrawString(int x, int y, char* text, Col32 fg, Col32 bg, Font font);
                void DrawString(int x, int y, char* text, uint fg, Font font);
                void DrawString(int x, int y, char* text, uint fg, uint bg, Font font);

                void DrawArray(int x, int y, int w, int h, uint* data);
                void DrawArray(int x, int y, int w, int h, Col32 trans, uint* data);

                void DrawFlatArray(int x, int y, int w, int h, uint color, byte* data);

                void DrawBitmap(int x, int y, void* bitmap);
                void DrawBitmap(int x, int y, Col32 trans, void* bitmap);
                void DrawBitmapFast(int x, int y, void* bitmap);
        };
    }

    namespace Colors
    {
        static const Col32 Black = { 0x00, 0x00, 0x00, 0xFF };
        static const Col32 DarkBlue = { 0x00, 0x00, 0x7F, 0xFF };
        static const Col32 DarkGreen = { 0x00, 0x7F, 0x00, 0xFF };
        static const Col32 DarkCyan = { 0x00, 0x7F, 0x7F, 0xFF };
        static const Col32 DarkRed = { 0x7F, 0x00, 0x00, 0xFF };
        static const Col32 DarkMagenta = { 0x7F, 0x00, 0x7F, 0xFF };
        static const Col32 DarkYellow = { 0x7F, 0x7F, 0x00, 0xFF };
        static const Col32 Gray = { 0xAF, 0xAF, 0xAF, 0xFF };
        static const Col32 DarkGray = { 0x5F, 0x5F, 0x5F, 0xFF };
        static const Col32 Blue = { 0x00, 0x00, 0xFF, 0xFF };
        static const Col32 Green = { 0x00, 0xFF, 0x00, 0xFF };
        static const Col32 Cyan = { 0x00, 0xFF, 0xFF, 0xFF };
        static const Col32 Red = { 0xFF, 0x00, 0x00, 0xFF };
        static const Col32 Magenta = { 0xFF, 0x00, 0xFF, 0xFF };
        static const Col32 Yellow = { 0xFF, 0xFF, 0x00, 0xFF };
        static const Col32 White = { 0xFF, 0xFF, 0xFF, 0xFF };
    }
}