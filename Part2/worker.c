/**
 * \file  worker.c
 * \brief File with implementation of worker functions
 * \author Diogo Andrade (89265)
 * \author Francisco Silveira (84802)
 */

#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include "worker.h"
#include "partfileinfo.h"
#include "workstate.h"

/**
 * \brief Worker Life Cicle
 * \param rank Rank of the worker
 */
void workerJob(int rank)
{
    int fileId, n, point;
    double val;
    unsigned int workState;

    while (1)
    {
        // receive work
        MPI_Recv(&workState, 1, MPI_UNSIGNED, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // finish when does not have more work
        if (workState == WORKFINISH)
            break;

        // processConvPoint
        MPI_Recv(&fileId, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&n, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        double x[n], y[n];
        MPI_Recv(&x, n, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&y, n, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&point, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        val = computeValue(n, x, y, point);

        // sendPartialResults
        MPI_Send(&rank, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(&fileId, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(&point, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(&val, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }
}

/**
 * \brief Compute the circular cross-correlation
 * \param n Number of Points
 * \param x Elements of signal x
 * \param y Elements of signal y
 * \param point Actual Point
 * \return circular cross-correlation
 */
double computeValue(int n, double *x, double *y, int point)
{
    double val;
    val = 0;
    for (int k = 0; k < n; k++)
        val += (x[k] * y[(point + k) % n]);
    return val;
}