#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "dispatcher.h"
#include "partfileinfo.h"
#include "workstate.h"
#include "convertchar.h"

int nFileNames;
char **fileNames;
PARTFILEINFO *fileInfos;
int fileIdProcessed;
FILE *fp;
int sizeReaded;
int *workers;
int workState;
int msgSent;
int msgReceived;

void dispatcherJob(int nFiles, char *files[], int nProcess)
{
    int workStateJob, p;

    // init workers state
    workers = (int *)malloc((nProcess - 1) * sizeof(int));
    for (p = 0; p < nProcess; p++)
    {
        workers[p] = 0;
    }

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
    {
        MPI_Send(&workState, 1, MPI_UNSIGNED, p, 0, MPI_COMM_WORLD);
    }

    // printProcessingResults
    printProcessingResults();
}

void waitWorkers(int last)
{
    int flag;
    PARTFILEINFO partialInfo;
    flag = 0;

    MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);

    while (flag)
    {
        // receive partial data results
        MPI_Recv(&partialInfo, sizeof(PARTFILEINFO), MPI_BYTE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // savePartialResults
        savePartialResults(&partialInfo);
        msgReceived++;

        // update worker state
        workers[partialInfo.nProcess - 1] = 0;

        MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);
    }

    if (last)
    {
        while (msgReceived < msgSent)
        {
            // receive partial data results
            MPI_Recv(&partialInfo, sizeof(PARTFILEINFO), MPI_BYTE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // savePartialResults
            savePartialResults(&partialInfo);
            msgReceived++;
        }
    }
}

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

int giveJobToWorker(int rank)
{
    unsigned char buff;
    int endPosLastStr;
    unsigned char buf[1024];
    PARTFILEINFO partialInfo;

    // check if all work is done
    if (fileIdProcessed == nFileNames)
    {
        return -1;
    }

    // open file if it is not open
    if (fp == NULL)
    {
        fp = fopen(fileNames[fileIdProcessed], "r");
        if (fp == NULL)
        {
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
    MPI_Send(&buf, 1024, MPI_CHAR, rank, 0, MPI_COMM_WORLD);

    // mensagem com estrutura para armazenar resultados
    MPI_Send(&partialInfo, sizeof(PARTFILEINFO), MPI_BYTE, rank, 0, MPI_COMM_WORLD);

    workers[rank - 1] = 1;
    msgSent++;

    return 0;
}

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