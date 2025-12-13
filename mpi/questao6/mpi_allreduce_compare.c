#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define MSG_SIZE 100000000

int main() {
    int my_rank, comm_sz;
    MPI_Status status;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    int *my_array = malloc(MSG_SIZE * sizeof(int));
    int *temp_array = malloc(MSG_SIZE * sizeof(int));
    int *recv_array = malloc(MSG_SIZE * sizeof(int));

    for (int i = 0; i < MSG_SIZE; i++)
        my_array[i] = 1;

    /*                RING ALLREDUCE                          */
    for (int i = 0; i < MSG_SIZE; i++)
        temp_array[i] = my_array[i];

    int dest = (my_rank + 1) % comm_sz;
    int source = (my_rank - 1 + comm_sz) % comm_sz;

    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();

    for (int step = 1; step < comm_sz; step++) {

        MPI_Sendrecv_replace(temp_array, MSG_SIZE, MPI_INT,
                             dest, 0,
                             source, 0,
                             MPI_COMM_WORLD, &status);

        // acumular soma
        for (int i = 0; i < MSG_SIZE; i++)
            my_array[i] += temp_array[i];
    }

    double end = MPI_Wtime();
    double local_time = end - start;

    double ring_time;
    MPI_Reduce(&local_time, &ring_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    /*                BUTTERFLY ALLREDUCE                     */

    for (int i = 0; i < MSG_SIZE; i++)
        my_array[i] = 1;

    MPI_Barrier(MPI_COMM_WORLD);
    start = MPI_Wtime();

    int steps = 0;
    while ((1 << steps) < comm_sz) steps++;

    for (int i = 0; i < steps; i++) {
        int partner = my_rank ^ (1 << i);

        MPI_Sendrecv(my_array, MSG_SIZE, MPI_INT,
                     partner, 0,
                     recv_array, MSG_SIZE, MPI_INT,
                     partner, 0,
                     MPI_COMM_WORLD, &status);

        // soma a parte recebida
        for (int j = 0; j < MSG_SIZE; j++)
            my_array[j] += recv_array[j];
    }

    end = MPI_Wtime();
    local_time = end - start;

    double butterfly_time;
    MPI_Reduce(&local_time, &butterfly_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (my_rank == 0) {
        printf("\nTempo total (Ring):      %.6f s\n", ring_time);
        printf("Tempo total (Butterfly): %.6f s\n", butterfly_time);

        printf("\n(msg = %d)\n", MSG_SIZE);
        printf("\n(comm_sz = %d processos)\n\n", comm_sz);
    }

    free(my_array);
    free(temp_array);
    free(recv_array);

    MPI_Finalize();
    return 0;
}