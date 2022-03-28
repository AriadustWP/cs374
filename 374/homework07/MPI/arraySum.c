/* arraySum.c uses an array to sum the values in an input file,
 *  whose name is specified on the command-line.
 * 
 * Summation process is parallelized using MPI method
 *
 * Joel Adams, Fall 2005
 * for CS 374 (HPC) at Calvin College.
 *
 * Modified by: Won Seok Park
 * Date: November 07, 2021
 * CS 374 Calvin University
 */

#include <mpi.h>
#include <stdio.h>      /* I/O stuff */
#include <stdlib.h>     /* calloc, etc. */

void readArray(char * fileName, double ** a, int * n);
double sumArray(double * a, int numValues) ;

int main(int argc, char * argv[])
{
  int  howMany, id = -1, numProcesses = -1;
  int* displs;
  double totalSum;
  double * a;

  if (argc != 2) {
    fprintf(stderr, "\n*** Usage: arraySum <inputFile>\n\n");
    exit(1);
  }

  readArray(argv[1], &a, &howMany);

  // MPI initialization
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);

  double 
    startTime     = 0, 
    ioTime        = 0, 
    scatterTime   = 0, 
    reductionTime = 0, 
    totalTime     = 0,
    localSum      = 0;
  
  double* aRcv; 
  int* counts;

  int numSent = howMany / numProcesses;
  int numRemain = howMany % numProcesses;

  counts = (int*) malloc(numProcesses * sizeof(int));
  displs = (int*) malloc(numProcesses * sizeof(int));
  aRcv = (double*) malloc(numSent * sizeof(double));

  startTime = MPI_Wtime();

  // Master responsible for i/o
	if (id == 0 )
	{
		readArray(argv[1], &a, &howMany);
		printf("Read in: %d\n", howMany);
		ioTime = MPI_Wtime() - startTime;
	}

  // If array doesn't slice nicely, use scatterv
  if (numRemain) {
    for (int i = 0; i < numProcesses; i++) {
      displs[i] = 0;
      if (i < numRemain) {
        counts[i] = numSent + 1;
      } else {
        counts[i] = numSent;
      }
    }
    MPI_Scatterv(a, counts, displs, MPI_DOUBLE, aRcv, counts[id], MPI_DOUBLE, 0, MPI_COMM_WORLD);
  } else {
    MPI_Scatter(a, numSent, MPI_DOUBLE, aRcv, numSent, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  }
  scatterTime = MPI_Wtime() - startTime - ioTime;

  localSum = sumArray(aRcv, numSent);
	MPI_Reduce(&localSum, &totalSum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	reductionTime = MPI_Wtime() - startTime - (scatterTime + ioTime);

  if (id == 0 )
	{
		totalTime = MPI_Wtime() - startTime;
		
		printf("The sum of the values in the input file '%s' is %g\n",
		   argv[1], totalSum);
		printf("Processing Time\nTotal: %f\nI/O: %f\nScatter: %f\nSum: %f\n",
			   totalTime, ioTime, scatterTime, reductionTime);
	}

  free(a);
  free(counts);
  free(displs);
  free(aRcv);

  MPI_Finalize();

  return 0;
}

/* readArray fills an array with values from a file.
 * Receive: fileName, a char*,
 *          a, the address of a pointer to an array,
 *          n, the address of an int.
 * PRE: fileName contains N, followed by N double values.
 * POST: a points to a dynamically allocated array
 *        containing the N values from fileName
 *        and n == N.
 */

void readArray(char * fileName, double ** a, int * n) {
  int count, howMany;
  double * tempA;
  FILE * fin;

  fin = fopen(fileName, "r");
  if (fin == NULL) {
    fprintf(stderr, "\n*** Unable to open input file '%s'\n\n",
                     fileName);
    exit(1);
  }

  fscanf(fin, "%d", &howMany);
  tempA = calloc(howMany, sizeof(double));
  if (tempA == NULL) {
    fprintf(stderr, "\n*** Unable to allocate %d-length array",
                     howMany);
    exit(1);
  }

  for (count = 0; count < howMany; count++)
   fscanf(fin, "%lf", &tempA[count]);

  fclose(fin);

  *n = howMany;
  *a = tempA;
}

/* sumArray sums the values in an array of doubles.
 * Receive: a, a pointer to the head of an array;
 *          numValues, the number of values in the array.
 * Return: the sum of the values in the array.
 */

double sumArray(double * a, int numValues) {
  int i;
  double result = 0.0;

  for (i = 0; i < numValues; ++i) {
    result += a[i];
  }

  return result;
}

