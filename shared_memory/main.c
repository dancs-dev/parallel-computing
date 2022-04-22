/**
 * @file main.c
 * @brief Main source file for shared memory parallel program.
 * @date 28/10/2021
 * @author dancs-dev
 *
 * Compile using:
 * gcc -o shared-memory.o main.c matrix.c -lpthread -Wall -Wextra -Wconversion
 *
 * This links the pthread library, as required, and displays maximum warnings.
 *
 * Run using: ./shared-memory.out -a ARRAYSIZE -p PRECISION -w NUMBEROFTHREADS
 *
 */


// Standard header includes
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>


// Project header includes
#include "matrix.h"


// To enable protected reads, uncomment the below line:
// It will lock more than one column at a time. I believe this is safe, but it
// has not been thoroughly tested. Intended usage: this should be disabled.
// #define PROTECTED_READS

// The below flag is used to setup the program for testing with the Python
// script. It prints out the double matrix after each thread finishes.
// #define TEST_MODE


// Default settings
double PRECISION    = 0.001;
int ARRAY_DIMENSION = 4;
int WORKERS         = 1;


// Global variables
double** doubleMatrix;
pthread_mutex_t* mutexArray;

#ifdef TEST_MODE
pthread_mutex_t printThread;
#endif

// Function declarations
void relaxationWorker(int *tid);
double averageNeighbours(double** matrix, int x, int y);
void lockMutexes(pthread_mutex_t* array, int row);
void unlockMutexes(pthread_mutex_t* array, int row);


// Function definitions
int main(int argc, char **argv)
{
    while(true)
    {
        int c;
        c = getopt(argc, argv, "a:p:w:");
        if (c == -1)
        {
            break;
        }
        switch(c)
        {
            case 'a':
                ARRAY_DIMENSION = atoi(optarg);
                if (ARRAY_DIMENSION < 3)
                {
                    return -1;
                }
                printf("Set array dimension to: %d\n", ARRAY_DIMENSION);
                break;

            case 'p':
                PRECISION = atof(optarg);
                if (PRECISION < 0.0 || PRECISION > 1.0)
                {
                    return -1;
                }
                printf("Set precision to: %f\n", PRECISION);
                break;

            case 'w':
                WORKERS = atoi(optarg);
                if (WORKERS < 1)
                {
                    return -1;
                }
                printf("Set number of workers to: %d\n", WORKERS);
                break;
        }
    }

    doubleMatrix = createDoubleMatrix(ARRAY_DIMENSION);
    mutexArray = createMutexArray(ARRAY_DIMENSION);

    #ifdef TEST_MODE
    if (pthread_mutex_init(&printThread, NULL) != 0)
    {
        perror("pthread_join() error");
        exit(-1);
    }
    #endif

    // Create a pthread type pointer array.
    pthread_t workers[WORKERS];

    // clock_t start, end;
    // double cpuTimeUsed;

    // start = clock();
    // printDoubleMatrix(doubleMatrix, ARRAY_DIMENSION);

    for (int i = 0; i < WORKERS; i++)
    {
        // Call create, takes:
        // address of pthread,
        // array of attributes (or NULL for default),
        // reference to a function,
        // input to worker thread function (passed in as void pointer).
        if (pthread_create((void *)&workers[i], NULL,
            (void*(*)(void*))relaxationWorker, (void *)&i) != 0)
        {
            perror("pthread_create() error");
            return -1;
        }
    }

    for (int i = 0; i < WORKERS; i++)
    {
        if (pthread_join(workers[i], NULL) != 0)
        {
            perror("pthread_join() error");
            return -1;
        }
    }

    // end = clock();

    #ifndef TEST_MODE
    printf("\nResult:\n");
    printDoubleMatrix(doubleMatrix, ARRAY_DIMENSION);
    #endif
    #ifdef PROTECTED_READS
    printf("Protected reads were enabled.\n");
    #endif

    // cpuTimeUsed = ((double) (end - start)) / CLOCKS_PER_SEC;
    // printf("\nCPU time used: %fs\n", cpuTimeUsed);

    freeDoubleMatrix(doubleMatrix, ARRAY_DIMENSION);
    freeMutexArray(mutexArray, ARRAY_DIMENSION);

    return 0;
}

// This method should guarantee a precision of at least the precision that is
// set. It may be more precise as the threads finish. I.e., if precision is
// reached, and there are 20 threads running, all of the threads will finish
// their computations (through the whole array) before potentially returning. If
// in the iteration, a calculation was done that was out of a precision, it will
// do another precision before returning. This may seem wasteful, but the
// overhead compared to other solutions is actually reasonably small.
void relaxationWorker(int *tid)
{
    bool balanced = true;

    while (true)
    {
        for (int x = 1; x < ARRAY_DIMENSION - 1; x++)
        {
            // Initial design idea involved protecting each value in matrix with
            // a mutex. That gave a rubbish efficiency. Let's protect each row
            // instead: we may spend some more time blocked, but at least we can
            // avoid the tremendous overhead from setting the lock for each
            // element in the 2D array.
            lockMutexes(mutexArray, x);
            for (int y = 1; y < ARRAY_DIMENSION - 1; y++)
            {
                double average = averageNeighbours(doubleMatrix, x, y);
                if (balanced)
                {
                    if ((average - doubleMatrix[x][y]) > PRECISION)
                    {
                        balanced = false;
                    }
                }
                doubleMatrix[x][y] = average;
            }
            unlockMutexes(mutexArray, x);
        }
        if (balanced)
        {
            break;
        }
        balanced = true;
    }
    #ifdef TEST_MODE
    // We need to use a mutex to protect when print, otherwise we will have
    // multiple threads printing simultaneously and the output will be rubbish.
    if (pthread_mutex_lock(&printThread) != 0)
    {
        perror("pthread_mutex_lock() error");
        exit(-1);
    }
    printf("Thread\n");
    printDoubleMatrix(doubleMatrix, ARRAY_DIMENSION);
    if (pthread_mutex_unlock(&printThread) != 0)
    {
        perror("pthread_mutex_unlock() error");
        exit(-1);
    }
    #endif
}

double averageNeighbours(double** matrix, int x, int y)
{
    double avg;
    avg = matrix[x - 1][y] + matrix[x + 1][y] + matrix[x][y - 1] +
        matrix[x][y + 1];
    avg = avg / 4.0;
    return avg;
}

void lockMutexes(pthread_mutex_t* array, int row)
{
    #ifdef PROTECTED_READS
    if (pthread_mutex_lock(&array[row - 1]) != 0)
    {
        perror("pthread_mutex_lock() error");
        exit(-1);
    }
    #endif

    if (pthread_mutex_lock(&array[row]) != 0)
    {
        perror("pthread_mutex_lock() error");
        exit(-1);
    }

    #ifdef PROTECTED_READS
    if (pthread_mutex_lock(&array[row + 1]) != 0)
    {
        perror("pthread_mutex_lock() error");
        exit(-1);
    }
    #endif
}

void unlockMutexes(pthread_mutex_t* array, int row)
{
    // If we use protected reads, ensure we release the locks in reverse order
    // from which we used them.
    #ifdef PROTECTED_READS
    if (pthread_mutex_unlock(&array[row + 1]) != 0)
    {
        perror("pthread_mutex_unlock() error");
        exit(-1);
    }
    #endif
    if (pthread_mutex_unlock(&array[row]) != 0)
    {
        perror("pthread_mutex_unlock() error");
        exit(-1);
    }
    #ifdef PROTECTED_READS
    if (pthread_mutex_unlock(&array[row - 1]) != 0)
    {
        perror("pthread_mutex_unlock() error");
        exit(-1);
    }
    #endif
}
