#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

int main() {

    int my_rank, comm_sz;
    int count = 5;  // tamanho do vetor local em cada processo
    int i;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    // Vetor local com valores aleatórios
    int* local_vec = malloc(count * sizeof(int));
    int* prefix_vec = malloc(count * sizeof(int));

    // Semente baseada no rank (para não gerar sequências iguais)
    srand(time(NULL) + my_rank);

    // Gera elementos aleatórios entre 1 e 9
    for (i = 0; i < count; i++)
        local_vec[i] = (rand() % 9) + 1;

    // Imprime vetor original de cada processo
    printf("Proc %d vetor local: ", my_rank);
    for (i = 0; i < count; i++)
        printf("%d ", local_vec[i]);
    printf("\n");

    // MPI_Scan calcula prefixo entre processos:
    // prefix_vec[k] = soma dos elementos local_vec[k] + dos processos anteriores
    MPI_Scan(local_vec, prefix_vec, count, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);

    // Imprime as somas de prefixos
    printf("Proc %d vetor prefixo: ", my_rank);
    for (i = 0; i < count; i++)
        printf("%d ", prefix_vec[i]);
    printf("\n");

    free(local_vec);
    free(prefix_vec);

    MPI_Finalize();
    return 0;
}
