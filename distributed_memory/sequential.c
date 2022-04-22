/**
 * @file sequential.c
 * @brief Main sequential source file for use in automatic testing of parallel
 * version.
 * @date 03/01/2022
 * @author dancs-dev
 *
 * Compile using:
 * gcc -o sequential.o sequential.c matrix_sequential.c
 *
 * Run using: ./sequential.o -a ARRAYSIZE -p PRECISION
 * Example: ./sequential.o -a 4 -p 0.001
 *
 */


// Standard header includes
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <math.h>


// Project header includes
#include "matrix_sequential.h"


// Default settings
double PRECISION    = 0.001;
int ARRAY_DIMENSION = 4;


// Global variables
double** doubleMatrix;
double** doubleMatrixCopy;

// Function declarations
void relaxation();
double averageNeighbours(double** matrix, int x, int y);

// Function definitions
int main(int argc, char **argv)
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

    doubleMatrix = createDoubleMatrix(ARRAY_DIMENSION);
    doubleMatrixCopy = createDoubleMatrix(ARRAY_DIMENSION);

    relaxation();

    printf("\nResult:\n");
    printDoubleMatrix(doubleMatrix, ARRAY_DIMENSION);

    freeDoubleMatrix(doubleMatrix, ARRAY_DIMENSION);
    freeDoubleMatrix(doubleMatrixCopy, ARRAY_DIMENSION);

    return 0;
}

void relaxation()
{
    bool balanced = true;


    while (true)
    {
        for(int i = 0; i < ARRAY_DIMENSION; i++)
        {
            memcpy(doubleMatrixCopy[i], doubleMatrix[i], sizeof(double) *
                ARRAY_DIMENSION);
        }

        for (int x = 1; x < ARRAY_DIMENSION - 1; x++)
        {
            for (int y = 1; y < ARRAY_DIMENSION - 1; y++)
            {
                double average = averageNeighbours(doubleMatrixCopy, x, y);
                if (balanced)
                {
                    if (fabs(average - doubleMatrix[x][y]) > PRECISION)
                    {
                        balanced = false;
                    }
                }
                doubleMatrix[x][y] = average;
            }
        }
        if (balanced)
        {
            break;
        }
        balanced = true;
    }
}

double averageNeighbours(double** matrix, int x, int y)
{
    double avg;
    avg = matrix[x - 1][y] + matrix[x + 1][y] + matrix[x][y - 1] +
        matrix[x][y + 1];
    avg = avg / 4.0;
    return avg;
}
