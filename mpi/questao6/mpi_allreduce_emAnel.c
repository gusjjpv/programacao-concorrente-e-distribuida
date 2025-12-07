#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char** argv) {
    int my_rank, comm_sz;
    int my_val, temp_val, sum;
    int dest, source;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    // Cada processo gera seu valor local
    my_val = my_rank + 1;   // só para teste (P0=1, P1=2, ...)

    temp_val = my_val;
    sum = my_val;

    dest = (my_rank + 1) % comm_sz;            // envia para o próximo no anel
    source = (my_rank - 1 + comm_sz) % comm_sz; // recebe do anterior

    // Algoritmo Allreduce via anel (ring)
    for (int i = 1; i < comm_sz; i++) {
        MPI_Sendrecv_replace(&temp_val, 1, MPI_INT,
                             dest, 0,
                             source, 0,
                             MPI_COMM_WORLD, &status);

        sum += temp_val;
    }

    MPI_Barrier(MPI_COMM_WORLD);

    printf("Processo %d -> soma global calculada: %d\n", my_rank, sum);

    MPI_Finalize();
    return 0;
}