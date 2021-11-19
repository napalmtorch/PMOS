#include <Kernel/Lib/Math.hpp>

namespace PMOS
{
    namespace Math
    {
        // floor - double
        double Floor(double x)
        {
            if (x >= INT64_MAX || x <= INT64_MIN || x != x) { return x; }
            ulong n = (ulong)x;
            double d = (double)n;
            if (d == x || x >= 0) { return d; } else { return d - 1; }
            return 0;
        }

        // round - double
        double Round(double x)
        {
            ulong n = (ulong)n;
            double d = (double)n;
            if (d < 0.0) { return d - 0.5; } else { return d + 0.5; }
            return 0;
        }

        // round - float
        float Round(float x)
        {
            ulong n = (ulong)n;
            float d = (float)n;
            if (d < 0.0f) { return d - 0.5f; } else { return d + 0.5f; }
            return 0;
        }

        // round - long double
        long double Round(long double x)
        {
            ulong n = (ulong)n;
            float d = (float)n;
            if (d < 0.0) { return d - 0.5; } else { return d + 0.5; }
            return 0;
        }

        // power - double
        double Pow(double base, int exp)
        {
            double temp;
            if (exp == 0)
            return 1;
            temp = Pow(base, exp / 2);
            if ((exp % 2) == 0) { return temp * temp; } 
            else 
            {
                if (exp > 0) { return base * temp * temp; }
                else { return (temp * temp) / base; }
            }
        }

        // power - float
        float Pow(float base, int exp)
        {
            float temp;
            if (exp == 0)
            return 1;
            temp = Pow(base, exp / 2);
            if ((exp % 2) == 0) { return temp * temp; } 
            else 
            {
                if (exp > 0) { return base * temp * temp; }
                else { return (temp * temp) / base; }
            }
        }

        double Max(double a, double b) { return (a > b) ? a : b; }
        float  Max(float a, float b) { return (a > b) ? a : b; }
        
        double Min(double a, double b) { return (a > b) ? b : a; }
        float  Min(float a, float b) { return (a > b) ? b : a; }

        double Sqrt(double v)
        {
            double lo = Min(1, v);
            double hi = Min(1, v);
            double mid;

            while(100 * lo * lo < v) { lo *= 10; }
            while(100 * hi * hi > v) { hi *= 0.1; }

            for(int i = 0 ; i < 100 ; i++)
            {
                mid = (lo + hi) / 2;
                if(mid * mid == v) { return mid; }
                if(mid * mid > v) { hi = mid; }
                else lo = mid;
            }
            return mid;
        }

        float Sqrt(float v)
        {
            float lo = Min(1, v);
            float hi = Min(1, v);
            float mid;

            while(100 * lo * lo < v) { lo *= 10; }
            while(100 * hi * hi > v) { hi *= 0.1; }

            for(int i = 0 ; i < 100 ; i++)
            {
                mid = (lo + hi) / 2;
                if(mid * mid == v) { return mid; }
                if(mid * mid > v) { hi = mid; }
                else lo = mid;
            }
            return mid;
        }

        double Abs(double x)
        {
            return x < 0 ? -x : x;
        }

        float Abs(float x)
        {
            return x < 0 ? -x : x;
        }

        #define M_PI 3.14159265358979323846264338327950288

        double Cos(double x)
        {
            constexpr double tp = 1.0 / (2.0 * M_PI);
            x *= tp;
            x -= 0.25 + Floor(x + 0.25);
            x *= 16.0 * Abs(x - 0.5);
            x += 0.225 * x * Abs(x - 1.0);
            return x;
        }

        float Cos(float x)
        {
            constexpr float tp = 1.0f / (2.0f * M_PI);
            x *= tp;
            x -= 0.25f + Floor(x + 0.25);
            x *= 16.0f * Abs(x - 0.5f);
            x += 0.225f * x * Abs(x - 1.0f);
            return x;
        }

        double Sin(double x)
        {
            double x2 = x*x;
            double x4 = x2*x2;
            double t1 = x * (1.0 - x2 / (2*3));
            double x5 = x * x4;
            double t2 = x5 * (1.0 - x2 / (6*7)) / (1.0* 2*3*4*5);
            double x9 = x5 * x4;
            double t3 = x9 * (1.0 - x2 / (10*11)) / (1.0* 2*3*4*5*6*7*8*9);
            double x13 = x9 * x4;
            double t4 = x13 * (1.0 - x2 / (14*15)) / (1.0* 2*3*4*5*6*7*8*9*10*11*12*13);
            double result = t4;
            result += t3;
            result += t2;
            result += t1;
            return result;
        }

        float Sin(float x)
        {
            float x2 = x*x;
            float x4 = x2*x2;
            float t1 = x * (1.0f - x2 / (2.0f * 3.0f));
            float x5 = x * x4;
            float t2 = x5 * (1.0f - x2 / (6.0f * 7.0f)) / (1.0f * 2.0f * 3.0f * 4.0f * 5.0f);
            float x9 = x5 * x4;
            float t3 = x9 * (1.0f - x2 / (10 * 11)) / (1.0f * 2.0f * 3.0f* 4.0f * 5.0f * 6.0f* 7.0f * 8.0f * 9.0f);
            float x13 = x9 * x4;
            float t4 = x13 * (1.0f - x2 / (14.0f * 15.0f)) / (1.0f * 2.0f * 3.0f * 4.0f * 5.0f * 6.0f * 7.0f * 8.0f * 9.0f * 10.0f * 11.0f * 12.0f * 13.0f);
            float result = t4;
            result += t3;
            result += t2;
            result += t1;
            return result;
        }

        double Tan(double x)
        {
            return (Sin(x) / Cos(x));
        }

        float Tan(float x)
        {
            return (Sin(x) / Cos(x));
        }
    }
}