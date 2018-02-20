## Objectives
* Learn how to use the readelf tool to find out about symbols in object files
* Get hands on experience with the symbol table of file, and learn more about symbol tables

In this assignment, I filled hw1.c with code which:

causes my netid (and nothing else) to be printed on the first line of output when the program is run.
cause gcc -Wall hw1.c to issue zero warnings (and zero errors, duh).
cause the output of readelf -s hw1.o to have identical values in the bolded sections of the output below:
Symbol table ‘.symtab’ contains 18 entries:

