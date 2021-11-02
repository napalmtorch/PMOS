#pragma once
#include <Kernel/Lib/Types.hpp>
#include <Kernel/Core/Service.hpp>
#include <Kernel/Core/Debug.hpp>

namespace PMOS
{
    namespace Services
    {
        class ServiceManager
        {
            private:
                static const size_t MaxCount = 64;
                Service** Services;
                size_t Count;

            public:
                void Initialize();
                void Register(Service* s);
                void Unregister(Service* s);

            public:
                void Start(Service* s);
                void Start(char* name);
                void Stop(Service* s);
                void Stop(char* name);
                void Print(DebugMode mode);

            private:
                int GetFreeIndex();

            public:
                Service* Get(int i);
                Service* Get(char* name);
        };
    }
}