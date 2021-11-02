#pragma once
#include <Kernel/Lib/Types.hpp>

namespace PMOS
{
    struct Point
    {
        public:
            int X, Y;

        public:
            Point() { X = 0; Y = 0; }
            Point(int x, int y) { X = x; Y = y; }
    };

    struct Rectangle
    {
        public:
            int X, Y, Width, Height;

        public:
            Rectangle() { X = 0; Y = 0; Width = 0; Height = 0; }
            Rectangle(int x, int y, int w, int h) { X = x; Y = y; Width = w; Height = h; }
    };

    // check if position is inside rectangle
    static inline bool RectangleContains(int rx, int ry, int rw, int rh, int x, int y) { return x >= rx && x < rx + rw && y >= ry && y < ry + rh; }
    static inline bool RectangleContains(Rectangle rect, int x, int y) { return x >= rect.X && x < rect.X + rect.Width && y >= rect.Y && y < rect.Y + rect.Height; }
    static inline bool RectangleContains(Rectangle rect, Point pos) { return RectangleContains(rect, pos.X, pos.Y); }
    static inline bool RectangleContains(Rectangle rect1, Rectangle rect2) { return RectangleContains(rect1, rect2.X, rect2.Y); }

    namespace Math
    {
        double      Floor(double x);
        double      Round(double x);
        float       Round(float x);
        long double Round(long double x);
        double      Pow(double base, int exp);
        float       Pow(float base, int exp);
    }
}