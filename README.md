# README #

Simple implementation of a virtual machine using indirection of opcodes to functions through function pointers (i.e. no compilation, no jit, just interpretation of intermediate bytecode )

VM: the virtual machine
AsmParser: a simple class reading text (containing a  made-up asm syntax) and spitting out binary to be interpreted.