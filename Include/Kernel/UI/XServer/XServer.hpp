#pragma once
#include <Kernel/Lib/Types.hpp>
#include <Kernel/Core/Service.hpp>
#include <Kernel/Graphics/Graphics.hpp>
#include <Kernel/UI/XServer/Taskbar.hpp>

namespace PMOS
{
    namespace UI
    {
        namespace XServer
        {
            class XServerHost : public Service
            {
                public:
                    Graphics::VESACanvas Canvas;
                    XTaskbar* Taskbar;
                    Graphics::Bitmap* Wallpaper;

                private:
                    char FPSString[64];
                    int FPS, Frames, Time, LastTime;
                    int DrawTick, FPSLimit, DrawTime, LastDrawTime;

                public:
                    XServerHost();
                    void Initialize() override;
                    void Start() override;
                    void Stop() override;

                public:
                    void Update();
                    void Draw();
            };
        }
    }
}