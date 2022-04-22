/**
 * @file matrix.h
 * @brief Header utility file to manage creation and destruction of arrays used.
 * @date 03/01/2022
 * @author dancs-dev
 */

#pragma once


double* createDoubleMatrix(int dimension);

double getElemFromDoubleMatrix(double* matrix, int dimension, int x, int y);

double getElemFromDoubleMatrixBuffer(double* buffer, int dimension, int x, int y);

void freeDoubleMatrix(double *matrix);

void printDoubleMatrix(double *matrix, int dimension);
