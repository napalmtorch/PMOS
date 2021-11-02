#include <iostream>
#include "Console.hpp"
#include "Debugger.hpp"
#include "Shell.hpp"

int main()
{
    Console::Initialize();
    
    // print startup message
    Console::WriteLine("PurpleMoon File System Manager", ConsoleColor::Cyan);
    Console::WriteLine("Version 2.0", ConsoleColor::Gray);

    // initialize debugger
    Debugger::Enable();

    // initialize shell host
    Shell::Initialize();

    // main loop
    while (true)
    {
        // get input string
        std::string line;
        std::getline(std::cin, line);

        // parse shell command
        Shell::ParseCommand(line);
    }
    return 0;
}