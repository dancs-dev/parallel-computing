# Parallel Computing

Adventures in parallel computing using pthreads (shared memory) and MPI (distributed memory) during University.
These projects were very interesting and insightful.
The programs both perform a large computation (relaxing a 2D matrix).

## Shared memory

### How to run

Using gcc:
1. Build using `gcc -o shared-memory.out main.c matrix.c -lpthread -Wall -Wextra -Wconversion`.
1. Run using `./shared-memory.out -a ARRAYSIZE -p PRECISION -w NUMBEROFTHREADS`.

## Distributed memory

### How to run

Using mpicc:
1. Build using `mpicc -Wall -Wextra -o distributed-memory.out main.c matrix.c`.
1. Run using `mpirun ./distributed-memory.out -a ARRAYSIZE -p PRECISION`.
