#include "VM.hpp"

#pragma region OPs implementation

void VM::op_nop() // NO OPERATION - does nothing
{ }

void VM::op_err() // ERROR - all available opcodes not assigned to an operator function should point to this
{
	//...
	std::cout << "error @ IP=" << IP_REGISTER << std::endl; // TODO: add more debug info
	setFlag(VMFlags::TRM);
	//...
}

///////////////////////////////////////////////////////////////
// IMMEDIATES
///////////////////////////////////////////////////////////////

// INIT
void VM::op_initi() // vmMemory - initializes vmMemory sized a bytes
{
	reset(b.shortValue);

	state.vmMemory = (byte*)malloc((size_t)state.memSize);
}


// LOAD 
void VM::op_loadi() 
{
	state.reg[a].value = 0;
	state.reg[a].byteValue = state.vmMemory[b.uvalue];
}

// LOAD SHORT 
void VM::op_loadsi()
{
	state.reg[a].value = 0;
	state.reg[a].shortValue = *(short*)(&state.vmMemory[b.uvalue]);
}

// LOAD BYTE
void VM::op_loadbi()
{
	state.reg[a].value = 0;
	state.reg[a].byteValue = state.vmMemory[b.uvalue];
}

// STORE
void VM::op_stori() 
{
	*(FP32*)(&state.vmMemory[b.uvalue]) = state.reg[a];
}

// STORE SHORT
void VM::op_storsi() // STORE - stores byte in register a into memory location b
{
	*(short*)(&state.vmMemory[b.uvalue]) = state.reg[a].shortValue;
}

// STORE BYTE
void VM::op_storbi() // STORE - stores byte in register a into memory location b
{
	state.vmMemory[b.uvalue] = state.reg[a].byteValue;
}

void VM::op_movi() // MOVE - A = B
{
	state.reg[a].value = b.value;
	ZFTest;
}

void VM::op_cmpi() // COMPARE - sets ZF if reg[a] and reg[b] are equal; AF if reg[a]>reg[b]
{
	setFlag(VMFlags::ZF, (state.reg[a].value == b.value));
	setFlag(VMFlags::AF, (state.reg[a].value > b.value));
} // cmpbi? cmpsi?

// NO SENSE to have immediate unary operators like NOT

void VM::op_andi() // AND - A = A & B
{
	state.reg[a].value &= b.value;
	ZFTest;
}

void VM::op_nandi() // NAND - A = ~(A & B)
{
	state.reg[a].value = ~(state.reg[a].value & b.value);
	ZFTest;
}

void VM::op_nori() // NOR - A = ~(A | B)
{
	state.reg[a].value = ~(state.reg[a].value | b.value);
	ZFTest;
}

void VM::op_ori() // OR - A = A | B
{
	state.reg[a].value |= b.value;
	ZFTest;
}

void VM::op_xori() // XOR - A = A ^ B
{
	state.reg[a].value ^= b.value;
	ZFTest;
}

void VM::op_rshi() // RIGHT SHIFT - A = A >> B
{
	state.reg[a].value >>= b.byteValue;
	ZFTest;
}

void VM::op_lshi() // LEFT SHIFT - A = A << B
{
	state.reg[a].value <<= b.byteValue;
	ZFTest;
}

void VM::op_addi() // ADD - A = A + B
{
	state.reg[a].value += b.value;

	ZFTest;
	OFTest;
}

void VM::op_subi() // SUBTRACT - A = A - B
{
	state.reg[a].value -= b.value;
	ZFTest;
	OFTest;
}

void VM::op_muli() // MULTIPLY - A = A * B
{
	state.reg[a].value = (state.reg[a].value * b.value) >> 16; // shift to adjust for FP representation (could pre-shift operands)
	ZFTest;
	OFTest;
}

void VM::op_divi() // DIVIDE - A = A / B
{
	state.reg[a].value = (state.reg[a].value << 8) / (b.value >> 8); // shift to adjust for FP representation 
	ZFTest;
	OFTest;
}

void VM::op_modi() // MODULUS - A = A % B
{
	state.reg[a].value = (state.reg[a].value) % b.value;
	ZFTest;
	OFTest;
}

// NO SENSE to have immediate unary operators like SIN or COS

void VM::op_rndi()
{
	state.reg[a].value = rand() % b.value;
}

// no TIME

void VM::op_dbgi() // DEBUG - writes single char to console
{
	std::cout << ((char)b.byteValue);
}

void VM::op_jmpi() // JUMP - sets instruction pointer to b
{
	// WARNING: no checks for now!!

	IP_REGISTER = b.uvalue - 1; // IP will be incremented anyway
}

void VM::op_jzi() // JUMP IF ZERO - sets instruction pointer to b only if ZF is set; 
{
	// WARNING: no checks for now!!

	if (getFlag(VMFlags::ZF))
	{
		IP_REGISTER = b.uvalue - 1; // IP will be incremented anyway
	}
}

void VM::op_jnzi() // JUMP IF NOT ZERO - sets instruction pointer to b only if ZF is not set
{
	// WARNING: no checks for now!!
	
	if (!getFlag(VMFlags::ZF))
	{
		IP_REGISTER = b.uvalue - 1; // IP will be incremented anyway
	}
}

void VM::op_jai() // JUMP IF ABOVE - sets instruction pointer to b only if ZF is set
{
	// WARNING: no checks for now!!
	
	if (getFlag(VMFlags::AF))
	{
		IP_REGISTER = b.uvalue - 1; // IP will be incremented anyway
	}
}

void VM::op_jnai() // JUMP IF NOT ABOVE - sets instruction pointer to a only if AF is not set
{
	// WARNING: no checks for now!!
	
	if (!getFlag(VMFlags::ZF))
	{
		IP_REGISTER = b.uvalue - 1; // IP will be incremented anyway
	}
}

void VM::op_trmi()
{
	setFlag(VMFlags::TRM);
	//free(state.vmMemory);
	//state.memSize = 0;
} // TODO: accept a termination code

void VM::op_calli()
{
	// push IP
	++SP_REGISTER;
	TOS = IP_REGISTER;

	IP_REGISTER = b.uvalue;
}


/////////////////////////////////////////////////
// DEFERRED
/////////////////////////////////////////////////



// LOAD - loads FP32 at memory location pointed by register b into register a. 
// ES: 
// MOV  addr  @25	     ; sets addr to 25
// LOAD value memLoc     ; load from memory location addr (= 25) into register value
void VM::op_load()
{
	state.reg[a] = *(FP32*)(&state.vmMemory[state.reg[b.b1].uvalue]);
}

// LOAD SHORT 
void VM::op_loads()
{
	state.reg[a].value = 0;
	state.reg[a].shortValue = *(short*)(&state.vmMemory[state.reg[b.b1].uvalue]);
}

// LOAD BYTE
void VM::op_loadb()
{
	state.reg[a].value = 0;
	state.reg[a].byteValue = state.vmMemory[state.reg[b.b1].uvalue];
}

// STORE - stores FP32 in register a into memory location pointed by register b
// ES:
// MOV storAddr @34
// MOV value 27
// STOR value storAddr
void VM::op_stor()
{
	*(FP32*)(&state.vmMemory[state.reg[b.b1].uvalue]) = state.reg[a];
}

// STORE SHORT
void VM::op_stors() // STORS - stores short in register a into memory location b
{
	*(short*)(&state.vmMemory[state.reg[b.b1].uvalue]) = state.reg[a].shortValue;
}

// STORE BYTE
void VM::op_storb() // STORE - stores byte in register a into memory location b
{
	state.vmMemory[state.reg[b.b1].uvalue] = state.reg[a].byteValue;
}

// TODO: load/store for shortValue and Value; stori doesn't seem to be suitable unless i widen a operand... to write immediates to memory we've to movi then stor...

void VM::op_mov() // MOVE - A = B
{
	state.reg[a].value = state.reg[b.b1].value;
	ZFTest;
}

void VM::op_cmp() // COMPARE - sets ZF if reg[a] and reg[b] are equal; AF if reg[a]>reg[b]
{
	setFlag(VMFlags::ZF, (state.reg[a].value == state.reg[b.b1].value));
	setFlag(VMFlags::AF, (state.reg[a].value > state.reg[b.b1].value));
}

// BOOL/BIT

void VM::op_not() // NEGATE - A = ~A
{
	state.reg[a].value = ~state.reg[a].value;
	ZFTest;
}

void VM::op_and() // AND - A = A & B
{
	state.reg[a].value &= state.reg[b.b1].value;
	ZFTest;
}

void VM::op_nand() // NAND - A = ~(A & B)
{
	state.reg[a].value = ~(state.reg[a].value & state.reg[b.b1].value);
	ZFTest;
}

void VM::op_or() // OR - A = A | B
{
	state.reg[a].value |= state.reg[b.b1].value;
	ZFTest;
}

void VM::op_xor() // XOR - A = A ^ B
{
	state.reg[a].value ^= state.reg[b.b1].value;
	ZFTest;
}

void VM::op_rsh() // RIGHT SHIFT - A = A >> B
{
	state.reg[a].value >>= state.reg[b.b1].byteValue;
	ZFTest;
}

void VM::op_lsh() // LEFT SHIFT - A = A << B
{
	state.reg[a].value <<= state.reg[b.b1].byteValue;
	ZFTest;
}


// ARITHMETIC

void VM::op_add() // ADD - A = A + B
{
	state.reg[a].value += state.reg[b.b1].value;

	ZFTest;
	OFTest;
}

void VM::op_sub() // SUBTRACT - A = A - B
{
	state.reg[a].value -= state.reg[b.b1].value;
	ZFTest;
	OFTest;
}

void VM::op_mul() // MULTIPLY - A = A * B
{
	state.reg[a].value = (state.reg[a].value * state.reg[b.b1].value) >> 16; // shift to adjust for FP representation (could pre-shift operands)
	ZFTest;
	OFTest;
}

void VM::op_div() // DIVIDE - A = A / B
{
	state.reg[a].value = (state.reg[a].value << 16) / state.reg[b.b1].value; // shift to adjust for FP representation
	ZFTest;
	OFTest;
}

void VM::op_mod() // MODULUS - A = A % B
{
	state.reg[a].value = (state.reg[a].value) % state.reg[b.b1].value;
	ZFTest;
	OFTest;
}

void VM::op_sin() // SINE - A = SIN(B)
{
	state.reg[a].value = (slong)(sin((float)state.reg[b.b1].value / (float)65536) * 65536); // mul/div for FP representation
	ZFTest;
	OFTest;
} // TODO: precision setting and LUT 

void VM::op_cos() // COSINE - A = COS(B)
{
	state.reg[a].value = (slong)(cos((float)state.reg[b.b1].value / (float)65536) * 65536); // mul/div for FP representation
	ZFTest;
	OFTest;
} // TODO: precision setting and LUT 


// VARIOUS (WIP)
void VM::op_rnd() // RANDOM - A = RAND(B) (random number from 0 to B)
{
	state.reg[a].value = rand() % state.reg[b.b1].value;
}

void VM::op_time() // TIME - A = seconds elapsed (since program start) in FP32
{
	state.reg[a].value = (((clock() - state.startTime) << 16) / CLOCKS_PER_SEC);
}

void VM::op_dbg() // DEBUG - writes single char to console
{
	std::cout << ((char)state.reg[b.b1].byteValue);
}


// FLOW

void VM::op_trm() // TERMINATE - terminates the program
{
	setFlag(VMFlags::TRM);
	//free(state.vmMemory);
	//state.memSize = 0;
} // TODO: accept a termination code

void VM::op_jmp() // JUMP - sets instruction pointer to b
{
	// WARNING: no checks for now!!

	IP_REGISTER = state.reg[b.b1].uvalue - 1; // IP will be incremented anyway
}

void VM::op_jz() // JUMP IF ZERO - sets instruction pointer to b only if ZF is set; 
{
	// WARNING: no checks for now!!

	if (getFlag(VMFlags::ZF))
	{
		IP_REGISTER = state.reg[b.b1].uvalue - 1; // IP will be incremented anyway
	}
}

void VM::op_jnz() // JUMP IF NOT ZERO - sets instruction pointer to b only if ZF is not set
{
	// WARNING: no checks for now!!

	if (!getFlag(VMFlags::ZF))
	{
		IP_REGISTER = state.reg[b.b1].uvalue - 1; // IP will be incremented anyway
	}
}

void VM::op_ja() // JUMP IF ABOVE - sets instruction pointer to b only if ZF is set
{
	// WARNING: no checks for now!!

	if (getFlag(VMFlags::AF))
	{
		IP_REGISTER = state.reg[b.b1].uvalue - 1; // IP will be incremented anyway
	}
}

void VM::op_jna() // JUMP IF NOT ABOVE - sets instruction pointer to a only if AF is not set
{
	// WARNING: no checks for now!!

	if (!getFlag(VMFlags::ZF))
	{
		IP_REGISTER = state.reg[b.b1].uvalue - 1; // IP will be incremented anyway
	}
}

void VM::op_jmem()
{
	throw "Not Implemented";
}

void VM::op_call()
{
	// push IP
	++SP_REGISTER;
	TOS = IP_REGISTER;

	IP_REGISTER = state.reg[b.b1].uvalue;
}

void VM::op_ret()
{
	//pop IP
	IP_REGISTER = TOS;
	--SP_REGISTER;
	// TODO: RESULT register?
}

void VM::op_yld()
{
	// nothing to do... machine should just recognize the instruction and stop
	// maybe call a callback
}

void VM::op_push()
{
	++SP_REGISTER;
	TOS = state.reg[a];
}

void VM::op_pop()
{
	state.reg[a] = TOS;
	--SP_REGISTER;
}


// OPs implementation
#pragma endregion

#pragma region opTable init

const VM::pOperatorFn VM::opTable[256] = 
{
	//////////////////////////////
	// IMMEDIATES
	//////////////////////////////

	/*  0x00 */ &VM::op_nop,
	/*  0x01 */ &VM::op_err,
	/*  0x02 */ &VM::op_initi,
	/*  0x03 */ &VM::op_loadi,
	/*  0x04 */ &VM::op_loadsi,
	/*  0x05 */ &VM::op_loadbi,
	/*  0x06 */ &VM::op_stori,
	/*  0x07 */ &VM::op_storsi,
	/*  0x08 */ &VM::op_storbi,
	/*  0x09 */ &VM::op_movi,
	/*  0x0A */ &VM::op_cmpi,

	&VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err,

	// BOOL/BIT
	/* 0x10 */ &VM::op_err, // not 
	/* 0x11 */ &VM::op_andi,
	/* 0x12 */ &VM::op_nandi,
	/* 0x13 */ &VM::op_ori,
	/* 0x14 */ &VM::op_xori,
	/* 0x15 */ &VM::op_rshi,
	/* 0x16 */ &VM::op_lshi,

	&VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err,

	// ARITHMETIC
	/* 0x20 */ &VM::op_addi,
	/* 0x21 */ &VM::op_subi,
	/* 0x22 */ &VM::op_muli,
	/* 0x23 */ &VM::op_divi,
	/* 0x24 */ &VM::op_modi,
	/* 0x25 */ &VM::op_err, // sin
	/* 0x26 */ &VM::op_err, // cos

	&VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err,

	// VARIOUS (WIP)
	/* 0x30 */ &VM::op_rndi,
	/* 0x31 */ &VM::op_err, // time
	/* 0x32 */ &VM::op_dbgi,

	&VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err,

	// FLOW
	/* 0x40 */ &VM::op_trmi,
	/* 0x41 */ &VM::op_jmpi,
	/* 0x42 */ &VM::op_jzi,
	/* 0x43 */ &VM::op_jnzi,
	/* 0x44 */ &VM::op_jai,
	/* 0x45 */ &VM::op_jnai,
	/* 0x46 */ &VM::op_jmem,

	&VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err,
	/* 0x50 */ &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err,
	/* 0x60 */ &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err,
	/* 0x70 */ &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err,


	//////////////////////////////
	// DEFERRED (0x80 + ...)
	//////////////////////////////

	// MEMORY OPS / INIT
	/*  0x00 */ &VM::op_nop,
	/*  0x01 */ &VM::op_err,
	/*  0x02 */ &VM::op_err, // init
	/*  0x03 */ &VM::op_load,
	/*  0x04 */ &VM::op_loads,
	/*  0x05 */ &VM::op_loadb,
	/*  0x06 */ &VM::op_stor,
	/*  0x07 */ &VM::op_stors,
	/*  0x08 */ &VM::op_storb,
	/*  0x09 */ &VM::op_mov,
	/*  0x0A */ &VM::op_cmp,
	
	&VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err,

	// BOOL/BIT
	/* 0x10 */ &VM::op_not,
	/* 0x11 */ &VM::op_and,
	/* 0x12 */ &VM::op_nand,
	/* 0x13 */ &VM::op_or,
	/* 0x14 */ &VM::op_xor,
	/* 0x15 */ &VM::op_rsh,
	/* 0x16 */ &VM::op_lsh,

	&VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err,

	// ARITHMETIC
	/* 0x20 */ &VM::op_add,
	/* 0x21 */ &VM::op_sub,
	/* 0x22 */ &VM::op_mul,
	/* 0x23 */ &VM::op_div,
	/* 0x24 */ &VM::op_mod,
	/* 0x25 */ &VM::op_sin,
	/* 0x26 */ &VM::op_cos,

	&VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err,

	// VARIOUS (WIP)
	/* 0x30 */ &VM::op_rnd,
	/* 0x31 */ &VM::op_time,
	/* 0x32 */ &VM::op_dbg,

	&VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err,

	// FLOW
	/* 0x40 */ &VM::op_trm,
	/* 0x41 */ &VM::op_jmp,
	/* 0x42 */ &VM::op_jz,
	/* 0x43 */ &VM::op_jnz,
	/* 0x44 */ &VM::op_ja,
	/* 0x45 */ &VM::op_jna,
	/* 0x46 */ &VM::op_jmem,

	&VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err,
	/* 0x50 */ &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err,
	/* 0x60 */ &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err,
	/* 0x70 */ &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err, &VM::op_err,

};

// opTable init
#pragma endregion 