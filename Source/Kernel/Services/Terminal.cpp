#include <Kernel/Services/Terminal.hpp>
#include <Kernel/Core/Kernel.hpp>

namespace PMOS
{
    namespace Services
    {
        TextModeTerminal::TextModeTerminal() : Service("term", ServiceType::Utility)
        {

        }

        void TextModeTerminal::Initialize()
        {
            Service::Initialize();

            Legacy = false;   
            Kernel::ServiceMgr.Register(this);
            Kernel::ServiceMgr.Start(this);
        }

        void TextModeTerminal::Start()
        {
            Service::Start();
            
            SetCursorPos(0, 0, true);
            ForeColor = Col4::White;
            BackColor = Col4::Black;

            if (Legacy) { Width = Kernel::VGA->GetWidth(); Height = Kernel::VGA->GetHeight(); }
            else { Width = Kernel::VESA->GetWidth() / 8; Height = Kernel::VESA->GetHeight() / 16; }
            
            Kernel::Debug.Info("W = %d, H = %d", Width, Height);
        }

        void TextModeTerminal::Stop()
        {
            Service::Stop();
        }

        void TextModeTerminal::Clear()
        {        
            Clear(BackColor);
        }

        void TextModeTerminal::Clear(Col4 bg)
        {
            if (!Started) { return; }
            BackColor = bg;

            if (Legacy)
            {
                for (size_t i = 0; i < Kernel::VGA->GetWidth() * Kernel::VGA->GetHeight() * 2; i += 2)
                {
                    Kernel::VGA->GetBuffer()[i + 0] = 0x20;
                    Kernel::VGA->GetBuffer()[i + 1] = (byte)(((byte)bg << 4) | (byte)ForeColor);
                }
                SetCursorPos(0, 0, true);
                return;
            }

            Canvas.Clear(Canvas.ConvertColor(bg));
            CursorX = 0;
            CursorY = 0;
        }

        void TextModeTerminal::NewLine()
        {
            CursorX = 0;
            CursorY++;
            if (CursorY >= Height)
            {
                Scroll();
            }
            UpdateCursor();
        }

        void TextModeTerminal::Delete()
        {
            if (Legacy)
            {
                if (CursorX > 0)
                {
                    CursorX--;
                    Kernel::VGA->DrawChar(CursorX, CursorY, 0x20, (byte)ForeColor, (byte)BackColor);
                }
                else if (CursorY > 0)
                {
                    CursorX = Width - 1;
                    CursorY--;
                    Kernel::VGA->DrawChar(CursorX, CursorY, 0x20, (byte)ForeColor, (byte)BackColor);
                }
                UpdateCursor();
                return;
            }

            if (CursorX > 0)
            {
                CursorX--;
                Canvas.DrawChar(CursorX * 8, CursorY * 16, 0x20,  Canvas.ConvertColor(ForeColor), Canvas.ConvertColor(BackColor), Fonts::Serif8x16);
            }
            else if (CursorY > 0)
            {
                CursorX = Width - 1;
                CursorY--;
                Canvas.DrawChar(CursorX * 8, CursorY * 16, 0x20,  Canvas.ConvertColor(ForeColor), Canvas.ConvertColor(BackColor), Fonts::Serif8x16);
            }
            UpdateCursor();
        }

        void TextModeTerminal::Scroll()
        {
            if (!Started) { return; }
            if (Legacy)
            {
                uint len = Kernel::VGA->GetWidth() * Kernel::VGA->GetHeight() * 2;
                uint lineLen = Kernel::VGA->GetWidth() * 2;
                Kernel::Debug.WriteLine("LEN: %d   LINE_LEN: %d", len, lineLen);
                Memory::Copy((void*)Kernel::VGA->GetBuffer(), (void*)((uint)Kernel::VGA->GetBuffer() + lineLen), len - lineLen);
                for (int i = 0; i < Kernel::VGA->GetWidth(); i++) { Kernel::VGA->DrawChar(i, Kernel::VGA->GetHeight() - 1, 0x20, (byte)ForeColor, (byte)BackColor); }
                SetCursorPos(0, Kernel::VGA->GetHeight() - 1, true);
                return;
            }

            uint len = Kernel::VESA->GetWidth() * Kernel::VESA->GetHeight() * 4;
            uint lineLen = (Kernel::VESA->GetWidth() * 4) * 16;
            Memory::Copy((void*)Kernel::VESA->ModeInfoBlock.PhysicalBase, (void*)((uint)Kernel::VESA->ModeInfoBlock.PhysicalBase + lineLen), len - lineLen);
            Memory::Set((void*)((uint)Kernel::VESA->ModeInfoBlock.PhysicalBase + (len - lineLen)), Canvas.ConvertColor(BackColor).ToInteger(), lineLen);
            SetCursorPos(0, Height - 1, true);
        }

        void TextModeTerminal::WriteChar(char c) { WriteChar(c, ForeColor, BackColor); }
        
        void TextModeTerminal::WriteChar(char c, Col4 fg) { WriteChar(c, fg, BackColor); }
        
        void TextModeTerminal::WriteChar(char c, Col4 fg, Col4 bg) 
        { 
            if (!Started) { return; }
            if (Legacy)
            {
                if (c == '\n') { NewLine(); }
                else if (c >= 32 && c < 126) 
                { 
                    Kernel::VGA->DrawChar((ushort)CursorX, (ushort)CursorY, c, (byte)fg, (byte)bg); 
                    CursorX++;
                    if (CursorX >= Kernel::VGA->GetWidth()) { NewLine(); }
                }
                return;
            }

            if (c == '\n') { NewLine(); }
            else if (c >= 32 && c < 126)
            {
                Canvas.DrawChar(CursorX * 8, CursorY * 16, c, Canvas.ConvertColor(fg), Canvas.ConvertColor(bg), Fonts::Serif8x16);
                CursorX++;
                if (CursorX >= Width) { NewLine(); }
            }
        }
        
        void TextModeTerminal::Write(char* txt) { Write(txt, ForeColor, BackColor); }
        
        void TextModeTerminal::Write(char* txt, Col4 fg) { Write(txt, fg, BackColor); }
        
        void TextModeTerminal::Write(char* txt, Col4 fg, Col4 bg)
        {
            int i = 0;
            while (txt[i] != 0)
            {
                WriteChar(txt[i], fg, bg);
                i++;
            }
            UpdateCursor();
        }
        
        void TextModeTerminal::WriteLine(char* txt) { WriteLine(txt, ForeColor, BackColor); }
        
        void TextModeTerminal::WriteLine(char* txt, Col4 fg) { WriteLine(txt, fg, BackColor); }
        
        void TextModeTerminal::WriteLine(char* txt, Col4 fg, Col4 bg)
        {
            int i = 0;
            while (txt[i] != 0)
            {
                WriteChar(txt[i], fg, bg);
                i++;
            }
            NewLine();
            UpdateCursor();
        }

        void TextModeTerminal::EnableCursor(int top, int bottom) { Kernel::VGA->EnableCursor(top, bottom); }
        
        void TextModeTerminal::DisableCursor() { Kernel::VGA->DisableCursor(); }
        
        void TextModeTerminal::UpdateCursor() 
        { 
            if (Legacy) { Kernel::VGA->SetCursorPos(CursorX, CursorY); }
        }
        
        void TextModeTerminal::SetCursorPos(int x, int y, bool update)
        {
            if (x >= 0 && x < Kernel::VGA->GetWidth()) { CursorX = x; }
            if (y >= 0 && y < Kernel::VGA->GetHeight()) { CursorY = y; }
            if (update) { UpdateCursor(); }
        }

        void TextModeTerminal::SetCursorX(int x, bool update)
        {
            if (x < 0 || x >= Kernel::VGA->GetWidth()) { return; }
            CursorX = x;
            if (update) { UpdateCursor(); }
        }

        void TextModeTerminal::SetCursorY(int y, bool update)
        {
            if (y < 0 || y >= Kernel::VGA->GetHeight()) { return; }
            CursorY = y;
            if (update) { UpdateCursor(); }
        }

        void TextModeTerminal::SetColors(Col4 fg, Col4 bg) { ForeColor = fg; BackColor = bg; }
        
        void TextModeTerminal::SetForeColor(Col4 fg) { ForeColor = fg; }
        
        void TextModeTerminal::SetBackColor(Col4 bg) { BackColor = bg; }

        Point TextModeTerminal::GetCursorPos() { return Point(CursorX, CursorY); }
        
        int TextModeTerminal::GetCursorX() { return CursorX; }
        
        int TextModeTerminal::GetCursorY() { return CursorY; }

        Col4 TextModeTerminal::GetForeColor() { return ForeColor; }
        
        Col4 TextModeTerminal::GetBackColor() { return BackColor; }
    }
}