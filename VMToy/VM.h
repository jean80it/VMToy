#pragma once

#define OPPARAMS VMState &state, byte a, FP32 b
#define ZFTest state.ZF = (0==state.reg[a].value)
#define OFTest state.OF = (state.reg[a].value>255)

#include <functional>
#include <vector>
#include <map>

#include "types.h"

class VM
{
	const static std::vector<std::function<void(OPPARAMS)>> opTable;

#pragma region operators functions declaration

	static void op_nop(OPPARAMS); // NO OPERATION - does nothing
	static void op_err(OPPARAMS); // ERROR - all available opcodes not assigned to an operator function should point to this
	static void op_init(OPPARAMS); // vmMemory - initializes vmMemory sized a bytes
	static void op_load(OPPARAMS); // LOAD - loads byte at memory location b into register a
	static void op_loadi(OPPARAMS); // LOAD IMMEDIATE - loads value b into register a
	static void op_stor(OPPARAMS); // STORE - stores byte in register a into memory location b
	
	// TODO: load/store for shortValue and Value; stori doesn't seem to be suitable unless i widen a operand... to write immediates to memory we've to loadi then stor...
	static void op_mov(OPPARAMS); // MOVE - A = B
	static void op_cmp(OPPARAMS); // COMPARE - sets ZF if reg[a] and reg[b] are equal; AF if reg[a]>reg[b]
	
	// BOOL/BIT
	static void op_not(OPPARAMS); // NEGATE - A = ~A
	static void op_and(OPPARAMS); // AND - A = A & B
	static void op_nand(OPPARAMS); // NAND - A = ~(A & B)
	static void op_nor(OPPARAMS); // NOR - A = ~(A | B)
	static void op_or(OPPARAMS); // OR - A = A | B
	static void op_xor(OPPARAMS); // XOR - A = A ^ B
	static void op_rsh(OPPARAMS); // RIGHT SHIFT - A = A >> B
	static void op_lsh(OPPARAMS); // LEFT SHIFT - A = A << B
	
	// ARITHMETIC
	static void op_add(OPPARAMS); // ADD - A = A + B
	static void op_sub(OPPARAMS); // SUBTRACT - A = A - B
	static void op_mul(OPPARAMS); // MULTIPLY - A = A * B
	static void op_div(OPPARAMS); // DIVIDE - A = A / B
	static void op_mod(OPPARAMS); // MODULUS - A = A % B
	static void op_sin(OPPARAMS); // SINE - A = SIN(B)
	static void op_cos(OPPARAMS); // COSINE - A = COS(B)
	
	// VARIOUS (WIP)
	static void op_rnd(OPPARAMS); // RANDOM - A = RAND(B) (random number from 0 to B)
	static void op_time(OPPARAMS); // TIME - A = milliseconds elapsed (since program start) in FP32
	static void op_dbg(OPPARAMS); // DEBUG - writes single char to console
	
	// FLOW
	static void op_trm(OPPARAMS); // TERMINATE - terminates the program
	static void op_jmp(OPPARAMS); // JUMP - sets instruction pointer to a; WARNING: no checks for now!!
	static void op_jz(OPPARAMS); // JUMP IF ZERO - sets instruction pointer to a only if ZF is set; WARNING: no checks for now!!
	static void op_jnz(OPPARAMS); // JUMP IF NOT ZERO - sets instruction pointer to a only if ZF is not set; WARNING: no checks for now!!
	static void op_ja(OPPARAMS); // JUMP IF ABOVE - sets instruction pointer to a only if ZF is set; WARNING: no checks for now!!
	static void op_jna(OPPARAMS); // JUMP IF NOT ABOVE - sets instruction pointer to a only if AF is not set; WARNING: no checks for now!!
	
	// TODO: jumps that take memory locations as parameters

	// operators functions declaration
#pragma endregion

public:
	std::vector<instruction> program_;

	VMState state;

	VM(){}

	VM(std::vector<instruction> & program)
	{
		load(program);
	}

	void load(std::vector<instruction> & program)
	{
		program_ = program;
	}

	void execStep()
	{
		auto instr = program_[state.IP];
		(opTable[std::get<0>(instr)])(state, std::get<1>(instr), std::get<2>(instr));
	}
};
