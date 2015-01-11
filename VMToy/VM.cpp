#include "VM.h"

#pragma region opTable init

const std::vector<std::function<void(OPPARAMS)>> VM::opTable =
{
	// MEMORY OPS / INIT
	/*  0 */ VM::op_nop,
	/*  1 */ VM::op_err,
	/*  2 */ VM::op_init,
	/*  3 */ VM::op_load,
	/*  4 */ VM::op_loadi,
	/*  5 */ VM::op_stor,
	/*  6 */ VM::op_mov,
	/*  7 */ VM::op_cmp,

	VM::op_err, VM::op_err, // skipping 8, 9

	// BOOL/BIT
	/* 10 */ VM::op_not,
	/* 11 */ VM::op_and,
	/* 12 */ VM::op_nand,
	/* 13 */ VM::op_nor,
	/* 14 */ VM::op_or,
	/* 15 */ VM::op_xor,
	/* 16 */ VM::op_rsh,
	/* 17 */ VM::op_lsh,

	VM::op_err, VM::op_err, // skipping 18, 19

	// ARITHMETIC
	/* 20 */ VM::op_add,
	/* 21 */ VM::op_sub,
	/* 22 */ VM::op_mul,
	/* 23 */ VM::op_div,
	/* 24 */ VM::op_mod,
	/* 25 */ VM::op_sin,
	/* 26 */ VM::op_cos,

	VM::op_err, VM::op_err, VM::op_err, // skipping 27, 28, 29

	// VARIOUS (WIP)
	/* 30 */ VM::op_rnd,
	/* 31 */ VM::op_time,
	/* 32 */ VM::op_dbg,

	VM::op_err, VM::op_err, VM::op_err, VM::op_err, VM::op_err, VM::op_err, VM::op_err, // skipping 33, 34, 35, 36, 37, 38, 39

	// FLOW
	/* 40 */ VM::op_trm,
	/* 41 */ VM::op_jmp,
	/* 42 */ VM::op_jz,
	/* 43 */ VM::op_jnz,
	/* 44 */ VM::op_ja,
	/* 45 */ VM::op_jna,
};

// opTable init
#pragma endregion 