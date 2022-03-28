/* Compute/draw mandelbrot set using MPI/MPE commands
 *
 * Written Winter, 1998, W. David Laverell.
 *
 * Refactored Winter 2002, Joel Adams. 
 */

#include <stdlib.h>
#include <stdio.h>
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
    const double SPACING     = 0.003;

    int        n        = 0,
               ix       = 0,
               iy       = 0,
               button   = 0,
               id       = 0,
               numProcesses = 0;
    double     x        = 0.0,
               y        = 0.0,
               c_real   = 0.0,
               c_imag   = 0.0,
               x_center = 1.16,
               y_center = -0.10;

   double startTime, totalTime;

   MPE_XGraph graph;

   MPI_Init(&argc,&argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &id);
   MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);

   int * chunkPoints = (int *) malloc( (WINDOW_HEIGHT * WINDOW_WIDTH * sizeof(MPE_Color)) / numProcesses );
   int * chunkFinal = (int *) malloc( WINDOW_HEIGHT * WINDOW_WIDTH * sizeof(MPE_Color) );

   int chunkSize = WINDOW_HEIGHT / numProcesses;
   int chunkRemain = WINDOW_HEIGHT % numProcesses;
   int i = 0;
   
   startTime = MPI_Wtime();
    
/*
    // Uncomment this block for interactive use
    printf("\nEnter spacing (.005): "); fflush(stdout);
    scanf("%lf",&spacing);
    printf("\nEnter coordinates of center point (0,0): "); fflush(stdout);
    scanf("%lf %lf", &x_center, &y_center);
    printf("\nSpacing=%lf, center=(%lf,%lf)\n",
            spacing, x_center, y_center);
*/

   // When array cleanly divides
   if(chunkRemain == 0) {
      for (iy = id*chunkSize; iy < (id+1)*chunkSize; iy++)
      {
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
               chunkPoints[(i) * WINDOW_WIDTH + ix] = MPE_PINK;
            } else {
               chunkPoints[(i) * WINDOW_WIDTH + ix] = MPE_BLACK;
            }
         }
         i++;
      }
   }
   // When array doesn't cleanly divide 
   else {
      // printf("chunksize: %d, remainder: %d\n", chunkSize, chunkRemain);
      // Last processor deals with remainder
      if (id == numProcesses - 1) {
         for (iy = id*chunkSize; iy < id*chunkSize + chunkRemain; iy++) {
            for (ix = 0; ix < WINDOW_WIDTH; ix++) {
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
                  chunkPoints[(i) * WINDOW_WIDTH + ix] = MPE_PINK;
               } else {
                  chunkPoints[(i) * WINDOW_WIDTH + ix] = MPE_BLACK;
               }
            }
            i++;
         }
      } 
      // Other processors doing parallel things
      else {
         for (iy = id*chunkSize; iy < (id+1)*chunkSize; iy++) {
            for (ix = 0; ix < WINDOW_WIDTH; ix++) {
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
                  chunkPoints[(i) * WINDOW_WIDTH + ix] = MPE_PINK;
               } else {
                  chunkPoints[(i) * WINDOW_WIDTH + ix] = MPE_BLACK;
               }
            }
            printf("ix: %d\n", ix);
            i++;
         }
      }
   }

   
   MPI_Gather(chunkPoints, chunkSize * WINDOW_WIDTH, MPI_INT, chunkFinal, chunkSize * WINDOW_WIDTH, MPI_INT, 0, MPI_COMM_WORLD);
   
   totalTime = MPI_Wtime() - startTime;
   
   // pause until mouse-click so the program doesn't terminate
   if (id == 0) {  
      printf("\nClick in the window to continue...%d\n",iy);
      printf("\nProcessing time: %f\n",totalTime);
      MPE_Open_graphics( &graph, MPI_COMM_WORLD, 
                     getDisplay(),
                     -1, -1,
                     WINDOW_WIDTH, WINDOW_HEIGHT, 0 );
         for (iy = 0; iy < WINDOW_HEIGHT; iy++) {
            for (ix = 0; ix < WINDOW_WIDTH; ix++) {
               MPE_Draw_point(graph, ix, iy, chunkFinal[WINDOW_WIDTH*iy + ix]);
            }
         }
      printf("\nClick in the window to continue...\n");
      MPE_Get_mouse_press( graph, &ix, &iy, &button );
      MPE_Close_graphics( &graph );
   }

   free(chunkPoints);
   free(chunkFinal);

   MPI_Finalize();
   return 0;
}

