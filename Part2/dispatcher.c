#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "dispatcher.h"
#include "partfileinfo.h"
#include "workstate.h"

int nFileNames;
char **fileNames;
PARTFILEINFO *fileInfos;
int fileIdProcessed;
int nReaded;
int *workers;
unsigned int workState;
int msgSent;
int msgReceived;

void dispatcherJob(int nFiles, char *files[], int nProcess)
{
    int workStateJob, p;

    // init workers state
    workers = (int *)malloc((nProcess - 1) * sizeof(int));
    for (p = 0; p < nProcess; p++)
        workers[p] = 0;

    // storeFileNames
    storeFileNames(nFiles, files);

    // init work
    msgSent = 0;
    msgReceived = 0;
    workStateJob = WORKIN;
    while (workStateJob == WORKIN)
    {
        for (p = 0; p < (nProcess - 1); p++)
        {
            if (workers[p] == 0)
            {
                if (giveJobToWorker(p + 1) == -1)
                {
                    workStateJob = WORKFINISH;
                    break;
                }
            }
        }
        waitWorkers(0);
    }
    waitWorkers(1);

    // finish work
    workState = WORKFINISH;
    for (p = 1; p < nProcess; p++)
        MPI_Send(&workState, 1, MPI_UNSIGNED, p, 0, MPI_COMM_WORLD);

    // printProcessingResults
    printProcessingResults();
}

void waitWorkers(int last)
{
    int flag;
    int fileId, point, nProcess;
    double val;

    flag = 0;

    // check if some worker have finished the work
    MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);

    while (flag)
    {
        // receive partial data results
        MPI_Recv(&fileId, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&point, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&val, 1, MPI_DOUBLE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&nProcess, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // savePartialResults
        savePartialResults(fileId, point, val);
        msgReceived++;

        // update worker state
        workers[nProcess - 1] = 0;

        // check if some worker have finished the work
        MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);
    }

    // wait for all workers
    if (last)
    {
        while (msgReceived < msgSent)
        {
            // receive partial data results
            MPI_Recv(&fileId, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&point, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&val, 1, MPI_DOUBLE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&nProcess, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // savePartialResults
            savePartialResults(fileId, point, val);
            msgReceived++;
        }
    }
}

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

        // close file
        fclose(fp);

        fileInfos[i] = partialInfo;
    }
}

int giveJobToWorker(int rank)
{
    int fileId, n, point;
    double *x, *y;

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

void savePartialResults(int fileId, int point, double val)
{
    // save result
    fileInfos[fileId].valCurrent[point] = val;
}

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