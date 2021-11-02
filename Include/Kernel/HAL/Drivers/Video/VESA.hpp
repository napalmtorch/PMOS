#pragma once
#include <Kernel/Lib/Types.hpp>
#include <Kernel/Graphics/Graphics.hpp>
#include <Kernel/Core/Service.hpp>

namespace PMOS
{
    namespace HAL
    {
        // info block
        typedef struct
        {
            char      Signature[4];
            ushort    Version;
            ushort    OEMString[2];
            byte      Capabilities[4];
            ushort    VideoMode[2];
            ushort    TotalMemory;
        } ATTR_PACK VESAInfoBlock;

        // mode info block
        typedef struct
        {
            ushort    Attributes;
            byte      WindowA, WindowB;
            ushort    Granularity;
            ushort    WindowSize;
            ushort    SegmentA, SegmentB;
            ushort    WindowFunction[2];
            ushort    Pitch, Width, Height;
            byte      CharWidth, CharHeight, Planes, Depth, Banks;
            byte      MemoryModel, BankSize, image_pages;
            byte      Reserved0;
            byte      RedMask, RedPosition;
            byte      GreenMask, GreenPosition;
            byte      BlueMask, BluePosition;
            byte      RSVMask, RSVPosition;
            byte      DirectColor;
            uint      PhysicalBase;
            uint      Reserved1;
            ushort    Reserved2;
        } ATTR_PACK VESAModeInfoBlock;

        // video mode
        typedef struct
        {
            ushort Width;
            ushort Height;
            byte   Depth;
        } ATTR_PACK VESAMode;

        namespace Drivers
        {
            class VESAController : public Service
            {
                public:
                VESAInfoBlock InfoBlock;
                VESAModeInfoBlock ModeInfoBlock;
                VESAMode CurrentMode;
                bool InfoBlockSet;
                void* Buffer;
                uint BufferSize;
                VESAMode AvailableModes[256];

            public:
                VESAController();
                void Initialize() override;
                void Start() override;
                void Stop() override;
                void Display();

            private:
                void PopulateInfoBlock();
                void GetAvailableModes();

            public:
                bool SetMode(short w, short h);
                bool IsModeAvailable(short w, short h);

            public:
                void Clear();
                void Clear(uint color);
                void ClearDirect();
                void ClearDirect(uint color);

                void SetPixel(short x, short y, uint color);
                void SetPixel(int index, uint color);
                void SetPixelDirect(short x, short y, uint color);
                void SetPixelDirect(int index, uint color);

                uint GetPixel(short x, short y);
                uint GetPixel(int index);
                uint GetPixelDirect(short x, short y);
                uint GetPixelDirect(int index);

                ushort     GetWidth();
                ushort     GetHeight();
                ColDepth   GetDepth();
                void*      GetBuffer();
            };
        }
    }
}