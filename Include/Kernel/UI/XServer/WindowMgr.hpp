#pragma once
#include <Kernel/Lib/Types.hpp>
#include <Kernel/UI/Control.hpp>
#include <Kernel/Core/Service.hpp>

namespace PMOS
{
    namespace UI
    {
        class WindowManager : public Service
        {
            public:
                Window** Windows;
                Window*  ActiveWindow;
                uint     WindowCount;
                uint     ActiveIndex;
                uint MaxCount;

            public:
                WindowManager();
                void Initialize() override;
                void Start() override;
                void Stop() override;
                void Update();
                void Draw();
            
            public:
                bool Open(Window* win);
                bool Close(Window* win);

            public:
                uint GetWindowCount();
                int  GetWindowIndex(Window* win);
                Window* GetWindow(int index);
                bool IsWindowRunning(Window* win);
                
            private:
                Window* SetActiveWindow(Window* win);
        };
    }
}