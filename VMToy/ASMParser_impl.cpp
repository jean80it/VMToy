#include "AsmParser_impl.hpp"

#pragma region opNameCodeMap init
const std::map<std::string, ushort> ASMParser_Impl::opNameCodeMap =
{
	// MEMORY OPS / INIT - 0x00
	{ "NOP",	0x00 },
	{ "ERR",	0x01 },
	{ "INIT",	0x02 },
	{ "LOAD",	0x03 },
	{ "LOADS",	0x04 },
	{ "LOADB",	0x05 },
	{ "STOR",	0x06 },
	{ "STORS",	0x07 },
	{ "STORB",	0x08 },
	{ "MOV",	0x09 },
	{ "CMP",	0x0A },

	// BOOL/BIT - 0x10
	{ "NOT",	0x10 },
	{ "AND",	0x11 },
	{ "NAND",	0x12 },
	{ "OR",		0x13 },
	{ "XOR",	0x14 },
	{ "RSH",	0x15 },
	{ "LSH",	0x16 },

	// ARITHMETIC - 0x20
	{ "ADD",	0x20 },
	{ "SUB",	0x21 },
	{ "MUL",	0x22 },
	{ "DIV",	0x23 },
	{ "MOD",	0x24 },
	{ "SIN",	0x25 },
	{ "COS",	0x26 },

	// VARIOUS (WIP)
	{ "RND",	0x30 },
	{ "TIME",	0x31 },
	{ "DBG",	0x32 },

	// FLOW
	{ "TRM",	0x40 },
	{ "JMP",	0x41 },
	{ "JZ",		0x42 },
	{ "JNZ",	0x43 },
	{ "JA",		0x44 },
	{ "JNA",	0x45 },
	{ "JMEM",	0x46 },

	// TODO: YLD
};
//  opNameCodeMap init
#pragma endregion