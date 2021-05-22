#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include "worker.h"
#include "partfileinfo.h"
#include "convertchar.h"
#include "workstate.h"

void workerJob(int rank)
{
    unsigned char buff[1024];
    PARTFILEINFO partialInfo;
    unsigned int workState;

    while (1)
    {
        // receive work
        MPI_Recv(&workState, 1, MPI_UNSIGNED, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // finish when does not have more work
        if (workState == WORKFINISH)
            break;

        // getDataChunk
        MPI_Recv(&buff, 1024, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&partialInfo, sizeof(PARTFILEINFO), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        processDataChunk(buff, &partialInfo);

        // sendPartialResults
        MPI_Send(&partialInfo, sizeof(PARTFILEINFO), MPI_BYTE, 0, 0, MPI_COMM_WORLD);

        memset(buff, 0, 1024);
    }
}

void processDataChunk(unsigned char *buf, PARTFILEINFO *partialInfo)
{
    int wordSize, countConsonantsWord, insideWord;
    unsigned char buff;

    // init counters
    partialInfo->countWords = 0;
    partialInfo->biggestWord = 0;
    for (int i = 0; i < 30; i++)
    {
        partialInfo->countWordsSize[i] = 0;
        for (int z = 0; z < 30; z++)
            partialInfo->countConsonants[i][z] = 0;
    }

    // find words
    wordSize = 0;
    countConsonantsWord = 0;
    insideWord = 0;
    for (int i = 0; i < partialInfo->textSize; i++)
    {
        // remove special char
        buff = convertChar(buf, &i);

        // in word
        if (inWord(buff))
        {
            // check if is a consonant
            if (isConsonant(buff))
                countConsonantsWord++;

            // increase size of word
            if (!isMerge(buff))
            {
                wordSize++;

                if (!insideWord)
                {
                    partialInfo->countWords++;
                    insideWord = 1;
                }
            }
        }
        // end of word
        else if (isSpace(buff) || isPunct(buff) || isSeparation(buff))
        {
            insideWord = 0;

            // check if is the biggest word
            if (partialInfo->biggestWord < wordSize)
                partialInfo->biggestWord = wordSize;

            partialInfo->countWordsSize[wordSize]++;
            partialInfo->countConsonants[wordSize][countConsonantsWord]++;
            wordSize = 0;
            countConsonantsWord = 0;
        }
    }
}