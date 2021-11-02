#pragma once
#include <Kernel/Lib/Types.hpp>
#include <Kernel/Lib/Math.hpp>
#include <Kernel/Graphics/Graphics.hpp>
#include <Kernel/Graphics/Bitmap.hpp>
#include <Kernel/UI/Style.hpp>

namespace PMOS
{
    namespace UI
    {
        const byte IMG_TITLEBAR_CLOSE[8 * 7] = 
        {
            1, 1, 0, 0, 0, 0, 1, 1,
            0, 1, 1, 0, 0, 1, 1, 0,
            0, 0, 1, 1, 1, 1, 0, 0,
            0, 0, 0, 1, 1, 0, 0, 0,
            0, 0, 1, 1, 1, 1, 0, 0,
            0, 1, 1, 0, 0, 1, 1, 0,
            1, 1, 0, 0, 0, 0, 1, 1,
        };

        const byte IMG_TITLEBAR_MAX[8 * 7] = 
        {
            1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1,
            1, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 1,
            1, 0, 0, 0, 0, 0, 0, 1,
            1, 1, 1, 1, 1, 1, 1, 1,
        };

        const byte IMG_TITLEBAR_MIN[8 * 7] = 
        {
            0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,
            1, 1, 1, 1, 1, 1, 1, 0,
        };

        const byte IMG_TITLEBAR_RES[8 * 7] = 
        {
            0, 0, 1, 1, 1, 1, 1, 1,
            0, 0, 1, 1, 1, 1, 1, 1,
            0, 1, 1, 1, 1, 1, 0, 1,
            0, 1, 1, 1, 1, 1, 0, 1,
            0, 1, 0, 0, 0, 1, 0, 1,
            0, 1, 0, 0, 0, 1, 1, 1,
            0, 1, 1, 1, 1, 1, 0, 0,
        };

        const byte IMG_CHECMARK[7 * 7] = 
        {
            0, 0, 0, 0, 0, 0, 1,
            0, 0, 0, 0, 0, 1, 1,
            1, 0, 0, 0, 1, 1, 1,
            1, 1, 0, 1, 1, 1, 0,
            1, 1, 1, 1, 1, 0, 0,
            0, 1, 1, 1, 0, 0, 0,
            0, 0, 1, 0, 0, 0, 0,
        };
        
        enum class ControlType
        {
            Null,
            Container,
            Window,
            Button,
            CheckBox,
            TextBox,
            ListBox,
            Label,
            TabControl,
            TabPage,

        };

        enum class DockStyle
        {
            None,
            Left,
            Top,
            Right,
            Bottom,
            Fill,
        };

        enum class Alignment
        {
            TopLeft,
            TopCenter,
            TopRight,
            MiddleLeft,
            MiddleCenter,
            MiddleRight,
            BottomLeft,
            BottomCenter,
            BottomRight,
        };

        typedef struct
        {
            bool CanFocus;
            bool CanClick;
            bool CanRefresh;
            bool Focused;
            bool Enabled;
            bool Visible;
            bool Child;
            bool Toggled;
        } ATTR_PACK ControlFlags;

        typedef struct
        {
            bool Hover;
            bool Down;
            bool Up;
            bool Clicked;
            bool UnClicked;
            bool Leave;
        } ATTR_PACK MouseFlags;

        class Control;
        class Button;
        class Container;
        class Window;

        Container* GetParent(void* parent);

        // control generation methods - returns new control
        Button*    CreateButton(int x, int y, char* text);
        Button*    CreateButton(int x, int y, char* text, char* name);
        Button*    CreateButton(int x, int y, char* text, Container* parent);
        Button*    CreateButton(int x, int y, char* text, char* name, Container* parent);
        Container* CreateContainer(int x, int y, int w, int h);
        Container* CreateContainer(int x, int y, int w, int h, char* name);
        Window*    CreateWindow(int x, int y, int w, int h, char* name);
        Window*    CreateWindow(int x, int y, int w, int h, char* title, char* name);
        Window*    CreateWindow(int x, int y, int w, int h, char* title, char* name, char* args);

        class Control
        {
            public:
                char*        Text;
                char*        Name;
                char*        Tag;
                void*        Parent;
                ControlType  Type;
                ControlFlags Flags;
                Alignment    TextAlign;
                Rectangle    Bounds;
                MouseFlags   MSFlags;
                VisualStyle  Style;

            public:
                void (*Click)(void* win, void* control);
                void (*MouseDown)(void* win, void* control);
                void (*MouseUp)(void* win, void* control);
                void (*MouseHover)(void* win, void* control);
                void (*MouseLeave)(void* win, void* control);
                void (*MouseEnter)(void* win, void* control);

            public:
                void OnClick();
                void OnMouseDown();
                void OnMouseUp();
                void OnMouseHover();
                void OnMouseLeave();
                void OnMouseEnter();

            public:
                Control();
                Control(int x, int y, int w, int h, ControlType type);
                Control(int x, int y, int w, int h, ControlType type, void* parent);
                void Dispose();
                void InvokeRefresh();

            public:
                virtual void OnCreate();
                virtual void OnDestroy();
                virtual void Update();
                virtual void Draw();
                virtual void Refresh();

            public:
                void SetPosition(int x, int y);
                void SetPosition(Point pos);
                void SetSize(int w, int h);
                void SetSize(Point size);
                void SetBounds(int x, int y, int w, int h);
                void SetBounds(Point pos, Point size);
                void SetBounds(Rectangle bounds);
                void SetText(char* text);
                void SetName(char* name);
                void SetTag(char* tag);
                void SetStyle(VisualStyle* style);
                void Toggle(bool state);
        };
    }
}