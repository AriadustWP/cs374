/*
 * Won Seok Park
 * CS 374: Fall 2021
 * Sept. 14th 2021
 *
 * messagePassingMW.c
 * Combination of message passing and master-worker pattern
 *   where master process starts the message passing and calculates
 *   total processing time, and worker processes pass a message down to the 
 *   next process until last process sends message back to the master process
 *   to complete the ring
 */

#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

int main(int argc, char** argv) {

  const int SIZE = (MPI_MAX_PROCESSOR_NAME * 4) * sizeof(int);

  //TODO: use malloc for array allocation
  int id = -1, numWorkers = -1;
  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &id);
  MPI_Comm_size(MPI_COMM_WORLD, &numWorkers);

  // Dynamically allocate memory to an answer array 
  int * ansArray = (int*) malloc(SIZE);

  if ( id == 0 ) {  // process 0 is the master 
    // Start measuring time
    double startTime = MPI_Wtime();

    ansArray[id] = id;
    MPI_Send(ansArray, id+1, MPI_INT, id+1, 1, MPI_COMM_WORLD);
    MPI_Recv(ansArray, numWorkers, MPI_INT, numWorkers - 1, 1, MPI_COMM_WORLD, &status);

    // Calculate processing time
    double finishTime = MPI_Wtime() - startTime;

    // Print entire array
    for (int i = 0; i <numWorkers; i++) {
        printf("%d ",ansArray[i]);
    }

    printf("\nDone in %f seconds\n", finishTime);

  } else if ( id == numWorkers - 1) { // Last id sends message back to the master process
    MPI_Recv(ansArray, id, MPI_INT, id-1, 1, MPI_COMM_WORLD, &status);
    ansArray[id] = id;
    MPI_Send(ansArray, id+1, MPI_INT, 0, 1, MPI_COMM_WORLD);
  } else { // processes with ids > 0 are workers
    MPI_Recv(ansArray, id, MPI_INT, id-1, 1, MPI_COMM_WORLD, &status);
    ansArray[id] = id;
    MPI_Send(ansArray, id+1, MPI_INT, id+1, 1, MPI_COMM_WORLD);
  }

  // Free up the memory
  free(ansArray);
  MPI_Finalize();
  return 0;
}

