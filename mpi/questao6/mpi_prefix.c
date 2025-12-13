#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(void) {
    int my_rank, comm_sz;
    MPI_Status status;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    int my_val = my_rank + 1;
    int prefix_sum = my_val;
    int temp = my_val;  

    int dest   = (my_rank + 1) % comm_sz;
    int source = (my_rank - 1 + comm_sz) % comm_sz;

    for (int step = 1; step < comm_sz; step++) {

        MPI_Sendrecv_replace(&temp, 1, MPI_INT,
                             dest, 0,
                             source, 0,
                             MPI_COMM_WORLD, &status);

        int sender = (my_rank - step + comm_sz) % comm_sz;

        if (sender < my_rank)
            prefix_sum += temp;
    }

    printf("Processo %d: prefix_sum = %d\n", my_rank, prefix_sum);

    MPI_Finalize();
    return 0;
}
