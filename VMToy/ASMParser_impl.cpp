#include "AsmParser_impl.hpp"

#pragma region opNameCodeMap init
const std::map<std::string, ushort> ASMParser_Impl::opNameCodeMap =
{
	// MEMORY OPS / INIT
	{ "NOP", 0 },
	{ "ERR", 1 },
	{ "INIT", 2 },
	{ "LOAD", 3 },
	{ "LOADI", 4 },
	{ "MOV", 5 },
	{ "CMP", 6 },
	{ "STOR", 7 },
	{ "STORS", 8 },
	{ "STORB", 9 },

	// TODO: load, load byte, load short

	// BOOL/BIT
	{ "NOT", 10 },
	{ "AND", 11 },
	{ "NOR", 12 },
	{ "NAND", 13 },
	{ "OR", 14 },
	{ "XOR", 15 },
	{ "RSH", 16 },
	{ "LSH", 17 },


	// ARITHMETIC
	{ "ADD", 20 },
	{ "SUB", 21 },
	{ "MUL", 22 },
	{ "DIV", 23 },
	{ "MOD", 24 },
	{ "SIN", 25 },
	{ "COS", 26 },

	// VARIOUS (WIP)
	{ "RND", 30 },
	{ "TIME", 31 },
	{ "DBG", 32 },

	// FLOW
	{ "TRM", 40 },
	{ "JMP", 41 },
	{ "JZ", 42 },
	{ "JNZ", 43 },
	{ "JA", 44 },
	{ "JNA", 45 },

	// TODO: YLD
};
//  opNameCodeMap init
#pragma endregion