#include "dbgUtils.hpp"
#include "ASMParser_impl.hpp"
#include "VM.hpp"

using namespace std;

int main(int argn, char** argv)
{
	ASMParser_Impl p("test.jasm");
	VM vm(p.program);

	while (!vm.state.term)
	{
		vm.execStep();
	}

	getchar();
}

