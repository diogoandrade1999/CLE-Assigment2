#ifndef WORKER_H
#define WORKER_H

#include "partfileinfo.h"

void workerJob(int rank);
void processDataChunk(unsigned char *buf, PARTFILEINFO *partialInfo);

#endif