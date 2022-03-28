// TODO: Allow pthreadReduction to handle all numbers including odds
// For now, it only handles binary number of threads or non-binary number that can be subdivided into two binary number of threads

#include "barrier.h"
#include <math.h>
#include <stdio.h>

long double pthreadReductionSum( long double reduceArray[], unsigned int numThreads, unsigned int id, unsigned int minId) {
	// Calculate largest numThreads for binary reduction
	int binaryNumThread = pow(2, (int)log2(numThreads));

	// If binary reduction works
	if ((numThreads & (numThreads-1)) == 0 && numThreads != 0) {
		for ( int i=2; i <= numThreads; i*=2 ) {
			pthreadBarrier(numThreads);
			printf("id: %d, step 2", id);
			if ( id % i == 0 ) {
				reduceArray[id] += reduceArray[(int)id + (i/2)];
			}
		}
	} 
	// If binary reduction doesn't work
	else {
		// Chop up threads into binary form
		int minId2 = binaryNumThread;
		if (id < binaryNumThread) {
			reduceArray[minId] = pthreadReductionSum(reduceArray, binaryNumThread, id, minId);
			printf("id: %d, step 1", id);
		} 
		else {
			reduceArray[minId2] = pthreadReductionSum(reduceArray, numThreads - binaryNumThread, id, minId2);
			printf("id: %d, step 1", id);
		}
		pthreadBarrier(numThreads);
		reduceArray[minId] += reduceArray[minId2];
	}
	printf("id: %d, value: %Lf\n", id, reduceArray[minId]);

	return reduceArray[minId];
}