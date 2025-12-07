#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char** argv) {

    int my_rank, comm_sz;
    int local_value;       // valor inicial x[i]
    int prefix_value;      // valor acumulado no processo
    int d, k, dist;        // variáveis de controle
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    // Verificando se n é potência de 2
    int tmp = comm_sz;
    k = 0;
    while (tmp > 1) {
        if (tmp % 2 != 0) {
            if (my_rank == 0)
                printf("Erro: número de processos deve ser potência de 2.\n");
            MPI_Finalize();
            return 1;
        }
        tmp /= 2;
        k++;
    }

    // Cada processo recebe seu valor x[i]
    // Aqui apenas pedimos valores ao processo 0 para facilitar

    if (my_rank == 0) {
        printf("Digite os %d valores do vetor:\n", comm_sz);
    }

    // Processo 0 distribui um valor por processo
    int* vetor = NULL;
    if (my_rank == 0) {
        vetor = malloc(comm_sz * sizeof(int));
        for (int i = 0; i < comm_sz; i++)
            scanf("%d", &vetor[i]);
    }

    // Scatter: cada processo recebe X[i]
    MPI_Scatter(vetor, 1, MPI_INT,
                &local_value, 1, MPI_INT,
                0, MPI_COMM_WORLD);

    if (my_rank == 0)
        free(vetor);

    // Prefixo começa com seu próprio valor
    prefix_value = local_value;

    // --------- ALGORITMO EM k FASES EM ÁRVORE ----------
    for (d = 0; d < k; d++) {

        dist = 1 << d;   // 2^d

        int received_val;

        if (my_rank >= dist) {
            // Recebe valor do processo (rank - dist)
            MPI_Recv(&received_val, 1, MPI_INT,
                     my_rank - dist, 0,
                     MPI_COMM_WORLD, &status);

            // Acumula soma
            prefix_value += received_val;
        }

        // Envia seu valor atual (prefix_value) para (rank + dist)
        if (my_rank + dist < comm_sz) {
            MPI_Send(&prefix_value, 1, MPI_INT,
                     my_rank + dist, 0,
                     MPI_COMM_WORLD);
        }
    }

    // ---------------- SAÍDA FINAL ----------------
    MPI_Barrier(MPI_COMM_WORLD);
    printf("Processo %d -> prefixo = %d\n", my_rank, prefix_value);

    MPI_Finalize();
    return 0;
}
