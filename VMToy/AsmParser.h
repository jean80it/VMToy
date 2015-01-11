#pragma once

#include <vector>
#include <tuple>

#include "dbgUtil.h"
#include "types.h"

enum parserStates { PS_skip_WS_1, PS_comment, PS_label, PS_error, PS_coll_op, PS_skip_WS_2, PS_coll_arg1, PS_skip_WS_3, PS_arg_2_addr, PS_arg_2_n, PS_arg_2_n_f };

class Parser
{
	typedef void(Parser::*pActionFn) ();
	
	char currentChar = 0;
	bool got13 = false;
	int line = 1;
	int column = 0;

	char nameStorage[NAMEMAXSIZE] = {};
	int nameStorageIdx = 0;


	char arg1Storage[ARG1MAXSIZE] = {};
	int arg1StorageIdx = 0;

	FP32 arg2;
	int arg2decimalDiv = 10;
	bool arg2neg = false;
	
	const static std::map<std::string, ushort> opNameCodeMap;

	static const pActionFn actionsTable[128][11];
	static const parserStates transitionsTable[128][11];

	parserStates vmStatus = PS_skip_WS_1;
	parserStates vmNextStatus = PS_skip_WS_1;

	std::map<std::string, ushort> labels;
	std::map<std::string, byte> identifiers;
	std::map<std::string, std::vector<ushort>> unsatisfiedLabels;

	int regCount = 0;

#pragma region actions functions

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
	}

	void fn_nop(){}

	void fn_err()
	{
		dbgPrintf("Error %d on line %d, col %d", 1, line, column);
	}

	void fn_name_stor()
	{
		nameStorage[(nameStorageIdx)++] = currentChar;
	}

	bool needsLabel(int opcode)
	{
		return opcode > 41; // trivial solution for now
	}

	

	void fn_op_done()
	{
		dbgPrintf("operator found: '%s'\n", nameStorage);
		
		auto operandPair = opNameCodeMap.find("f");

		if (operandPair != opNameCodeMap.end()) 
		{
			ushort opcode = operandPair->second;

			if (!needsLabel(opcode))
			{
				auto lblPair = labels.find(arg1Storage);

				if (lblPair != labels.end())
				{
					program.push_back(instruction(opcode, 0, FP32(lblPair->second)));
				}
				else
				{
					unsatisfiedLabels[arg1Storage].push_back(program.size() - 1); // current instruction needs reference to yet-to-be-specified label with given name
					program.push_back(instruction(opcode, 0, FP32(0)));
					dbgPrintf("unknown label '%s' at: %d\n", nameStorage, program.size()-1);
				}
			}
			else // do not need label: retrieve or create identifier
			{
				auto idPair = identifiers.find(arg1Storage);
				if (idPair != identifiers.end())
				{
					// identifier found
					program.push_back(instruction(opcode, idPair->second, arg2));
				}
				else
				{
					identifiers.insert({arg1Storage, regCount});
					program.push_back(instruction(opcode, regCount, arg2));
					++regCount;
				}
			}
		}
		else
		{
			// signal error
		}

		resetOpName();
	}

	void fn_lbl_done()
	{
		dbgPrintf("label found: '%s'\n", nameStorage);

		int labelValue = program.size(); // label points to next-to-current instruction

		labels.insert({ nameStorage, labelValue }); // insert label into list of labels

		// try to fix earlier jumps to yet to be specified labels
		auto unsPair = unsatisfiedLabels.find(nameStorage);
		if (unsPair != unsatisfiedLabels.end()) // if there are previous jumps to undefined label with name == to last inserted label
		{
			for each (ushort idx in unsPair->second) // for each pointed instruction
			{
				std::get<2>(program[idx]) = labelValue; // set actual jump address
				dbgPrintf("fixed label '%s' at: %d\n", nameStorage, idx);
			}
		}

		resetOpName();
	}

	void fn_arg1_stor()
	{
		arg1Storage[arg1StorageIdx++] = currentChar;
	}

	void fn_arg1_done()
	{
		dbgPrintf("arg1 found: '%s'\n", arg1Storage);

		resetArg1();
	}

	void fn_n_stor()
	{
		// store before point
		arg2.shortValue = arg2.shortValue * 10 + (int)(currentChar - '0');
	}

	void fn_n_stor_f()
	{
		arg2.fract = arg2.fract + (((unsigned int)(currentChar - '0')) << 16) / arg2decimalDiv;
		arg2decimalDiv *= 10;
	}

	void fn_neg_n()
	{
		arg2neg = true;
	}

	void fn_addr_stor()
	{
		arg2.uvalue = arg2.uvalue * 10 + (int)(currentChar - '0');
	}

	void fn_n_done()
	{
		if (arg2neg)
		{
			arg2.shortValue = -arg2.shortValue;
		}

		dbgPrintf("arg2 found: %f  (@%lu)\n", (((float)arg2.value) / 65536.0f), arg2.uvalue);
		resetArg2();
	}

	void fn_addr_done(){} // ????????????????

// actions functions
#pragma endregion 

	void feed(char c)
	{
		currentChar = c;

		if (got13 && currentChar == 10)
		{
			got13 = false;
			return;
		}

		if (currentChar == 13)
		{
			got13 = true;
			++line;
			column = 0;
		}
		else
		{
			if (currentChar == 10)
			{
				++line;
				column = 0;
			}
			else
			{
				++column;
			}
		}

		got13 = false;

		vmNextStatus = transitionsTable[currentChar][vmStatus];
		(this->*actionsTable[currentChar][vmStatus])();
		vmStatus = vmNextStatus;
	}

public:

	std::vector<instruction> program;

	Parser(){}

	Parser(char* sourcePath)
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
	}
};
