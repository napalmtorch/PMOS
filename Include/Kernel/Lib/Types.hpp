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

/*
// i will work on string class
// start implementing here - idk how you prefer it to be named lol ok xD
template<class K, class V> class Map
{
private:
    K* keys = nullptr;
    V* values = nullptr;

    size_t count = 0;
public:
    Map() {}

    // get value by key
    V& Get(K name)
    {
        // dummy variable, just to return
        V dummy;

        // check if keys/values are not null
        if (keys == nullptr || values == nullptr) return dummy;

        // kernel panic is a good sign right ig xD

        // loop through keys
        for (int i = 0; i < count; i++)
        {
            // if key is found
            if (name == keys[i])
            {
                // return value
                return values[i];
            }
        }

        // return dummy
        return dummy;
    }

    // add new key/value pair
    void Set(K name, V value)
    {
        // check if keys/values are not null
        if (keys == nullptr || values == nullptr)
        {
            // allocate memory for keys/values
            keys = new K[1];
            values = new V[1];

            // set key/value
            keys[0] = name;
            values[0] = value;

            // increase count
            count++;
        }
        else
        {
            // allocate memory for new keys/values
            K* newKeys = new K[count + 1];
            V* newValues = new V[count + 1];

            // loop through keys
            for (int i = 0; i < count; i++)
            {
                // set new keys/values
                newKeys[i] = keys[i];
                newValues[i] = values[i];
            }

            // set new key/value
            newKeys[count] = name;
            newValues[count] = value;

            // delete old keys/values
            delete keys;
            delete values;

            // set new keys/values
            keys = newKeys;
            values = newValues;

            // increase count
            count++;
        }
    }

    // remove key/value pair
    void Remove(K name)
    {
        // check if keys/values are not null
        if (keys == nullptr || values == nullptr) return;

        // loop through keys
        for (int i = 0; i < count; i++)
        {
            // if key is found
            if (name == keys[i])
            {
                // remove key/value
                V* newValues = new V[count - 1];
                K* newKeys = new K[count - 1];

                // loop through keys
                for (int j = 0; j < count; j++)
                {
                    // if key is found
                    if (j == i)
                    {
                        // skip
                        continue;
                    }
                    else
                    {
                        // set new keys/values
                        newKeys[j] = keys[j];
                        newValues[j] = values[j];
                    }
                }

                // decrease count
                count--;

                // break loop
                break;
            }
        }
    }

    // remove all keys/values
    void Clear()
    {
        // delete keys/values
        delete keys;
        delete values;

        // set keys/values to null
        keys = nullptr;
        values = nullptr;

        // set count to 0
        count = 0;
    }

    // check if key exists
    bool Contains(K name)
    {
        // check if keys/values are not null
        if (keys == nullptr || values == nullptr) return false;

        // loop through keys
        for (int i = 0; i < count; i++)
        {
            // if key is found
            if (name == keys[i])
            {
                // return true
                return true;
            }
        }

        // return false
        return false;
    }

    // get count
    size_t Count()
    {
        return count;
    }

    // get keys
    K* Keys()
    {
        return keys;
    }

    // get values
    V* Values()
    {
        return values;
    }

    // override []
    V &operator[](K name)
    {
        // check if the key exists
        if (!Contains(name))
        {
            // add key/value pair
            Set(name, V());
        }

        // return value
        return Get(name);
    }
};

// fixed it lmao
// yes wait no its cuz you were still typing a few seconds ago when I ran it
*/