/*
 * File:     mpi_odd_even.c
 * Purpose:  Parallel odd-even sort, modified to measure execution times:
 *           - minimum time
 *           - mean time
 *           - median time
 *           following Section 3.6 (IPP - Peter Pacheco).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define REPS 5         /* Number of repetitions for timing */
const int RMAX = 100;

/* Function prototypes */
void Usage(char* program);
void Print_list(int local_A[], int local_n, int rank);
void Merge_low(int local_A[], int temp_B[], int temp_C[], int local_n);
void Merge_high(int local_A[], int temp_B[], int temp_C[], int local_n);
void Generate_list(int local_A[], int local_n, int my_rank);
int  Compare(const void* a_p, const void* b_p);

void Get_args(int argc, char* argv[], int* global_n_p, int* local_n_p, 
              char* gi_p, int my_rank, int p, MPI_Comm comm);
void Sort(int local_A[], int local_n, int my_rank, 
          int p, MPI_Comm comm);
void Odd_even_iter(int local_A[], int temp_B[], int temp_C[],
          int local_n, int phase, int even_partner, int odd_partner,
          int my_rank, int p, MPI_Comm comm);
void Print_local_lists(int local_A[], int local_n, 
          int my_rank, int p, MPI_Comm comm);
void Print_global_list(int local_A[], int local_n, int my_rank,
          int p, MPI_Comm comm);
void Read_list(int local_A[], int local_n, int my_rank, int p,
          MPI_Comm comm);

/*-------------------------------------------------------------------*/
int main(int argc, char* argv[]) {

   int my_rank, p;
   char g_i;
   int *local_A;
   int global_n;
   int local_n;
   MPI_Comm comm;
   double times[REPS]; /* store times of each repetition */

   MPI_Init(&argc, &argv);
   comm = MPI_COMM_WORLD;
   MPI_Comm_size(comm, &p);
   MPI_Comm_rank(comm, &my_rank);

   /* Read input */
   Get_args(argc, argv, &global_n, &local_n, &g_i, my_rank, p, comm);

   local_A = (int*) malloc(local_n * sizeof(int));

   /* ----------------------------------------------------------
    * Perform REPS repetitions to measure execution time
    * ---------------------------------------------------------- */
   for (int rep = 0; rep < REPS; rep++) {

      /* regenerate input data each repetition */
      if (g_i == 'g') {
         Generate_list(local_A, local_n, my_rank);
      } else {
         Read_list(local_A, local_n, my_rank, p, comm);
      }

      MPI_Barrier(comm);
      double start = MPI_Wtime();

      /* Run parallel odd-even sort */
      Sort(local_A, local_n, my_rank, p, comm);

      MPI_Barrier(comm);
      double finish = MPI_Wtime();

      times[rep] = finish - start;
   }

   /* ----------------------------------------------------------
    * Compute min, mean, median (done by rank 0)
    * ---------------------------------------------------------- */
   if (my_rank == 0) {

      /* sort times[] to compute median */
      for (int i = 0; i < REPS-1; i++)
         for (int j = i+1; j < REPS; j++)
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

      double median =
         (REPS % 2 == 1) ? times[REPS/2]
                         : (times[REPS/2 - 1] + times[REPS/2]) / 2.0;

      printf("\n================ Timing results ================\n");
      printf("Repetitions: %d\n", REPS);
      printf("Minimum time : %e seconds\n", min);
      printf("Mean time    : %e seconds\n", mean);
      printf("Median time  : %e seconds\n", median);
      printf("================================================\n\n");
   }

   /* Print final list */
   free(local_A);

   MPI_Finalize();
   return 0;
} /* main */


/*-------------------------------------------------------------------
 * Function:   Generate_list
 * Purpose:    Fill list with random ints
 */
void Generate_list(int local_A[], int local_n, int my_rank) {
   int i;
   srandom(my_rank+1);
   for (i = 0; i < local_n; i++)
      local_A[i] = random() % RMAX;
}


/*-------------------------------------------------------------------
 * Function:  Usage
 */
void Usage(char* program) {
   fprintf(stderr, "usage:  mpirun -np <p> %s <g|i> <global_n>\n",
       program);
   fprintf(stderr, "   global_n must be divisible by p\n");
   fflush(stderr);
}


/*-------------------------------------------------------------------
 * Function:    Get_args
 */
void Get_args(int argc, char* argv[], int* global_n_p, int* local_n_p, 
         char* gi_p, int my_rank, int p, MPI_Comm comm) {

   if (my_rank == 0) {
      if (argc != 3) {
         Usage(argv[0]);
         *global_n_p = -1;
      } else {
         *gi_p = argv[1][0];
         *global_n_p = atoi(argv[2]);
         if (*global_n_p % p != 0)
            *global_n_p = -1;
      }
   }

   MPI_Bcast(gi_p, 1, MPI_CHAR, 0, comm);
   MPI_Bcast(global_n_p, 1, MPI_INT, 0, comm);

   if (*global_n_p <= 0) {
      MPI_Finalize();
      exit(-1);
   }

   *local_n_p = *global_n_p/p;
}


/*-------------------------------------------------------------------
 * Function:   Read_list
 */
void Read_list(int local_A[], int local_n, int my_rank, int p,
         MPI_Comm comm) {

   int *temp;

   if (my_rank == 0) {
      temp = (int*) malloc(p*local_n*sizeof(int));
      printf("Enter the elements of the list\n");
      for (int i = 0; i < p*local_n; i++)
         scanf("%d", &temp[i]);
   } 

   MPI_Scatter(temp, local_n, MPI_INT,
               local_A, local_n, MPI_INT, 0, comm);

   if (my_rank == 0)
      free(temp);
}


/*-------------------------------------------------------------------
 * Function:   Print_global_list
 */
void Print_global_list(int local_A[], int local_n, int my_rank, int p, 
      MPI_Comm comm) {
   int* A;
   if (my_rank == 0) {
      A = (int*) malloc(p*local_n*sizeof(int));
   }

   MPI_Gather(local_A, local_n, MPI_INT,
              A,        local_n, MPI_INT,
              0, comm);

   if (my_rank == 0) {
      printf("Global sorted list:\n");
      for (int i = 0; i < p*local_n; i++)
         printf("%d ", A[i]);
      printf("\n\n");
      free(A);
   }
}


/*-------------------------------------------------------------------
 * qsort comparator
 */
int Compare(const void* a_p, const void* b_p) {
   int a = *((int*)a_p);
   int b = *((int*)b_p);
   return (a > b) - (a < b);
}


/*-------------------------------------------------------------------
 * Sort: odd-even transposition sort
 */
void Sort(int local_A[], int local_n, int my_rank, 
         int p, MPI_Comm comm) {

   int phase;
   int *temp_B = malloc(local_n*sizeof(int));
   int *temp_C = malloc(local_n*sizeof(int));

   int even_partner, odd_partner;

   if (my_rank % 2 != 0) {
      even_partner = my_rank - 1;
      odd_partner = my_rank + 1;
      if (odd_partner == p) odd_partner = MPI_PROC_NULL;
   } else {
      even_partner = my_rank + 1;
      if (even_partner == p) even_partner = MPI_PROC_NULL;
      odd_partner = my_rank - 1;
   }

   qsort(local_A, local_n, sizeof(int), Compare);

   for (phase = 0; phase < p; phase++) {
      Odd_even_iter(local_A, temp_B, temp_C, local_n, phase,
                    even_partner, odd_partner, my_rank, p, comm);
   }

   free(temp_B);
   free(temp_C);
}


/*-------------------------------------------------------------------
 * Odd-even iteration
 */
void Odd_even_iter(int local_A[], int temp_B[], int temp_C[],
        int local_n, int phase, int even_partner, int odd_partner,
        int my_rank, int p, MPI_Comm comm) {

   MPI_Status status;

   if (phase % 2 == 0) {
      if (even_partner >= 0) {
         MPI_Sendrecv(local_A, local_n, MPI_INT, even_partner, 0,
                      temp_B, local_n, MPI_INT, even_partner, 0,
                      comm, &status);

         if (my_rank % 2 != 0)
            Merge_high(local_A, temp_B, temp_C, local_n);
         else
            Merge_low(local_A, temp_B, temp_C, local_n);
      }
   } else {
      if (odd_partner >= 0) {
         MPI_Sendrecv(local_A, local_n, MPI_INT, odd_partner, 0,
                      temp_B, local_n, MPI_INT, odd_partner, 0,
                      comm, &status);

         if (my_rank % 2 != 0)
            Merge_low(local_A, temp_B, temp_C, local_n);
         else
            Merge_high(local_A, temp_B, temp_C, local_n);
      }
   }
}


/*-------------------------------------------------------------------
 * Merge_low
 */
void Merge_low(int my_keys[], int recv_keys[], int temp_keys[],
               int local_n) {

   int m_i = 0, r_i = 0, t_i = 0;

   while (t_i < local_n) {
      if (my_keys[m_i] <= recv_keys[r_i])
         temp_keys[t_i++] = my_keys[m_i++];
      else
         temp_keys[t_i++] = recv_keys[r_i++];
   }

   memcpy(my_keys, temp_keys, local_n*sizeof(int));
}


/*-------------------------------------------------------------------
 * Merge_high
 */
void Merge_high(int my_keys[], int recv_keys[], int temp_keys[],
                int local_n) {

   int ai = local_n-1;
   int bi = local_n-1;
   int ci = local_n-1;

   while (ci >= 0) {
      if (my_keys[ai] >= recv_keys[bi])
         temp_keys[ci--] = my_keys[ai--];
      else
         temp_keys[ci--] = recv_keys[bi--];
   }

   memcpy(my_keys, temp_keys, local_n*sizeof(int));
}


/*-------------------------------------------------------------------
 * Print list (debug)
 */
void Print_list(int local_A[], int local_n, int rank) {
   printf("%d: ", rank);
   for (int i = 0; i < local_n; i++)
      printf("%d ", local_A[i]);
   printf("\n");
}


/*-------------------------------------------------------------------
 * Print all local lists
 */
void Print_local_lists(int local_A[], int local_n, 
         int my_rank, int p, MPI_Comm comm) {

   int* A;
   MPI_Status status;

   if (my_rank == 0) {
      A = (int*) malloc(local_n*sizeof(int));
      Print_list(local_A, local_n, my_rank);

      for (int q = 1; q < p; q++) {
         MPI_Recv(A, local_n, MPI_INT, q, 0, comm, &status);
         Print_list(A, local_n, q);
      }
      free(A);

   } else {
      MPI_Send(local_A, local_n, MPI_INT, 0, 0, comm);
   }
}
