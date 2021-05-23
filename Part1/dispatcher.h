/**
 * \file  dispatcher.h
 * \brief File with dispatcher functions
 * \author Diogo Andrade (89265)
 * \author Francisco Silveira (84802)
 */

#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "partfileinfo.h"

/**
 * \brief Dispatcher Life Cicle
 * \param nFileNames Number of files
 * \param fileNames Array with file names
 * \param nProcess Number of processes
 */
void dispatcherJob(int nFileNames, char *filesNames[], int nProcess);

/**
 * \brief Wait for workers response
 * \param wait Last check - waiting for all processing requests to respond
 */
void waitWorkers(int wait);

/**
 * \brief Tell workers to stop
 * \param nProcess Number of processes
 */
void stopWorkers(int nProcess);

/**
 * \brief Store files info
 * \param nFileNames Number of files
 * \param fileNames Array with file names
 */
void storeFileNames(int nFileNames, char *fileNames[]);

/**
 * \brief Print results
 */
void printProcessingResults();

/**
 * \brief Give Job to woker
 * \param rank Rank of the worker
 * \return -1 if work is no complete otherwise 0
 */
int giveJobToWorker(int rank);

/**
 * \brief Save results of processed data
 * \param partialInfo Results of processed data
 */
void savePartialResults(PARTFILEINFO *partialInfo);

#endif