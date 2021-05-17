#include <mpi.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "workstate.h"
#include "worker.h"
#include "dispatcher.h"

int main(int argc, char *argv[])
{
    int rank, size, errorArgs, opt, nFileNames;
    char delim[] = " ";
    char *fileNames[30], *ptr;
    clock_t t0, t1;

    // init MPI
    MPI_Init(&argc, &argv);

    // get rank
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // get size
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // rank 0 is the dispatcher
    if (rank == 0)
    {
        // processCommandLine
        errorArgs = 0;
        while ((opt = getopt(argc, argv, "f:h")) != -1)
        {
            switch (opt)
            {
            case 'f':
                nFileNames = 0;
                ptr = strtok(optarg, delim);
                while (ptr != NULL)
                {
                    fileNames[nFileNames] = ptr;
                    nFileNames++;
                    ptr = strtok(NULL, delim);
                }
                break;
            case 'h':
                printf("Execution:\n\tmpiexex -n <number_of_process> ./main -f \"<file_names_paths>\"\n");
                errorArgs = 1;
                break;
            case '?':
                printf("Invlid option: %c\n", optopt);
                errorArgs = 1;
                break;
            }
        }

        if (errorArgs == 0)
        {
            //defineTimeOrigin
            t0 = clock();

            dispatcherJob(nFileNames, fileNames, size);

            //getTime
            t1 = clock();

            //printProcessingTime
            printf("Processing Time: %fs\n", (double)(t1 - t0) / CLOCKS_PER_SEC);
        }
        else
        {
            int workState = WORKFINISH;
            for (int n = 1; n < size; n++)
            {
                MPI_Send(&workState, 1, MPI_UNSIGNED, n, 0, MPI_COMM_WORLD);
            }
        }
    }
    // others ranks are the workers
    else
    {
        workerJob(rank);
    }

    // finish MPI
    MPI_Finalize();

    return 0;
}