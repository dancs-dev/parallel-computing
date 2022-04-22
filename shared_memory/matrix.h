/**
 * @file matrix.h
 * @brief Header utility file to manage creation and destruction of arrays used.
 * @date 28/10/2021
 * @author dancs-dev
 */

#pragma once


double** createDoubleMatrix(int dimension);

pthread_mutex_t* createMutexArray(int dimension);

void freeDoubleMatrix(double **matrix, int dimension);

void freeMutexArray(pthread_mutex_t *array, int dimension);

void printDoubleMatrix(double **matrix, int dimension);
