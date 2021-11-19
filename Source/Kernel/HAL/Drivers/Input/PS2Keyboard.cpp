#include <Kernel/HAL/Drivers/Input/PS2Keyboard.hpp>
#include <Kernel/Core/Kernel.hpp>

void PS2KeyboardCallback(Registers32 regs)
{
    // check status
    byte status = PMOS::HAL::Ports::Read8(0x64);
    if ((status & 0b00000001) == 0) { return; }

    // read scancode
    byte code = PMOS::HAL::Ports::Read8(0x60);

    // handle keyboard input
    PMOS::Kernel::Keyboard->Handle(code);

    // unused register argument
    UNUSED(regs);
}

namespace PMOS
{
    namespace HAL
    {
        namespace Drivers
        {
            // keyboard constructor
            PS2Keyboard::PS2Keyboard() : Service("kbps2", ServiceType::Driver)
            {

            }

            void PS2Keyboard::Initialize()
            {
                Service::Initialize();
                Kernel::ServiceMgr.Register(this);
            }

            // enable keyboard driver
            void PS2Keyboard::Start()
            {
                // base
                Service::Start();

                // register interrupt
                Kernel::InterruptMgr.Register(IRQ1, (ISR)PS2KeyboardCallback);

                HAL::Ports::Write8(0x64, 0xAE);

                // clear states
                ClearKeymap();

                Kernel::Debug.OK("Started PS/2 keyboard driver");
            }

            // disable keyboard driver
            void PS2Keyboard::Stop()
            {
                // base
                Service::Stop();

                // unregister interrupt
                Kernel::InterruptMgr.Unregister(IRQ1);

                // message
                Kernel::Debug.Warning("Stopped PS/2 keyboard driver");
            }

            // handle keyboard input
            void PS2Keyboard::Handle(byte key)
            {
                CurrentKey = key;

                if (key <= 0x58) { Keymap[(int)key] = true; }
                else if (key >= 0x81 && key < 0xD8) { Keymap[(int)key - 128] = false; }
                
                if (CurrentStream != nullptr)
                {
                    char* out = (char*)CurrentStream->ToArray();

                    if (key == (byte)KeyPressed::BACKSPACE)
                    {
                        // handle backspace
                        if (StringUtil::Length(out) > 0)
                        {
                            if (CurrentStream->GetSize() == 0) { return; }
                            StringUtil::Delete(out);
                            CurrentStream->Seek(CurrentStream->GetPosition() - 1);
                            if (TerminalOutput) { Kernel::Terminal->Delete(); }
                        }
                    }
                    else if (key == (byte)KeyPressed::LSHIFT) { LShiftDown = true; }
                    else if (key == (byte)KeyPressed::RSHIFT) { RShiftDown = true; }
                    else if (key == (byte)KeyReleased::LSHIFT) { LShiftDown = false; }
                    else if (key == (byte)KeyReleased::RSHIFT) { RShiftDown = false; }
                    else if (key == (byte)KeyPressed::CAPS_LOCK) { CapsDown = !CapsDown; }
                    else if (key == (byte)KeyPressed::LCTRL) { }
                    else if (key == (byte)KeyPressed::LALT) { }
                    else if (key == (byte)KeyPressed::ESCAPE) { }
                    else if (key == (byte)KeyPressed::ENTER)
                    {
                        if (TerminalOutput) { Kernel::Terminal->NewLine(); }
                        if (OnEnterPressed != nullptr) { OnEnterPressed(CurrentStream); }
                    }
                    else if (key < 58)
                    {
                        char ascii = 0;
                        bool caps = (((LShiftDown || RShiftDown) && !CapsDown) || ((!LShiftDown && !RShiftDown) && CapsDown));

                        if (caps) { ascii = KBLayoutUS.Uppercase[key]; }
                        else      { ascii = KBLayoutUS.Lowercase[key]; }

                        CurrentStream->Write(ascii);
                        if (TerminalOutput) { Kernel::Terminal->WriteChar(ascii); Kernel::Terminal->UpdateCursor(); }
                    }
                }
            }

            // set current keyboard stream
            void PS2Keyboard::SetStream(Stream* stream) { CurrentStream = stream; }

            // return current keyboard stream
            Stream* PS2Keyboard::GetStream() { return CurrentStream; }

            // clear all states in keymap array
            void PS2Keyboard::ClearKeymap() { for (ushort i = 0; i < 256; i++) { Keymap[i] = 0; } }

            // get key states and store them in keymap array
            void PS2Keyboard::UpdateKeymap()
            {
                
            }

            bool PS2Keyboard::IsKeyDown(Key key)
            {
                return Keymap[(int)key];
            }

            bool PS2Keyboard::IsKeyUp(Key key)
            {
                return Keymap[(int)key] == false ? true : false;
            }
        }
    }
}