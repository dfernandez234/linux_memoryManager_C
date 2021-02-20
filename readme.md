gcc -g -c memoryPageAllocation.c -o mm.o
gcc -g -c test.c -o test.o
gcc -g -c glthreads/glthread.c -o glthreads/glthread.o
gcc -g mm.o glthreads/glthread.o test.o -o test
