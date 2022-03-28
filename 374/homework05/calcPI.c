/* calcPi.c calculates PI using the integral of the unit circle.
 * Since the area of the unit circle is PI, 
 *  PI = 4 * the area of a quarter of the unit circle 
 *    (i.e., its integral from 0.0 to 1.0)
 *
 * Joel Adams, Fall 2013 for CS 374 at Calvin College.
 */

#include <mpi.h>
#include "integral.h"   // integrate()
#include <stdio.h>      // printf(), etc.
#include <stdlib.h>     // exit()
#include <math.h>       // sqrt() 
#include <time.h>


/* function for unit circle (x^2 + y^2 = 1)
 * parameter: x, a double
 * return: sqrt(1 - x^2)
 */
double f(double x) {
   return sqrt(1.0 - x*x);
}

/* retrieve desired number of trapezoids from commandline arguments
 * parameters: argc: the argument count
 *             argv: the argument vector
 * return: the number of trapezoids to be used.
 */            
unsigned long long processCommandLine(int argc, char** argv) {
   if (argc == 1) {
       return 1;
   } else if (argc == 2) {
//       return atoi( argv[1] );
       return strtoull( argv[1], 0, 10 );
   } else {
       fprintf(stderr, "Usage: ./calcPI [numTrapezoids]");
       exit(1);
   }
}
 

int main(int argc, char** argv) {
   long double approximatePI = 0;
   const long double REFERENCE_PI = 3.141592653589793238462643383279L;
   unsigned long long numTrapezoids = processCommandLine(argc, argv);

	// MPI variables
	int id = -1;
	int numProcesses = -1;
	long double totalPI = 0;
	long double chunkSize;

	double startTime = 0.0, totalTime = 0.0;
	
	// MPI Init
	MPI_Init(&argc, &argv); 
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);

	startTime = MPI_Wtime();

	// Calculate chunkSize
	chunkSize = 1.0 / numProcesses;

	// If number of Trapezoids divide cleanly
	if ( numTrapezoids % numProcesses == 0.0 ) {
		approximatePI = integrateTrap( (long double)id * chunkSize, (long double)(id+1) * chunkSize, numTrapezoids/numProcesses) * 4.0;
	}
	//if Remainder exists
	else  {
		// Last processor takes care of remainder
		if ( id == numProcesses - 1 ) {
			approximatePI = integrateTrap( (long double)id * chunkSize, (long double)(id+1) * chunkSize, numTrapezoids%numProcesses) * 4.0;
		}
		// Rest does what they would normally do
		else {
			approximatePI = integrateTrap( (long double)id * chunkSize, (long double)(id+1) * chunkSize, numTrapezoids/numProcesses) * 4.0;
		}
	}

	// Sum local values together
	MPI_Reduce(&approximatePI, &totalPI, 1, MPI_LONG_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	totalTime = MPI_Wtime() - startTime;

	if (id == 0) {
		printf("My ID: %d, Using %llu trapezoids, the approximate vs actual values of PI are:\n%.30Lf\n%.30Lf\n", id,
			numTrapezoids, totalPI, REFERENCE_PI);
		printf("Processing time: %f\n", totalTime);
	}

	MPI_Finalize();
	return 0;
}



