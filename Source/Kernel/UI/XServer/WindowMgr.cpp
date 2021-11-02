#include <Kernel/UI/XServer/WindowMgr.hpp>
#include <Kernel/Core/Kernel.hpp>

namespace PMOS
{
    namespace UI
    {
        WindowManager::WindowManager() : Service("winmgr", ServiceType::Utility)
        {

        }

        void WindowManager::Initialize()
        {
            Service::Initialize();

            Kernel::ServiceMgr.Register(this);
            Kernel::ServiceMgr.Start(this);
        }

        void WindowManager::Start()
        {
            Service::Start();

            MaxCount = 256;

            if (Windows != nullptr) { Kernel::MemoryMgr.FreeArray((void**)Windows, MaxCount); }
            Windows = (Window**)Kernel::MemoryMgr.Allocate(MaxCount * sizeof(Window*), true, AllocationType::System);
            WindowCount = 0;

            ActiveIndex = -1;
            ActiveWindow = nullptr;
        }

        void WindowManager::Stop()
        {
            Service::Stop();

            if (Windows != nullptr) 
            { 
                for (uint i = 0; i < MaxCount; i++) { if (Windows[i] != nullptr) { Windows[i]->Dispose(); }}
                Kernel::MemoryMgr.FreeArray((void**)Windows, MaxCount); 
            }
            WindowCount = 0;
        }

        bool m_down = false;
        void WindowManager::Update()
        {
            // skip if no windows are open
            if (WindowCount == 0) { return; }
            if (Windows == nullptr) { return; }

            // get mouse position
            int mx = Kernel::Mouse->GetX();
            int my = Kernel::Mouse->GetY();

            // temporary counters
            uint hover = 0, moving = 0, resizing = 0;
            int top_hover_index;
            int last_hover = -1;
            int active = 0;

            // check if shell is currently being used
            bool shell_hover = false;

            // loop through window
            for (uint i = 0; i < WindowCount; i++)
            {
                // validate window
                if (Windows[i] != nullptr)
                {
                    // handle close request
                    if (Windows[i]->XFlags.ExitRequest) 
                    { 
                        (void)Close(Windows[i]); 
                        return; 
                    }

                    // increment counters
                    if (!shell_hover)
                    {
                        if (RectangleContains(Windows[i]->Bounds, mx, my)) { hover++; }
                        if (RectangleContains(Windows[i]->Bounds, mx, my) && (int)i > top_hover_index) { top_hover_index = i; }
                        if (Windows[i]->XFlags.Moving) { moving++; }
                        if (Windows[i]->XFlags.Resizing) { resizing++; }
                        if (Windows[i] == ActiveWindow) { active = i; }
                        Windows[i]->Flags.Focused = (Windows[i] == ActiveWindow);
                        if ((int)i > last_hover && i < WindowCount - 1) { last_hover = i; }
                    }

                    // update window
                    Windows[i]->Update();
                }
            }

            // set active index
            ActiveIndex = active;

            // clicked away from all windows
            if (hover + moving + resizing == 0 && Kernel::Mouse->IsLeftPressed()) { ActiveWindow = nullptr; ActiveIndex = -1; }

            // validate no windows are being manipulated
            if (moving == 0 && resizing == 0 && !shell_hover)
            {
                // loop through window
                for (uint i = 0; i < WindowCount; i++)
                {
                    // validate window
                    if (Windows[i] != nullptr)
                    {
                        if (ActiveWindow == nullptr)
                        {
                            if (RectangleContains(Windows[i]->Bounds, mx, my))
                            {
                                if (Kernel::Mouse->IsLeftPressed() && !m_down) { SetActiveWindow(GetWindow(i)); m_down = true; break; }
                            }
                        }
                        else
                        {
                            if (RectangleContains(Windows[i]->Bounds, mx, my))
                            {
                                if (Kernel::Mouse->IsLeftPressed())
                                {
                                    if (ActiveIndex != i && ActiveWindow != GetWindow(i) && !RectangleContains(ActiveWindow->Bounds, mx, my) && !m_down) { SetActiveWindow(GetWindow(i)); m_down = true; break; }
                                }
                            }
                        }
                    }
                }
            }

            if (!Kernel::Mouse->IsLeftPressed()) { m_down = false; }
        }

        void WindowManager::Draw()
        {
            // skip if no windows are open
            if (WindowCount == 0) { return; }
            if (Windows == nullptr) { return; }

            for (uint i = 0; i < WindowCount; i++)
            {
                if (Windows[i] != nullptr)
                {
                    Windows[i]->Refresh();
                }
            }
        }
    
        bool WindowManager::Open(Window* win)
        {
            // validate window
            if (win == nullptr) { return false; }
            if (Windows == nullptr) { return false; }

            if (win == nullptr) { Kernel::Debug.Error("Unable to open null window"); return false; }
            if (WindowCount >= MaxCount) { Kernel::Debug.Error("Maximum amount of open windows reached"); return false; }

            Windows[WindowCount] = win;
            WindowCount++;
            ActiveWindow = win;
            ActiveIndex = GetWindowIndex(win);
            win->Flags.Focused = true;
            win->OnCreate();
            Kernel::Debug.Info("Opened window: %s", win->Name);
            return Windows[WindowCount - 1];
        }

        bool WindowManager::Close(Window* win)
        {
            if (win == nullptr) { Kernel::Debug.Error("Unable to close window null window"); return false; }
            if (Windows == nullptr) { return false; }
            if (WindowCount == 0) { return false; }

            for (size_t i = 0; i < MaxCount; i++)
            {
                if (Windows[i] == nullptr) { continue; }
                if (Windows[i] == win)
                {
                    Kernel::Debug.Info("Closed window: %s", win->Name);
                    if (ActiveIndex == i || ActiveWindow == win) { ActiveIndex = -1; ActiveWindow = nullptr; }
                    win->Dispose();
                    Kernel::MemoryMgr.Free(win);
                    Windows[i] = nullptr;
                    return true;
                }
            }

            Kernel::Debug.Error("Unable to close window that is not running");
            return false;
        }

        uint WindowManager::GetWindowCount() { return WindowCount; }

        int  WindowManager::GetWindowIndex(Window* win)
        {
            if (WindowCount == 0) { return -1; }
            if (Windows == nullptr) { return -1; }
            if (win == nullptr) { return -1; }

            for (size_t i = 0; i < MaxCount; i++)
            {
                if (Windows[i] == nullptr) { continue; }
                if (Windows[i] == win) { return i; }
            }
            return -1;
        }

        Window* WindowManager::GetWindow(int index)
        {
            if (WindowCount == 0) { return nullptr; }
            if (Windows == nullptr) { return nullptr; }
            if (index >= MaxCount) { return nullptr; }
            return Windows[index];
        }

        bool WindowManager::IsWindowRunning(Window* win)
        {
            // don't bother if list is empty
            if (WindowCount == 0) { return false; }
            if (Windows == nullptr) { return false; }
            if (win == nullptr) { return false; }

            for (size_t i = 0; i < WindowCount; i++)
            {
                if (Windows[i] == nullptr) { continue; }
                if (Windows[i] == win) { return true; }
            }

            // return nullptr if not running
            return false;
        }
        
        Window* WindowManager::SetActiveWindow(Window* win)
        {
            // validate window list
            if (WindowCount == 0) { return nullptr; }
            if (Windows == nullptr) { return nullptr; }

            // validate window
            if (win == nullptr) { return nullptr; }
            if (!IsWindowRunning(win)) { return nullptr; }

            int index = -1;
            for (size_t i = 0; i < WindowCount; i++) { if (Windows[i] == win) { index = i; break; } }
            if (index < 0 || index >= (int)WindowCount) { Kernel::Debug.Error("Invalid index while setting active window"); return nullptr; }

            // shift windows
            byte* start = (byte*)((uint)Windows + (sizeof(Window*) * index));
            byte* end   = (byte*)((uint)Windows + (sizeof(Window*) * 512));      
            for (uint i = (uint)start; i < (uint)end; i += sizeof(Window*))
            {
                if (i + sizeof(Window*) < (uint)end) { Memory::Copy((void*)i, (void*)(i + sizeof(Window*)), sizeof(Window*)); }
            }

            Kernel::Debug.Info("Finished sorting windows");

            // set active window
            ActiveWindow = win;

            // replace last window
            Windows[WindowCount - 1] = win;
            Kernel::Debug.Info("Set active window to %s", win->Name);
            ActiveIndex = GetWindowIndex(win);

            // return window pointer
            Kernel::Debug.Info("Returning active window");
            return ActiveWindow;
        }
    }
}