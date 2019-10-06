/* messagePassing.c
 * ... illustrates the use of the MPI_Send() and MPI_Recv() commands...
 * Joel Adams, Calvin College, November 2009.
 * 
 * CS 374, Project 02
 * 
 * Modified by: Won Seok Park (wp22)
 * Place: Calvin University
 * Date: Sep 26th 2019
 */
 
#include <stdio.h>
#include <mpi.h>
#include <math.h>   // sqrt()
#include <string.h>
#include <stdlib.h>
 
int main(int argc, char** argv) {
    int id = -1, numProcesses = -1; 
    char * sendString = NULL;
    char * receivedString = NULL;
    MPI_Status status;
 
    // Initiate Parallelization
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);

    const int SIZE = (numProcesses * 4) * sizeof(char) + 1;
    sendString = (char*) malloc( SIZE );
    receivedString = (char*) malloc( SIZE );

    if ( id == 0 ) {  // Master initiates the message sending and calculate and print out the result
        strcpy(sendString, "0 ");

        double start = MPI_Wtime();

        MPI_Send(sendString, strlen(sendString) + 1, MPI_CHAR, (id+1) % numProcesses, 0, MPI_COMM_WORLD);
        MPI_Recv(receivedString, SIZE, MPI_CHAR, numProcesses - 1, 0, 
                    MPI_COMM_WORLD, &status);

        double end = MPI_Wtime();
        double time = end - start;

        printf("%s\ntime: %f sec\n",receivedString,time);
    } else {          // Workers receive and send message to the next worker
        MPI_Recv(receivedString, SIZE, MPI_CHAR,(id-1) % numProcesses, 0, 
                    MPI_COMM_WORLD, &status);
        // Concatenate received message and sender's id
        sprintf(sendString, "%s%d ", receivedString, id);
        MPI_Send(sendString, strlen(sendString) + 1, MPI_CHAR, (id+1) % numProcesses, 0, MPI_COMM_WORLD);
    }

    // Memory clean up
    free(sendString); free(receivedString);

    // Terminates parallelization
    MPI_Finalize();
    return 0;
}

