#pragma once
#include <Kernel/Lib/Types.hpp>
#include <Kernel/Lib/Math.hpp>
#include <Kernel/Lib/Memory.hpp>
#include <Kernel/Graphics/Graphics.hpp>

namespace PMOS
{
    namespace UI
    {
        enum class BorderStyle
        {
            None,
            FixedSingle,
            Fixed3D,
        };

        struct VisualStyle
        {
            // properties
            char*       Name;
            BorderStyle Border;
            uint        BorderSize;
            bool        BorderInvert;
            uint        Colors[8];
            Font*       FontStyle;

            // get color
            uint GetColor(int index) 
            { 
                if (index >= 16) { return (uint)0xFF000000; }
                return Colors[index];
            }
        };

        static const VisualStyle ButtonStyle = 
        {
            "Default",
            BorderStyle::Fixed3D,
            1,
            false,
            {
                0xFFAFAFAF,
                0xFF000000,
                0xFFFFFFFF,
                0xFF7F7F7F,
                0xFF000000,
                0xFFCFCFCF,
                0xFF0000FF,
                0xFFFFFFFF,
            },
            (Font*)&Fonts::Serif8x8,
        };

        static const VisualStyle WindowStyle
        {
            "Default",
            BorderStyle::Fixed3D,
            1,
            false,
            {
                0xFFAFAFAF,
                0xFF000000,
                0xFFFFFFFF,
                0xFF7F7F7F,
                0xFF000000,
                0xFF0000FF,
                0xFFFFFFFF,
                0x00000000,
            },
            (Font*)&Fonts::Serif8x8,
        };

        static const VisualStyle TextBoxStyle
        {
            "Default",
            BorderStyle::Fixed3D,
            1,
            false,
            {
                0xFFFFFFFF,
                0xFF000000,
                0xFFAFAFAF,
                0xFF7F7F7F,
                0xFF000000,
                0xFF0000FF,
                0xFFFFFFFF,
                0x00000000,
            },
            (Font*)&Fonts::Serif8x8,
        };

        static const VisualStyle CheckBoxStyle
        {
            "Default",
            BorderStyle::Fixed3D,
            1,
            false,
            {
                0xFFFFFFFF,
                0xFF000000,
                0xFFAFAFAF,
                0xFF7F7F7F,
                0xFF000000,
                0xFF0000FF,
                0xFFFFFFFF,
                0x00000000,
            },
            (Font*)&Fonts::Serif8x8,
        };

        // copy style
        static inline void CopyStyle(VisualStyle* dest, VisualStyle* src)
        {
            dest->Name = src->Name;
            dest->BorderSize = src->BorderSize;
            dest->Border = src->Border;
            dest->FontStyle = src->FontStyle;
            dest->BorderInvert = src->BorderInvert;
            for (byte i = 0; i < 8; i++) { dest->Colors[i] = src->Colors[i]; }
        }
    }
}