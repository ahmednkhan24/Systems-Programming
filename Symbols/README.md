## Objectives
* Learn how to use the readelf tool to find out about symbols in object files
* Get hands on experience with the symbol table of file, and learn more about symbol tables

In this assignment, I filled hw1.c with code which:

causes my netid (and nothing else) to be printed on the first line of output when the program is run.
cause gcc -Wall hw1.c to issue zero warnings (and zero errors, duh).
cause the output of readelf -s hw1.o to have identical values in the bolded sections of the output below:
Symbol table ‘.symtab’ contains 18 entries:

Num:	Value	Size	Type	Bind	Vis	Ndx	Name
0:	0000000000000000	0	NOTYPE	LOCAL	DEFAULT	UND	 
1:	0000000000000000	0	FILE	LOCAL	DEFAULT	ABS	hw1.c
2:	0000000000000000	0	SECTION	LOCAL	DEFAULT	1	 
3:	0000000000000000	0	SECTION	LOCAL	DEFAULT	3	 
4:	0000000000000000	0	SECTION	LOCAL	DEFAULT	4	 
5:	0000000000000000	0	SECTION	LOCAL	DEFAULT	5	 
6:	0000000000000000	0	SECTION	LOCAL	DEFAULT	7	 
7:	0000000000000000	0	SECTION	LOCAL	DEFAULT	8	 
8:	0000000000000000	0	SECTION	LOCAL	DEFAULT	6	 
9:	0000000000000000	81	FUNC	GLOBAL	DEFAULT	1	main
10:	0000000000000000	0	NOTYPE	GLOBAL	DEFAULT	UND	printf
11:	0000000000000051	38	FUNC	GLOBAL	DEFAULT	1	all
12:	0000000000000004	4	OBJECT	GLOBAL	DEFAULT	COM	your
13:	0000000000000077	38	FUNC	GLOBAL	DEFAULT	1	cs361
14:	0000000000000004	1	OBJECT	GLOBAL	DEFAULT	3	are
15:	0000000000000004	4	OBJECT	GLOBAL	DEFAULT	COM	belong
16:	0000000000000000	4	OBJECT	GLOBAL	DEFAULT	4	to
17:	0000000000000008	37	OBJECT	GLOBAL	DEFAULT	COM	us