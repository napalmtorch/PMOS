#pragma once
#include <Kernel/Lib/Types.hpp>

namespace PMOS
{
    enum class ServiceType : byte
    {
        KernelComponent     = 0x01,
        Driver              = 0x02,
        Utility             = 0x03,
        Application         = 0x04,
    };

    class Service
    {
        protected:
            char Name[64];
            bool Started;
            ServiceType Type;

        public:
            Service(char* name, ServiceType type);
            virtual void Initialize();
            virtual void Start();
            virtual void Stop();

        public:
            char* GetName();
            ServiceType GetType();
            bool IsStarted();

        public:
            static const char* GetTypeString(ServiceType type);
    };
}