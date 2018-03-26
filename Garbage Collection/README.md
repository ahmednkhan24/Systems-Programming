## Objectives
* Get experience working with existing C code
* Get experience working with operating system defined protocols/systems
* Learn about garbage collection

In this homework, I built a basic, conservative garbage collector for C programs. Starting from the set of root pointers present in the stack and global variables, I traverse the “object graph” (in the case of malloc, a chunk graph) to find all chunks reachable from the root. These are marked using the third lowest order bit in the size field of each chunk.

Program passes all tests in main.c - that is, it frees all garbage memory.

Runtime performance is not a major goal for this assignment. However, the program should finish well within 2 seconds.