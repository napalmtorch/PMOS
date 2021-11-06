#include <Kernel/UI/Window.hpp>
#include <Kernel/Core/Kernel.hpp>

namespace PMOS
{
    namespace UI
    {
          // on title bar close clicked
        void Window_OnCloseClicked(void* win, void* control)
        {
            if (win != nullptr) { ((Window*)win)->XFlags.ExitRequest = true; }
        }

        // on title bar maximize/restore clicked
        void Window_OnMaxClicked(void* win, void* control)
        {

        }

        // on title bar minimize clicked
        void Window_OnMinClicked(void* win, void* control)
        {

        }

        Window::Window() : Container()
        {

        }

        Window::Window(int x, int y, int w, int h, char* name) : Container(x, y, w, h)
        {
            Type = ControlType::Window;
            SetText("Window");
            SetName(name);

            SetStyle((VisualStyle*)&WindowStyle);

            Flags.Child = false;
            OverrideDraw = true;
            DrawControls = true;
            State = WindowState::Normal;

            // close button
            BtnClose = CreateButton(0, 0, "", "BtnClose", this);
            BtnClose->SetSize(16, 14);
            BtnClose->Click = Window_OnCloseClicked;
            BtnClose->TextVisible = false;

            // max button
            BtnMax = CreateButton(0, 0, "", "BtnMax", this);
            BtnMax->SetSize(16, 14);
            BtnMax->Click = Window_OnMaxClicked;
            BtnMax->TextVisible = false;

            // min button
            BtnMin = CreateButton(0, 0, "", "BtnMin", this);
            BtnMin->SetSize(16, 14);
            BtnMin->Click = Window_OnMinClicked;
            BtnMin->TextVisible = false;

            // update and refresh
            Update();
            Draw();
            Refresh();
        }

        Window::Window(int x, int y, int w, int h, char* title, char* name) : Container(x, y, w, h)
        {
            Type = ControlType::Window;
            SetText(title);
            SetName(name);

            SetStyle((VisualStyle*)&WindowStyle);

            Flags.Child = false;
            OverrideDraw = true;
            DrawControls = true;
            State = WindowState::Normal;

            // close button
            BtnClose = CreateButton(0, 0, "", "BtnClose", this);
            BtnClose->SetSize(16, 14);
            BtnClose->Click = Window_OnCloseClicked;
            BtnClose->TextVisible = false;

            // max button
            BtnMax = CreateButton(0, 0, "", "BtnMax", this);
            BtnMax->SetSize(16, 14);
            BtnMax->Click = Window_OnMaxClicked;
            BtnMax->TextVisible = false;

            // min button
            BtnMin = CreateButton(0, 0, "", "BtnMin", this);
            BtnMin->SetSize(16, 14);
            BtnMin->Click = Window_OnMinClicked;
            BtnMin->TextVisible = false;

            // update and refresh
            Update();
            Draw();
            Refresh();
        }

        Window::Window(int x, int y, int w, int h, char* title, char* name, char* args) : Container(x, y, w, h)
        {
            Type = ControlType::Window;
            SetText(title);
            SetName(name);
            
            if (args != nullptr && StringUtil::Length(args) > 0)
            {
                Arguments = (char*)MemAlloc(StringUtil::Length(args) + 1, true, AllocationType::String);
                StringUtil::Copy(Arguments, args);
            }

            SetStyle((VisualStyle*)&WindowStyle);

            Flags.Child = false;
            OverrideDraw = true;
            DrawControls = true;
            State = WindowState::Normal;

            // close button
            BtnClose = CreateButton(0, 0, "", "BtnClose", this);
            BtnClose->SetSize(16, 14);
            BtnClose->Click = Window_OnCloseClicked;
            BtnClose->TextVisible = false;

            // max button
            BtnMax = CreateButton(0, 0, "", "BtnMax", this);
            BtnMax->SetSize(16, 14);
            BtnMax->Click = Window_OnMaxClicked;
            BtnMax->TextVisible = false;

            // min button
            BtnMin = CreateButton(0, 0, "", "BtnMin", this);
            BtnMin->SetSize(16, 14);
            BtnMin->Click = Window_OnMinClicked;
            BtnMin->TextVisible = false;

            // update and refresh
            Update();
            Draw();
            Refresh();
        }

        void Window::OnCreate()
        {
            Container::OnCreate();
        }

        void Window::OnDestroy()
        {
            Container::OnDestroy();
            BtnClose->Dispose();
            BtnMax->Dispose();
            BtnMin->Dispose();
            MemFree(BtnClose);
            MemFree(BtnMax);
            MemFree(BtnMin);
            if (Arguments != nullptr) { MemFree(Arguments); }
            Kernel::Debug.Info("Destroyed window: ", Name);
        }

        bool move_click;
        int mx_start, my_start;
        void Window::Update()
        {
            Container::Update();
            OverrideDraw = true;

            TitleBarBounds.X = 1;
            TitleBarBounds.Y = 1;
            TitleBarBounds.Width = Bounds.Width - 3;
            TitleBarBounds.Height = 18;

            ClientBounds.X = 1;
            ClientBounds.Y = 1 + TitleBarBounds.Height;
            ClientBounds.Width = Bounds.Width - 3;
            ClientBounds.Height = Bounds.Height - (TitleBarBounds.Height + 2);

            HandleEvents();

            if (BtnClose != nullptr)
            {
                BtnClose->Update();
                BtnClose->SetPosition(TitleBarBounds.X + TitleBarBounds.Width - 18, TitleBarBounds.Y + 2);
            }

            if (BtnMax != nullptr)
            {
                BtnMax->Update();
                BtnMax->SetPosition(BtnClose->Bounds.X - 18, BtnClose->Bounds.Y);
            }

            if (BtnMin != nullptr)
            {
                BtnMin->Update();
                BtnMin->SetPosition(BtnMax->Bounds.X - 18, BtnMax->Bounds.Y);
            }
        }

        void Window::HandleEvents()
        {
            if (XFlags.Moving) { Flags.Focused = true; }
            if (Flags.Focused && Flags.Enabled)// && Kernel::XServer.WindowMgr.ActiveWindow == this)
            {
                if (State == WindowState::Normal)
                {
                    // get mouse position
                    int mx = Kernel::Mouse->GetX();
                    int my = Kernel::Mouse->GetY();

                    // check for movement
                    if (RectangleContains(Bounds.X + TitleBarBounds.X, Bounds.Y + TitleBarBounds.Y, TitleBarBounds.Width, TitleBarBounds.Height, mx, my))
                    {
                        if (Kernel::Mouse->IsLeftPressed() && !BtnClose->MSFlags.Hover && !BtnMax->MSFlags.Hover && !BtnMin->MSFlags.Hover)
                        {
                            if (!move_click)
                            {
                                mx_start = mx - Bounds.X;
                                my_start = my - Bounds.Y;
                                Kernel::Debug.Info("Starting moving window  : ", Name);
                                move_click = true;
                            }
                            XFlags.Moving = true;
                        }
                    }

                    // relocate window
                    if (XFlags.Moving)
                    {
                        Bounds.X = mx - mx_start;
                        Bounds.Y = my - my_start;
                        if (Bounds.Y < 0) { Bounds.Y = 0; }
                    }

                    // mouse release
                    if (!Kernel::Mouse->IsLeftPressed())
                    {
                        if (XFlags.Moving)
                        {
                            Kernel::Debug.Write("[");
                            Kernel::Debug.Write("  >>  ", Col4::Cyan);
                            Kernel::Debug.Write("] ");
                            Kernel::Debug.Write("Finished moving window : ", Name);
                            Kernel::Debug.Write(", X = %d", Bounds.X);
                            Kernel::Debug.WriteLine(", Y = %d", Bounds.Y);
                        }

                        XFlags.Moving = false;
                        XFlags.Resizing = false;
                        move_click = false;
                    }
                }
            }

            // mouse release
            if (!Kernel::Mouse->IsLeftPressed())
            {
                XFlags.Moving = false;
                XFlags.Resizing = false;
                move_click = false;
            }
        }

        void Window::Draw()
        {
            // draw background
            Renderer.DrawFilledRect(ClientBounds, Style.GetColor(0));

            // draw border
            if (Style.Border == BorderStyle::FixedSingle) { Renderer.DrawRect(0, 0, Bounds.Width, Bounds.Height, Style.BorderSize, Style.GetColor(2)); }
            else if (Style.Border == BorderStyle::Fixed3D) { Renderer.DrawRect3D(0, 0, Bounds.Width, Bounds.Height, false, Style.GetColor(2), Style.GetColor(3), Style.GetColor(4)); }

            // draw title bar
            Renderer.DrawFilledRect(TitleBarBounds, Style.GetColor(5));

            // draw text
            if (Text != nullptr) 
            { 
                if (Icon == nullptr) 
                { 
                    Renderer.DrawString(4, 7, Text, Style.GetColor(6), (*Style.FontStyle)); 
                }
                else
                {
                    Renderer.DrawBitmap(4, 2, Colors::Magenta, Icon);
                    Renderer.DrawString(24, 7, Text, Style.GetColor(6), (*Style.FontStyle));
                }
            }

            // draw buttons
            if (BtnClose != nullptr) { BtnClose->Draw(); }
            if (BtnMax   != nullptr) { BtnMax->Draw();   }
            if (BtnMin   != nullptr) { BtnMin->Draw();   }

            // draw base
            Container::Draw();
        }

        void Window::Refresh()
        {
            if (XFlags.Moving)
            {
                Kernel::XServer->Canvas.DrawRect(Bounds, 2, 0xFFFFFFFF);
                Kernel::XServer->Canvas.DrawRect(Bounds, 1, 0xFF000000);
            }
            else
            {
                Container::Refresh();

                // draw button icons
                Kernel::XServer->Canvas.DrawFlatArray(Bounds.X + BtnClose->Bounds.X + 4, Bounds.Y + BtnClose->Bounds.Y + 3, 8, 7, Style.GetColor(1), (byte*)IMG_TITLEBAR_CLOSE);
                Kernel::XServer->Canvas.DrawFlatArray(Bounds.X + BtnMax->Bounds.X + 4, Bounds.Y + BtnMax->Bounds.Y + 3, 8, 7, Style.GetColor(1), (byte*)IMG_TITLEBAR_MAX);
                Kernel::XServer->Canvas.DrawFlatArray(Bounds.X + BtnMin->Bounds.X + 4, Bounds.Y + BtnMin->Bounds.Y + 3, 8, 7, Style.GetColor(1), (byte*)IMG_TITLEBAR_MIN);
            }
        }
    }
}