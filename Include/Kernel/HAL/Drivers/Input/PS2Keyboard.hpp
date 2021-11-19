#pragma once
#include <Kernel/Lib/Types.hpp>
#include <Kernel/Lib/Stream.hpp>
#include <Kernel/Core/Service.hpp>
#include <Kernel/HAL/Drivers/Input/Key.hpp>

namespace PMOS
{
    namespace HAL
    {
        typedef struct
        {
            char Uppercase[60];
            char Lowercase[60];
        } ATTR_PACK KeyboardLayout;


        namespace Drivers
        {
            static const KeyboardLayout KBLayoutUS
            {
                "??!@#$%^&*()_+??QWERTYUIOP{}??ASDFGHJKL:\"~?|ZXCVBNM<>???? \0",
                "??1234567890-=??qwertyuiop[]??asdfghjkl;'`?\\zxcvbnm,./??? \0",
            };

            //static const char* KBLayoutUS = "??!@#$%^&*()_+??QWERTYUIOP{}??ASDFGHJKL:\"~?|ZXCVBNM<>????";

            class PS2Keyboard : public Service
            {
                private:
                    byte       Keymap[256];
                    byte       CurrentKey;
                    byte       PreviousKey;
                    Stream* CurrentStream;

                private:
                    bool LShiftDown, RShiftDown;
                    bool CapsDown;
                    bool EnterHandled;

                public:
                    void (*OnEnterPressed)(Stream* stream);
                    bool TerminalOutput;

                public:
                    PS2Keyboard();

                public:
                    void Initialize() override;
                    void Start() override;
                    void Stop() override;
                    void Handle(byte key);
                    void SetStream(Stream* stream);

                public:
                    bool IsKeyDown(Key key);
                    bool IsKeyUp(Key key);

                public:
                    Stream* GetStream();

                private:
                    void ClearKeymap();
                    void UpdateKeymap();
            };
        }
    }
}