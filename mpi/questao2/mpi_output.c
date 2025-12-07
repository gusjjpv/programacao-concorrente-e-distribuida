#include <stdio.h>
#include <mpi.h>

int main(void) {
    int my_rank, comm_sz;
    MPI_Status status;

    MPI_Init(NULL, NULL); 
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz); 
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 

    if (my_rank != 0) {
        MPI_Recv(NULL, 0, MPI_BYTE, my_rank - 1, 0, MPI_COMM_WORLD, &status);
    }

    printf("Proc %d of %d > Does anyone have a toothpick?\n",
           my_rank, comm_sz);

    if (my_rank != comm_sz - 1) {
        MPI_Send(NULL, 0, MPI_BYTE, my_rank + 1, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}