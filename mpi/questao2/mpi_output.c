#include <stdio.h>
#include <mpi.h> 

int main(void) {
    int my_rank, comm_sz;

    MPI_Init(NULL, NULL); 
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz); 
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 

    for (int q = 0; q < comm_sz; q++) {
        if (my_rank == q) {
            printf("Proc %d of %d > Does anyone have a toothpick?\n",
                   my_rank, comm_sz);
            fflush(stdout);
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}