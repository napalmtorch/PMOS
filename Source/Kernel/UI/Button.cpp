#include <Kernel/UI/Button.hpp>
#include <Kernel/Core/Kernel.hpp>

namespace PMOS
{
    namespace UI
    {
        Button::Button() : Control() { }

        Button::Button(int x, int y, char* text) : Control(x, y, 92, 22, ControlType::Button)
        {
            SetName("BUTTON");
            SetText(text);
            TextVisible = true;
        }

        Button::Button(int x, int y, char* text, void* parent) : Control(x, y, 92, 22, ControlType::Button, parent)
        {
            SetName("BUTTON");
            SetText(text);
            TextVisible = true;
        }

        void Button::OnCreate()
        {
            Control::OnCreate();
        }

        void Button::OnDestroy()
        {
            Control::OnDestroy();
        }

        void Button::Update()
        {
            Control::Update();
        }

        void Button::Draw()
        {
            Control::Draw();
            Style.BorderInvert = MSFlags.Down;

            uint bg = Style.GetColor(0);
            if (MSFlags.Hover && !MSFlags.Down) { bg = Style.GetColor(5); }

            // draw standalone
            if (Parent == nullptr)
            {
                // draw background
                Kernel::XServer->Canvas.DrawFilledRect(Bounds, bg);

                // draw border
                if (Style.Border == BorderStyle::FixedSingle) { Kernel::XServer->Canvas.DrawRect(Bounds, Style.BorderSize, Style.GetColor(2)); }
                else if (Style.Border == BorderStyle::Fixed3D) { Kernel::XServer->Canvas.DrawRect3D(Bounds, Style.BorderInvert, Style.GetColor(2), Style.GetColor(3), Style.GetColor(4)); }

                // draw text
                if (TextVisible)
                {
                    int tw = StringUtil::Length(Text) * (Style.FontStyle->Width  + Style.FontStyle->SpacingX);
                    int th = Style.FontStyle->Height + Style.FontStyle->SpacingY;

                    if (TextAlign == Alignment::MiddleCenter)
                    {
                        int tx = Bounds.X + (Bounds.Width / 2) - (tw / 2);
                        int ty = Bounds.Y + (Bounds.Height / 2) - (th / 2);
                        Kernel::XServer->Canvas.DrawString(tx, ty, Text, Style.GetColor(1), (*Style.FontStyle));
                    }
                }
            }
            else
            {
                // draw background
                GetParent(Parent)->Renderer.DrawFilledRect(Bounds, bg);

                // draw border
                if (Style.Border == BorderStyle::FixedSingle) { GetParent(Parent)->Renderer.DrawRect(Bounds, Style.BorderSize, Style.GetColor(2)); }
                else if (Style.Border == BorderStyle::Fixed3D) { GetParent(Parent)->Renderer.DrawRect3D(Bounds, Style.BorderInvert, Style.GetColor(2), Style.GetColor(3), Style.GetColor(4)); }

                // draw text
                if (TextVisible)
                {
                    int tw = StringUtil::Length(Text) * (Style.FontStyle->Width  + Style.FontStyle->SpacingX);
                    int th = Style.FontStyle->Height + Style.FontStyle->SpacingY;

                    if (TextAlign == Alignment::MiddleCenter)
                    {
                        int tx = Bounds.X + (Bounds.Width / 2) - (tw / 2);
                        int ty = Bounds.Y + (Bounds.Height / 2) - (th / 2);
                        GetParent(Parent)->Renderer.DrawString(tx, ty, Text, Style.GetColor(1), (*Style.FontStyle));
                    }
                }
            }
        }

        void Button::Refresh()
        {
            Control::Refresh();
        }
    }
}