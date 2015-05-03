#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <sstream>
#include <string>
#include <cstdio>
#include <unordered_map>

#include "dbgUtils.hpp"
#include "ASMParser.hpp"
#include "types.hpp"

#define IMMEDIATE_OP(x) x
#define DEFERRED_OP(x) (x | 128)

class ASMParser_Impl : public ASM_Parser
{

	char nameStorage[NAMEMAXSIZE] = {};
	int nameStorageIdx = 0;

	char arg1Storage[ARG1MAXSIZE] = {};
	int arg1StorageIdx = 0;

	char arg2Storage[ARG2MAXSIZE] = {};
	int arg2StorageIdx = 0;

	FP32 arg2;
	int arg2decimalDiv = 10;
	bool arg2neg = false;

	const static std::map<std::string, ushort> opNameCodeMap;

	std::map<std::string, ushort> labels;
	std::map<std::string, byte> identifiers;
	std::map<std::string, std::vector<ushort>> unknownLabels;

	int regCount = 0;

public:
	std::vector<instruction> program;

private:
	bool needsLabel(int opcode)
	{
		return opcode >= 0x41; // trivial solution for now
	}

	void resetOpName()
	{
		memset(&(nameStorage), 0, NAMEMAXSIZE);
		nameStorageIdx = 0;
	}

	void resetArg1()
	{
		memset(&(arg1Storage), 0, ARG1MAXSIZE);
		arg1StorageIdx = 0;
	}

	void resetArg2()
	{
		arg2 = 0;
		arg2decimalDiv = 10;
		arg2neg = false;

		memset(&(arg2Storage), 0, ARG2MAXSIZE);
		arg2StorageIdx = 0;
	}



	// unexpected character for current state: signal if in debug mode
	void fsmAction_error() override
	{
		dbgPrintf("> Error processing '%c' ( # %d ) while in state %d on line %d, col %d\n", currentInput, currentInput, state, line, column);
		// TODO: set some error state
	}

	// nop (and no error)
	void fsmAction_none() override
	{}

	bool getIdentifierCode(char * identifier, byte & code)
	{
		auto idPair = identifiers.find(identifier);
		if (idPair != identifiers.end())
		{
			code = idPair->second;
			return true;
		}
		else
		{
			code = 255;
			return false;
		}
	}

	byte getOrSetIdentifierCode(char * identifier)
	{
		auto idPair = identifiers.find(identifier);
		if (idPair != identifiers.end())
		{
			// identifier found
			return idPair->second;
		}
		else
		{
			identifiers.insert({ identifier, regCount });
			auto result = regCount;
			++regCount;
			return result;
		}
	}

	void fsmAction_instr_done() override // TODO: tell immediate operators apart from deferred ones
	{
		dbgPrintf("instruction completed: '%s'\n", nameStorage);

		// arg2 adjustments
		if (arg2neg)
		{
			arg2.shortValue = -arg2.shortValue;
		}

		if (arg2StorageIdx > 0) // arg2 is a named register 
		{
			arg2.byteValue = getOrSetIdentifierCode(arg2Storage);
		}

		// retrieve operand code
		auto operandPair = opNameCodeMap.find(nameStorage);

		if (operandPair != opNameCodeMap.end()) // if operator is in operators list
		{
			ushort opcode = operandPair->second; // get operator code

			if (needsLabel(opcode)) // is this an operator that would require just a label? (because usually first identifier is a register)
			{
				auto lblPair = labels.find(arg1Storage);

				if (lblPair != labels.end())
				{
					program.push_back(instruction(IMMEDIATE_OP(opcode), 0, FP32(lblPair->second)));
				}
				else
				{
					// is it a variable?
					byte idCode;
					if (getIdentifierCode(arg1Storage, idCode))
					{
						program.push_back(instruction(DEFERRED_OP(opcode), 0, idCode));
					}
					else
					{ 
						unknownLabels[arg1Storage].push_back(program.size()); // current instruction needs reference to yet-to-be-specified label with given name
						program.push_back(instruction(DEFERRED_OP(opcode), 0, FP32(0)));
						dbgPrintf("unknown label '%s' at: %d\n", arg1Storage, program.size() - 1);
					}
				}
			}
			else // instruction is not "label only": retrieve or create identifier
			{
				if (arg2StorageIdx == 0) // we just wrote to FP32 value and didn't accumulate any names: arg2 is immediate value, and this is an immediate instruction
				{
					program.push_back(instruction(IMMEDIATE_OP(opcode), getOrSetIdentifierCode(arg1Storage), arg2)); // immediate mode (|64)
				}
				else
				{   // we collected an identifier rather than an immediate value for arg2

					byte idCode;
					if (getIdentifierCode(arg2Storage, idCode)) // check the identifier is amongst already known register names
					{
						program.push_back(instruction(DEFERRED_OP(opcode), getOrSetIdentifierCode(arg1Storage), idCode));
					}
					else
					{
						// look for label and use it as an immediate value
						auto lblPair = labels.find(arg2Storage);

						if (lblPair != labels.end())
						{
							program.push_back(instruction(IMMEDIATE_OP(opcode), getOrSetIdentifierCode(arg1Storage), FP32(lblPair->second))); // immediate mode (|64)
						}
						else
						{
							// forward label as immediate
							unknownLabels[arg2Storage].push_back(program.size()); // current instruction needs reference to yet-to-be-specified label with given name
							program.push_back(instruction(IMMEDIATE_OP(opcode), getOrSetIdentifierCode(arg1Storage), FP32(0)));
							dbgPrintf("unknown label '%s' at: %d\n", arg2Storage, program.size() - 1);
						}
					}
				}
			}
		}
		else
		{
			// signal error
		}

		resetOpName();
		resetArg1();
		resetArg2();
	}

	// invoked every time a new character is fed, while in "operator collection" or "label collection" states (and "skip spaces 1" to start op collection)
	void fsmAction_name_stor() override
	{ 
		nameStorage[(nameStorageIdx)++] = currentInput;
	}

	// invoked upon completion of a label (i.e. after a return while in "label" state)
	void fsmAction_lbl_done() override
	{
		dbgPrintf("label found: '%s'\n", nameStorage);

		int labelValue = program.size(); // label points to next-to-current instruction

		labels.insert({ nameStorage, labelValue }); // insert label into list of labels

		// try to fix earlier jumps to yet to be specified labels
		auto unsPair = unknownLabels.find(nameStorage);
		if (unsPair != unknownLabels.end()) // if there are previous jumps to undefined label with name == to last inserted label
		{
			for each (ushort idx in unsPair->second) // for each pointed instruction
			{
				std::get<2>(program[idx]) = labelValue; // set actual jump address
				dbgPrintf("fixed label '%s' at: %d\n", nameStorage, idx);
			}
		}

		resetOpName(); // same variables are reused for op and label names
		resetArg1();
		resetArg2();
	}

	// arg1 is a string holding name of register, or label to jump to; arg1_stor collects characters belonging to it
	void fsmAction_arg1_stor() override
	{
		arg1Storage[arg1StorageIdx++] = currentInput;
	}

	// sets we have to negate the value, eventually
	void fsmAction_neg_n()
	{
		arg2neg = true;
	}

	// incremental update of (FP32 type) arg2 integer part
	void fsmAction_n_stor() override
	{
		// store before point
		arg2.shortValue = arg2.shortValue * 10 + (int)(currentInput - '0');
	}

	// incremental update of (FP32 type) arg2 fractional part
	void fsmAction_n_stor_f() override
	{
		arg2.fract = arg2.fract + (((unsigned int)(currentInput - '0')) << 16) / arg2decimalDiv;
		arg2decimalDiv *= 10;
	}

	// incremental update of (FP32 type) arg2 as an address
	void fsmAction_addr_stor() override
	{
		arg2.uvalue = arg2.uvalue * 10 + (int)(currentInput - '0');
	}


	void fsmAction_arg2_name_stor() override
	{
		arg2Storage[arg2StorageIdx++] = currentInput;
	}

	void fsmAction_set_arg2_char() override
	{
		arg2.byteValue = (byte)currentInput;
	}

public:

	ASMParser_Impl()
	{
		identifiers.insert({ "FLAGS", regCount++ }); 
		identifiers.insert({ "IP", regCount++ }); // INSTRUCTION POINTER
		identifiers.insert({ "SP", regCount++ }); // STACK POINTER
		identifiers.insert({ "CB", regCount++ }); // CODE BASE: automatically set by machine on jumps, according to "instructions map" table, is added to IP
	}

	ASMParser_Impl(char* sourcePath) : ASMParser_Impl()
	{
		parseAsm(sourcePath);
	}

	void parseAsm(char* filename)
	{
		const int bufSize = 65535;
		char buf[bufSize] = {};

		FILE *f;
		f = fopen(filename, "r");
		int bytesRead = 0;
		do
		{
			bytesRead = fread(&buf, 1, bufSize, f);
			for (int i = 0; i < bytesRead; ++i)
			{
				feed(buf[i]);
			}
		} while (bytesRead == bufSize);

		// TODO: check unsatisfied labels have been all satisfied
	}
};
