#include "AsmParser.h"

bool needsLabel(int opcode)
{
	return opcode > 41; // trivial solution for now
}

void Parser::resetOpName()
{
	memset(&(nameStorage), 0, NAMEMAXSIZE);
	nameStorageIdx = 0;
}

void Parser::resetArg1()
{
	memset(&(arg1Storage), 0, ARG1MAXSIZE);
	arg1StorageIdx = 0;
}

void Parser::resetArg2()
{
	arg2 = 0;
	arg2decimalDiv = 10;
	arg2neg = false;
}

#pragma region parser FSM actions implementation

void Parser::fn_nop(){}

void Parser::fn_err()
{
	dbgPrintf("Error %d on line %d, col %d", 1, line, column);
}

void Parser::fn_name_stor()
{
	nameStorage[(nameStorageIdx)++] = currentChar;
}

void Parser::fn_op_done()
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
				dbgPrintf("unknown label '%s' at: %d\n", nameStorage, program.size() - 1);
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
				identifiers.insert({ arg1Storage, regCount });
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

void Parser::fn_lbl_done()
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

void Parser::fn_arg1_stor()
{
	arg1Storage[arg1StorageIdx++] = currentChar;
}

void Parser::fn_arg1_done()
{
	dbgPrintf("arg1 found: '%s'\n", arg1Storage);

	resetArg1();
}

void Parser::fn_n_stor()
{
	// store before point
	arg2.shortValue = arg2.shortValue * 10 + (int)(currentChar - '0');
}

void Parser::fn_n_stor_f()
{
	arg2.fract = arg2.fract + (((unsigned int)(currentChar - '0')) << 16) / arg2decimalDiv;
	arg2decimalDiv *= 10;
}

void Parser::fn_neg_n()
{
	arg2neg = true;
}

void Parser::fn_addr_stor()
{
	arg2.uvalue = arg2.uvalue * 10 + (int)(currentChar - '0');
}

void Parser::fn_n_done()
{
	if (arg2neg)
	{
		arg2.shortValue = -arg2.shortValue;
	}

	dbgPrintf("arg2 found: %f  (@%lu)\n", (((float)arg2.value) / 65536.0f), arg2.uvalue);
	resetArg2();
}

void Parser::fn_addr_done(){} // ????????????????

// parser FSM actions implementation
#pragma endregion 

#pragma region actions table
	const Parser::pActionFn Parser::actionsTable[128][11] = {
		//          PS_skip_WS_1	PS_comment		PS_label		PS_error	PS_coll_op		PS_skip_WS_2	PS_coll_arg1	PS_skip_WS_3	PS_arg_2_addr	PS_arg_2_n		PS_arg_2_n_f
		/* 000   */{ &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 001 ? */{ &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 002 ? */{ &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 003 ? */{ &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 004 ? */{ &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 005 ? */{ &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 006 ? */{ &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 007   */{ &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 008   */{ &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 009   */{ &Parser::fn_nop, &Parser::fn_nop, &Parser::fn_lbl_done, &Parser::fn_err, &Parser::fn_op_done, &Parser::fn_nop, &Parser::fn_arg1_done, &Parser::fn_nop, &Parser::fn_addr_done, &Parser::fn_n_done, &Parser::fn_n_done },
		/* 010   */{ &Parser::fn_nop, &Parser::fn_nop, &Parser::fn_lbl_done, &Parser::fn_err, &Parser::fn_op_done, &Parser::fn_nop, &Parser::fn_arg1_done, &Parser::fn_nop, &Parser::fn_addr_done, &Parser::fn_n_done, &Parser::fn_n_done },
		/* 011 ? */{ &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 012 ? */{ &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 013   */{ &Parser::fn_nop, &Parser::fn_nop, &Parser::fn_lbl_done, &Parser::fn_err, &Parser::fn_op_done, &Parser::fn_nop, &Parser::fn_arg1_done, &Parser::fn_nop, &Parser::fn_addr_done, &Parser::fn_n_done, &Parser::fn_n_done },
		/* 014 ? */{ &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 015 ? */{ &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 016 ? */{ &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 017 ? */{ &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 018 ? */{ &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 019 ? */{ &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 020 ¶ */{ &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 021 § */{ &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 022 ? */{ &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 023 ? */{ &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 024 ? */{ &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 025 ? */{ &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 026 ? */{ &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 027 ? */{ &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 028 ? */{ &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 029 ? */{ &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 030 ? */{ &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 031 ? */{ &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 032   */{ &Parser::fn_nop, &Parser::fn_nop, &Parser::fn_lbl_done, &Parser::fn_err, &Parser::fn_op_done, &Parser::fn_nop, &Parser::fn_arg1_done, &Parser::fn_nop, &Parser::fn_addr_done, &Parser::fn_n_done, &Parser::fn_n_done },
		/* 033 ! */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 034 " */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 035 # */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 036 $ */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 037 % */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 038 & */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 039 ' */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 040 ( */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 041 ) */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 042 * */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 043 + */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 044 , */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 045 - */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_neg_n, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 046 . */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err },
		/* 047 / */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 048 0 */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_n_stor, &Parser::fn_addr_stor, &Parser::fn_n_stor, &Parser::fn_n_stor_f },
		/* 049 1 */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_n_stor, &Parser::fn_addr_stor, &Parser::fn_n_stor, &Parser::fn_n_stor_f },
		/* 050 2 */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_n_stor, &Parser::fn_addr_stor, &Parser::fn_n_stor, &Parser::fn_n_stor_f },
		/* 051 3 */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_n_stor, &Parser::fn_addr_stor, &Parser::fn_n_stor, &Parser::fn_n_stor_f },
		/* 052 4 */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_n_stor, &Parser::fn_addr_stor, &Parser::fn_n_stor, &Parser::fn_n_stor_f },
		/* 053 5 */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_n_stor, &Parser::fn_addr_stor, &Parser::fn_n_stor, &Parser::fn_n_stor_f },
		/* 054 6 */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_n_stor, &Parser::fn_addr_stor, &Parser::fn_n_stor, &Parser::fn_n_stor_f },
		/* 055 7 */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_n_stor, &Parser::fn_addr_stor, &Parser::fn_n_stor, &Parser::fn_n_stor_f },
		/* 056 8 */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_n_stor, &Parser::fn_addr_stor, &Parser::fn_n_stor, &Parser::fn_n_stor_f },
		/* 057 9 */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_n_stor, &Parser::fn_addr_stor, &Parser::fn_n_stor, &Parser::fn_n_stor_f },
		/* 058 : */{ &Parser::fn_nop, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 059 ; */{ &Parser::fn_nop, &Parser::fn_nop, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_nop, &Parser::fn_nop, &Parser::fn_nop, &Parser::fn_nop, &Parser::fn_nop, &Parser::fn_nop, &Parser::fn_nop },
		/* 060 < */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 061 = */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 062 > */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 063 ? */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 064 @ */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 065 A */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 066 B */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 067 C */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 068 D */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 069 E */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 070 F */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 071 G */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 072 H */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 073 I */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 074 J */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 075 K */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 076 L */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 077 M */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 078 N */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 079 O */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 080 P */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 081 Q */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 082 R */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 083 S */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 084 T */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 085 U */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 086 V */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 087 W */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 088 X */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 089 Y */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 090 Z */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 091 [ */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 092 \ */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 093 ] */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 094 ^ */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 095 _ */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 096 ` */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 097 a */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 098 b */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 099 c */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 100 d */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 101 e */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 102 f */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 103 g */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 104 h */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 105 i */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 106 j */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 107 k */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 108 l */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 109 m */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 110 n */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 111 o */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 112 p */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 113 q */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 114 r */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 115 s */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 116 t */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 117 u */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 118 v */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 119 w */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 120 x */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 121 y */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 122 z */{ &Parser::fn_name_stor, &Parser::fn_nop, &Parser::fn_name_stor, &Parser::fn_err, &Parser::fn_name_stor, &Parser::fn_arg1_stor, &Parser::fn_arg1_stor, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 123 { */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 124 | */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 125 } */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 126 ~ */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err },
		/* 127 ? */{ &Parser::fn_err, &Parser::fn_nop, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err, &Parser::fn_err }
	};
// actions table
#pragma endregion

#pragma region transitions table
	const parserStates Parser::transitionsTable[128][11] = {
		//          PS_skip_WS_1	PS_comment		PS_label		PS_error	PS_coll_op		PS_skip_WS_2	PS_coll_arg1	PS_skip_WS_3	PS_arg_2_addr	PS_arg_2_n		PS_arg_2_n_f
		/* 000   */{ PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 001 ? */{ PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 002 ? */{ PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 003 ? */{ PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 004 ? */{ PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 005 ? */{ PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 006 ? */{ PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 007   */{ PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 008   */{ PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 009   */{ PS_skip_WS_1, PS_comment, PS_error, PS_error, PS_skip_WS_2, PS_skip_WS_2, PS_skip_WS_3, PS_skip_WS_3, PS_skip_WS_1, PS_skip_WS_1, PS_error },
		/* 010   */{ PS_skip_WS_1, PS_skip_WS_1, PS_skip_WS_1, PS_error, PS_skip_WS_1, PS_skip_WS_1, PS_skip_WS_1, PS_skip_WS_1, PS_skip_WS_1, PS_skip_WS_1, PS_skip_WS_1 },
		/* 011 ? */{ PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 012 ? */{ PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 013   */{ PS_skip_WS_1, PS_skip_WS_1, PS_skip_WS_1, PS_error, PS_skip_WS_1, PS_skip_WS_1, PS_skip_WS_1, PS_skip_WS_1, PS_skip_WS_1, PS_skip_WS_1, PS_skip_WS_1 },
		/* 014 ? */{ PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 015 ? */{ PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 016 ? */{ PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 017 ? */{ PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 018 ? */{ PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 019 ? */{ PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 020 ¶ */{ PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 021 § */{ PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 022 ? */{ PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 023 ? */{ PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 024 ? */{ PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 025 ? */{ PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 026 ? */{ PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 027 ? */{ PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 028 ? */{ PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 029 ? */{ PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 030 ? */{ PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 031 ? */{ PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 032   */{ PS_skip_WS_1, PS_comment, PS_error, PS_error, PS_skip_WS_2, PS_skip_WS_2, PS_skip_WS_3, PS_skip_WS_3, PS_skip_WS_1, PS_skip_WS_1, PS_error },
		/* 033 ! */{ PS_error, PS_comment, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 034 " */{ PS_error, PS_comment, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 035 # */{ PS_error, PS_comment, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 036 $ */{ PS_error, PS_comment, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 037 % */{ PS_error, PS_comment, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 038 & */{ PS_error, PS_comment, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 039 ' */{ PS_error, PS_comment, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 040 ( */{ PS_error, PS_comment, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 041 ) */{ PS_error, PS_comment, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 042 * */{ PS_error, PS_comment, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 043 + */{ PS_error, PS_comment, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 044 , */{ PS_error, PS_comment, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 045 - */{ PS_error, PS_comment, PS_error, PS_error, PS_error, PS_error, PS_error, PS_arg_2_n, PS_error, PS_error, PS_error },
		/* 046 . */{ PS_error, PS_comment, PS_error, PS_error, PS_error, PS_error, PS_error, PS_arg_2_n, PS_error, PS_arg_2_n_f, PS_error },
		/* 047 / */{ PS_error, PS_comment, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 048 0 */{ PS_error, PS_comment, PS_label, PS_error, PS_error, PS_coll_arg1, PS_coll_arg1, PS_arg_2_n, PS_arg_2_addr, PS_arg_2_n, PS_arg_2_n_f },
		/* 049 1 */{ PS_error, PS_comment, PS_label, PS_error, PS_error, PS_coll_arg1, PS_coll_arg1, PS_arg_2_n, PS_arg_2_addr, PS_arg_2_n, PS_arg_2_n_f },
		/* 050 2 */{ PS_error, PS_comment, PS_label, PS_error, PS_error, PS_coll_arg1, PS_coll_arg1, PS_arg_2_n, PS_arg_2_addr, PS_arg_2_n, PS_arg_2_n_f },
		/* 051 3 */{ PS_error, PS_comment, PS_label, PS_error, PS_error, PS_coll_arg1, PS_coll_arg1, PS_arg_2_n, PS_arg_2_addr, PS_arg_2_n, PS_arg_2_n_f },
		/* 052 4 */{ PS_error, PS_comment, PS_label, PS_error, PS_error, PS_coll_arg1, PS_coll_arg1, PS_arg_2_n, PS_arg_2_addr, PS_arg_2_n, PS_arg_2_n_f },
		/* 053 5 */{ PS_error, PS_comment, PS_label, PS_error, PS_error, PS_coll_arg1, PS_coll_arg1, PS_arg_2_n, PS_arg_2_addr, PS_arg_2_n, PS_arg_2_n_f },
		/* 054 6 */{ PS_error, PS_comment, PS_label, PS_error, PS_error, PS_coll_arg1, PS_coll_arg1, PS_arg_2_n, PS_arg_2_addr, PS_arg_2_n, PS_arg_2_n_f },
		/* 055 7 */{ PS_error, PS_comment, PS_label, PS_error, PS_error, PS_coll_arg1, PS_coll_arg1, PS_arg_2_n, PS_arg_2_addr, PS_arg_2_n, PS_arg_2_n_f },
		/* 056 8 */{ PS_error, PS_comment, PS_label, PS_error, PS_error, PS_coll_arg1, PS_coll_arg1, PS_arg_2_n, PS_arg_2_addr, PS_arg_2_n, PS_arg_2_n_f },
		/* 057 9 */{ PS_error, PS_comment, PS_label, PS_error, PS_error, PS_coll_arg1, PS_coll_arg1, PS_arg_2_n, PS_arg_2_addr, PS_arg_2_n, PS_arg_2_n_f },
		/* 058 : */{ PS_label, PS_comment, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 059 ; */{ PS_comment, PS_comment, PS_comment, PS_error, PS_comment, PS_comment, PS_comment, PS_comment, PS_comment, PS_comment, PS_comment },
		/* 060 < */{ PS_error, PS_comment, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 061 = */{ PS_error, PS_comment, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 062 > */{ PS_error, PS_comment, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 063 ? */{ PS_error, PS_comment, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 064 @ */{ PS_error, PS_comment, PS_error, PS_error, PS_error, PS_error, PS_error, PS_arg_2_addr, PS_error, PS_error, PS_error },
		/* 065 A */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 066 B */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 067 C */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 068 D */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 069 E */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 070 F */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 071 G */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 072 H */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 073 I */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 074 J */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 075 K */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 076 L */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 077 M */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 078 N */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 079 O */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 080 P */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 081 Q */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 082 R */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 083 S */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 084 T */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 085 U */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 086 V */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 087 W */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 088 X */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 089 Y */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 090 Z */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 091 [ */{ PS_error, PS_comment, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 092 \ */{ PS_error, PS_comment, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 093 ] */{ PS_error, PS_comment, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 094 ^ */{ PS_error, PS_comment, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 095 _ */{ PS_error, PS_comment, PS_label, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 096 ` */{ PS_error, PS_comment, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 097 a */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 098 b */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 099 c */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 100 d */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 101 e */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 102 f */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 103 g */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 104 h */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 105 i */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 106 j */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 107 k */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 108 l */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 109 m */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 110 n */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 111 o */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 112 p */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 113 q */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 114 r */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 115 s */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 116 t */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 117 u */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 118 v */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 119 w */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 120 x */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 121 y */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 122 z */{ PS_coll_op, PS_comment, PS_label, PS_error, PS_coll_op, PS_coll_arg1, PS_coll_arg1, PS_error, PS_error, PS_error, PS_error },
		/* 123 { */{ PS_error, PS_comment, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 124 | */{ PS_error, PS_comment, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 125 } */{ PS_error, PS_comment, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 126 ~ */{ PS_error, PS_comment, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
		/* 127 ? */{ PS_error, PS_comment, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error, PS_error },
	};

// transitions table
#pragma endregion 

#pragma region opNameCodeMap init
	const std::map<std::string, ushort> Parser::opNameCodeMap =
	{
		// MEMORY OPS / INIT
		{ "NOP", 0 },
		{ "ERR", 1 },
		{ "INIT", 2 },
		{ "LOAD", 3 },
		{ "LOADI", 4 },
		{ "STOR", 5 },
		{ "MOV", 6 },
		{ "CMP", 7 },

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
	};
	//  opNameCodeMap init
#pragma endregion