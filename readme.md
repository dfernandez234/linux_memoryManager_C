gcc -g -c memoryPageAllocation.c -o mm.o <br>
gcc -g -c test.c -o test.o <br>
gcc -g -c glthreads/glthread.c -o glthreads/glthread.o <br>
gcc -g mm.o glthreads/glthread.o test.o -o test <br>
