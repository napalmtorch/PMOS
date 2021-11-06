#pragma once
#include <Kernel/Lib/Types.hpp>
#include <Kernel/Lib/String.hpp>
#include <Kernel/Lib/Memory.hpp>

template<class V> class HashMap
{
    private:
        PMOS::String* Keys;
        V* Values;
        size_t Count;

    public:
        HashMap() { Clear(); }

        void Clear()
        {
            if (Keys != nullptr)
            {
                for (size_t i = 0; i < Count; i++) { Keys[i].Dispose(); }
                delete Keys;
            }
            if (Values != nullptr) { delete Values; }

            Keys = nullptr;
            Values = nullptr;
            Count = 0;
        }

        void Add(PMOS::String key, V val)
        {
            PMOS::String* keys = new PMOS::String[Count + 1];
            V* values    = new V[Count + 1];

            if (Keys != nullptr)
            {
                for (size_t i = 0; i < Count; i++)
                {
                    if (Keys[i].GetLength() == 0) { continue; }
                    keys[i] = Keys[i];
                    Keys[i].Dispose();
                }
                delete Keys;
            }

            if (Values != nullptr)
            {
                for (size_t i = 0; i < Count; i++) { values[i] = Values[i]; }
                delete Values;
            }
            
            keys[Count] = key;
            values[Count] = val;

            Keys = keys;
            Values = values;
            Count++;
        }

        V& Get(PMOS::String key)
        {
            if (Count == 0 || Keys == nullptr) { V v; return v; }
            for (size_t i = 0; i < Count; i++)
            {
                if (Keys[i].GetLength() == 0) { continue; }
                if (Keys[i].Equals(key)) { return Values[i]; }
            }
            
            V v;
            return v;
        }

        PMOS::String* GetKeys() { return Keys; }

        V* GetValues() { return Values; }

        size_t GetCount() { return Count; }

        bool Contains(PMOS::String key)
        {
            if (Count == 0 || Keys == nullptr) { return false; }
            for (size_t i = 0; i < Count; i++)
            {
                if (Keys[i].Equals(key)) { return true; }
            }
            return false;
        }

        V& operator[](PMOS::String key)
        {
            if (!Contains(key)) { V v; Add(key, v); }
            return Get(key);
        }
};

template<class K, class V> class Map
{
    private:
        K* Keys;
        V* Values;
        size_t Count;

    public:
        Map() { Clear(); }

        void Clear()
        {
            if (Keys != nullptr) { MemFree(Keys); }
            if (Values != nullptr) { MemFree(Values); }

            Keys   = nullptr;
            Values = nullptr;
            Count  = 0;
        }

        void Add(K name, V value)
        {
            K* keys   = new K[Count + 1];
            V* values = new V[Count + 1];

            if (Keys != nullptr)
            {
                for (size_t i = 0; i < Count; i++) { keys[i] = Keys[i]; }
                delete Keys;
            }

            if (Values != nullptr)
            {
                for (size_t i = 0; i < Count; i++) { values[i] = Values[i]; }
                delete Values;
            }

            keys[Count] = name;
            values[Count] = value;

            Keys   = keys;
            Values = values;
            Count  += 1;
        }

        V& Get(K key)
        {
            if (Count == 0 || Keys == nullptr) { V v; return v; }
            for (size_t i = 0; i < Count; i++)
            {
                if (Keys[i] == nullptr) { continue; }
                if (Keys[i] == key) { return Values[i]; }
            }
            
            V v;
            return v;
        }

        K* GetKeys() { return Keys; }

        V* GetValues() { return Values; }

        size_t GetCount() { return Count; }

        bool Contains(K key)
        {
            if (Count == 0 || Keys == nullptr) { return false; }
            for (size_t i = 0; i < Count; i++)
            {
                if (Keys[i] == key) { return true; }
            }
            return false;
        }

        V& operator[](K key)
        {
            if (!Contains(key)) { V v; Add(key, v); }
            return Get(key);
        }
};
