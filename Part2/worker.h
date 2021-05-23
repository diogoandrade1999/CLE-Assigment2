/**
 * \file  worker.h
 * \brief File with worker functions
 * \author Diogo Andrade (89265)
 * \author Francisco Silveira (84802)
 */
#ifndef WORKER_H
#define WORKER_H

#include "partfileinfo.h"

/**
 * \brief Worker Life Cicle
 * \param rank Rank of the worker
 */
void workerJob(int rank);

/**
 * \brief Compute the circular cross-correlation
 * \param n Number of Points
 * \param x Elements of signal x
 * \param y Elements of signal y
 * \param point Actual Point
 * \return circular cross-correlation
 */
double computeValue(int n, double *x, double *y, int point);

#endif