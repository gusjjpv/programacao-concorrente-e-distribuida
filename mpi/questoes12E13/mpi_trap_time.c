/* 
 * File:     mpi_trap3_time_ll.c
 * Purpose:  Parallel trapezoidal rule with timing (min, mean, median)
 *           using 64-bit counters to support very large n.
 *
 * IPP: Section 3.4.2 (parallel trapezoidal rule)
 *      Section 3.6 (performance measurement)
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define REPS 5   /* Número de repetições para medir tempos */

/* Protótipos */
void Get_input(int my_rank, int comm_sz,
               double* a_p, double* b_p, long long* n_p);

double Trap(double left_endpt, double right_endpt,
            long long trap_count, double base_len);

double f(double x);

/* ---------------------------- MAIN ------------------------------ */
int main(void) {
   int my_rank, comm_sz;
   long long n, local_n;
   double a, b, h, local_a, local_b;
   double local_int, total_int;

   double times[REPS];

   MPI_Init(NULL, NULL);
   MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
   MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

   Get_input(my_rank, comm_sz, &a, &b, &n);

   /* Comprimento da base */
   h = (b - a) / (double) n;

   /* Número de trapézios locais */
   local_n = n / comm_sz;

   local_a = a + my_rank * local_n * h;
   local_b = local_a + local_n * h;

   /* -------------------------------------------------------------
    * Medição de desempenho (Seção 3.6 — Pacheco)
    * ------------------------------------------------------------- */
   for (int rep = 0; rep < REPS; rep++) {

      MPI_Barrier(MPI_COMM_WORLD);
      double start = MPI_Wtime();

      local_int = Trap(local_a, local_b, local_n, h);

      MPI_Reduce(&local_int, &total_int, 1,
                 MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

      MPI_Barrier(MPI_COMM_WORLD);
      double finish = MPI_Wtime();

      times[rep] = finish - start;

      if (my_rank == 0 && rep == REPS - 1) {
         printf("Última execução, integral = %.15e (não é análise de tempo)\n",
                total_int);
      }
   }

   /* -------------------------------------------------------------
    * Cálculo de mínimo, média e mediana (rank 0)
    * ------------------------------------------------------------- */
   if (my_rank == 0) {

      /* Ordena tempos (REPS pequeno) */
      for (int i = 0; i < REPS - 1; i++)
         for (int j = i + 1; j < REPS; j++)
            if (times[j] < times[i]) {
               double tmp = times[i];
               times[i] = times[j];
               times[j] = tmp;
            }

      double min = times[0];

      double mean = 0.0;
      for (int i = 0; i < REPS; i++)
         mean += times[i];
      mean /= REPS;

      double median;
      if (REPS % 2 == 1)
         median = times[REPS / 2];
      else
         median = (times[REPS / 2 - 1] + times[REPS / 2]) / 2.0;

      printf("\n=== Medições de tempo (%d execuções) ===\n", REPS);
      printf("Tempo mínimo : %e s\n", min);
      printf("Tempo médio  : %e s\n", mean);
      printf("Tempo mediano: %e s\n", median);
      printf("========================================\n");
   }

   MPI_Finalize();
   return 0;
}

/* ------------------------ Get_input ---------------------------- */
void Get_input(int my_rank, int comm_sz,
               double* a_p, double* b_p, long long* n_p) {

   if (my_rank == 0) {
      printf("Enter a, b, and n\n");
      scanf("%lf %lf %lld", a_p, b_p, n_p);
   }

   MPI_Bcast(a_p, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
   MPI_Bcast(b_p, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
   MPI_Bcast(n_p, 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);
}

/* --------------------------- Trap ------------------------------ */
double Trap(double left_endpt, double right_endpt,
            long long trap_count, double base_len) {

   double estimate, x;
   long long i;

   estimate = (f(left_endpt) + f(right_endpt)) / 2.0;

   for (i = 1; i <= trap_count - 1; i++) {
      x = left_endpt + i * base_len;
      estimate += f(x);
   }

   estimate = estimate * base_len;
   return estimate;
}

/* ---------------------------- f(x) ----------------------------- */
double f(double x) {
   return x * x;
}
