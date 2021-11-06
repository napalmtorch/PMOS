#include <Kernel/UI/Container.hpp>
#include <Kernel/Core/Kernel.hpp>

namespace PMOS
{
    namespace UI
    {
        Container::Container() : Control()
        {
            
        }

        Container::Container(int x, int y, int w, int h) : Control(x, y, w, h, ControlType::Container)
        {
            // set flags
            Flags.Child = false;
            OverrideDraw = false;
            DrawControls = true;
            
            // initialize renderer
            RenderImage = new Graphics::Bitmap(w, h, 32);
            Renderer = Graphics::BitmapCanvas(RenderImage);

            // set style
            SetStyle((VisualStyle*)&WindowStyle);

            // refresh
            Update();
        }

        void Container::OnCreate()
        {
            Control::OnCreate();
        }

        void Container::OnDestroy()
        {
            if (RenderImage != nullptr) { RenderImage->Dispose(); MemFree(RenderImage); RenderImage = nullptr; }
            Renderer.SetBitmap(nullptr);

            if (ControlCount > 0)
            {
                for (uint i = 0; i < ControlCount; i++) { MemFree(Controls[i]); }
            }
            MemFree(Controls);

            Control::OnDestroy();
        }

        void Container::Update()
        {
            Control::Update();

            if (Flags.Enabled)
            {
                if (Bounds.Width != WidthOld || Bounds.Height != HeightOld)
                {
                    if (RenderImage != nullptr) { RenderImage->Dispose(); MemFree(RenderImage); }
                    RenderImage = new Graphics::Bitmap(Bounds.Width, Bounds.Height, 32);
                    Renderer.SetBitmap(RenderImage);
                    InvokeRefresh();
                    WidthOld  = Bounds.Width;
                    HeightOld = Bounds.Height;
                }

                // update controls
                if (Controls != nullptr)
                {
                    for (size_t i = 0; i < ControlCount; i++)
                    {
                        if (Controls[i] != nullptr) 
                        { 
                            if (!Flags.Focused) { Controls[i]->Flags.Focused = false; }
                            Controls[i]->Update(); 
                        }
                    }
                }
            }
        }

        void Container::Draw()
        {
            Control::Draw();

            // clear background
            Renderer.SetBitmap(RenderImage);

            if (!OverrideDraw)
            {
                Renderer.DrawFilledRect(0, 0, Bounds.Width, Bounds.Height, Style.GetColor(0));
                
                // draw border
                if (Style.Border == BorderStyle::FixedSingle) { Renderer.DrawRect(0, 0, Bounds.Width, Bounds.Height, Style.BorderSize, Style.GetColor(2)); }
                else if (Style.Border == BorderStyle::Fixed3D) { Renderer.DrawRect3D(0, 0, Bounds.Width, Bounds.Height, false, Style.GetColor(2), Style.GetColor(3), Style.GetColor(4)); }
            }

            // draw controls
            if (DrawControls && Controls != nullptr)
            {
                for (size_t i = 0; i < ControlCount; i++)
                {
                    if (Controls[i] != nullptr) { Controls[i]->Draw(); }
                }
            }
        }

        void Container::Refresh()
        {
            Control::Refresh();

            // draw render buffer
            Kernel::XServer->Canvas.DrawBitmapFast(Bounds.X, Bounds.Y, RenderImage);

            // refresh controls
            if (DrawControls && Controls != nullptr)
            {
                for (size_t i = 0; i < ControlCount; i++)
                {
                    if (Controls[i] != nullptr) { Controls[i]->Refresh(); }
                }
            }
        }
    
        bool Container::AddControl(Control* control)
        {
            // temporary array
            Control** temp = new Control*[ControlCount + 1];

            // copy controls to temporary array
            if (ControlCount > 0)
            { for (size_t i = 0; i < ControlCount; i++) { temp[i] = Controls[i]; } }

            // delete old array if exists
            if (Controls != nullptr) { MemFree(Controls); }

            // assign new array
            Controls = temp;

            // add new control
            Controls[ControlCount] = control;

            // increment control count and return
            ControlCount++;
            return true;
        }

        bool Container::RemoveControl(Control* control)
        {
            if (ControlCount == 0) { return false; }
            if (control == nullptr) { return false; }

            bool found = false;
            for (size_t i = 0; i < ControlCount; i++)
            {
                if (Controls[i] != nullptr && Controls[i] == control)
                {
                    // temporary array
                    found = true;
                    break;
                }
            }

            if (found)
            {
                // create temporary array
                Control** temp = new Control*[ControlCount - 1];

                // copy all except specified
                int index = 0;
                for (size_t i = 0; i < ControlCount; i++)
                {
                    if (Controls[i] != control) { temp[index] = Controls[i]; index++; }
                }

                // delete old array
                MemFree(Controls);

                // assign new array
                Controls = temp;

                // decrement and return success
                ControlCount--;
                return true;
            }
            else { return false; }
        }

        bool Container::RemoveControlAt(int index)
        {
            if (index < 0 || index >= (int)ControlCount) { return false; }
            if (ControlCount == 0) { return false; }

            bool found = false;
            for (size_t i = 0; i < ControlCount; i++)
            {
                if (i == (uint)index && Controls[i] != nullptr)
                {
                    // temporary array
                    found = true;
                    break;
                }
            }

            if (found)
            {
                // create temporary array
                Control** temp = new Control*[ControlCount - 1];

                // copy all except specified
                int x = 0;
                for (size_t i = 0; i < ControlCount; i++)
                {
                    if (i != (uint)index) { temp[x] = Controls[i]; x++; }
                }

                // delete old array
                MemFree(Controls);

                // assign new array
                Controls = temp;

                // decrement and return success
                ControlCount--;
                return true;
            }
        }

        uint Container::GetControlCount() { return ControlCount; }
    }
}