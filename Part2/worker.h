#ifndef WORKER_H
#define WORKER_H

#include "partfileinfo.h"

void workerJob(int rank);
double computeValue(int n, double *x, double *y, int point);

#endif