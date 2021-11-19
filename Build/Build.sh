# Clear the console
clear

# Setup build directory
rm -r 'Build/Output'
mkdir 'Build/Output'
mkdir 'Build/Output/Objs/'

# NASM assembler files
nasm -felf32 'Source/Boot/Bootstrap.asm' -o 'Build/Output/Bootstrap.o'
nasm -felf32 'Source/Boot/GDT.asm' -o 'Build/Output/Objs/GDT.o'
nasm -felf32 'Source/Kernel/HAL/Interrupts/IRQs.asm' -o 'Build/Output/Objs/IRQs.o'
nasm -felf32 'Source/Kernel/HAL/RealMode.asm' -o 'Build/Output/Objs/RealMode.o'

# Entry C++ file
i686-elf-g++ -w -IInclude -c "Source/Boot/Entry.cpp" -o "Build/Output/Objs/Entry.o" -fno-use-cxa-atexit -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -Wno-write-strings -Wno-unused-variable

# Kernel/Core/
for file in Source/Kernel/Core/*.cpp 
do
infile=$(basename $file)
outfile="$(echo $infile | sed 's/cpp/o/g')"
i686-elf-g++ -w -IInclude -c "Source/Kernel/Core/$infile" -o "Build/Output/Objs/$outfile" -fno-use-cxa-atexit -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -Wno-write-strings -Wno-unused-variable
done

# Kernel/Programs/
for file in Source/Kernel/Programs/*.cpp 
do
infile=$(basename $file)
outfile="$(echo $infile | sed 's/cpp/o/g')"
i686-elf-g++ -w -IInclude -c "Source/Kernel/Programs/$infile" -o "Build/Output/Objs/$outfile" -fno-use-cxa-atexit -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -Wno-write-strings -Wno-unused-variable
done

# Kernel/Graphics/
for file in Source/Kernel/Graphics/*.cpp 
do
infile=$(basename $file)
outfile="$(echo $infile | sed 's/cpp/o/g')"
i686-elf-g++ -w -IInclude -c "Source/Kernel/Graphics/$infile" -o "Build/Output/Objs/$outfile" -fno-use-cxa-atexit -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -Wno-write-strings -Wno-unused-variable
done

# Kernel/HAL/
for file in Source/Kernel/HAL/*.cpp 
do
infile=$(basename $file)
outfile="$(echo $infile | sed 's/cpp/o/g')"
i686-elf-g++ -w -IInclude -c "Source/Kernel/HAL/$infile" -o "Build/Output/Objs/$outfile" -fno-use-cxa-atexit -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -Wno-write-strings -Wno-unused-variable
done

# Kernel/HAL/Interrupts/
for file in Source/Kernel/HAL/Interrupts/*.cpp 
do
infile=$(basename $file)
outfile="$(echo $infile | sed 's/cpp/o/g')"
i686-elf-g++ -w -IInclude -c "Source/Kernel/HAL/Interrupts/$infile" -o "Build/Output/Objs/$outfile" -fno-use-cxa-atexit -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -Wno-write-strings -Wno-unused-variable
done

# Kernel/HAL/Drivers/Video/
for file in Source/Kernel/HAL/Drivers/Video/*.cpp 
do
infile=$(basename $file)
outfile="$(echo $infile | sed 's/cpp/o/g')"
i686-elf-g++ -w -IInclude -c "Source/Kernel/HAL/Drivers/Video/$infile" -o "Build/Output/Objs/$outfile" -fno-use-cxa-atexit -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -Wno-write-strings -Wno-unused-variable
done

# Kernel/HAL/Drivers/Input/
for file in Source/Kernel/HAL/Drivers/Input/*.cpp 
do
infile=$(basename $file)
outfile="$(echo $infile | sed 's/cpp/o/g')"
i686-elf-g++ -w -IInclude -c "Source/Kernel/HAL/Drivers/Input/$infile" -o "Build/Output/Objs/$outfile" -fno-use-cxa-atexit -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -Wno-write-strings -Wno-unused-variable
done

# Kernel/HAL/Drivers/Storage/
for file in Source/Kernel/HAL/Drivers/Storage/*.cpp 
do
infile=$(basename $file)
outfile="$(echo $infile | sed 's/cpp/o/g')"
i686-elf-g++ -w -IInclude -c "Source/Kernel/HAL/Drivers/Storage/$infile" -o "Build/Output/Objs/$outfile" -fno-use-cxa-atexit -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -Wno-write-strings -Wno-unused-variable
done

# Kernel/Services/
for file in Source/Kernel/Services/*.cpp 
do
infile=$(basename $file)
outfile="$(echo $infile | sed 's/cpp/o/g')"
i686-elf-g++ -w -IInclude -c "Source/Kernel/Services/$infile" -o "Build/Output/Objs/$outfile" -fno-use-cxa-atexit -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -Wno-write-strings -Wno-unused-variable
done

# Kernel/UI/
for file in Source/Kernel/UI/*.cpp
do
infile=$(basename $file)
outfile="$(echo $infile | sed 's/cpp/o/g')"
i686-elf-g++ -w -IInclude -c "Source/Kernel/UI/$infile" -o "Build/Output/Objs/$outfile" -fno-use-cxa-atexit -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -Wno-write-strings -Wno-unused-variable
done

# Kernel/UI/XServer
for file in Source/Kernel/UI/XServer/*.cpp 
do
infile=$(basename $file)
outfile="$(echo $infile | sed 's/cpp/o/g')"
i686-elf-g++ -w -IInclude -c "Source/Kernel/UI/XServer/$infile" -o "Build/Output/Objs/$outfile" -fno-use-cxa-atexit -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -Wno-write-strings -Wno-unused-variable
done

# Kernel/VM/
for file in Source/Kernel/VM/*.cpp 
do
infile=$(basename $file)
outfile="$(echo $infile | sed 's/cpp/o/g')"
i686-elf-g++ -w -IInclude -c "Source/Kernel/VM/$infile" -o "Build/Output/Objs/$outfile" -fno-use-cxa-atexit -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -Wno-write-strings -Wno-unused-variable
done

# Kernel/Lib/
for file in Source/Kernel/Lib/*.cpp 
do
infile=$(basename $file)
outfile="$(echo $infile | sed 's/cpp/o/g')"
i686-elf-g++ -w -IInclude -c "Source/Kernel/Lib/$infile" -o "Build/Output/Objs/$outfile" -fno-use-cxa-atexit -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti -Wno-write-strings -Wno-unused-variable
done

# Link all files
cd 'Build/Output/Objs'
i686-elf-gcc -T '../../Linker.ld' -o '../Kernel.bin' -ffreestanding -O2 -nostdlib *.o '../Bootstrap.o' -lgcc
cd '../../../'

# Create ISO image
mkdir -p 'Build/Output/isodir/boot/grub'
cp 'Build/Output/Kernel.bin' 'Build/Output/isodir/boot/kernel.bin'
cp 'Build/GrubMenu.cfg' 'Build/Output/isodir/boot/grub/grub.cfg'
grub-mkrescue -o  'PMOS.iso' 'Build/Output/isodir'
cp 'PMOS.iso' 'PMOS.img'

# Run QEMU instance of operating system
qemu-system-i386 -m 256M -vga std -hda 'Disk.img' -cdrom 'PMOS.iso' -serial stdio -boot d -soundhw ac97 -enable-kvm -rtc base=localtime -cpu host