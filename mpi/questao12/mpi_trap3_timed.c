/* File:     mpi_trap3_timed.c
 * Purpose:  Parallel trapezoidal rule with per-process timing.
 *
 * Compile:  mpicc -g -Wall -o mpi_trap3_timed mpi_trap3_timed.c
 * Run:      mpiexec -n <number of processes> ./mpi_trap3_timed
 *
 * Notes:
 *   - Assumes n is evenly divisible by comm_sz (same as original).
 *   - Measures wall-clock time per process using MPI_Wtime().
 *   - Time measured from just before local Trap(...) call until after MPI_Reduce returns.
 */

#include <stdio.h>
#include <mpi.h>

/* Get the input values */
void Get_input(int my_rank, int comm_sz, double* a_p, double* b_p,
      int* n_p);

/* Calculate local integral  */
double Trap(double left_endpt, double right_endpt, int trap_count,
   double base_len);

/* Function we're integrating */
double f(double x);

int main(void) {
   int my_rank, comm_sz, n, local_n;
   double a, b, h, local_a, local_b;
   double local_int, total_int;

   MPI_Init(NULL, NULL);
   MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
   MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

   Get_input(my_rank, comm_sz, &a, &b, &n);

   h = (b - a) / n;
   local_n = n / comm_sz;

   local_a = a + my_rank * local_n * h;
   local_b = local_a + local_n * h;

   /* Synchronize before timing and computation */
   MPI_Barrier(MPI_COMM_WORLD);

   double t_start = MPI_Wtime();

   local_int = Trap(local_a, local_b, local_n, h);

   /* Include reduction time in the measurement (as in Pacheco Sec. 3.6) */
   MPI_Reduce(&local_int, &total_int, 1, MPI_DOUBLE, MPI_SUM, 0,
         MPI_COMM_WORLD);

   double t_end = MPI_Wtime();
   double local_time = t_end - t_start;

   /* Gather times at root so we can print them in order */
   double *times = NULL;
   if (my_rank == 0) {
      times = malloc(comm_sz * sizeof(double));
   }

   MPI_Gather(&local_time, 1, MPI_DOUBLE, times, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

   /* Print the result and the per-process timings (only rank 0) */
   if (my_rank == 0) {
      printf("With n = %d trapezoids and p = %d processes,\n", n, comm_sz);
      printf("Estimate of integral from %f to %f = %.15e\n", a, b, total_int);
      printf("\nPer-process times (seconds):\n");
      for (int i = 0; i < comm_sz; i++) {
         printf("  Process %2d: %12.8f s\n", i, times[i]);
      }
      free(times);
   }

   MPI_Finalize();
   return 0;
} /* main */

/*------------------------------------------------------------------
 * Function:     Get_input
 * Purpose:      Get the user input: the left and right endpoints
 *               and the number of trapezoids
 */
void Get_input(int my_rank, int comm_sz, double* a_p, double* b_p,
      int* n_p) {

   if (my_rank == 0) {
      printf("Enter a, b, and n\n");
      fflush(stdout);
      scanf("%lf %lf %d", a_p, b_p, n_p);
   }
   MPI_Bcast(a_p, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
   MPI_Bcast(b_p, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
   MPI_Bcast(n_p, 1, MPI_INT, 0, MPI_COMM_WORLD);
}  /* Get_input */

/*------------------------------------------------------------------
 * Function:     Trap
 * Purpose:      Serial function for estimating a definite integral
 */
double Trap(
      double left_endpt  /* in */,
      double right_endpt /* in */,
      int    trap_count  /* in */,
      double base_len    /* in */) {
   double estimate, x;
   int i;

   estimate = (f(left_endpt) + f(right_endpt)) / 2.0;
   for (i = 1; i <= trap_count - 1; i++) {
      x = left_endpt + i * base_len;
      estimate += f(x);
   }
   estimate = estimate * base_len;

   return estimate;
} /* Trap */

/*------------------------------------------------------------------
 * Function:    f
 * Purpose:     Compute value of function to be integrated
 */
double f(double x) {
   return x * x;
} /* f */
