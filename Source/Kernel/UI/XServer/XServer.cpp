#include <Kernel/UI/XServer/XServer.hpp>
#include <Kernel/Core/Kernel.hpp>

namespace PMOS
{
    namespace UI
    {
        namespace XServer
        {
            UI::Window* test_win;

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

                test_win = CreateWindow(128, 128, 320, 240, "Test Window", "testwin");
                Kernel::WinMgr->Open(test_win);
            }

            void XServerHost::Stop()
            {
                Service::Stop();

                Kernel::ServiceMgr.Stop(Kernel::WinMgr);
                Kernel::MemoryMgr.Free(Canvas.Buffer);
            }

            void XServerHost::Update()
            {
                Time = Kernel::RTC.GetSecond();
                if (Time != LastTime)
                {
                    FPS = Frames;
                    Frames = 0;

                    Kernel::ThreadMgr.CalculateCPUUsage();

                    String::Clear(FPSString);
                    String::FromDecimal(FPS, FPSString);
                    String::Append(FPSString, " FPS");

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
                if (FPSLimit == 0) { Draw(); }
                if (DrawTick >= 1000 / FPSLimit) { Draw(); DrawTick = 0; }
            }

            void XServerHost::Draw()
            {
                Frames++;

                Canvas.Clear({ 0x5F, 0x00, 0x5F, 0xFF });

                Canvas.DrawString(0, 0, FPSString, Colors::White, Fonts::Serif8x8);

                float cpu = Kernel::ThreadMgr.GetCPUUsage();
                char temp[64];
                Canvas.DrawString(0, 16, String::FromFloat(cpu, temp, 4), Colors::White, Fonts::Serif8x8);

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