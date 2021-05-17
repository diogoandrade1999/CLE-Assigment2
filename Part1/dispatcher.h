#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "partfileinfo.h"

void dispatcherJob(int nFileNames, char *filesNames[], int nProcess);
void waitWorkers(int last);
void storeFileNames(int nFileNames, char *fileNames[]);
void printProcessingResults();
int giveJobToWorker(int rank);
void savePartialResults(PARTFILEINFO *partialInfo);

#endif