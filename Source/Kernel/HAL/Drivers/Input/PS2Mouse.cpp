#include <Kernel/HAL/Drivers/Input/PS2Mouse.hpp>
#include <Kernel/Core/Kernel.hpp>

void PS2MouseCallback(uint* regs)
{
    PMOS::Kernel::Mouse->OnInterrupt();
    UNUSED(regs);
}

namespace PMOS
{
    namespace HAL
    {
        namespace Drivers
        {
            PS2Mouse::PS2Mouse() : Service("msps2", ServiceType::Driver)
            {

            }

            void PS2Mouse::Initialize()
            {
                Service::Initialize();

                Kernel::ServiceMgr.Register(this);
                Kernel::ServiceMgr.Start(this);
            }

            void PS2Mouse::Start()
            {
                Service::Start();

                SetScreenBounds(0, 0, Kernel::VESA->GetWidth(), Kernel::VESA->GetHeight());
                SetPosition(ScreenBounds.Width / 2, ScreenBounds.Height / 2);

                // clear data
                Offset      = 100;
                Buttons     = 0;
                Cycle       = 0;
                LeftButton  = false;
                RightButton = false;
                Memory::Set(Buffer, 0, sizeof(Buffer));

                // register interrupt
                Kernel::InterruptMgr.Register(IRQ12, (ISR)PS2MouseCallback);

                // setup device
                Ports::Write8(0x64, 0xA8);
                Ports::Write8(0x64, 0x20);
                byte status = (Ports::Read8(0x60) | 2);
                Ports::Write8(0x64, 0x60);
                Ports::Write8(0x60, status);
                Ports::Write8(0x64, 0xD4);
                Ports::Write8(0x60, 0xF4);
                (void)Ports::Read8(0x60);
            }

            void PS2Mouse::Stop()
            {
                Service::Stop();
                
                Kernel::InterruptMgr.Unregister(IRQ12);
            }

            void PS2Mouse::OnInterrupt()
            {
                byte status = Ports::Read8(0x64);
                if ((!(status & 1)) == 1) { Cycle = 0; return; }
                if ((!(status & 2)) == 0) { Cycle = 0; return; }
                if (!(status & 0x20))     { Cycle = 0; return; }

                if (Cycle == 0) { Buffer[0] = Ports::Read8(0x60); }
                else if (Cycle == 1) { Buffer[1] = Ports::Read8(0x60); }
                else if (Cycle == 2)
                {
                    Buffer[2] = Ports::Read8(0x60);
                    OnMove(Buffer[1], -Buffer[2]);
                    LeftButton  = ((Buffer[0] & 0b00000001));
                    RightButton = ((Buffer[0] & 0b00000010) >> 1);
                    Cycle = 0;
                    return;
                }
                else { Kernel::Debug.Error("PS/2 mouse cycle overflow exception"); return; }

                Cycle++;
                return;
            }

            void PS2Mouse::OnMove(sbyte x, sbyte y)
            {
                SetPosition(Position.X + x, Position.Y + y);
            }

            void PS2Mouse::SetPosition(int x, int y)
            {
                Position.X = x;
                Position.Y = y;

                if (Position.X < ScreenBounds.X) { Position.X = ScreenBounds.X; }
                if (Position.Y < ScreenBounds.Y) { Position.Y = ScreenBounds.Y; }
                if (Position.X >= ScreenBounds.X + ScreenBounds.Width) { Position.X  = ScreenBounds.X + ScreenBounds.Width - 1; }
                if (Position.Y >= ScreenBounds.Y + ScreenBounds.Height) { Position.Y = ScreenBounds.Y + ScreenBounds.Height - 1; }
            }

            void PS2Mouse::SetPosition(Point pos) { SetPosition(pos.X, pos.Y); }

            void PS2Mouse::SetScreenBounds(int x, int y, int w, int h)
            {
                ScreenBounds.X = x;
                ScreenBounds.Y = y;
                ScreenBounds.Width = w;
                ScreenBounds.Height = h;
            }

            void PS2Mouse::SetScreenBounds(Point pos, Point size) { SetScreenBounds(pos.X, pos.Y, size.X, size.Y); }

            void PS2Mouse::SetScreenBounds(Rectangle bounds) { SetScreenBounds(bounds.X, bounds.Y, bounds.Width, bounds.Height); }

            bool PS2Mouse::IsLeftPressed() { return LeftButton; }

            bool PS2Mouse::IsRightPressed() { return RightButton; }
            
            Point PS2Mouse::GetPosition() { return Point(Position.X, Position.Y); }
            
            int PS2Mouse::GetX() { return Position.X; }

            int PS2Mouse::GetY() { return Position.Y; }

            Rectangle PS2Mouse::GetScreenBounds() { return Rectangle(ScreenBounds.X, ScreenBounds.Y, ScreenBounds.Width, ScreenBounds.Height); }
        }
    }
}