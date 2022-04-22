/**
 * @file main.c
 * @brief Main source file for distributed memory parallel program.
 * @date 29/12/2021
 * @author dancs-dev
 *
 * Compile using:
 * mpicc -Wall -Wextra -o distributed-memory.o main.c matrix.c
 *
 * Run using: mpirun ./distributed-memory.o -a ARRAYSIZE -p PRECISION
 * Example: mpirun ./distributed-memory.o -a 10 -p 0.001
 *
 * The array size must be greater than the number of processors used.
 *
 */

#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "matrix.h"


// Default settings
double PRECISION    = 0.001;
int ARRAY_DIMENSION = 30;


// Global variables (actually private to each process, as we are on distrubted
// memory using MPI).
double* doubleMatrix;

// Function declarations
double averageNeighbours(double* matrix, int x, int y);

int main(int argc, char** argv)
{
    while(true)
    {
        int c;
        c = getopt(argc, argv, "a:p:");
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
        }
    }

    int ok;
    // Initialize the MPI environment
    ok = MPI_Init(&argc, &argv);
    if (ok != MPI_SUCCESS)
    {
        printf("Error initialising MPI.\n");
        MPI_Abort(MPI_COMM_WORLD, ok);
    }

    // Get the number of processes
    int world_size;
    ok = MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    if (ok != MPI_SUCCESS)
    {
        printf("Error getting world size.\n");
        MPI_Abort(MPI_COMM_WORLD, ok);
    }

    // Get the rank of the process
    int world_rank;
    ok = MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    if (ok != MPI_SUCCESS)
    {
        printf("Error getting world rank.\n");
        MPI_Abort(MPI_COMM_WORLD, ok);
    }


    // Assign a default number of rows that each processor will process. For
    // example, if 10 processors and a 10x10 array, each processor will be
    // assigned 1 row.
    int numRowsPerProc = ARRAY_DIMENSION / world_size;

    // Allocate memory so ready to calculate and store the number of elements
    // each processor will accept and process. This was used for scatterv. Now
    // it is used to gather at the end, as well as allocate buffer sizes for the
    // rows per processor, as well as figure out what data to send/receive
    // to/from above/below processors via the MPI messaging system.
    int* sendCounts = (int*) malloc(sizeof(int) * world_size);
    int* recvCounts = (int*) malloc(sizeof(int) * world_size);
    int* sendDisplacements = (int*) malloc(sizeof(int) * world_size);
    int* recvDisplacements = (int*) malloc(sizeof(int) * world_size);

    // Loop to calculate the sendCounts and sendDisplacements arrays. All except
    // first/last elem will receive the same number of elements. The last
    // processor will receive the remaining rows. For example, 10 processors and
    // an 11x11 array, the last proc will relax for 2 rows (but will need to
    // also receive the row prior so can calculate average).
    for (int i = 0; i < world_size; i++)
    {
        // Send over the data the process will relax, as well as the rows below
        // and above.
        sendCounts[i] = (numRowsPerProc * ARRAY_DIMENSION) + (ARRAY_DIMENSION *
            2);
        recvCounts[i] = sendCounts[i] - (ARRAY_DIMENSION * 2);

        sendDisplacements[i] = (numRowsPerProc * ARRAY_DIMENSION * i) -
            ARRAY_DIMENSION;
        recvDisplacements[i] = (numRowsPerProc * ARRAY_DIMENSION * i);
        // For the last processor, only need to send one extra row; the row
        // prior.
        if (i == (world_size - 1))
        {
            numRowsPerProc = (ARRAY_DIMENSION % (world_size)) + numRowsPerProc;
            sendCounts[i] = (numRowsPerProc * ARRAY_DIMENSION) +
                ARRAY_DIMENSION;
            recvCounts[i] = sendCounts[i] - (ARRAY_DIMENSION);
        }
        // For the first processor, only need to send one extra row; the row
        // after.
        else if (i == 0)
        {
            sendCounts[i] = (numRowsPerProc * ARRAY_DIMENSION) +
                (ARRAY_DIMENSION);
            recvCounts[i] = sendCounts[i] - (ARRAY_DIMENSION);
            recvDisplacements[i] = ARRAY_DIMENSION;
            sendDisplacements[i] = i;
        }
    }

    // Re-calculate the numRowsPerProc to what it should be, depending on the
    // rank of the processor. It was modified in the loop above so must be
    // redone.
    numRowsPerProc = ARRAY_DIMENSION / world_size;
    if (world_rank == (world_size - 1))
    {
        numRowsPerProc = (ARRAY_DIMENSION % (world_size)) + numRowsPerProc;
    }
    else if (world_rank == 0)
    {
        numRowsPerProc--;
    }

    // Create the double matrix per processor. OK to replicate per processor as
    // not modified, only read from, until computation is finished and root
    // processor collates the results. Replication avoids communication overhead
    // of distributing it to each processor. As discussed in the report, this
    // could be optimised to reduce memory usage.
    doubleMatrix = createDoubleMatrix(ARRAY_DIMENSION);

    // Double matrix buffer is for storing input data received by each proc.
    double *doubleMatrixBuffer = (double*) malloc(sizeof(double) *
        sendCounts[world_rank]);
    double *doubleMatrixBufferCopy = (double*) malloc(sizeof(double) *
        sendCounts[world_rank]);

    // Initialise buffer with correct values.
    for(int i = 0; i < sendCounts[world_rank]; i++)
    {
        doubleMatrixBuffer[i] = doubleMatrix[sendDisplacements[world_rank] + i];
    }

    while(true)
    {
        // Distribute sections of double matrix to all processors.
        // Send prior rows.
        if (world_rank > 0)
        {
            ok = MPI_Send(doubleMatrixBuffer + ARRAY_DIMENSION, ARRAY_DIMENSION,
                MPI_DOUBLE, world_rank - 1, 0, MPI_COMM_WORLD);
            if (ok != MPI_SUCCESS)
            {
                printf("Error sending start rows to below processors.\n");
                MPI_Abort(MPI_COMM_WORLD, ok);
            }
        }

        // Receive ending rows
        if (world_rank < world_size - 1)
        {
            MPI_Status stat;
            ok = MPI_Recv(doubleMatrixBuffer + ((numRowsPerProc + 1) *
                ARRAY_DIMENSION), ARRAY_DIMENSION, MPI_DOUBLE, world_rank + 1,
                0, MPI_COMM_WORLD, &stat);
            if (ok != MPI_SUCCESS)
            {
                printf("Error receiving start rows from above processors.\n");
                MPI_Abort(MPI_COMM_WORLD, ok);
            }
        }

        // Minus 1 as proc count starts at 0.
        // Send ending rows
        if (world_rank < world_size - 1)
        {
            ok = MPI_Send(doubleMatrixBuffer + (numRowsPerProc *
            ARRAY_DIMENSION), ARRAY_DIMENSION, MPI_DOUBLE, world_rank + 1, 1,
            MPI_COMM_WORLD);
            if (ok != MPI_SUCCESS)
            {
                printf("Error sending end rows to above processors.\n");
                MPI_Abort(MPI_COMM_WORLD, ok);
            }
        }

        // Receive prior rows
        if (world_rank > 0)
        {
            MPI_Status stat;
            ok = MPI_Recv(doubleMatrixBuffer, ARRAY_DIMENSION, MPI_DOUBLE,
                world_rank - 1, 1, MPI_COMM_WORLD, &stat);
            if (ok != MPI_SUCCESS)
            {
                printf("Error receiving end rows from below processors.\n");
                MPI_Abort(MPI_COMM_WORLD, ok);
            }
        }

        // Copy to double matrix buffer copy. This is so we can relax the matrix
        // using averages calculated from double matrix buffer copy, and store
        // the relaxed iteration in double matrix buffer.
        memcpy(doubleMatrixBufferCopy, doubleMatrixBuffer, sizeof(double) *
            sendCounts[world_rank]);

        // Initialise flag to true. Set to false if difference between current
        // and new average is outside precision.
        int balanced = true;

        // Each proc loops through their buffer, starting with rows that they
        // are responsible for averaging. Remember, numRowsPerProc corresponds
        // to the raw number of rows they are working on, not including the
        // additional extra prior/ending rows needed.
        for(int i = 1; i < numRowsPerProc + 1; i++)
        {
            // If the last row (corresponding to the full matrix), then skip the
            // iteration of the loop. We do not edit the outer elements of the
            // array. This is required.
            if (world_rank == (world_size - 1) && i == (numRowsPerProc))
                continue;

            // Iterate between 1 and second from last element of each row. As
            // before, we do not edit the outer elements of the array.
            for(int ii = 1; ii < ARRAY_DIMENSION - 1; ii++)
            {
                double average = averageNeighbours(doubleMatrixBufferCopy, ii,
                    i);

                if (balanced && (fabs(average - getElemFromDoubleMatrix(
                    doubleMatrixBufferCopy, ARRAY_DIMENSION, ii, i))) >
                    PRECISION)
                {
                    balanced = false;
                }
                // Buffer to maintain integrity of double matrix buffer when
                // averaging (rather than editing double matrix buffer copy).
                doubleMatrixBuffer[(i * ARRAY_DIMENSION) + ii] = average;
            }
        }

        // Check the balance at the root processor. World size should not be too
        // big so probably fine and cheaper to create this on stack rather than
        // heap.
        int balancedArray[world_size];
        ok = MPI_Gather(&balanced, 1, MPI_INT, &balancedArray, 1,
            MPI_INT, 0, MPI_COMM_WORLD);
        if (ok != MPI_SUCCESS)
        {
            printf("Error gathering precision reached flags.\n");
            MPI_Abort(MPI_COMM_WORLD, ok);
        }

        int done = true;

        if (world_rank == 0)
        {
            for (int i = 0; i < world_size; i++)
            {
                if (balancedArray[i] != true)
                {
                    done = false;
                    // No need to check further if we know one part not yet
                    // balanced.
                    break;
                }
            }
        }

        // Broadcast whether or not precision reached.
        ok = MPI_Bcast(&done, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if (ok != MPI_SUCCESS)
        {
            printf("Error broadcasting precision reached status.\n");
            MPI_Abort(MPI_COMM_WORLD, ok);
        }

        if (done) break;
    }

    // Gather and update root double matrix with relaxed values from each proc.
    ok = MPI_Gatherv(doubleMatrixBuffer + ARRAY_DIMENSION,
        recvCounts[world_rank], MPI_DOUBLE, doubleMatrix, recvCounts,
        recvDisplacements, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    if (ok != MPI_SUCCESS)
    {
        printf("Error gathering solution.\n");
        MPI_Abort(MPI_COMM_WORLD, ok);
    }

    if(world_rank == 0)
    {
        printf("Result:\n");
        printDoubleMatrix(doubleMatrix, ARRAY_DIMENSION);
    }

    // Finalize the MPI environment.
    ok = MPI_Finalize();
    if (ok != MPI_SUCCESS)
    {
        printf("Error finalising MPI environment.\n");
        MPI_Abort(MPI_COMM_WORLD, ok);
    }
}



double averageNeighbours(double* matrix, int x, int y)
{
    double avg;
    avg = getElemFromDoubleMatrix(matrix, ARRAY_DIMENSION, x - 1, y) +
        getElemFromDoubleMatrix(matrix, ARRAY_DIMENSION, x + 1, y) +
        getElemFromDoubleMatrix(matrix, ARRAY_DIMENSION, x, y + 1) +
        getElemFromDoubleMatrix(matrix, ARRAY_DIMENSION, x, y - 1);
    avg = avg / 4.0;
    return avg;
}
