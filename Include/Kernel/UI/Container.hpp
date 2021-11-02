#pragma once
#include <Kernel/Lib/Types.hpp>
#include <Kernel/UI/Control.hpp>

namespace PMOS
{
    namespace UI
    {   
        class Container : public Control
        {
            protected:
                Graphics::Bitmap* RenderImage;
                uint    ControlCountOld;
                uint    ControlCount;

            public:
                Control** Controls;
                Graphics::BitmapCanvas Renderer;
                bool OverrideDraw;
                bool DrawControls;
                int WidthOld, HeightOld;

            public:
                Container();
                Container(int x, int y, int w, int h);
                void OnCreate() override;
                void OnDestroy() override;
                void Update() override;
                void Draw() override;
                void Refresh() override;
            
            public:
                bool AddControl(Control* control);
                bool RemoveControl(Control* control);
                bool RemoveControlAt(int index);
                uint GetControlCount();
        };
    }
}