#pragma once

// extern c short-form
#define extc extern "C"

extc
{
    // unsigned types
    typedef unsigned char  byte;
    typedef unsigned short ushort;
    typedef unsigned int   uint;
    typedef unsigned long  ulong;

    // signed types
    typedef signed char sbyte;
    
    // size
    typedef unsigned long int size_t;

    // packed attributes short-form
    #define ATTR_PACK __attribute__((packed))

    // null
    #define NULL 0

    // used to eliminate unused compiler warnings
    #define UNUSED(x) (void)(x)

    // signed minimums
    # define INT8_MIN                (-128)
    # define INT16_MIN               (-32767-1)
    # define INT32_MIN               (-2147483647-1)
    # define INT64_MIN               (-__INT64_C(9223372036854775807)-1)

    // signed maximums
    # define INT8_MAX                (127)
    # define INT16_MAX               (32767)
    # define INT32_MAX               (2147483647)
    # define INT64_MAX               (__INT64_C(9223372036854775807))

    // unsigned maximums
    # define UINT8_MAX                (255)
    # define UINT16_MAX               (65535)
    # define UINT32_MAX               (4294967295U)
    # define UINT64_MAX               (__UINT64_C(18446744073709551615))

    // get lower half of 32-bit value
    #define LowBits16(address) (ushort)((address) & 0xFFFF)

    // get higher half of 32-bit value
    #define HighBits16(address) (ushort)(((address) & 0xFFFF0000) >> 16)

    // get bit from value
    extc inline bool GetBit(uint number, byte bit) { return (number & (1 << (bit - 1))) != 0; }


    // variadic arguments - borrowed from stdarg.h
    typedef char* va_list;

    #define __va_argsiz(t)	\
        (((sizeof(t) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))

    #define va_start(ap, pN)	\
        ((ap) = ((va_list) (&pN) + __va_argsiz(pN)))

    #define va_end(ap)	((void)0)

    #define va_arg(ap, t)					\
        (((ap) = (ap) + __va_argsiz(t)),		\
        *((t*) (void*) ((ap) - __va_argsiz(t))))
}

template<typename T> class Array
{
    public:
        T Data;
        size_t Count;

    public:
        // new constructor
        Array(uint count)
        {
            T* d = new T[count];
            Data = (*d);
            Count = count;
        }
        // existing constructor
        Array(T& data, uint count)
        {
            Data = data;
            Count = count;
        }
};

