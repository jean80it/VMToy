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

	void resetOpName();

	void resetArg1();

	void resetArg2();

#pragma region parser FSM actions declaration

	void fn_nop();

	void fn_err();

	void fn_name_stor();

	void fn_op_done();

	void fn_lbl_done();

	void fn_arg1_stor();

	void fn_arg1_done();

	void fn_n_stor();

	void fn_n_stor_f();

	void fn_neg_n();

	void fn_addr_stor();

	void fn_n_done();

	void fn_addr_done(); // ????????????????

// parser FSM actions declaration
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
