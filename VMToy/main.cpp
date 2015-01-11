#define DEBUG
#include "AsmParser.h"
#include "VM.h"

using namespace std;

char* printBin(byte b)
{
	//static char res[9];
	char* res = (char*)malloc(9);
	res[8] = 0;
	for (int i = 0; i<8; ++i) // write leading zeroes, too
	{
		res[i] = (b>>7)+'0';
		b<<=1;
	}
	return res;
}


int main(int argn, char** argv)
{
	VMState s;
	//op_init(s,0,FP32(400));
	//op_loadi(s,0,FP32(3,0)); // A = 3
	//op_loadi(s,1,FP32(5,0)); // B = 5
	//op_sin(s,2,FP32(34314)); 
	//op_add(s,0, FP32(0,0,1,0)); // A = A + B 
	//op_stor(s,0,FP32(0,0,0,0)); // store (just a byte!) in mem[0]
	//op_trm(s,0,FP32());


	Parser p("test.jasm");
	VM vm(p.program);

	while (!vm.state.term)
	{
		vm.execStep();
	}

	getchar();
}

