/**
 * \file  dispatcher.c
 * \brief File with implementation of dispatcher functions
 * \author Diogo Andrade (89265)
 * \author Francisco Silveira (84802)
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "dispatcher.h"
#include "partfileinfo.h"
#include "workstate.h"

// Number of files
int nFileNames;
// Array with file names
char **fileNames;
// Stored files info
PARTFILEINFO *fileInfos;
// Id of file in process
int fileIdProcessed;
// Number of points readed
int nReaded;
// Workers States
int *workers;
// Number of processes
int nProc;
// Number of messages send
int msgSent;
// Number of messages received
int msgReceived;

/**
 * \brief Dispatcher Life Cicle
 * \param nFileNames Number of files
 * \param fileNames Array with file names
 * \param nProcess Number of processes
 */
void dispatcherJob(int nFiles, char *files[], int nProcess)
{
    unsigned int workState;
    int p;

    // init workers state
    nProc = nProcess;
    workers = (int *)malloc((nProcess - 1) * sizeof(int));
    for (p = 0; p < nProcess; p++)
        workers[p] = 0;

    // storeFileNames
    storeFileNames(nFiles, files);

    // init work
    msgSent = 0;
    msgReceived = 0;
    workState = WORKIN;
    while (workState == WORKIN)
    {
        for (p = 0; p < (nProcess - 1); p++)
        {
            if (workers[p] == 0)
            {
                if (giveJobToWorker(p + 1) == -1)
                {
                    workState = WORKFINISH;
                    break;
                }
            }
        }
        waitWorkers(0);
    }
    waitWorkers(1);

    // stop workers
    stopWorkers(nProc);

    // printProcessingResults
    printProcessingResults();
}

/**
 * \brief Wait for workers response
 * \param wait Last check - waiting for all processing requests to respond
 */
void waitWorkers(int wait)
{
    int flag;
    int fileId, point, nProcess;
    double val;

    flag = 0;

    // check if some worker have finished the work
    MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);

    // while receive processed data from workers or if is last check wait for all workers
    while (flag || (wait && msgReceived < msgSent))
    {
        // receive partial data results
        MPI_Recv(&nProcess, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&fileId, 1, MPI_INT, nProcess, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&point, 1, MPI_INT, nProcess, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&val, 1, MPI_DOUBLE, nProcess, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // savePartialResults
        savePartialResults(fileId, point, val);
        msgReceived++;

        // update worker state
        workers[nProcess - 1] = 0;

        // check if some worker have finished the work
        MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);
    }
}

/**
 * \brief Tell workers to stop
 * \param nProcess Number of processes
 */
void stopWorkers(int nProcess)
{
    unsigned int workState;

    // stop workers
    workState = WORKFINISH;
    for (int p = 1; p < nProcess; p++)
        MPI_Send(&workState, 1, MPI_UNSIGNED, p, 0, MPI_COMM_WORLD);
}

/**
 * \brief Store files info
 * \param nFileNames Number of files
 * \param fileNames Array with file names
 */
void storeFileNames(int nFiles, char *files[])
{
    FILE *fp;

    // store files
    nFileNames = nFiles;
    fileNames = files;
    fileIdProcessed = 0;
    nReaded = 0;
    fileInfos = (PARTFILEINFO *)malloc(nFiles * sizeof(PARTFILEINFO));
    for (int i = 0; i < nFiles; i++)
    {
        PARTFILEINFO partialInfo;
        partialInfo.fileId = i;
        partialInfo.valCurrent = 0;

        // open file
        fp = fopen(fileNames[i], "rb");
        if (fp == NULL)
        {
            // stop workers
            stopWorkers(nProc);

            // finish MPI
            MPI_Finalize();

            printf("ERROR: Failed to open the file!\n");
            exit(1);
        }

        // read file data
        fread(&partialInfo.n, sizeof(int), 1, fp);

        partialInfo.x = (double *)malloc(partialInfo.n * sizeof(double));
        partialInfo.y = (double *)malloc(partialInfo.n * sizeof(double));
        partialInfo.valPrevious = (double *)malloc(partialInfo.n * sizeof(double));
        partialInfo.valCurrent = (double *)malloc(partialInfo.n * sizeof(double));

        fread(partialInfo.x, sizeof(double), partialInfo.n, fp);
        fread(partialInfo.y, sizeof(double), partialInfo.n, fp);
        fread(partialInfo.valPrevious, sizeof(double), partialInfo.n, fp);

        if (feof(fp))
        {
            // stop workers
            stopWorkers(nProc);

            // finish MPI
            MPI_Finalize();

            printf("ERROR: Format file is Wrong!\n");
            exit(1);
        }
        // close file
        fclose(fp);

        fileInfos[i] = partialInfo;
    }
}

/**
 * \brief Give Job to woker
 * \param rank Rank of the worker
 * \return -1 if work is no complete otherwise 0
 */
int giveJobToWorker(int rank)
{
    int fileId, n, point;
    double *x, *y;
    unsigned int workState;

    // check if all work is done
    if (fileIdProcessed == nFileNames)
        return -1;

    workState = WORKIN;
    MPI_Send(&workState, 1, MPI_UNSIGNED, rank, 0, MPI_COMM_WORLD);

    //get file info
    fileId = fileIdProcessed;
    n = fileInfos[fileIdProcessed].n;
    x = fileInfos[fileIdProcessed].x;
    y = fileInfos[fileIdProcessed].y;
    point = nReaded;

    nReaded++;
    // check if reach end of file
    if (n < nReaded)
    {
        fileIdProcessed++;
        nReaded = 0;
    }

    // mensagem com os dados para serem processados
    MPI_Send(&fileId, 1, MPI_INT, rank, 0, MPI_COMM_WORLD);
    MPI_Send(&n, 1, MPI_INT, rank, 0, MPI_COMM_WORLD);
    MPI_Send(x, n, MPI_DOUBLE, rank, 0, MPI_COMM_WORLD);
    MPI_Send(y, n, MPI_DOUBLE, rank, 0, MPI_COMM_WORLD);
    MPI_Send(&point, 1, MPI_INT, rank, 0, MPI_COMM_WORLD);

    workers[rank - 1] = 1;
    msgSent++;

    return 0;
}

/**
 * \brief Save results of processed data
 * \param fileId Id of file of the processed data
 * \param point Point
 * \param val Circular cross-correlation value
 */
void savePartialResults(int fileId, int point, double val)
{
    // save result
    fileInfos[fileId].valCurrent[point] = val;
}

/**
 * \brief Print results
 */
void printProcessingResults()
{
    int foundExpectedRes;

    // print processing results
    for (int i = 0; i < nFileNames; i++)
    {
        printf("File name: %s\n", fileNames[i]);

        foundExpectedRes = 1;
        for (int k = 0; k < fileInfos[i].n; k++)
            if (fileInfos[i].valCurrent[k] != fileInfos[i].valPrevious[k])
            {
                printf("\tResult: Expected result not found!\n");
                foundExpectedRes = 0;
                break;
            }
        if (foundExpectedRes == 1)
            printf("\tResult: Expected result founded!\n");
    }

    // clean space
    free(fileInfos);
}