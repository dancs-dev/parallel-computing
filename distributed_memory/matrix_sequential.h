/**
 * @file matrix_sequential.h
 * @brief Header utility file to manage creation and destruction of arrays used.
 * @date 03/03/2022
 * @author dancs-dev
 */

#pragma once


double** createDoubleMatrix(int dimension);

void freeDoubleMatrix(double **matrix, int dimension);

void printDoubleMatrix(double **matrix, int dimension);
