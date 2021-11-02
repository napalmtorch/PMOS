#include "Shell.hpp"
#include "StringUtil.hpp"
#include "Console.hpp"
#include "Debugger.hpp"
#include "ATA.hpp"
#include "FileSystem.hpp"

namespace Shell
{
	// list of commands
	std::vector<ShellCommand> CommandList;
	std::string CurrentPath;

	// initialize shell host
	void Initialize()
	{
		// register default commands
        RegisterCommand("EXIT", "Exit the application", "exit", Commands::EXIT);
		RegisterCommand("CLS", "Clear the screen", "cls", Commands::CLS);
		RegisterCommand("INFO", "Show disk image information", "info", Commands::INFO);
		RegisterCommand("SCRIPT", "Run an executable script", "script [path]", Commands::SCRIPT);
		RegisterCommand("HELP", "Show list of commands", "help [-u : show usage]", Commands::HELP);
		RegisterCommand("MOUNT", "Mount current disk image", "mount", Commands::MOUNT);
		RegisterCommand("FORMAT", "Format current disk image", "format", Commands::FORMAT);
		RegisterCommand("CREATE", "Create a new disk image at specified path", "create [bytes] [path]", Commands::CREATE);
		RegisterCommand("LOAD", "Load disk image file from spcified path", "load [path]", Commands::LOAD);
		RegisterCommand("UNLOAD", "Unload current disk image file", "unload", Commands::UNLOAD);
		RegisterCommand("SAVE", "Save disk image file to specified path", "save [path]", Commands::SAVE);
		RegisterCommand("DUMP", "Dump disk image data with at position", "dump [sector] [bytes]", Commands::DUMP);
		RegisterCommand("LBE", "List all data block entries", "lbe", Commands::LBE);
		RegisterCommand("LFE", "List all file and directory entries", "lfe", Commands::LFE);
		RegisterCommand("CD", "Set the current directory", "cd [path]", Commands::CD);
		RegisterCommand("DIR", "List contents of directory", "dir [style(-s/-e)] [path]", Commands::DIR);
		RegisterCommand("MKDIR", "Create a new directory", "mkdir [path]", Commands::MKDIR);
		RegisterCommand("ADD", "Add new file from host machine", "add [image_path] [host_path]", Commands::ADD);
		RegisterCommand("REMOVE", "Remove existing file from disk image", "remove [path]", Commands::REMOVE);
		RegisterCommand("COPY", "Copy file or directory from source to destination", "copy [dest_path] [src_path]", Commands::COPY);
		RegisterCommand("MOVE", "Move file or directory from source to destination", "move [dest_path] [src_path]", Commands::MOVE);

		// set current path as root by default
		CurrentPath = "/";

		// print input caret
		PrintCaret();
	}

	// register shell command 
	bool RegisterCommand(const char* name, const char* help, const char* usage, void (*exec)(std::string))
	{
		ShellCommand cmd;
		cmd.Name = (char*)name;
		cmd.Help = (char*)help;
		cmd.Usage = (char*)usage;
		cmd.Execute = exec;
		CommandList.push_back(cmd);
		return false;
	}

	// print current path caret
	void PrintCaret()
	{
		Console::Write(CurrentPath, ConsoleColor::Cyan);
		Console::Write(":- ");
	}

	// parse and execute shell commmand
	void ParseCommand(std::string input)
	{
		// skip if no text is entered
		if (input.length() == 0) { return;  }

		// get command and convert to upper case
		std::string cmd = input.substr(0, input.find(' '));
		std::for_each(cmd.begin(), cmd.end(), [](char& c) { c = ::toupper(c); });

		if (StringUtil::Equals(cmd, "CLEAR")) { ParseCommand("CLS"); return; }

		// loop through commands
		for (size_t i = 0; i < CommandList.size(); i++)
		{
			// found match
			if (!strcmp(CommandList.at(i).Name, cmd.c_str()) && CommandList.at(i).Execute != nullptr)
			{
				// execute command and return
				CommandList.at(i).Execute(input);
				PrintCaret();
				return;
			}
		}

		// invalid command was entered
		Debugger::ThrowError("Invalid command");
		PrintCaret();
	}
}

namespace Commands
{
    // exit the application
    void EXIT(std::string input)
    {
        exit(1);
    }

	// clear the screen
	void CLS(std::string input)
	{
		Console::Clear();
	}

	void INFO(std::string input)
	{
		//FileSystem::InfoBlockHeader info = FileSystem::GetInfoBlock();

		Console::Write("\n");
		Console::WriteLine("PurpleMoon File System Manager", ConsoleColor::Cyan);
		Console::WriteLine("Version 2.0", ConsoleColor::Gray);
		Console::Write("\n");
		FileSystem::MasterFS.PrintDiskInformation();
	}

	void SCRIPT(std::string input)
	{
		// get path
		std::string path = input.substr(7, input.length() - 7);
		if (path.length() == 0) { Debugger::ThrowError("Invalid path"); return; }

		// load script
		std::ifstream file;
		file.open(path);
		std::vector<std::string> lines;
		if (file.good())
		{
			while (!file.eof())
			{
				std::string line;
				getline(file, line);
				lines.push_back(line);
			}
		}
		else { Debugger::ThrowError("Unable to locate file " + path); return; }
		file.close();

		Debugger::ThrowOK("Loaded script");
		for (size_t i = 0; i < lines.size(); i++)
		{
			Shell::ParseCommand(lines.at(i));
		}
		Debugger::ThrowOK("Finished executing script");
	}

	void HELP(std::string input)
	{
		std::vector<std::string> args = StringUtil::Split(input, ' ');
		bool usage = false;
		if (args.size() >= 2) { if (StringUtil::Equals(args.at(1), "-u")) { usage = true; } }

		Console::Write("-- ", ConsoleColor::DarkGray);
		Console::Write(" COMMAND LIST ", ConsoleColor::Green);
		Console::WriteLine("----------------------------------", ConsoleColor::DarkGray);
		for (size_t i = 0; i < Shell::CommandList.size(); i++)
		{
			if (Shell::CommandList.at(i).Execute != nullptr)
			{
				// show usage
				if (usage)
				{
					Console::Write(Shell::CommandList.at(i).Name);
					Console::Write("    ");
					Console::WriteLine(Shell::CommandList.at(i).Usage);
				}
				// show description
				else
				{
					Console::Write(Shell::CommandList.at(i).Name);
					Console::Write("    ");
					Console::WriteLine(Shell::CommandList.at(i).Help);
				}
			}
		}
	}

	void MOUNT(std::string input)
	{
		FileSystem::MasterFS.Mount();
	}

	void FORMAT(std::string input)
	{
		uint32_t size = ATADisk::GetSize();
		if (size == 0) { Debugger::ThrowError("No disk image loaded"); return; }

		// format disk with specified size
		FileSystem::MasterFS.Format(size);
	}

	void DUMP(std::string input)
	{
		std::vector<std::string> args = StringUtil::Split(input, ' ');
		if (args.size() == 3)
		{
			// get start sector
			std::string startstr = args.at(1);
			size_t start = (size_t)StringUtil::ToDecimal(startstr);

			// get bytes to dump
			std::string sizestr = args.at(2);
			size_t size = (size_t)StringUtil::ToDecimal(sizestr);

			// validate size
			if (size == 0) { Debugger::ThrowError("Invalid size"); return; }
			if (size > 512) { size = 512; }

			// read sector from disk and attempt to print
			uint8_t* src = ATADisk::Read(start);
			if (src != nullptr) { Debugger::DumpMemory(src, size); }
			else { Debugger::ThrowError("No disk image loaded"); }
			free(src);
		}
		// invalid arguments
		else { Debugger::ThrowError("Invalid arguments. Sector and offset required."); }
	}

	void CREATE(std::string input)
	{
		// get file size
		std::vector<std::string> args = StringUtil::Split(input, ' ');
		if (args.size() >= 2)
		{
			std::string sizestr = args.at(1);
			int size = StringUtil::ToDecimal(sizestr);

			// get file name
			int name_offset = 8 + (int)sizestr.length();
			std::string filename = input.substr(name_offset, input.length() - name_offset);

			// started message
			std::string msg;
			msg.append("Attempting to create ");
			msg.append(StringUtil::FromDecimal(size / 1024 / 1024));
			msg.append("MB image file at ");
			Debugger::ThrowMessage(msg, filename, MsgType::System);

			// create file
			ATADisk::CreateImageFile(filename, size);
		}
		// invalid arguments
		else { Debugger::ThrowError("Invalid arguments"); return; }
	}

	void LOAD(std::string input)
	{
		// get file name
		std::string filename = input.substr(5, input.length() - 5);

		// message
		std::string msg;
		Debugger::ThrowMessage("Attempting to load image file at ", filename, MsgType::System);

		// load file
		if (ATADisk::LoadImageFile(filename)) { Debugger::ThrowOK("Done."); }
		else { Debugger::ThrowMessage("Unable to load image file at ", filename, MsgType::System); }
	}

	void UNLOAD(std::string input)
	{
		ATADisk::UnloadImageFile();
		FileSystem::MasterFS.Unmount();
	}

	void SAVE(std::string input)
	{
		// message
		std::string msg;
		Debugger::ThrowMessage("Attempting to save image file at ", ATADisk::CurrentFile, MsgType::System);
		FileSystem::MasterFS.WriteTables();

		// save file
		if (!ATADisk::SaveImageFile(ATADisk::CurrentFile)) { Debugger::ThrowError("Unable to save disk image"); }
	}

	void LBE(std::string input)
	{
		if (ATADisk::GetSize() == 0) { Debugger::ThrowError("No disk image loaded"); return; }
		if (!FileSystem::MasterFS.IsMounted()) { Debugger::ThrowError("File system is not mounted"); return; }
		FileSystem::MasterFS.PrintBlockTable();
	}

	void LFE(std::string input)
	{
		if (ATADisk::GetSize() == 0) { Debugger::ThrowError("No disk image loaded"); return; }
		if (!FileSystem::MasterFS.IsMounted()) { Debugger::ThrowError("File system is not mounted"); return; }
		FileSystem::MasterFS.PrintEntryTable();
	}

	void CD(std::string input)
	{
		if (ATADisk::GetSize() == 0) { Debugger::ThrowError("No disk image loaded"); return; }
		if (!FileSystem::MasterFS.IsMounted()) { Debugger::ThrowError("File system is not mounted"); return; }

		// get path from input
		if (input.length() <= 3) { return; }
		std::string path = input.substr(3, input.length() - 3);
		if (path.length() == 0) { return; }

		// full path
		if (path[0] == '/')
		{
			if (path[path.length() - 1] == '/' && path.length() > 1) { path = path.substr(0, path.length() - 1); }
			if (!FileSystem::MasterFS.IODirectoryExists(path)) { Debugger::ThrowError("Unable to locate directory " + path); return; }
			Shell::CurrentPath = path;
		}
		// relative path
		else
		{
			std::string full_path = Shell::CurrentPath;
			if (StringUtil::Equals(Shell::CurrentPath, "/")) { full_path += path; }
			else { full_path += "/" + path; }
			if (full_path[full_path.length() - 1] == '/') { full_path = full_path.substr(0, full_path.length() - 1); }
			if (!FileSystem::MasterFS.IODirectoryExists(full_path)) { Debugger::ThrowError("Unable to locate directory " + full_path); return; }
			Shell::CurrentPath = full_path;
		}
	}

	void DIR(std::string input)
	{
		if (ATADisk::GetSize() == 0) { Debugger::ThrowError("No disk image loaded"); return; }
		if (!FileSystem::MasterFS.IsMounted()) { Debugger::ThrowError("File system is not mounted"); return; }

		// get path from input
		if (input.length() <= 4) { FileSystem::MasterFS.PrintDirectoryContents(Shell::CurrentPath); return; }
		std::string path = input.substr(4, input.length() - 4);
		if (path.length() == 0) {  return; }

		if (path[0] == '/')
		{
			if (path[path.length() - 1] == '/' && path.length() > 1) { path = path.substr(0, path.length() - 1); }
			if (!FileSystem::MasterFS.IODirectoryExists(path)) { Debugger::ThrowError("Unable to locate directory " + path); return; }			
			FileSystem::MasterFS.PrintDirectoryContents(path);
		}
		// relative path
		else
		{
			std::string full_path = Shell::CurrentPath;
			if (StringUtil::Equals(Shell::CurrentPath, "/")) { full_path += path; }
			else { full_path += "/" + path; }
			if (full_path[full_path.length() - 1] == '/') { full_path = full_path.substr(0, full_path.length() - 1); }
			if (!FileSystem::MasterFS.IODirectoryExists(full_path)) { Debugger::ThrowError("Unable to locate directory " + full_path); return; }
			FileSystem::MasterFS.PrintDirectoryContents(full_path);
		}
	}

	void MKDIR(std::string input)
	{
		if (ATADisk::GetSize() == 0) { Debugger::ThrowError("No disk image loaded"); return; }
		if (!FileSystem::MasterFS.IsMounted()) { Debugger::ThrowError("File system is not mounted"); return; }

		// make directory in current path
		if (input.length() <= 6) { Debugger::ThrowError("Directory name required"); return ; }

		std::string path = input.substr(6, input.length() - 6);
		if (path.length() == 0) { Debugger::ThrowError("Directory name required"); return; }
		
		// full path
		if (path[0] == '/')
		{
			if (path[path.length() - 1] == '/') { path = path.substr(0, path.length() - 1); }
			if (FileSystem::MasterFS.IODirectoryExists(path)) { Debugger::ThrowError("Directory already exists"); return; }
			FileSystem::DirectoryEntry dir = FileSystem::MasterFS.IOCreateDirectory(path);
			if (dir.Type == FileSystem::EntryType::Null || strlen(dir.Name) == 0) { Debugger::ThrowError("Invalid directory"); return; }
			Debugger::ThrowOK("Successfully created directory " + path);
		}
		// relative path
		else
		{
			std::string full_path = Shell::CurrentPath;
			if (StringUtil::Equals(Shell::CurrentPath, "/")) { full_path += path; }
			else { full_path += "/" + path; }
			if (full_path[full_path.length() - 1] == '/') { full_path = full_path.substr(0, full_path.length() - 1); }
			FileSystem::DirectoryEntry dir = FileSystem::MasterFS.IOCreateDirectory(full_path);
			if (dir.Type == FileSystem::EntryType::Null || strlen(dir.Name) == 0) { Debugger::ThrowError("Invalid directory"); return; }
			Debugger::ThrowOK("Successfully created directory " + path);
		}
	}

	void ADD(std::string input)
	{
		if (ATADisk::GetSize() == 0) { Debugger::ThrowError("No disk image loaded"); return; }
		if (!FileSystem::MasterFS.IsMounted()) { Debugger::ThrowError("File system is not mounted"); return; }

		std::vector<std::string> args = StringUtil::Split(input, ' ');
		if (args.size() == 0) { Debugger::ThrowError("Destination and source file required"); return; }

		if (args.size() == 3)
		{
			std::string dest = args.at(1);
			std::string src = args.at(2);

			if (FileSystem::MasterFS.IOFileExists(dest)) { Debugger::ThrowError("Destination file already exists"); return; }
			FileSystem::FileEntry file = FileSystem::MasterFS.IOImportFile(dest, src);
			if (file.Type != FileSystem::EntryType::File || strlen(file.Name) == 0) { Debugger::ThrowError("Unable to add file"); return; }
			Debugger::ThrowOK("Successfully added file " + dest);
		}
	}

	void REMOVE(std::string input)
	{
		if (ATADisk::GetSize() == 0) { Debugger::ThrowError("No disk image loaded"); return; }
		if (!FileSystem::MasterFS.IsMounted()) { Debugger::ThrowError("File system is not mounted"); return; }

		if (input.length() <= 7) { Debugger::ThrowError("File or directory name required"); return; }
		std::string path = input.substr(7, input.length() - 7);
		if (path.length() == 0) { Debugger::ThrowError("Invalid file or directory name"); return; }
		if (path[path.length() - 1] == '/') { path = path.substr(0, path.length() - 1); }
		if (path[0] != '/')
		{
			if (StringUtil::Equals(Shell::CurrentPath, "/")) { path = Shell::CurrentPath + path; }
			else { path = Shell::CurrentPath + "/" + path; }
		}
		Console::WriteLine(path);

		// check if file
		bool file = false;
		bool found = false;
		if (FileSystem::MasterFS.IODirectoryExists(path)) { file = false; found = true; }
		if (!found && FileSystem::MasterFS.IOFileExists(path)) { file = true; found = true; }
		if (!found) { Debugger::ThrowError("Unable to locate file for deletion"); return; }

		if (file) 
		{ 
			if (!FileSystem::MasterFS.IODeleteFile(path)) { Debugger::ThrowError("Unable to delete file " + path); return; }
			Debugger::ThrowOK("Successully deleted file " + path);
			return;
		}
		else
		{
			if (!FileSystem::MasterFS.IODeleteDirectory(path)) { Debugger::ThrowError("Unable to delete directory " + path); return; }
			Debugger::ThrowOK("Successfully delete directory " + path);
			return;
		}
	}

	void COPY(std::string input)
	{
		if (ATADisk::GetSize() == 0) { Debugger::ThrowError("No disk image loaded"); return; }
		if (!FileSystem::MasterFS.IsMounted()) { Debugger::ThrowError("File system is not mounted"); return; }
	}

	void MOVE(std::string input)
	{
		if (ATADisk::GetSize() == 0) { Debugger::ThrowError("No disk image loaded"); return; }
		if (!FileSystem::MasterFS.IsMounted()) { Debugger::ThrowError("File system is not mounted"); return; }
	}
}