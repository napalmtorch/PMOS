#pragma once
#include <Kernel/Lib/Types.hpp>
#include <Kernel/Lib/Stream.hpp>
#include <Kernel/Core/Service.hpp>
#include <Kernel/Core/Debug.hpp>

namespace PMOS
{
    struct Command
    {
        public:
            char* Name;
            char* Help;
            char* Usage;
            void (*Execute)(char*, Array<char**>);

        public:
            Command() { Name = "\0"; Help = "\0"; Usage = "\0"; Execute = nullptr; }
            Command(char* name, char* help, char* usage, void (*exec)(char* input, Array<char**> args))
            {
                Name = name;
                Help = help;
                Usage = usage;
                Execute = exec;
            }
    };

    namespace Services
    {
        class CommandLine : public Service
        {
            public:
                Command** Commands;
                size_t Count;
                char* CurrentPath;
                bool Terminated;

            public:
                char** CommandBuffer;
                char** CommandArgs;
                int BufferPos;
                uint CommandArgsCount;
                Debugger Debug;

            public:
                static const int MaxCommandCount = 256;
                static const int MaxBufferCount = 256;

            public:
                CommandLine();
                void Initialize() override;
                void Start() override;
                void Stop() override;

            public:
                void PrintCaret();
                static void OnEnterPressed(Stream* input);
                void RegisterCommand(Command cmd);
                void PushCommand(char* input);
                void PopCommand();
                void Execute();
                void FreeCommands();
                char* PathToCLIPath(char* path);

            private:
                int GetFreeIndex();
                int GetFreeBufferIndex();
        };
    }

    namespace CommandMethods
    {
        void CLS(char* input, Array<char**> args);
        void HELP(char* input, Array<char**> args);
        void ECHO(char* input, Array<char**> args);
        void HEAP(char* input, Array<char**> args);
        void SERVICES(char* input, Array<char**> args);
        void THREADS(char* input, Array<char**> args);
        void MMAP(char* input, Array<char**> args);
        void ENDLESS(char* input, Array<char**> args);
        void TIME(char* input, Array<char**> args);
        void INFO(char* input, Array<char**> args);
        void SYSINFO(char* input, Array<char**> args);
        void MEM(char* input, Array<char**> args);
        void PERF(char* input, Array<char**> args);
        void LSPCI(char* input, Array<char**> args);
        void CD(char* input, Array<char**> args);
        void DIR(char* input, Array<char**> args);
        void XSERVER(char* input, Array<char**> args);
        void VESAMODES(char* input, Array<char**> args);
        void RUN(char* input, Array<char**> args);
    }
}