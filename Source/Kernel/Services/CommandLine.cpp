#include <Kernel/Services/CommandLine.hpp>
#include <Kernel/Core/Kernel.hpp>

namespace PMOS
{
    namespace Services
    {
        CommandLine::CommandLine() : Service("clihost", ServiceType::Utility)
        {

        }

        void CommandLine::Initialize()
        {
            Service::Initialize();

            Debug.SetMode(DebugMode::Terminal);
            Kernel::ServiceMgr.Register(this);
            Kernel::ServiceMgr.Start(this);
        }

        void CommandLine::Start()
        {
            Service::Start();
            Terminated = false;


            Commands = (Command**)Kernel::MemoryMgr.Allocate(MaxCommandCount * sizeof(Command*), true, AllocationType::System);
            Count = 0;

            CommandBuffer = (char**)Kernel::MemoryMgr.Allocate(MaxBufferCount * sizeof(char*), true, AllocationType::System);
            BufferPos = 0;

            CurrentPath = (char*)Kernel::MemoryMgr.Allocate(512, true, AllocationType::String);
            String::Append(CurrentPath, "/");

            RegisterCommand(Command("CLS", "Clear the screen", "cls", CommandMethods::CLS));
            RegisterCommand(Command("HELP", "Show list of commands", "help", CommandMethods::HELP));
            RegisterCommand(Command("ECHO", "Print a string of text", "echo [text] ", CommandMethods::ECHO));
            RegisterCommand(Command("HEAP", "Show list of heap allocations", "heap", CommandMethods::HEAP));
            RegisterCommand(Command("MMAP", "Show multiboot memory map entries", "mmap", CommandMethods::MMAP));
            RegisterCommand(Command("SERVICES", "Show list of registered services", "services", CommandMethods::SERVICES));
            RegisterCommand(Command("ENDLESS", "Increment a number forever to test performance", "endless", CommandMethods::ENDLESS));
            RegisterCommand(Command("THREADS", "Show list of running threads", "threads", CommandMethods::THREADS));
            RegisterCommand(Command("TIME", "Get current date and time information", "time", CommandMethods::TIME));
            RegisterCommand(Command("INFO", "Show operating system information", "info", CommandMethods::INFO));
            RegisterCommand(Command("SYSINFO", "Show hardware information", "sysinfo", CommandMethods::SYSINFO));
            RegisterCommand(Command("MEM", "Show memory information", "mem", CommandMethods::MEM));
            RegisterCommand(Command("PERF", "Show performance information", "perf", CommandMethods::PERF));
            RegisterCommand(Command("LSPCI", "Show list of detected PCI devices", "lspci", CommandMethods::LSPCI));
            RegisterCommand(Command("CD", "Set the current directory", "cd [path]", CommandMethods::CD));
            RegisterCommand(Command("DIR", "List contents of directory", "dir [path?] ", CommandMethods::DIR));
            RegisterCommand(Command("XSERVER", "Start graphical user interface", "xserver", CommandMethods::XSERVER));
            RegisterCommand(Command("VESAMODES", "Show list of supported VESA video modes", "veasmodes", CommandMethods::VESAMODES));
            RegisterCommand(Command("RUN", "Run an executable binary file", "run [file]", CommandMethods::RUN));
            RegisterCommand(Command("DUMP", "Dump memory at specified address", "dump [addr] [size]", CommandMethods::DUMP));
            RegisterCommand(Command("PANIC", "Force a kernel level exception", "panic", CommandMethods::PANIC));

            KBData = (byte*)Kernel::MemoryMgr.Allocate(512, true, AllocationType::String);
            KBStream = Stream(KBData, 512);
            Kernel::Keyboard->SetStream(&KBStream);
            Kernel::Keyboard->TerminalOutput = true;
            Kernel::Keyboard->OnEnterPressed = OnEnterPressed;

            Kernel::Debug.OK("Started command line interface");
        }

        void CommandLine::Stop()
        {
            Service::Stop();

            if (Commands != nullptr) 
            { 
                for (uint i = 0; i < MaxCommandCount; i++) { if (Commands[i] != nullptr) { Kernel::MemoryMgr.Free(Commands[i]); } }
                Kernel::MemoryMgr.Free(Commands); 
            }

            if (CommandBuffer != nullptr)
            {
                for (uint i = 0; i < MaxBufferCount; i++) { if (CommandBuffer[i] != nullptr) { Kernel::MemoryMgr.Free(CommandBuffer[i]); } }
            }

            if (CurrentPath != nullptr) { Kernel::MemoryMgr.Free(CurrentPath); }

            Kernel::MemoryMgr.Free(KBData);
        }

        void CommandLine::PrintCaret()
        {
            Kernel::Terminal->Write("root", Col4::Magenta);
            Kernel::Terminal->Write("@");
            Kernel::Terminal->Write(CurrentPath, Col4::Green);
            Kernel::Terminal->Write("> ");
        }

        void CommandLine::OnEnterPressed(Stream* input)
        {
            Kernel::CLI->PushCommand((char*)input->Data);
            Memory::Set(input->Data, 0, input->GetSize());
            input->Seek(0);
        }

        void CommandLine::RegisterCommand(Command cmd)
        {
            int i = GetFreeIndex();
            if (i < 0 || i >= MaxCommandCount) { Kernel::Debug.Error("Maximum amount of register commands has been reached"); return; }

            Command* new_cmd = (Command*)Kernel::MemoryMgr.Allocate(sizeof(Command), true, AllocationType::System);
            new_cmd->Name = cmd.Name;
            new_cmd->Help = cmd.Help;
            new_cmd->Usage = cmd.Usage;
            new_cmd->Execute = cmd.Execute;
            Commands[i] = new_cmd;
            Count++;
            Kernel::Debug.Info("Registered command '%s'", cmd.Name);
        }

        void CommandLine::PushCommand(char* input)
        {
            if (BufferPos < 0 || BufferPos >= MaxBufferCount) { Debug.Error("Command buffer overflow exception"); return; }
            int len = String::Length(input) + 1;
            char* cmd = (char*)Kernel::MemoryMgr.Allocate(len, true, AllocationType::String);
            Memory::Copy(cmd, input, len);
            CommandBuffer[BufferPos] = cmd;
            BufferPos++;
        }

        void CommandLine::PopCommand()
        {
            if (BufferPos < 0) { Debug.Error("Command buffer underflow exception"); return; }
            BufferPos--;
        }

        void CommandLine::FreeCommands()
        {
            for (size_t i = 0; i < MaxBufferCount; i++)
            {
                if (CommandBuffer[i] != nullptr) 
                { 
                    Kernel::MemoryMgr.Free(CommandBuffer[i]); 
                    CommandBuffer[i] = nullptr;
                }
            }
        }

        void CommandLine::Execute()
        {
            if (BufferPos < 0) { BufferPos = 0; return; }

            int pos = 0;
            int count = BufferPos;
            if (count == 0) { return; }
            while (pos < count)
            {
                char* input = CommandBuffer[pos];
                if (String::Length(input) == 0) { PopCommand(); pos++; continue; }

                if (CommandArgs != nullptr)
                {
                    for (size_t i = 0; i < CommandArgsCount; i++) { Kernel::MemoryMgr.Free(CommandArgs[i]); }
                    Kernel::MemoryMgr.Free(CommandArgs);
                }
                CommandArgs = String::Split(input, ' ', &CommandArgsCount);
                if (CommandArgsCount == 0) { PopCommand(); pos++; continue; }

                char* cmd = (char*)Kernel::MemoryMgr.Allocate(String::Length(CommandArgs[0]), true, AllocationType::String);
                String::Copy(cmd, CommandArgs[0]);
                String::ToUpper(cmd);

                bool exec = false;
                for (size_t i = 0; i < MaxCommandCount; i++)
                {
                    if (Commands[i]->Execute == nullptr) { continue; }
                    if (String::Equals(Commands[i]->Name, cmd))
                    {
                        Array<char**> arr = Array<char**>(CommandArgs, CommandArgsCount);
                        Commands[i]->Execute(input, arr);
                        Kernel::MemoryMgr.Free(cmd);
                        exec = true;
                        pos++;
                        PopCommand();
                        break;
                    }
                }
                if (exec) { continue; }

                pos++;
                PopCommand();
                Debug.Error("Invalid command");
                Kernel::MemoryMgr.Free(cmd);
            }

            FreeCommands();    

            if (Terminated) 
            { 
                Kernel::Debug.WriteLine("Entering XServer...");
                Kernel::ServiceMgr.Stop(Kernel::Terminal);
                return;
            }

            PrintCaret();
        }

        int CommandLine::GetFreeIndex()
        {
            for (size_t i = 0; i < MaxCommandCount; i++)
            {
                if (Commands[i] == nullptr) { return (int)i; }
            }
            return -1;
        }

        char* CommandLine::PathToCLIPath(char* path)
        {
            // path is a full path
            if (path[0] == '/')
            {
                char* output = (char*)Kernel::MemoryMgr.Allocate(String::Length(path), true, AllocationType::String);
                String::Copy(output, path);
                if (output[String::Length(output) - 1] == ' ') { String::Delete(output); }
                return output;
            }
            // path is a relative path
            else
            {
                char* full_path = (char*)Kernel::MemoryMgr.Allocate(2048, true, AllocationType::String);
                String::Append(full_path, CurrentPath);
                if (full_path[String::Length(full_path) - 1] != '/') { String::Append(full_path, '/'); }
                String::Append(full_path, path);
                String::Append(full_path, '\0');
                return full_path;
            }
        }
    }

    namespace CommandMethods
    {
        void CLS(char* input, Array<char**> args)
        {
            Kernel::Terminal->Clear();
        }

        void HELP(char* input, Array<char**> args)
        {
            for (size_t i = 0; i < Kernel::CLI->MaxCommandCount; i++)
            {
                if (Kernel::CLI->Commands[i] == nullptr) { continue; }
                Kernel::Terminal->Write(Kernel::CLI->Commands[i]->Name);
                Kernel::Terminal->SetCursorX(21);
                Kernel::Terminal->WriteLine(Kernel::CLI->Commands[i]->Help);
            }
            Kernel::Terminal->NewLine();
        }

        void ECHO(char* input, Array<char**> args)
        {
            if (String::Length(input) < 6) { Kernel::CLI->Debug.Error("Expected string value"); return; }
            char* msg = (char*)(input + 5);
            Kernel::Terminal->WriteLine(msg);
        }

        void HEAP(char* input, Array<char**> args)
        {
            Kernel::MemoryMgr.PrintTable(DebugMode::Terminal);
        }

        void SERVICES(char* input, Array<char**> args)
        {
            Kernel::ServiceMgr.Print(DebugMode::Terminal);
        }

        void THREADS(char* input, Array<char**> args)
        {
            Kernel::ThreadMgr.Print(DebugMode::Terminal);
        }

        void MMAP(char* input, Array<char**> args)
        {
            Kernel::MemoryMgr.PrintMemoryMap(DebugMode::Terminal);
        }

        void ENDLESS(char* input, Array<char**> args)
        {
            int i = 0;
            while (true)
            {
                Kernel::CLI->Debug.WriteLine("%d", i);
                i++;
            }
        }

        void TIME(char* input, Array<char**> args)
        {
            char* time = Kernel::RTC.GetTimeString(false, true);
            Kernel::Terminal->WriteLine(time);
        }

        void INFO(char* input, Array<char**> args)
        {
            Kernel::Terminal->Write("PurpleMoon", Col4::Magenta);
            Kernel::Terminal->WriteLine(" OS");
            Kernel::Terminal->WriteLine("Version 0.2", Col4::DarkGray); 
        }

        void SYSINFO(char* input, Array<char**> args)
        {
            Kernel::CLI->Debug.WriteLine("CPU            %s", Kernel::CPU.Name);
            Kernel::CLI->Debug.WriteLine("RAM            %d MB(%d MB usable)", Kernel::MemoryMgr.GetRAMInstalled() / 1024 / 1024, Kernel::MemoryMgr.GetRAMReserved() / 1024 / 1024);
            Kernel::CLI->Debug.WriteLine("VIDEO          VESA-compatible adapter");
        }

        void MEM(char* input, Array<char**> args)
        {
            uint installed = Kernel::MemoryMgr.GetRAMInstalled();
            uint usable = Kernel::MemoryMgr.GetRAMReserved();
            uint used = Kernel::MemoryMgr.GetRAMUsed();

            Kernel::CLI->Debug.Write("USED        %d MB(%f", used / 1024 / 1024, (double)(((float)used / (float)usable) * 100.0f));
            Kernel::CLI->Debug.WriteUnformatted("%)\n");
            Kernel::CLI->Debug.WriteLine("USABLE      %d MB", usable / 1024 / 1024);
            Kernel::CLI->Debug.WriteLine("TOTAL       %d MB", installed / 1024 / 1024);
        }

        void PERF(char* input, Array<char**> args)
        {
            Kernel::ThreadMgr.CalculateCPUUsage();
            float cpu = Kernel::ThreadMgr.GetCPUUsage();

            Kernel::CLI->Debug.Write("CPU USAGE:    %f", (double)cpu);
            Kernel::CLI->Debug.WriteUnformatted("%\n");
            Kernel::CLI->Debug.WriteLine("RAM USAGE:    %d MB(%d bytes)", Kernel::MemoryMgr.GetRAMUsed() / 1024 / 1024, Kernel::MemoryMgr.GetRAMUsed());
            Kernel::CLI->Debug.WriteLine("HEAP ENTRIES: %d", Kernel::MemoryMgr.GetHeapCount());
        }

        void LSPCI(char* input, Array<char**> args)
        {
            Kernel::PCI.List(DebugMode::Terminal);
        }

        void CD(char* input, Array<char**> args)
        {
            char* dirname = (char*)Kernel::MemoryMgr.Allocate(String::Length(input), true, AllocationType::String);
            dirname = String::Copy(dirname, (char*)(input + 3));
            if (dirname == nullptr) { return; }
            if (String::Length(dirname) == 0) { Kernel::MemoryMgr.Free(dirname); return; }
            if (dirname[String::Length(dirname) - 1] == 0x20) { String::Delete(dirname); }

            // input directory is root
            if (String::Equals(dirname, "/")) 
            { 
                Kernel::MemoryMgr.Free(Kernel::CLI->CurrentPath); 
                Kernel::CLI->CurrentPath = (char*)Kernel::MemoryMgr.Allocate(4, true, AllocationType::String);
                String::Append(Kernel::CLI->CurrentPath, "/\0");
                Kernel::MemoryMgr.Free(dirname);
            }
            // input directory is a full path
            else if (dirname[0] == '/')
            {
                if (!Kernel::FileSys->IODirectoryExists(dirname)) { Kernel::CLI->Debug.Error("Unable to locate directory %s\n", dirname); Kernel::MemoryMgr.Free(dirname); return; }
                Kernel::MemoryMgr.Free(Kernel::CLI->CurrentPath);
                Kernel::CLI->CurrentPath = dirname;
            }
            // input directory is a relative path
            else
            {
                char* full_path = (char*)Kernel::MemoryMgr.Allocate(String::Length(dirname) + String::Length(Kernel::CLI->CurrentPath) + 8, true, AllocationType::String);
                String::Append(full_path, Kernel::CLI->CurrentPath);
                if (full_path[String::Length(full_path) - 1] != '/') { String::Append(full_path, '/'); }
                String::Append(full_path, dirname);
                String::Append(full_path, '\0');
                if (!Kernel::FileSys->IODirectoryExists(full_path)) { Kernel::CLI->Debug.Error("Unable to locate directory %s\n", dirname); Kernel::MemoryMgr.Free(dirname); Kernel::MemoryMgr.Free(full_path); return; }
                Kernel::MemoryMgr.Free(Kernel::CLI->CurrentPath);
                Kernel::CLI->CurrentPath = full_path;
                Kernel::MemoryMgr.Free(dirname);
            }
        }

        void DIR(char* input, Array<char**> args)
        {
            // no path provided
            if (String::Length(input) < 5) { Kernel::FileSys->PrintDirectoryContents(Kernel::CLI->CurrentPath); return; }

            char* dirname = (char*)Kernel::MemoryMgr.Allocate(String::Length(input), true, AllocationType::String);
            dirname = String::Copy(dirname, (char*)(input + 4));
            if (dirname[String::Length(dirname) - 1] == 0x20) { String::Delete(dirname); }

            if (dirname != nullptr)
            {
                if (String::Length(dirname) > 0)
                {
                    char* path = Kernel::CLI->PathToCLIPath(dirname);
                    if (!Kernel::FileSys->IODirectoryExists(path)) { Kernel::CLI->Debug.Error("Unable to locate directory %s", dirname); }
                    else { Kernel::FileSys->PrintDirectoryContents(path); }
                    Kernel::MemoryMgr.Free(path);
                }

                Kernel::MemoryMgr.Free(dirname);
            }
        }

        void XSERVER(char* input, Array<char**> args)
        {
            if (Kernel::XServer != nullptr) { Kernel::MemoryMgr.Free(Kernel::XServer); }
            if (Kernel::WinMgr != nullptr) { Kernel::MemoryMgr.Free(Kernel::WinMgr); }

            Kernel::WinMgr = new UI::WindowManager();
            Kernel::WinMgr->Initialize();

            Kernel::XServer = new UI::XServer::XServerHost();
            Kernel::XServer->Initialize();
        }

        char* GetDepthString(ColDepth depth)
        {
            switch (depth)
            {
                case ColDepth::Depth4Bit: { return "4 "; }
                case ColDepth::Depth8Bit: { return "8 "; }
                case ColDepth::Depth16Bit: { return "16"; }
                case ColDepth::Depth24Bit: { return "24"; }
                case ColDepth::Depth32Bit: { return "32"; }
                default: { return "0"; }
            }
        }

        void VESAMODES(char* input, Array<char**> args)
        {
            Kernel::CLI->Debug.WriteUnformatted("-------- ", Col4::DarkGray);
            Kernel::CLI->Debug.WriteUnformatted("VESA MODES", Col4::Green);
            Kernel::CLI->Debug.WriteUnformatted(" --------------------------------");
            Kernel::CLI->Debug.NewLine();
            Kernel::CLI->Debug.WriteUnformatted("ID        DEPTH    RESOLUTION\n", Col4::DarkGray);

            for (int i = 0; i < 256; i++)
            {
                if (Kernel::VESA->AvailableModes[i].Width > 0)
                {
                    Kernel::CLI->Debug.Write("0x%4x    ", i);
                    Kernel::CLI->Debug.Write("%s       ", GetDepthString((ColDepth)Kernel::VESA->AvailableModes[i].Depth));
                    Kernel::CLI->Debug.Write("%dx%d", Kernel::VESA->AvailableModes[i].Width, Kernel::VESA->AvailableModes[i].Height);
                    Kernel::CLI->Debug.NewLine();
                }
            }

            Kernel::CLI->Debug.NewLine();
        }

        void RUN(char* input, Array<char**> args)
        {
            VirtualMachine::ExecutableHeader header;
            VirtualMachine::Executable exec;
            
            byte prog[] = 
            {
                0x01, 0x00, 0xDE, 0xAD, 0xCA, 0xFE,
                0x00,
                0xFF,
            };


            header.CodeSize = 8;
            header.DataSize = 0;
            header.StackSize = 0x1000;
            header.RAMSize   = 0x10000;
            header.CodeAddress = 0;
            header.DataAddress = 0;
            header.StackAddress = header.RAMSize - header.StackSize;
            header.Type = (byte)VirtualMachine::ExecutableType::TerminalExecutable;

            if (!exec.Create(header, prog)) { Kernel::CLI->Debug.Error("Unable to run executable"); return; }
            
            Kernel::CLI->Debug.WriteLine("RAM(addr = 0x00000000, size = %d bytes), STACK(addr = 0x%8x, size = %d bytes)", exec.GetRAMSize(), exec.GetStackAddress(), exec.GetStackSize());
        }

        void DUMP(char* input, Array<char**> args)
        {
            if (args.Count < 2) { Kernel::CLI->Debug.Error("Expected address"); return; }
            if (args.Count < 3) { Kernel::CLI->Debug.Error("Expected size"); return; }

            char* addr_str = args.Data[1];
            char* size_str = args.Data[2];

            uint addr = String::ToHex(addr_str);
            uint size = String::ToDecimal(size_str);
            
            Kernel::CLI->Debug.DumpMemory((void*)addr, size);
        }

        void PANIC(char* input, Array<char**> args)
        {
            asm volatile("int $80");
        }
    }
}