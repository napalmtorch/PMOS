#pragma once
#include <Kernel/Lib/Types.hpp>
#include <Kernel/Core/Service.hpp>
#include <Kernel/Graphics/Graphics.hpp>
#include <Kernel/UI/Container.hpp>
#include <Kernel/UI/Button.hpp>

namespace PMOS
{
    namespace UI
    {
        namespace XServer
        {
            class XTaskbar : public Container
            {
                public:
                    Button* BtnMenu;

                private:
                    int Time, LastTime;
                    bool HasLeft;

                public:
                    XTaskbar();
                    void OnCreate() override;
                    void OnDestroy() override;
                    void Update() override;
                    void Draw() override;
                    void Refresh() override;

                private:
                    void OnTimerTick();
                    void DrawTime();
            };
        }
    }
}