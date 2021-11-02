#pragma once
#include <Kernel/Lib/Types.hpp>
#include <Kernel/Lib/Math.hpp>
#include <Kernel/Graphics/Graphics.hpp>
#include <Kernel/Core/Service.hpp>

namespace PMOS
{
    namespace Services
    {
        class TextModeTerminal : public Service
        {
            private:
                int Width, Height;
                int CursorX, CursorY;
                Col4 BackColor, ForeColor;

            public:
                bool Legacy;

            private:
                Graphics::VESADirectCanvas Canvas;

            public:
                TextModeTerminal();
                void Initialize() override;
                void Start() override;
                void Stop() override;

            public:
                void Clear();
                void Clear(Col4 bg);
                void NewLine();
                void Delete();
                void Scroll();

            public:
                void WriteChar(char c);
                void WriteChar(char c, Col4 fg);
                void WriteChar(char c, Col4 fg, Col4 bg);
                void Write(char* txt);
                void Write(char* txt, Col4 fg);
                void Write(char* txt, Col4 fg, Col4 bg);
                void WriteLine(char* txt);
                void WriteLine(char* txt, Col4 fg);
                void WriteLine(char* txt, Col4 fg, Col4 bg);

            public:
                void EnableCursor(int top, int bottom);
                void DisableCursor();
                void UpdateCursor();
                void SetCursorPos(int x, int y, bool update = true);
                void SetCursorX(int x, bool update = true);
                void SetCursorY(int y, bool update = true);
                void SetColors(Col4 fg, Col4 bg);
                void SetForeColor(Col4 fg);
                void SetBackColor(Col4 bg);

            public:
                Point GetCursorPos();
                int GetCursorX();
                int GetCursorY();

            public:
                Col4 GetForeColor();
                Col4 GetBackColor();

        };
    }
}