#include <Kernel/Core/Service.hpp>
#include <Kernel/Core/Kernel.hpp>

namespace PMOS
{
    Service::Service(char* name, ServiceType type)
    {
        // copy name
        for (size_t i = 0; i < StringUtil::Length(name); i++) { if (i < 64) { this->Name[i] = name[i]; } }

        // set type
        this->Type = type;
        this->Started = false;
    }

    void Service::Initialize()
    {
        
    }

    void Service::Start()
    {
        if (Started) { return; }
        Started = true;
    }

    void Service::Stop()
    {
        if (!Started) { return; }
        Started = false;
    }

    char* Service::GetName() { return Name; }

    ServiceType Service::GetType() { return Type; }

    bool Service::IsStarted() { return Started; }

    const char* Service::GetTypeString(ServiceType type)
    {
        switch (type)
        {
            case ServiceType::KernelComponent:  { return "KERNEL_COMPONENT "; }
            case ServiceType::Application:      { return "APPLICATION      "; }
            case ServiceType::Driver:           { return "DRIVER           "; }
            case ServiceType::Utility:          { return "UTILITY          "; }
            default:                            { return "UNKNOWN          "; }
        }
    }
}