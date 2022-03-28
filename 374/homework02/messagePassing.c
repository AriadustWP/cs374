/* messagePassing.c
 * ... illustrates the use of the MPI_Send() and MPI_Recv() commands...
 * Joel Adams, Calvin College, November 2009.
 *
 */
 
#include <stdio.h>
#include <mpi.h>
#include <math.h>   // sqrt()
#include <string.h>
#include <stdlib.h>
 
int odd(int number) { return number % 2; }
 
int main(int argc, char** argv) {
    int id = -1, numProcesses = -1; 
    char * sendString = NULL;
    char * receivedString = NULL;
    MPI_Status status;
    const int SIZE = (MPI_MAX_PROCESSOR_NAME * 4 + 1) * sizeof(char);
 
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);

    sendString = (char*) malloc( SIZE );
    receivedString = (char*) malloc( SIZE );
   

    if ( id == 0 ) {  // odd processors send, then received
        strcpy(sendString, "0 ");
        double start = MPI_Wtime();
        MPI_Send(sendString, strlen(sendString) + 1, MPI_CHAR, (id+1) % numProcesses, 0, MPI_COMM_WORLD);
        MPI_Recv(receivedString, SIZE, MPI_CHAR, numProcesses - 1, 0, 
                    MPI_COMM_WORLD, &status);
        double end = MPI_Wtime();
        double time = end - start;
        printf("%s\n",receivedString);
        printf("time: %f sec\n",time);
    } else {          // even processors receive, then send 
        MPI_Recv(receivedString, SIZE, MPI_CHAR,(id-1) % numProcesses, 0, 
                    MPI_COMM_WORLD, &status);
        sprintf(sendString, "%s%d ", receivedString, id);
        MPI_Send(sendString, strlen(sendString) + 1, MPI_CHAR, (id+1) % numProcesses, 0, MPI_COMM_WORLD);
    }
    free(sendString); free(receivedString);
    MPI_Finalize();
    return 0;
}

