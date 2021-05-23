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
 * \brief Processes pieces of the data file
 * \param buf Array with chars to be processed
 * \param partialInfo Partial data file info
 */
void processDataChunk(unsigned char *buf, PARTFILEINFO *partialInfo);

#endif