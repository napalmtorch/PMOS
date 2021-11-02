#include <Kernel/Graphics/Bitmap.hpp>
#include <Kernel/Core/Kernel.hpp>

namespace PMOS
{
    namespace Graphics
    {
        Bitmap::Bitmap() { }

        Bitmap::Bitmap(int w, int h, byte depth)
        {
            Create(w, h, depth);
        }

        Bitmap::Bitmap(char* fullname)
        {
            Kernel::Debug.Write("Loading ");
            Kernel::Debug.WriteLine(fullname);
            VFS::FileEntry file = Kernel::FileSys->IOOpenFile(fullname);
            if (file.Size == 0 || String::Length(file.Name) == 0) { Kernel::Debug.Error("Unable to locate file %s", fullname); return; }

            bmp_fileheader_t* h = (bmp_fileheader_t*)file.Data;
            uint offset = h->off_bits;

            bmp_infoheader_t* info = (bmp_infoheader_t*)(file.Data + sizeof(bmp_fileheader_t));

            Width = info->width;
            Height = info->height;
            ImageData = (byte*)((uint)file.Data + offset);
            Depth = info->bit_count;
            if (Width == 0 || Height == 0 || Depth == 0 || file.Size < 0 || String::Length(file.Name) == 0) 
            { Kernel::Debug.Error("Unable to parse bitmap %s", fullname); return; }

            uint* new_data = (uint*)Kernel::MemoryMgr.Allocate(Width * Height * 4, true, AllocationType::Bitmap);
            Size = Width * Height * 4;
            for (int yy = Height - 1; yy >= 0; yy--)
            {
                for (int xx = 0; xx < Width; xx++)
                {
                    if (Depth == 24)
                    {
                        uint offset = (3 * (xx + (yy * Width)));
                        uint color = RGBToPackedValue(ImageData[offset + 2], ImageData[offset + 1], ImageData[offset]);
                        uint output_offset = xx + ((Height - yy - 1) * Width);
                        new_data[output_offset] = color;
                    }
                    else if (Depth == 32)
                    {
                        uint offset = (4 * (xx + (yy * Width)));
                        uint color = RGBToPackedValue(ImageData[offset + 2], ImageData[offset + 1], ImageData[offset]);
                        uint output_offset = xx + ((Height - yy - 1) * Width);
                        new_data[output_offset] = color;
                    }
                }
            }  

            ImageData = (byte*)new_data;
            Kernel::Debug.OK("Successfully loaded bitmap %s", fullname);
            Kernel::MemoryMgr.Free((void*)file.Data);
        }

        void Bitmap::Dispose() 
        { 
            Kernel::MemoryMgr.Free(ImageData);
        }

        void Bitmap::Create(int w, int h, byte depth)
        {
            Width = w;
            Height = h;
            Depth = depth;
            ImageData = (byte*)Kernel::MemoryMgr.Allocate(Width * Height * (byte)Depth, true, AllocationType::Bitmap);
        }


        void Bitmap::Resize(int w, int h)
        {
            Kernel::Debug.Info("Resizing bitmap");
            uint* temp = (uint*)Kernel::MemoryMgr.Allocate(w * h * sizeof(uint), true, AllocationType::Bitmap);
            double x_ratio = (double)Width / (double)w;
            double y_ratio = (double)Height / (double)h;
            int px, py;
            for (int i = 0; i < h; i++)
            {
                for (int j = 0; j < w; j++)
                {
                    px = Math::Floor(j * x_ratio);
                    py = Math::Floor(i * y_ratio);
                    temp[(i * w) + j] = ((uint*)ImageData)[(int)((py * Width) + px)];
                }
            }
            if (ImageData != nullptr) { Kernel::MemoryMgr.Free(ImageData); }
            ImageData = (byte*)temp;
            Width = w;
            Height = h;
        }

        void Bitmap::SetPixel(short x, short y, uint color)
        {
            if (x < 0 || x >= Width || y < 0 || y >= Height) { return; }
            ((uint*)ImageData)[x + (y * Width)] = color;
        }

        void Bitmap::DrawFilledRect(short x, short y, short w, short h, uint color)
        {
            for (int yy = 0; yy < h; yy++)
            {
                for (int xx = 0; xx < w; xx++)
                {
                    SetPixel(x + xx, y + yy, color);
                }
            }
        }

        uint Bitmap::GetPixel(short x, short y)
        {
            if (x < 0 || x >= Width || y < 0 || y >= Height) { return 0; }
            return (((uint*)ImageData)[x + (y * Width)]);
        }
    }
}