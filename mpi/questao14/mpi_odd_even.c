/*
 * File:     mpi_odd_even.c
 * Purpose:  Implement parallel odd-even sort of an array of 
 *           nonnegative ints (pointer-swapping merge)
 *
 * Compile:  mpicc -g -Wall -o mpi_odd_even mpi_odd_even.c
 * Run:
 *    mpiexec -n <p> mpi_odd_even <g|i> <global_n> 
 *
 * Notes:
 * 1. global_n must be evenly divisible by p
 * 2. Except for debug output, process 0 does all I/O
 * 3. Optional -DDEBUG compile flag for verbose output
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

const int RMAX = 100;

/* ----------------- Prototypes ----------------- */
void Usage(char* program);
void Generate_list(int local_A[], int local_n, int my_rank);
void Read_list(int local_A[], int local_n, int my_rank, int p, MPI_Comm comm);
void Print_list(int local_A[], int local_n, int rank);
void Print_local_lists(int local_A[], int local_n, int my_rank, int p, MPI_Comm comm);
void Print_global_list(int local_A[], int local_n, int my_rank, int p, MPI_Comm comm);

void Get_args(int argc, char* argv[], int* global_n_p, int* local_n_p, 
         char* gi_p, int my_rank, int p, MPI_Comm comm);

int  Compare(const void* a_p, const void* b_p);

/* Modified signatures to support pointer swapping */
void Sort(int **local_A_ptr, int local_n, int my_rank, int p, MPI_Comm comm);
void Odd_even_iter(int **local_A_ptr, int **buffer_ptr, int recv_buf[],
                   int local_n, int phase, int even_partner,
                   int odd_partner, int my_rank, int p, MPI_Comm comm);
void Merge_low(int *A, int recv_keys[], int buffer[], int local_n);
void Merge_high(int *A, int recv_keys[], int buffer[], int local_n);

/* ----------------- main ----------------- */
int main(int argc, char* argv[]) {

    int my_rank, p;
    char g_i;
    int *local_A;
    int global_n, local_n;
    MPI_Comm comm;

    MPI_Init(&argc, &argv);
    comm = MPI_COMM_WORLD;
    MPI_Comm_size(comm, &p);
    MPI_Comm_rank(comm, &my_rank);

    Get_args(argc, argv, &global_n, &local_n, &g_i, my_rank, p, comm);

    local_A = (int*) malloc(local_n * sizeof(int));
    if (local_A == NULL) {
        fprintf(stderr, "Proc %d: malloc failed\n", my_rank);
        MPI_Abort(comm, 1);
    }

    if (g_i == 'g') {
        Generate_list(local_A, local_n, my_rank);
        Print_local_lists(local_A, local_n, my_rank, p, comm);
    } else {
        Read_list(local_A, local_n, my_rank, p, comm);
    }

    MPI_Barrier(comm);
    double start = MPI_Wtime();

    /* Pass pointer-to-pointer so Sort can swap the array pointer */
    Sort(&local_A, local_n, my_rank, p, comm);

    double finish = MPI_Wtime();
    double elapsed = finish - start;

    if (my_rank == 0)
        printf("Tempo de execução: %e segundos\n", elapsed);

    Print_global_list(local_A, local_n, my_rank, p, comm);

    /* main frees the final array pointer (whichever buffer it points to) */
    free(local_A);

    MPI_Finalize();
    return 0;
}  /* main */

/* ----------------- Utility functions ----------------- */

void Generate_list(int local_A[], int local_n, int my_rank) {
   int i;
   srandom(my_rank+1);
   for (i = 0; i < local_n; i++)
      local_A[i] = random() % RMAX;
} 

void Usage(char* program) {
   fprintf(stderr, "usage:  mpirun -np <p> %s <g|i> <global_n>\n", program);
   fprintf(stderr, "   - p: the number of processes \n");
   fprintf(stderr, "   - g: generate random, distributed list\n");
   fprintf(stderr, "   - i: user will input list on process 0\n");
   fprintf(stderr, "   - global_n: number of elements in global list");
   fprintf(stderr, " (must be evenly divisible by p)\n");
   fflush(stderr);
}  /* Usage */

void Get_args(int argc, char* argv[], int* global_n_p, int* local_n_p, 
         char* gi_p, int my_rank, int p, MPI_Comm comm) {

   if (my_rank == 0) {
      if (argc != 3) {
         Usage(argv[0]);
         *global_n_p = -1;  /* Bad args, quit */
      } else {
         *gi_p = argv[1][0];
         if (*gi_p != 'g' && *gi_p != 'i') {
            Usage(argv[0]);
            *global_n_p = -1;  /* Bad args, quit */
         } else {
            *global_n_p = atoi(argv[2]);
            if (*global_n_p % p != 0) {
               Usage(argv[0]);
               *global_n_p = -1;
            }
         }
      }
   }  /* my_rank == 0 */

   MPI_Bcast(gi_p, 1, MPI_CHAR, 0, comm);
   MPI_Bcast(global_n_p, 1, MPI_INT, 0, comm);

   if (*global_n_p <= 0) {
      MPI_Finalize();
      exit(-1);
   }

   *local_n_p = *global_n_p / p;
#  ifdef DEBUG
   printf("Proc %d > gi = %c, global_n = %d, local_n = %d\n",
      my_rank, *gi_p, *global_n_p, *local_n_p);
   fflush(stdout);
#  endif

}  /* Get_args */

void Read_list(int local_A[], int local_n, int my_rank, int p,
         MPI_Comm comm) {
   int i;
   int *temp = NULL;

   if (my_rank == 0) {
      temp = (int*) malloc(p * local_n * sizeof(int));
      if (temp == NULL) {
         fprintf(stderr, "Proc 0: malloc failed in Read_list\n");
         MPI_Abort(comm, 1);
      }
      printf("Enter the elements of the list\n");
      for (i = 0; i < p * local_n; i++)
         scanf("%d", &temp[i]);
   } 

   MPI_Scatter(temp, local_n, MPI_INT, local_A, local_n, MPI_INT,
       0, comm);

   if (my_rank == 0)
      free(temp);
}  /* Read_list */

void Print_list(int local_A[], int local_n, int rank) {
   int i;
   printf("%d: ", rank);
   for (i = 0; i < local_n; i++)
      printf("%d ", local_A[i]);
   printf("\n");
}  /* Print_list */

void Print_local_lists(int local_A[], int local_n, 
         int my_rank, int p, MPI_Comm comm) {
   int*       A;
   int        q;
   MPI_Status status;

   if (my_rank == 0) {
      A = (int*) malloc(local_n*sizeof(int));
      if (A == NULL) {
         fprintf(stderr, "Proc 0: malloc failed in Print_local_lists\n");
         MPI_Abort(comm, 1);
      }
      Print_list(local_A, local_n, my_rank);
      for (q = 1; q < p; q++) {
         MPI_Recv(A, local_n, MPI_INT, q, 0, comm, &status);
         Print_list(A, local_n, q);
      }
      free(A);
   } else {
      MPI_Send(local_A, local_n, MPI_INT, 0, 0, comm);
   }
}  /* Print_local_lists */

void Print_global_list(int local_A[], int local_n, int my_rank, int p, 
      MPI_Comm comm) {
   int* A;
   int i, n;

   if (my_rank == 0) {
      n = p * local_n;
      A = (int*) malloc(n * sizeof(int));
      if (A == NULL) {
         fprintf(stderr, "Proc 0: malloc failed in Print_global_list\n");
         MPI_Abort(comm, 1);
      }
      MPI_Gather(local_A, local_n, MPI_INT, A, local_n, MPI_INT, 0, comm);
      printf("Global list:\n");
      for (i = 0; i < n; i++)
         printf("%d ", A[i]);
      printf("\n\n");
      free(A);
   } else {
      /* non-root processes supply their local arrays to the gather */
      MPI_Gather(local_A, local_n, MPI_INT, NULL, 0, MPI_INT, 0, comm);
   }
}

/* ----------------- Sort / Odd-even with safe pointer swapping ----------------- */

/*
 * Sort:
 *  - receives pointer to pointer local_A_ptr (so it can change where local_A points)
 *  - allocates two auxiliary buffers:
 *      recv_buf: target for MPI_Recv (can be overwritten safely each phase)
 *      buffer:   target for merge output
 *  - After each merge, swaps *local_A_ptr and buffer pointer
 *  - At the end frees the auxiliary buffer that is not the final *local_A_ptr (only if it
 *    was allocated inside Sort), and frees recv_buf.
 *
 * Note: main() will free the final *local_A_ptr.
 */
void Sort(int **local_A_ptr, int local_n, int my_rank, int p, MPI_Comm comm) {

    int phase;
    int even_partner, odd_partner;
    int *recv_buf = NULL;
    int *buffer = NULL;
    int *orig_local = *local_A_ptr;  /* pointer originally allocated by main */

    /* allocate auxiliary buffers */
    recv_buf = (int*) malloc(local_n * sizeof(int));
    buffer   = (int*) malloc(local_n * sizeof(int));
    if (recv_buf == NULL || buffer == NULL) {
        fprintf(stderr, "Proc %d: malloc failed in Sort\n", my_rank);
        MPI_Abort(comm, 1);
    }

    /* Find partners */
    if (my_rank % 2 != 0) {
        even_partner = my_rank - 1;
        odd_partner  = my_rank + 1;
        if (odd_partner == p) odd_partner = MPI_PROC_NULL;
    } else {
        even_partner = my_rank + 1;
        if (even_partner == p) even_partner = MPI_PROC_NULL;
        odd_partner  = my_rank - 1;
    }

    /* Sort local list using built-in quick sort (on whatever *local_A_ptr points to) */
    qsort(*local_A_ptr, local_n, sizeof(int), Compare);

    /* Main odd-even loop: Odd_even_iter will do the receive into recv_buf and merge into buffer.
       After calling Odd_even_iter, we must swap *local_A_ptr and buffer (so next phase uses result). */
    for (phase = 0; phase < p; phase++) {
        Odd_even_iter(local_A_ptr, &buffer, recv_buf, local_n, phase,
                      even_partner, odd_partner, my_rank, p, comm);

        /* swap pointers: *local_A_ptr points to new sorted buffer; buffer becomes old array */
        int *tmp = *local_A_ptr;
        *local_A_ptr = buffer;
        buffer = tmp;
        /* Note: After swap, buffer holds the old array (may be orig_local or previously allocated buffer).
           The next merge will write into buffer (overwriting its contents). */
    }

    /* free recv_buf (always allocated here) */
    free(recv_buf);

    /* buffer currently points to the spare array (the one not pointed by *local_A_ptr).
       If that spare array is not the original array allocated in main (orig_local),
       it was allocated in Sort and should be freed. If it equals orig_local, do not free;
       main will free orig_local later. */
    if (buffer != orig_local) {
        free(buffer);
    }

    /* leave *local_A_ptr pointing to the final sorted local array (main will free) */
}

/*
 * Odd_even_iter:
 *  - partner is determined by phase (even_partner or odd_partner)
 *  - MPI_Sendrecv is used to exchange current local array with partner; recv into recv_buf
 *  - call Merge_low or Merge_high to merge A and recv_buf into buffer (buffer is *buffer_ptr)
 *  - IMPORTANT: Odd_even_iter does NOT swap pointers itself; Sort will swap after Odd_even_iter returns.
 *
 * Note: buffer_ptr is pointer-to-pointer so we can pass the current buffer pointer by reference.
 */
void Odd_even_iter(int **local_A_ptr, int **buffer_ptr, int recv_buf[],
                   int local_n, int phase, int even_partner,
                   int odd_partner, int my_rank, int p, MPI_Comm comm) {

    MPI_Status status;
    int partner = (phase % 2 == 0 ? even_partner : odd_partner);

    if (partner >= 0 && partner != MPI_PROC_NULL) {

        /* send current array (*local_A_ptr) and receive partner into recv_buf */
        MPI_Sendrecv(*local_A_ptr, local_n, MPI_INT, partner, 0,
                     recv_buf, local_n, MPI_INT, partner, 0, comm, &status);

        /* do merge: write result into the buffer pointed by *buffer_ptr */
        if (my_rank % 2 == 0) {
            if (phase % 2 == 0)
                Merge_low(*local_A_ptr, recv_buf, *buffer_ptr, local_n);
            else
                Merge_high(*local_A_ptr, recv_buf, *buffer_ptr, local_n);
        } else {
            if (phase % 2 == 0)
                Merge_high(*local_A_ptr, recv_buf, *buffer_ptr, local_n);
            else
                Merge_low(*local_A_ptr, recv_buf, *buffer_ptr, local_n);
        }
    }
    /* if partner < 0 (MPI_PROC_NULL), do nothing (idle in this phase) */
}

/*
 * Merge_low:
 *  - A: pointer to current local array
 *  - recv_keys: array received from partner
 *  - buffer: output array (must have size local_n)
 *  - merges the smallest local_n elements of (A U recv_keys) into buffer
 *
 * Note: does NOT swap pointers; Sort will swap afterwards.
 */
void Merge_low(int *A, int recv_keys[], int buffer[], int local_n) {
    int m_i = 0, r_i = 0, t_i = 0;

    while (t_i < local_n) {
        if (A[m_i] <= recv_keys[r_i]) {
            buffer[t_i++] = A[m_i++];
        } else {
            buffer[t_i++] = recv_keys[r_i++];
        }
    }
}

/*
 * Merge_high:
 *  - merges the largest local_n elements of (A U recv_keys) into buffer
 */
void Merge_high(int *A, int recv_keys[], int buffer[], int local_n) {
    int ai = local_n - 1;
    int bi = local_n - 1;
    int ci = local_n - 1;

    while (ci >= 0) {
        if (A[ai] >= recv_keys[bi]) {
            buffer[ci--] = A[ai--];
        } else {
            buffer[ci--] = recv_keys[bi--];
        }
    }
}

/* ----------------- Compare function ----------------- */
int Compare(const void* a_p, const void* b_p) {
    int a = *((int*)a_p);
    int b = *((int*)b_p);
    return (a > b) - (a < b);
}
