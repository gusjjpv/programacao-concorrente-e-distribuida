#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

int main(int argc, char* argv[]) {
    int my_rank, comm_sz;
    int n;
    double escalar;

    double *v1 = NULL, *v2 = NULL;
    double *local_v1, *local_v2;
    double local_sum = 0.0, global_sum = 0.0;

    int *sendcounts = NULL;
    int *displs = NULL;
    int local_n;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    /* Processo 0 lê entrada */
    if (my_rank == 0) {
        printf("Digite o tamanho dos vetores (n): ");
        fflush(stdout);
        scanf("%d", &n);

        printf("Digite o escalar: ");
        fflush(stdout);
        scanf("%lf", &escalar);

        v1 = malloc(n * sizeof(double));
        v2 = malloc(n * sizeof(double));

        printf("Digite os elementos do vetor 1:\n");
        for (int i = 0; i < n; i++){
            fflush(stdout);
            scanf("%lf", &v1[i]);
            }

        printf("Digite os elementos do vetor 2:\n");
        for (int i = 0; i < n; i++){
            fflush(stdout);
            scanf("%lf", &v2[i]);
        }

        /* Alocar e preencher sendcounts e displs */
        sendcounts = malloc(comm_sz * sizeof(int));
        displs = malloc(comm_sz * sizeof(int));

        int div = n / comm_sz;
        int resto = n % comm_sz;

        int offset = 0;
        for (int i = 0; i < comm_sz; i++) {
            sendcounts[i] = div + (i < resto ? 1 : 0);
            displs[i] = offset;
            offset += sendcounts[i];
        }
    }

    /* Broadcast de n e escalar */
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&escalar, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    /* Cada processo descobre seu local_n */
    int div = n / comm_sz;
    int resto = n % comm_sz;
    local_n = div + (my_rank < resto ? 1 : 0);

    /* Alocar vetores locais */
    local_v1 = malloc(local_n * sizeof(double));
    local_v2 = malloc(local_n * sizeof(double));

    /* Distribuir vetores com Scatterv */
    MPI_Scatterv(v1, sendcounts, displs, MPI_DOUBLE,
                 local_v1, local_n, MPI_DOUBLE,
                 0, MPI_COMM_WORLD);

    MPI_Scatterv(v2, sendcounts, displs, MPI_DOUBLE,
                 local_v2, local_n, MPI_DOUBLE,
                 0, MPI_COMM_WORLD);

    /* Multiplicar vetor 1 pelo escalar */
    for (int i = 0; i < local_n; i++)
        local_v1[i] *= escalar;

    /* Soma parcial da norma do vetor 2 */
    for (int i = 0; i < local_n; i++)
        local_sum += local_v2[i] * local_v2[i];

    MPI_Reduce(&local_sum, &global_sum, 1, MPI_DOUBLE,
               MPI_SUM, 0, MPI_COMM_WORLD);

    /* Coletar vetor 1 modificado */
    MPI_Gatherv(local_v1, local_n, MPI_DOUBLE,
                v1, sendcounts, displs, MPI_DOUBLE,
                0, MPI_COMM_WORLD);

    /* Processo 0 imprime */
    if (my_rank == 0) {
        printf("\nVetor 1 após multiplicação pelo escalar:\n");
        for (int i = 0; i < n; i++)
            printf("%.2f ", v1[i]);
        printf("\n");

        printf("\nNorma do vetor 2: %.6f\n", sqrt(global_sum));

        free(v1);
        free(v2);
        free(sendcounts);
        free(displs);
    }

    free(local_v1);
    free(local_v2);

    MPI_Finalize();
    return 0;
}