#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char** argv) {
    int my_rank, comm_sz;
    int my_val, temp_val;
    int prefix_sum;
    int dest, source;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    // valor local (poderia ser lido, aqui é apenas para teste)
    my_val = my_rank + 1;

    temp_val = my_val;
    prefix_sum = my_val;   // cada processo começa com seu valor

    dest = (my_rank + 1) % comm_sz;
    source = (my_rank - 1 + comm_sz) % comm_sz;

    // Para prefixos, fazemos p-1 etapas, mas só acumulamos até q+1 valores
    for (int etapa = 1; etapa < comm_sz; etapa++) {

        MPI_Sendrecv_replace(&temp_val, 1, MPI_INT,
                             dest, 0,
                             source, 0,
                             MPI_COMM_WORLD, &status);

        // acumula SOMENTE enquanto o processo ainda não recebeu valores
        // suficientes para formar seu prefixo
        if (etapa <= my_rank) {
            prefix_sum += temp_val;
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    printf("Processo %d -> prefixo = %d\n", my_rank, prefix_sum);

    MPI_Finalize();
    return 0;
}
