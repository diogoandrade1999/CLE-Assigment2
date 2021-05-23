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
#include "convertchar.h"

// Number of files
int nFileNames;
// Array with file names
char **fileNames;
// Stored files info
PARTFILEINFO *fileInfos;
// Id of file in process
int fileIdProcessed;
// Pointer of actual file in process
FILE *fp;
// Size of data readed from file in process
int sizeReaded;
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
    PARTFILEINFO partialInfo;

    flag = 0;

    // check if some worker have finished the work
    MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);

    // while receive processed data from workers or if is last check wait for all workers
    while (flag || (wait && msgReceived < msgSent))
    {
        // receive partial data results
        MPI_Recv(&partialInfo, sizeof(PARTFILEINFO), MPI_BYTE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // savePartialResults
        savePartialResults(&partialInfo);
        msgReceived++;

        // update worker state
        workers[partialInfo.nProcess - 1] = 0;

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
    // store files
    nFileNames = nFiles;
    fileNames = files;
    fileIdProcessed = 0;
    fp = NULL;
    sizeReaded = 0;
    fileInfos = (PARTFILEINFO *)malloc(nFileNames * sizeof(PARTFILEINFO));
    for (int i = 0; i < nFileNames; i++)
    {
        PARTFILEINFO partialInfo;
        partialInfo.fileId = i;
        partialInfo.countWords = 0;
        for (int i = 0; i < 30; i++)
        {
            partialInfo.countWordsSize[i] = 0;
            for (int z = 0; z < 30; z++)
                partialInfo.countConsonants[i][z] = 0;
        }
        partialInfo.biggestWord = 0;
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
    unsigned char buff;
    int endPosLastStr;
    unsigned char buf[1024];
    PARTFILEINFO partialInfo;
    unsigned int workState;

    // check if all work is done
    if (fileIdProcessed == nFileNames)
        return -1;

    // open file if it is not open
    if (fp == NULL)
    {
        fp = fopen(fileNames[fileIdProcessed], "r");
        if (fp == NULL)
        {
            // stop workers
            stopWorkers(nProc);

            // finish MPI
            MPI_Finalize();

            printf("ERROR: Failed to open the file!\n");
            exit(1);
        }
    }

    workState = WORKIN;
    MPI_Send(&workState, 1, MPI_UNSIGNED, rank, 0, MPI_COMM_WORLD);

    // save rank
    partialInfo.nProcess = rank;

    // save partial data info file
    partialInfo.fileId = fileIdProcessed;

    // read file
    endPosLastStr = 0;
    for (int i = 0; i < 1024; i++)
    {
        // reach end of file
        if (fscanf(fp, "%c", &buff) == EOF)
        {
            // clean data from actual file
            fclose(fp);
            fileIdProcessed++;
            fp = NULL;
            sizeReaded = 0;
            endPosLastStr = i;
            *(buf + i) = ' ';
            break;
        }
        // find end position of last string
        else if (isSpace(buff) || isSeparation(buff) || isPunct(buff))
            endPosLastStr = i;

        *(buf + i) = buff;
    }

    // check if reach the end
    if (fp != NULL)
    {
        // return to last string
        if (endPosLastStr != 1024)
            fseek(fp, sizeReaded + endPosLastStr, SEEK_SET);

        // save size of data readed
        sizeReaded += endPosLastStr;
    }

    // save partial data info file
    partialInfo.textSize = endPosLastStr + 1;

    // mensagem com os dados para serem processados
    MPI_Send(buf, 1024, MPI_CHAR, rank, 0, MPI_COMM_WORLD);

    // mensagem com estrutura para armazenar resultados
    MPI_Send(&partialInfo, sizeof(PARTFILEINFO), MPI_BYTE, rank, 0, MPI_COMM_WORLD);

    workers[rank - 1] = 1;
    msgSent++;

    return 0;
}

/**
 * \brief Save results of processed data
 * \param partialInfo Results of processed data
 */
void savePartialResults(PARTFILEINFO *partialInfo)
{
    PARTFILEINFO *storedInfo = &fileInfos[partialInfo->fileId];

    // store biggest word
    if (storedInfo->biggestWord < partialInfo->biggestWord)
        storedInfo->biggestWord = partialInfo->biggestWord;

    // store count of words
    storedInfo->countWords += partialInfo->countWords;

    for (int i = 0; i < 30; i++)
    {
        storedInfo->countWordsSize[i] += partialInfo->countWordsSize[i];

        for (int k = 0; k < 30; k++)
            storedInfo->countConsonants[i][k] += partialInfo->countConsonants[i][k];
    }
}

/**
 * \brief Print results
 */
void printProcessingResults()
{
    // print processing results
    for (int i = 0; i < nFileNames; i++)
    {
        printf("File name: %s\n", fileNames[i]);
        printf("Total number of words = %d\n", fileInfos[i].countWords);

        printf("Word length");
        for (int j = 0; j < 3; j++)
        {
            printf("\n");
            for (int k = 1; k <= fileInfos[i].biggestWord; k++)
                if (j == 0)
                    printf("\t%d ", k);
                else if (j == 1)
                    printf("\t%d ", fileInfos[i].countWordsSize[k]);
                else
                    printf("\t%3.2f ", ((double)fileInfos[i].countWordsSize[k] / (double)fileInfos[i].countWords) * 100);
        }

        for (int j = 0; j <= fileInfos[i].biggestWord; j++)
        {
            printf("\n");
            for (int k = 0; k <= fileInfos[i].biggestWord; k++)
                if (k == 0)
                    printf("%d ", j);
                else if (k >= j)
                    if (fileInfos[i].countConsonants[k][j] > 0 && fileInfos[i].countWordsSize[k] > 0)
                        printf("\t%1.1f ", ((double)fileInfos[i].countConsonants[k][j] / (double)fileInfos[i].countWordsSize[k]) * 100);
                    else
                        printf("\t%d ", 0);
                else
                    printf("\t ");
        }
        printf("\n\n");
    }
}