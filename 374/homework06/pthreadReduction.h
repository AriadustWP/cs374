// This reduction function only handles binary values. I've made an attempt to make it work with any number of threads, 
// but I couldn't make it work and it was consuming too much time, so decided to use this for current project as it appeared to not impact processing time
// I will try to see if I could make different approach to handle any number of threads in the future

/* 
 * pthreadReduction to implement thread-safe reduction method
 * to prevent race condition.
 *
 * Usage: ./calcPI2 [numIntervals] [numThreads]
 * 
 * Modified by: Won Seok Park
 * Modified date: November 6, 2021 
 * 
 * CS374 HIGH PERFORMANCE COMPUTING, CALVIN UNIVERSITY
 */

#include "barrier.h"

long double pthreadReductionSum( long double reduceArray[], unsigned int numThreads, unsigned int id) {
	for ( int i=2; i <= numThreads; i*=2 ) {
		// Wait for all threads to reach here
		pthreadBarrier(numThreads);
		// If id is collecting thread (multiple of 2, 4, 8, 16...)
		if ( id % i == 0 ) {
			// Collect values
			reduceArray[id] += reduceArray[id + (i/2)];
		}
	}
	barrierCleanup();
	return reduceArray[0];
}