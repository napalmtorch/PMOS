rm -r "FSManager/bin"
mkdir "FSManager/bin"

c++ -IFSManager/Include -c "FSManager/Source/Main.cpp" -o "FSManager/bin/Main.o"
c++ -IFSManager/Include -c "FSManager/Source/StringUtil.cpp" -o "FSManager/bin/StringUtil.o"
c++ -IFSManager/Include -c "FSManager/Source/Console.cpp" -o "FSManager/bin/Console.o"
c++ -IFSManager/Include -c "FSManager/Source/Debugger.cpp" -o "FSManager/bin/Debugger.o"
c++ -IFSManager/Include -c "FSManager/Source/ATA.cpp" -o "FSManager/bin/ATA.o"
c++ -IFSManager/Include -c "FSManager/Source/FileSystem.cpp" -o "FSManager/bin/FileSystem.o"
c++ -IFSManager/Include -c "FSManager/Source/Shell.cpp" -o "FSManager/bin/Shell.o"
c++ -o "./FSManager/bin/FSManager.o" "FSManager/bin/Main.o" "FSManager/bin/StringUtil.o" "FSManager/bin/Console.o" "FSManager/bin/Debugger.o" "FSManager/bin/ATA.o" "FSManager/bin/FileSystem.o" "FSManager/bin/Shell.o"
./FSManager/bin/FSManager.o