/**
 * \file  partfileinfo.h
 * \brief File with datastruct to store partial or final data file info
 * \author Diogo Andrade (89265)
 * \author Francisco Silveira (84802)
 */

#ifndef PARTFILEINFO_H
#define PARTFILEINFO_H

typedef struct
{
    int fileId;
    int textSize;
    int biggestWord;
    int countWords;
    int countWordsSize[30];
    int countConsonants[30][30];
    int nProcess;
} PARTFILEINFO;

#endif