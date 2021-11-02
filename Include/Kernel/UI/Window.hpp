#pragma once
#include <Kernel/Lib/Types.hpp>
#include <Kernel/UI/Control.hpp>
#include <Kernel/UI/Container.hpp>
#include <Kernel/UI/Button.hpp>

namespace PMOS
{
    namespace UI
    {     
        enum class WindowState
        {
            Minimized,
            Normal,
            Maximized,
        };
           
        typedef struct
        {
            bool Moving;
            bool Resizing;
            bool ExitRequest;
            bool IsVM;
        } ATTR_PACK WindowFlags;
             
        class Window : public Container
        {
            public:
                char* Arguments;
                Rectangle ClientBounds;
                Rectangle TitleBarBounds;
                WindowFlags XFlags;
                WindowState State;

            public:
                Button* BtnClose;
                Button* BtnMax;
                Button* BtnMin;
                Graphics::Bitmap* Icon;

            public:
                Window();
                Window(int x, int y, int w, int h, char* name);
                Window(int x, int y, int w, int h, char* title, char* name);
                Window(int x, int y, int w, int h, char* title, char* name, char* args);
                void OnCreate() override;
                void OnDestroy() override;
                void Update() override;
                void Draw() override;
                void Refresh() override;
                void HandleEvents();
        };
    }
}