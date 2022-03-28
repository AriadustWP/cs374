/* Compute/draw mandelbrot set using MPI/MPE commands using slice method
 *   to parallelize the calculation process
 *
 * Written Winter, 1998, W. David Laverell.
 *
 * Refactored Winter 2002, Joel Adams. 
 *
 * Modified by: Won Seok Park (with additional help from Jack Westel)
 * Modified date: October 13, 2021
 * 
 * CS374 HIGH PERFORMANCE COMPUTING, CALVIN UNIVERSITY
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <mpi.h>
#include <mpe.h>
#include "display.h"


/* compute the Mandelbrot-set function for a given
 *  point in the complex plane.
 *
 * Receive: doubles x and y,
 *          complex c.
 * Modify: doubles ans_x and ans_y.
 * POST: ans_x and ans_y contain the results of the mandelbrot-set
 *        function for x, y, and c.
 */
void compute(double x, double y, double c_real, double c_imag,
              double *ans_x, double *ans_y)
{
        *ans_x = x*x - y*y + c_real;
        *ans_y = 2*x*y + c_imag;
}

/* compute the 'distance' between x and y.
 *
 * Receive: doubles x and y.
 * Return: x^2 + y^2.
 */
double distance(double x, double y)
{
        return x*x + y*y;
}


int main(int argc, char* argv[])
{
   const int  WINDOW_HEIGHT = 900;
   const int  WINDOW_WIDTH  = 1200;
   const double SPACING     = 0.0025;

   int      n        = 0,
            ix       = 0,
            iy       = 0,
            button   = 0,
            id       = 0;
   double   x        = 0.0,
            y        = 0.0,
            c_real   = 0.0,
            c_imag   = 0.0,
            x_center = 1.16,
            y_center = 0.16;
   
   int* local_pixel_data; // stored ix, iy, n
   int* pixel_data;

   MPE_XGraph graph;

   // MPI related variables
   int numProcesses;

   MPI_Init(&argc,&argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &id);
   MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);

   local_pixel_data = (int *) calloc (WINDOW_WIDTH * WINDOW_HEIGHT , sizeof(int));
   pixel_data = (int *) calloc (WINDOW_WIDTH * WINDOW_HEIGHT , sizeof(int));

   double startTime, totalTime;

/*
   // Uncomment this block for interactive use
   printf("\nEnter spacing (.005): "); fflush(stdout);
   scanf("%lf",&spacing);
   printf("\nEnter coordinates of center point (0,0): "); fflush(stdout);
   scanf("%lf %lf", &x_center, &y_center);
   printf("\nSpacing=%lf, center=(%lf,%lf)\n",
         spacing, x_center, y_center);
*/

   startTime = MPI_Wtime();

   for (iy = id; iy < WINDOW_HEIGHT; iy+=numProcesses)
   {
      // Slicing parallelization
      for (ix = 0; ix < WINDOW_WIDTH; ix++)
      {
         c_real = (ix - 400) * SPACING - x_center;
         c_imag = (iy - 400) * SPACING - y_center;
         x = y = 0.0;
         n = 0;

         while (n < 50 && distance(x, y) < 4.0)
         {
            compute(x, y, c_real, c_imag, &x, &y);
            n++;
         }

         if (n < 50) {
            local_pixel_data[(iy) * WINDOW_WIDTH + ix] = MPE_PINK;
         } else {
            local_pixel_data[(iy) * WINDOW_WIDTH + ix] = MPE_BLACK;
         }
      }
   }

   MPI_Reduce(local_pixel_data, pixel_data, WINDOW_HEIGHT * WINDOW_WIDTH, MPI_INT, MPI_SUM, 0,
            MPI_COMM_WORLD);
   totalTime = MPI_Wtime() - startTime;

   // pause until mouse-click so the program doesn't terminate
   if (id == 0) {
      MPE_Open_graphics( &graph, MPI_COMM_WORLD, 
                     getDisplay(),
                     -1, -1,
                     WINDOW_WIDTH, WINDOW_HEIGHT, 0 );
      // Draw image of mandelbrot
      printf("processing time: %f\n",totalTime);
      for (iy = 0; iy < WINDOW_HEIGHT; iy++) {
         for (ix = 0; ix < WINDOW_WIDTH; ix++) {
            MPE_Draw_point(graph, ix, iy, pixel_data[WINDOW_WIDTH*iy + ix]);
         }
      }

      printf("\nClick in the window to continue...\n");
      MPE_Get_mouse_press( graph, &ix, &iy, &button );
      MPE_Close_graphics( &graph );
   }

   free(local_pixel_data);
   free(pixel_data);

   MPI_Finalize();
   return 0;
}

