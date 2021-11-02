#include <Kernel/Services/ServiceMgr.hpp>
#include <Kernel/Core/Kernel.hpp>

namespace PMOS
{
    namespace Services
    {
        void ServiceManager::Initialize()
        {
            Services = (Service**)Kernel::MemoryMgr.Allocate(sizeof(Service*) * MaxCount, true, AllocationType::System);
            Count = 0;
        }

        void ServiceManager::Register(Service* s)
        {
            if (s == nullptr) { return; }
            int i = GetFreeIndex();
            if (i < 0 || i >= MaxCount) { Kernel::Debug.Error("Maximum amount of registered services has been reached"); return; }
            Services[i] = s;
            Kernel::Debug.Info("Registered service '%s' : %s", s->GetName(), Service::GetTypeString(s->GetType()));
        }

        void ServiceManager::Unregister(Service* s)
        {
            if (s == nullptr) { return; }
            for (size_t i = 0; i < MaxCount; i++)
            {
                if (Services[i] == s)
                {
                    Services[i] = nullptr;     
                    Kernel::Debug.Info("Unregistered service '%s' : %s", s->GetName(), Service::GetTypeString(s->GetType()));
                    return;
                }
            }
        }

        void ServiceManager::Start(Service* s)
        {
            if (s == nullptr) { return; }
            for (size_t i = 0; i < MaxCount; i++)
            {
                if (Services[i] == s) 
                { 
                    Services[i]->Start(); 
                    Kernel::Debug.Info("Started service '%s'", Services[i]->GetName());
                }
            }
        }

        void ServiceManager::Start(char* name)
        {
            if (name == nullptr) { return; }
            if (String::Length(name) == 0) { return; }
            for (size_t i = 0; i < MaxCount; i++)
            {
                if (Services[i] == nullptr) { continue; }
                if (String::Equals(Services[i]->GetName(), name)) 
                { 
                    Services[i]->Start(); 
                    Kernel::Debug.Info("Started service '%s'", Services[i]->GetName());
                }
            }
        }

        void ServiceManager::Stop(Service* s)
        {
            if (s == nullptr) { return; }
            for (size_t i = 0; i < MaxCount; i++)
            {
                if (Services[i] == s) 
                { 
                    if (Services[i]->GetType() == ServiceType::KernelComponent) { Kernel::Debug.Error("Cannot terminate kernel component"); return; }
                    Services[i]->Stop();
                    Kernel::Debug.Info("Stopped service '%s'", Services[i]->GetName());
                }
            }
        }
        
        void ServiceManager::Stop(char* name)
        {
            if (name == nullptr) { return; }
            if (String::Length(name) == 0) { return; }
            for (size_t i = 0; i < MaxCount; i++)
            {
                if (Services[i] == nullptr) { continue; }
                if (String::Equals(Services[i]->GetName(), name)) 
                { 
                    if (Services[i]->GetType() == ServiceType::KernelComponent) { Kernel::Debug.Error("Cannot terminate kernel component"); return; }
                    Services[i]->Stop(); 
                    Kernel::Debug.Info("Stopped service '%s'", Services[i]->GetName());
                }
            }
        }

        void ServiceManager::Print(DebugMode mode)
        {
            DebugMode oldMode = Kernel::Debug.Mode;
            Kernel::Debug.SetMode(mode);
            Kernel::Debug.WriteUnformatted("-------- ", Col4::DarkGray);
            Kernel::Debug.WriteUnformatted("SERVICES", Col4::Green);
            Kernel::Debug.WriteUnformatted(" ----------------------------------");
            Kernel::Debug.NewLine();
            Kernel::Debug.WriteUnformatted("TYPE                 STATE  NAME\n", Col4::DarkGray);

            for (size_t i = 0; i < MaxCount; i++)
            {
                if (Services[i] == nullptr) { continue; }
                
                Kernel::Debug.Write("%s    %d      ", Service::GetTypeString(Services[i]->GetType()), (int)Services[i]->IsStarted());
                Kernel::Debug.Write("%s", Services[i]->GetName());
                Kernel::Debug.NewLine();
            }

            Kernel::Debug.NewLine();
            Kernel::Debug.SetMode(oldMode);
        }

        int ServiceManager::GetFreeIndex()
        {
            for (size_t i = 0; i < MaxCount; i++)
            {
                if (Services[i] == nullptr) { return (int)i; }
            }
            return -1;
        }

        Service* ServiceManager::Get(int i)
        {
            if (i < 0 || i >= MaxCount) { return nullptr; }
            return Services[i];
        }

        Service* ServiceManager::Get(char* name)
        {
            if (name == nullptr) { return nullptr; }
            if (String::Length(name) == 0) { return nullptr; }
            for (size_t i = 0; i < MaxCount; i++)
            {
                if (Services[i] == nullptr) { continue; }
                if (String::Equals(Services[i]->GetName(), name)) 
                { 
                    return Services[i];
                }
            }
            return nullptr;
        }
    }
}