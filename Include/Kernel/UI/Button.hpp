#pragma once
#include <Kernel/Lib/Types.hpp>
#include <Kernel/UI/Control.hpp>

namespace PMOS
{
    namespace UI
    {    
        class Button : public Control
        {
            public:
                bool TextVisible;

            public:
                // main
                Button();
                Button(int x, int y, char* text);
                Button(int x, int y, char* text, void* parent);
                void OnCreate() override;
                void OnDestroy() override;
                void Update() override;
                void Draw() override;
                void Refresh() override;
        };
    }
}