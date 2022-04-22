/**
 * @file matrix.c
 * @brief Source utility file to manage creation and destruction of arrays used.
 * @date 28/10/2021
 * @author dancs-dev
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "matrix.h"

double** createDoubleMatrix(int dimension)
{
    double topRow, leftColumn, bottomRow, rightColumn;
    topRow = leftColumn = 1.0;
    bottomRow = rightColumn = 0.0;

    double** matrix = (double**) malloc((unsigned long) dimension *
        sizeof(double*));

    for (int i = 0; i < dimension; i++)
    {
        matrix[i] = (double *) malloc((unsigned long) dimension * 
            sizeof(double));
    }

    for (int i = 0; i < dimension; i++)
    {
        for (int ii = 0; ii < dimension; ii++)
        {
            matrix[i][ii] = 0.0;
        }
    }

    for (int i = 0; i < dimension; i++)
    {
        matrix[0][i] = topRow;
        matrix[i][0] = leftColumn;
        matrix[dimension - 1][i] = bottomRow;
        matrix[i][dimension - 1] = rightColumn;
    }
    return matrix;
}

pthread_mutex_t* createMutexArray(int dimension)
{
    pthread_mutex_t* matrix = (pthread_mutex_t*) malloc((unsigned long)
        dimension * sizeof(pthread_mutex_t));

    for (int i = 0; i < dimension; i++)
    {
        if (pthread_mutex_init(&matrix[i], NULL) != 0)
        {
            perror("pthread_join() error");
            exit(-1);
        }
    }

    return matrix;
}

void freeDoubleMatrix(double **matrix, int dimension)
{
    // Free each row.
    for (int i = 0; i < dimension; i++)
    {
        free(matrix[i]);
    }

    // Free pointers to the rows.
    free(matrix);
}

void freeMutexArray(pthread_mutex_t *array, int dimension)
{
    for (int i = 0; i < dimension; i++)
    {
        if (pthread_mutex_destroy(&array[i]) != 0)
        {
            perror("pthread_mutex_destroy() error");
            exit(-1);
        }
    }
    free(array);
}

void printDoubleMatrix(double **matrix, int dimension)
{
    for(int i = 0; i < dimension; i++)
    {
        for(int ii = 0; ii < dimension; ii++)
        {
            printf(" %f ", matrix[i][ii]);
        }
        printf("\n");
    }
}
