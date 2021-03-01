# Linux C Memory Manager
This project implements a version of malloc() and free() functions from the #stdlib from scratch.

## How to run project
gcc -g -c memoryPageAllocation.c -o mm.o <br>
gcc -g -c test.c -o test.o <br>
gcc -g -c glthreads/glthread.c -o glthreads/glthread.o <br>
gcc -g mm.o glthreads/glthread.o test.o -o test <br>

## How to add to other projects
1. Download or clone repository.
2. Extract files: **mm.h**, **memoryPageAllocation.c**, **uapi_mm.h** files and **glthreads** folder on your project folder.
4. Incluce **#include "uapi_mm.h"** on your main header.
5. call function **XCALLOC()** to allocate structures and **XFREE()** to free memory space.
