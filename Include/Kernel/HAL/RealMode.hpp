#pragma once
#include <Kernel/Lib/Types.hpp>

extc
{
	typedef struct
	{
		ushort DI, SI, BP, SP, BX, DX, CX, AX;
		ushort GS, FS, ES, DS, EFlags;
	} ATTR_PACK Registers16;

	void CallInterrupt16(byte interrupt, Registers16* regs);
}