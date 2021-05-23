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
    int n;
    double *x;
    double *y;
    double *valPrevious;
    double *valCurrent;
} PARTFILEINFO;

#endif