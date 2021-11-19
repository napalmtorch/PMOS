#include <Kernel/UI/XServer/Taskbar.hpp>
#include <Kernel/Core/Kernel.hpp>

namespace PMOS
{
    namespace UI
    {
        namespace XServer
        {
            XTaskbar::XTaskbar() : Container(0, 0, 128, 22)
            {

            }

            void XTaskbar::OnCreate()
            {
                Container::OnCreate();
                Style.BorderSize = 0;

                BtnMenu = CreateButton(2, 2, "Menu", "BtnMenu", this);
                AddControl(BtnMenu);

                DrawTime();
            }

            void XTaskbar::OnDestroy()
            {
                Container::OnDestroy();
            }

            void XTaskbar::Update()
            {
                Container::Update();

                Flags.Focused = true;

                SetBounds(0, Kernel::VESA->GetHeight() - Bounds.Height, Kernel::VESA->GetWidth(), Bounds.Height);
                BtnMenu->SetBounds(2, 2, 64, 18);

                Time = Kernel::RTC.GetSecond();
                if (Time != LastTime)
                {
                    OnTimerTick();
                    LastTime = Time;
                }
            }

            void XTaskbar::Draw()
            {
                Container::Draw();

                if (MSFlags.Hover)
                {
                    DrawTime();
                    HasLeft = false;
                }

                if (!MSFlags.Hover && !HasLeft)
                {
                    DrawTime();
                    HasLeft = true;
                }
            }

            void XTaskbar::Refresh()
            {
                Container::Refresh();
            }

            void XTaskbar::OnTimerTick()
            {
                DrawTime();
            }

            void XTaskbar::DrawTime()
            {
                char* time = Kernel::RTC.GetTimeString(false, true);
                int tw = StringUtil::Length(time) * 8;
                Renderer.DrawString(Bounds.Width - tw - 4, (Bounds.Height / 2) - 4, time, Style.Colors[1], Style.Colors[0], Fonts::Serif8x8);
            }
        }
    }
}