#pragma once

#define ZFTest state.ZF = (0==state.reg[a].value)
#define OFTest state.OF = (state.reg[a].value>255)

#include <functional>
#include <vector>
#include <map>

#include "types.hpp"

class VM
{
	typedef void(VM::*pOperatorFn) ();

	static const pOperatorFn opTable[];

	byte a;
	FP32 b;

#pragma region operators functions declaration

	void op_nop(); // NO OPERATION - does nothing
	void op_err(); // ERROR - all available opcodes not assigned to an operator function should point to this
	void op_init(); // vmMemory - initializes vmMemory sized a bytes
	void op_load(); // LOAD - loads byte at memory location b into register a
	void op_loadi(); // LOAD IMMEDIATE - loads value b into register a
	void op_mov(); // MOVE - A = B
	void op_cmp(); // COMPARE - sets ZF if reg[a] and reg[b] are equal; AF if reg[a]>reg[b]
	// stori doesn't seem to be suitable unless i widen a operand... to write immediates to memory we've to loadi then stor...
	void op_stor(); // STORE - stores FP32 in register a into memory location b (in bytes)
	void op_stors(); // STORE Short - stores short in register a into memory location b (in bytes)
	void op_storb(); // STORE Byte - stores byte in register a into memory location b (in bytes)

	// BOOL/BIT
	void op_not(); // NEGATE - A = ~A
	void op_and(); // AND - A = A & B
	void op_nand(); // NAND - A = ~(A & B)
	void op_nor(); // NOR - A = ~(A | B)
	void op_or(); // OR - A = A | B
	void op_xor(); // XOR - A = A ^ B
	void op_rsh(); // RIGHT SHIFT - A = A >> B
	void op_lsh(); // LEFT SHIFT - A = A << B
	
	// ARITHMETIC
	void op_add(); // ADD - A = A + B
	void op_sub(); // SUBTRACT - A = A - B
	void op_mul(); // MULTIPLY - A = A * B
	void op_div(); // DIVIDE - A = A / B
	void op_mod(); // MODULUS - A = A % B
	void op_sin(); // SINE - A = SIN(B)
	void op_cos(); // COSINE - A = COS(B)
	
	// VARIOUS (WIP)
	void op_rnd(); // RANDOM - A = RAND(B) (random number from 0 to B)
	void op_time(); // TIME - A = milliseconds elapsed (since program start) in FP32
	void op_dbg(); // DEBUG - writes single char to console
	
	// FLOW
	void op_trm(); // TERMINATE - terminates the program
	void op_jmp(); // JUMP - sets instruction pointer to a; WARNING: no checks for now!!
	void op_jz(); // JUMP IF ZERO - sets instruction pointer to a only if ZF is set; WARNING: no checks for now!!
	void op_jnz(); // JUMP IF NOT ZERO - sets instruction pointer to a only if ZF is not set; WARNING: no checks for now!!
	void op_ja(); // JUMP IF ABOVE - sets instruction pointer to a only if ZF is set; WARNING: no checks for now!!
	void op_jna(); // JUMP IF NOT ABOVE - sets instruction pointer to a only if AF is not set; WARNING: no checks for now!!
	
	// TODO: jumps that take memory locations as parameters

	// operators functions declaration
#pragma endregion

public:

	VMState state;

	std::vector<instruction> program_;

	VM()
	{
		reset();
	}

	VM(std::vector<instruction> & program)
	{
		load(program);
		reset();
	}

	void reset()
	{
		state.AF = false;
		state.IP = 0;
		state.OF = false;
		state.ZF = false;
		state.startTime = clock();
		srand(state.startTime);
		state.term = false;
	}

	void load(std::vector<instruction> & program)
	{
		program_ = program;
	}

	void execStep()
	{
		auto instr = program_[state.IP];
		a = std::get<1>(instr);
		b = std::get<2>(instr);
		(this->*opTable[std::get<0>(instr)])();
		++state.IP; // TODO: check jumps
	}
};
