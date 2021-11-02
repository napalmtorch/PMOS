#pragma once
#include <Kernel/Lib/Types.hpp>

namespace PMOS
{
    namespace Graphics
    {
        typedef struct
        {
            ushort type;
            uint size;
            ushort reserved0;
            ushort reserved1;
            uint off_bits;
        } ATTR_PACK bmp_fileheader_t;

        typedef struct
        {
            uint size;
            uint width;
            uint height;
            ushort planes;
            ushort bit_count;
            uint compression;
            uint size_image;
            uint x_ppm;
            uint y_ppm;
            uint clr_used;
            uint clr_important;
        } bmp_infoheader_t;

        class Bitmap
        {
            public:
                int Width;
                int Height;
                byte Depth;
                byte* ImageData;
                byte* RawData;
                uint Size;

            public:
                Bitmap();
                Bitmap(int w, int h, byte depth);
                Bitmap(char* fullname);
                void Dispose();

                void Create(int w, int h, byte depth);
                void Resize(int w, int h);
                void SetPixel(short x, short y, uint color);
                void DrawFilledRect(short x, short y, short w, short h, uint color);
                uint GetPixel(short x, short y);
        };
    }
}