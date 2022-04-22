/**
 * @file matrix.c
 * @brief Source utility file to manage creation and destruction of arrays used.
 * @date 28/10/2021
 * @author dancs-dev
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "matrix.h"

// MPI often prefers 1D contiguous arrays, rather than arrays of arrays. So,
// create a 1D double array, and manipulate as needed into 2D array.
double* createDoubleMatrix(int dimension)
{
    double topRow, leftColumn, bottomRow, rightColumn;
    topRow = leftColumn = 1.0;
    bottomRow = rightColumn = 0.0;

    double* matrix = (double*) malloc(sizeof(double) * (unsigned long) dimension
    * (unsigned long) dimension);

    for (int i = 0; i < dimension; i++)
    {
        for (int ii = 0; ii < dimension; ii++)
        {
            if(i == 0)
            {
                matrix[(i * dimension) + ii] = topRow;
            }
            else if(ii == 0)
            {
                matrix[(i * dimension) + ii] = leftColumn;
            }
            else if(i == (dimension - 1))
            {
                matrix[(i * dimension) + ii] = bottomRow;
            }
            else if(ii == (dimension - 1))
            {
                matrix[(i * dimension) + ii] = rightColumn;
            }
            else
            {
                matrix[(i * dimension) + ii] = 0.0;
            }
        }
    }

    return matrix;
}

double getElemFromDoubleMatrix(double* matrix, int dimension, int x, int y)
{
    return matrix[(y * dimension) + x];
}

void freeDoubleMatrix(double *matrix)
{
    free(matrix);
}

void printDoubleMatrix(double *matrix, int dimension)
{
    for(int i = 0; i < dimension; i++)
    {
        for(int ii = 0; ii < dimension; ii++)
        {
            printf(" %f ", matrix[(i * dimension) + ii]);
        }
        printf("\n");
    }
}
