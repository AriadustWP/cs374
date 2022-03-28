/* Compute/draw mandelbrot set using MPI/MPE commands using Master-Worker
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
#include <math.h>
#include <sys/time.h>
#include <mpi.h>
#include <mpe.h>
#include "display.h"
#include <stdlib.h>


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
            y_center = 0.0;

   MPE_XGraph graph;
   MPI_Status status;

   int numProcesses = -1, wip_row_num = -1, next_row_number = 0;
   int draw_count = 0;
   double startTime, finalTime;

   // MPI stuff
   MPI_Init(&argc,&argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);

   // Dynamically allocating array for message passing
   int* recv_row = (int*) malloc(WINDOW_WIDTH * sizeof(MPI_INT));
   int* send_row = (int*) malloc(WINDOW_WIDTH * sizeof(MPI_INT));
/*
    // Uncomment this block for interactive use
    printf("\nEnter spacing (.005): "); fflush(stdout);
    scanf("%lf",&spacing);
    printf("\nEnter coordinates of center point (0,0): "); fflush(stdout);
    scanf("%lf %lf", &x_center, &y_center);
    printf("\nSpacing=%lf, center=(%lf,%lf)\n",
            spacing, x_center, y_center);
*/

   // Single Processor
   if (numProcesses == 1) {
      MPE_Open_graphics( &graph, MPI_COMM_WORLD, 
                     getDisplay(),
                     -1, -1,
                     WINDOW_WIDTH, WINDOW_HEIGHT, 0 );

      startTime = MPI_Wtime();

      for (ix = 0; ix < WINDOW_WIDTH; ix++)
      {
         for (iy = 0; iy < WINDOW_HEIGHT; iy++)
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
            // Colorful mandelbrot
            if (n < 15) {
               MPE_Draw_point(graph, ix, iy, MPE_BLUE);	  
            }
            else if (n < 19) {
               MPE_Draw_point(graph, ix, iy, MPE_WHITE);
            }
            else if (n < 24) {
               MPE_Draw_point(graph, ix, iy, MPE_YELLOW);
            }
            else if (n < 30) {
               MPE_Draw_point(graph, ix, iy, MPE_ORANGE);
            }
            else if (n < 40) {
               MPE_Draw_point(graph, ix, iy, MPE_PINK);
            }
            else {
               MPE_Draw_point(graph, ix, iy, MPE_BLACK);
            }
         }
      }

      finalTime = MPI_Wtime() - startTime;

      printf("\nClick in the window to continue...\n");
      printf("%f\n", finalTime);
      MPE_Get_mouse_press( graph, &ix, &iy, &button );

      MPE_Close_graphics( &graph );
   }
   // Multiprocessor
   else {
      // Master processor
      if (id == 0) {
         MPE_Open_graphics( &graph, MPI_COMM_WORLD, 
                           getDisplay(),
                           -1, -1,
                           WINDOW_WIDTH, WINDOW_HEIGHT, 0 );
                           
         // It starts with numProcesses as each process start with a row
         // -2 to incorporate "0" starting index and master processor not doing calculation
         next_row_number = numProcesses - 2;

         startTime = MPI_Wtime();

         // Continue sending next row until all rows were drawn
         while (draw_count < WINDOW_HEIGHT + 1) {
            MPI_Recv(recv_row, WINDOW_WIDTH, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            // Send row information to an idle processor
				if (next_row_number < WINDOW_HEIGHT) {
               MPI_Send(&next_row_number, 1, MPI_INT, status.MPI_SOURCE, 1, MPI_COMM_WORLD);
					next_row_number++;
			  	}
            
            // Draw image immediately (feat: Jack Westel)
            for(ix = 0; ix < WINDOW_WIDTH; ix++){
               MPE_Draw_point(graph, ix, status.MPI_TAG, recv_row[ix]);
            }

            draw_count++;			   
         }

         // Termination code
         for(int i = 1; i < numProcesses; i++){
            MPI_Send(&send_row, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
         }

         finalTime = MPI_Wtime() - startTime;
         
         printf("It took %fs\n", finalTime);
         printf("\nClick in the window to continue...\n");
         MPE_Get_mouse_press( graph, &ix, &iy, &button);
         
         MPE_Close_graphics( &graph );
      }
      // Worker Processor(s)
      else {
         // Each processor starts with a row to cut down communication overhead
         wip_row_num = id - 1;
         // Repeats until master processor say "no more job is left"
         while (1) {
            if(status.MPI_TAG == -1) {
               break;
            }

            for (ix = 0; ix < WINDOW_WIDTH; ix++) {
               c_real = (ix - 400) * SPACING - x_center;
               c_imag = (wip_row_num - 400) * SPACING - y_center;
               x = y = 0.0;
               n = 0;

               while (n < 50 && distance(x, y) < 4.0){
                  compute(x, y, c_real, c_imag, &x, &y);
                  n++;
               }
               // Colorful mandelbrot
               if (n < 13) {
                  send_row[ix] = MPE_BLUE;	  
               }
               else if (n < 16) {
                  send_row[ix] = MPE_CYAN;
               }
               else if (n < 20) {
                  send_row[ix] = MPE_GREEN;
               }
               else if (n < 25) {
                  send_row[ix] = MPE_YELLOW;
               }
               else if (n < 40) {
                  send_row[ix] = MPE_PINK;
               }
               else {
                  send_row[ix] = MPE_BLACK;
               }
            }
            // Send/Receive information to/from master processor 
            MPI_Send(send_row, WINDOW_WIDTH, MPI_INT, 0, wip_row_num, MPI_COMM_WORLD); 
            MPI_Recv(&wip_row_num, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);         
         }
      }
   }
   // Doing a good stewardship
   free(recv_row);
   free(send_row);
   MPI_Finalize();
   
   return 0;
}

