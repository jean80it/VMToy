#pragma once

#define ZFTest setFlag(VMFlags::ZF, (0==state.reg[a].value)) 
#define OFTest setFlag(VMFlags::OF,(state.reg[a].value>255))

#include <functional>
#include <vector>
#include <map>

#include "types.hpp"

#define FLAGS_REGISTER_IDX 0
#define IP_REGISTER_IDX 1
#define SP_REGISTER_IDX 2
#define CB_REGISTER_IDX 2

#define FLAGS_REGISTER (state.reg[FLAGS_REGISTER_IDX].shortValue)
#define IP_REGISTER (state.reg[IP_REGISTER_IDX].uvalue)
#define SP_REGISTER (state.reg[SP_REGISTER_IDX].uvalue)
#define CB_REGISTER (state.reg[CB_REGISTER_IDX].uvalue)

// Top Of Stack
#define TOS *((FP32*)&state.vmMemory[CB_REGISTER - (SP_REGISTER + 1) * sizeof(FP32)])

enum class VMFlags : unsigned char
{
	TRM = 1,
	AF = 2,
	OF = 4,
	ZF = 8,	
};

class VM
{
	typedef void(VM::*pOperatorFn) ();

	static const pOperatorFn opTable[];

	byte a;
	FP32 b;

#pragma region operators functions declaration

	void op_nop(); // NO OPERATION - does nothing
	void op_err(); // ERROR - all available opcodes not assigned to an operator function should point to this
	void op_load(); // LOAD - loads byte at memory location b into register a
	void op_loads(); // LOAD - loads byte at memory location b into register a
	void op_loadb(); // LOAD - loads byte at memory location b into register a
	void op_stor(); // STORE - stores FP32 in register a into memory location b (in bytes)
	void op_stors(); // STORE Short - stores short in register a into memory location b (in bytes)
	void op_storb(); // STORE Byte - stores byte in register a into memory location b (in bytes)
	void op_mov(); // MOVE - A = B
	void op_cmp(); // COMPARE - sets ZF if reg[a] and reg[b] are equal; AF if reg[a]>reg[b]

	// BOOL/BIT
	void op_not(); // NEGATE - A = ~A
	void op_and(); // AND - A = A & B
	void op_nand(); // NAND - A = ~(A & B)
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
	void op_jmem();

	void op_call();
	void op_ret();
	void op_yld();

	void op_push();
	void op_pop();


	// IMMEDIATES
	void op_initi(); // vmMemory - initializes vmMemory sized a bytes
	void op_loadi();
	void op_loadsi();
	void op_loadbi();
	void op_stori();
	void op_storsi();
	void op_storbi();
	void op_movi();
	void op_cmpi();
	void op_andi();
	void op_nandi();
	void op_nori();
	void op_ori();
	void op_xori();
	void op_rshi();
	void op_lshi();
	void op_addi();
	void op_subi();
	void op_muli();
	void op_divi();
	void op_modi();
	void op_rndi();
	void op_dbgi();
	void op_jmpi();
	void op_jzi();
	void op_jnzi();
	void op_jai();
	void op_jnai();
	void op_trmi();
	void op_calli();

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

	void setFlag(VMFlags f)
	{
		FLAGS_REGISTER |= (short)f;
	}

	void setFlag(VMFlags f, bool v)
	{
		short mask = v ? -1 : 0;
		FLAGS_REGISTER |= (((short)f) & mask);
		FLAGS_REGISTER &= ~(((short)f) & ~mask); // TODO: CHECK BEHAVIOUR
	}

	void resetFlag(VMFlags f)
	{
		FLAGS_REGISTER &= ~(short)f;
	}

	bool getFlag(VMFlags f)
	{
		return (FLAGS_REGISTER & (short)f) == (short)f;
	}

	void reset()
	{
		FLAGS_REGISTER = 0;
		IP_REGISTER = 0;

		state.startTime = clock();
		srand(state.startTime);
	}

	void reset(int memSize)
	{
		reset();
		state.memSize = memSize;
		SP_REGISTER = memSize; 
	}

	void load(std::vector<instruction> & program)
	{
		program_ = program;
	}

	void execStep()
	{
		auto instr = program_[IP_REGISTER];
		a = std::get<1>(instr);
		b = std::get<2>(instr);
		(this->*opTable[std::get<0>(instr)])();
		++IP_REGISTER; // TODO: check jumps
	}

	bool VM::isRunning()
	{
		return !getFlag(VMFlags::TRM);
	}
};
