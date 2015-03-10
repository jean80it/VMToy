#include "VM.h"

#pragma region OPs implementation

void VM::op_nop() // NO OPERATION - does nothing
{ }

void VM::op_err() // ERROR - all available opcodes not assigned to an operator function should point to this
{
	//...
	std::cout << "error @ IP=" << state.IP << std::endl; // TODO: add more debug info
	state.term = true;
	//...
}

void VM::op_init() // vmMemory - initializes vmMemory sized a bytes
{
	state.memSize = b.value;
	state.vmMemory = (byte*)malloc((size_t)state.memSize);
	state.startTime = clock();
	srand(state.startTime);
}

void VM::op_load() // LOAD - loads byte at memory location b into register a
{
	state.reg[a].value = 0;
	state.reg[a].byteValue = state.vmMemory[b.uvalue];
}

void VM::op_loadi() // LOAD IMMEDIATE - loads value b into register a
{
	state.reg[a].value = b.value;
}

void VM::op_stor() // STORE - stores byte in register a into memory location b
{
	state.vmMemory[b.uvalue] = state.reg[a].byteValue;
}

// TODO: load/store for shortValue and Value; stori doesn't seem to be suitable unless i widen a operand... to write immediates to memory we've to loadi then stor...

void VM::op_mov() // MOVE - A = B
{
	state.reg[a].value = state.reg[b.byteValue].value;
	ZFTest;
}

void VM::op_cmp() // COMPARE - sets ZF if reg[a] and reg[b] are equal; AF if reg[a]>reg[b]
{
	state.ZF = (state.reg[a].value == state.reg[b.byteValue].value);
	state.AF = (state.reg[a].value > state.reg[b.byteValue].value);
}

// BOOL/BIT

void VM::op_not() // NEGATE - A = ~A
{
	state.reg[a].value = ~state.reg[a].value;
	ZFTest;
}

void VM::op_and() // AND - A = A & B
{
	state.reg[a].value &= state.reg[a].value;
	ZFTest;
}

void VM::op_nand() // NAND - A = ~(A & B)
{
	state.reg[a].value = ~(state.reg[a].value & state.reg[b.byteValue].value);
	ZFTest;
}

void VM::op_nor() // NOR - A = ~(A | B)
{
	state.reg[a].value = ~(state.reg[a].value | state.reg[b.byteValue].value);
	ZFTest;
}

void VM::op_or() // OR - A = A | B
{
	state.reg[a].value |= state.reg[a].value;
	ZFTest;
}

void VM::op_xor() // XOR - A = A ^ B
{
	state.reg[a].value ^= state.reg[a].value;
	ZFTest;
}

void VM::op_rsh() // RIGHT SHIFT - A = A >> B
{
	state.reg[a].value >>= b.byteValue;
	ZFTest;
}

void VM::op_lsh() // LEFT SHIFT - A = A << B
{
	state.reg[a].value <<= b.byteValue;
	ZFTest;
}


// ARITHMETIC

void VM::op_add() // ADD - A = A + B
{
	state.reg[a].value += state.reg[b.byteValue].value;

	ZFTest;
	OFTest;
}

void VM::op_sub() // SUBTRACT - A = A - B
{
	state.reg[a].value -= state.reg[b.byteValue].value;
	ZFTest;
	OFTest;
}

void VM::op_mul() // MULTIPLY - A = A * B
{
	state.reg[a].value = (state.reg[a].value * state.reg[b.byteValue].value) >> 16; // shift to adjust for FP representation
	ZFTest;
	OFTest;
}

void VM::op_div() // DIVIDE - A = A / B
{
	state.reg[a].value = (state.reg[a].value << 16) / state.reg[b.byteValue].value; // shift to adjust for FP representation
	ZFTest;
	OFTest;
}

void VM::op_mod() // MODULUS - A = A % B
{
	state.reg[a].value = (state.reg[a].value) % state.reg[b.byteValue].value;
	ZFTest;
	OFTest;
}

void VM::op_sin() // SINE - A = SIN(B)
{
	state.reg[a].value = (slong)(sin((float)b.value / (float)65536) * 65536); // mul/div for FP representation
	ZFTest;
	OFTest;
}

void VM::op_cos() // COSINE - A = COS(B)
{
	state.reg[a].value = (slong)(cos((float)b.value / (float)65536) * 65536); // mul/div for FP representation
	ZFTest;
	OFTest;
}


// VARIOUS (WIP)
void VM::op_rnd() // RANDOM - A = RAND(B) (random number from 0 to B)
{
	state.reg[a].value = rand() % b.value;
}

void VM::op_time() // TIME - A = milliseconds elapsed (since program start) in FP32
{
	state.reg[a].value = (((clock() - state.startTime) << 16) / CLOCKS_PER_SEC);
}

void VM::op_dbg() // DEBUG - writes single char to console
{
	std::cout << ((char)b);
}


// FLOW

void VM::op_trm() // TERMINATE - terminates the program
{
	state.term = true;
	//free(state.vmMemory);
	//state.memSize = 0;
}

void VM::op_jmp() // JUMP - sets instruction pointer to a; WARNING: no checks for now!!
{
	state.IP = b.shortValue;
}

void VM::op_jz() // JUMP IF ZERO - sets instruction pointer to a only if ZF is set; WARNING: no checks for now!!
{
	if (state.ZF)
	{
		state.IP = b.shortValue;
	}
}

void VM::op_jnz() // JUMP IF NOT ZERO - sets instruction pointer to a only if ZF is not set; WARNING: no checks for now!!
{
	if (!state.ZF)
	{
		state.IP = b.shortValue;
	}
}

void VM::op_ja() // JUMP IF ABOVE - sets instruction pointer to a only if ZF is set; WARNING: no checks for now!!
{
	if (state.AF)
	{
		state.IP = b.shortValue;
	}
}

void VM::op_jna() // JUMP IF NOT ABOVE - sets instruction pointer to a only if AF is not set; WARNING: no checks for now!!
{
	if (!state.AF)
	{
		state.IP = b.shortValue;
	}
}

// OPs implementation
#pragma endregion

#pragma region opTable init

const VM::pOperatorFn VM::opTable[] = 
{
	// MEMORY OPS / INIT
	/*  0 */ &VM::op_nop,
	/*  1 */ &VM::op_err,
	/*  2 */ &VM::op_init,
	/*  3 */ &VM::op_load,
	/*  4 */ &VM::op_loadi,
	/*  5 */ &VM::op_stor,
	/*  6 */ &VM::op_mov,
	/*  7 */ &VM::op_cmp,

	&VM::op_err, &VM::op_err, // skipping 8, 9

	// BOOL/BIT
	/* 10 */ &VM::op_not,
	/* 11 */ &VM::op_and,
	/* 12 */ &VM::op_nand,
	/* 13 */ &VM::op_nor,
	/* 14 */ &VM::op_or,
	/* 15 */ &VM::op_xor,
	/* 16 */ &VM::op_rsh,
	/* 17 */ &VM::op_lsh,

	&VM::op_err, &VM::op_err, // skipping 18, 19

	// ARITHMETIC
	/* 20 */ &VM::op_add,
	/* 21 */ &VM::op_sub,
	/* 22 */ &VM::op_mul,
	/* 23 */ &VM::op_div,
	/* 24 */ &VM::op_mod,
	/* 25 */ &VM::op_sin,
	/* 26 */ &VM::op_cos,

	&VM::op_err, &VM::op_err, &VM::op_err, // skipping 27, 28, 29

	// VARIOUS (WIP)
	/* 30 */ &VM::op_rnd,
	/* 31 */ &VM::op_time,
	/* 32 */ &VM::op_dbg,

	&VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, // skipping 33, 34, 35, 36, 37, 38, 39

	// FLOW
	/* 40 */ &VM::op_trm,
	/* 41 */ &VM::op_jmp,
	/* 42 */ &VM::op_jz,
	/* 43 */ &VM::op_jnz,
	/* 44 */ &VM::op_ja,
	/* 45 */ &VM::op_jna,
};

// opTable init
#pragma endregion 