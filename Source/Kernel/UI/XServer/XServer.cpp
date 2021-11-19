#include <Kernel/UI/XServer/XServer.hpp>
#include <Kernel/Core/Kernel.hpp>

namespace PMOS
{
    namespace UI
    {
        namespace XServer
        {
            XServerHost::XServerHost() : Service("xserver", ServiceType::Utility)
            {

            }

            void XServerHost::Initialize()
            {
                Service::Initialize();

                Kernel::ServiceMgr.Register(this);
                Kernel::ServiceMgr.Start(this);
            }

            void XServerHost::Start()
            {
                Service::Start();

                Kernel::Debug.SetMode(DebugMode::Serial);
                Kernel::CLI->Terminated = true;
                Kernel::Keyboard->TerminalOutput = false;
                Kernel::Keyboard->SetStream(nullptr);

                Canvas.Initialize();
                FPSLimit = 60;
                
                Wallpaper = new Graphics::Bitmap("/sys/resources/wallpaper.bmp");
                Wallpaper->Resize(Kernel::VESA->GetWidth(), Kernel::VESA->GetHeight());

                Taskbar = new XTaskbar();
                Kernel::MemoryMgr.GetEntryFromPtr(Taskbar)->Type = (byte)AllocationType::UI;
                Taskbar->OnCreate();
            }

            void XServerHost::Stop()
            {
                Service::Stop();

                Taskbar->Dispose();
                MemFree(Taskbar);

                if (Wallpaper != nullptr) { Wallpaper->Dispose(); MemFree(Wallpaper); }

                Kernel::ServiceMgr.Stop(Kernel::WinMgr);
                MemFree(Canvas.Buffer);
            }

            void XServerHost::Update()
            {
                Time = Kernel::RTC.GetSecond();
                if (Time != LastTime)
                {
                    FPS = Frames;
                    Frames = 0;

                    Kernel::ThreadMgr.CalculateCPUUsage();

                    char temp[64];
                    StringUtil::Clear(FPSString);
                    StringUtil::FromDecimal(FPS, temp);
                    StringUtil::Append(FPSString, "FPS: ");
                    StringUtil::Append(FPSString, temp);

                    LastTime = Time;
                }

                DrawTime = Kernel::PIT.GetMilliseconds();
                if (DrawTime != LastDrawTime)
                {
                    DrawTick++;
                    LastDrawTime = DrawTime;
                }

                // force quit
                if (Kernel::Keyboard->IsKeyDown(HAL::Key::LeftCtrl) && Kernel::Keyboard->IsKeyDown(HAL::Key::Escape))
                {
                    Kernel::ServiceMgr.Stop(Kernel::XServer);
                    Kernel::ServiceMgr.Start(Kernel::Terminal);
                    Kernel::ServiceMgr.Start(Kernel::CLI);
                    Kernel::VESA->ClearDirect(0);
                    Kernel::Terminal->Clear(Kernel::Terminal->GetBackColor());
                    Kernel::CLI->PrintCaret();
                    return;
                }
        
                // draw
                if (FPSLimit == 0) { Draw(); return; }
                if (DrawTick >= 1000 / FPSLimit) { Draw(); DrawTick = 0; }
            }

            void XServerHost::Draw()
            {
                Frames++;

                if (Wallpaper == nullptr) { Canvas.Clear({ 0x5F, 0x00, 0x5F, 0xFF }); }
                else { Canvas.DrawBitmapFast(0, 0, Wallpaper); }

                Canvas.DrawString(0, 0, FPSString, Colors::White, Fonts::Serif8x8);

                float cpu = Kernel::ThreadMgr.GetCPUUsage();
                char temp[64];
                char temp2[64];
                StringUtil::FromFloat(cpu, temp, 4);
                StringUtil::Copy(temp2, "CPU: ");
                StringUtil::Append(temp2, temp);
                StringUtil::Append(temp2, "%");
                Canvas.DrawString(0, 16, temp2, Colors::White, Fonts::Serif8x8);

                if (Taskbar != nullptr)
                {
                    Taskbar->Update();
                    Taskbar->Refresh();
                }

                if (Kernel::WinMgr != nullptr)
                {
                    Kernel::WinMgr->Update();
                    Kernel::WinMgr->Draw();
                }

                Canvas.DrawArray(Kernel::Mouse->GetX(), Kernel::Mouse->GetY(), 12, 20, Colors::Magenta, (uint*)MS_CURSOR_DATA);

                Canvas.Display();
            }
        }
    }
}