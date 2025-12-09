#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main() {
    int my_rank, comm_sz;
    MPI_Init(NULL,NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    if (comm_sz < 2) {
        if (my_rank == 0) printf("Execute com pelo menos 2 processos.\n");
        MPI_Finalize();
        return 0;
    }

    int n = 4;  // Pode ser lido do usuário se quiser.
    int N = n * n;

    MPI_Datatype upper_type;
    int *blocklengths = malloc(n * sizeof(int));
    int *displs = malloc(n * sizeof(int));

    // Construindo os blocos do tipo triangular superior
    for (int i = 0; i < n; i++) {
        blocklengths[i] = n - i;         // nº de elementos na linha i
        displs[i] = i * n + i;           // deslocamento (em unidades de MPI_INT)
    }

    MPI_Type_indexed(n, blocklengths, displs, MPI_INT, &upper_type);
    MPI_Type_commit(&upper_type);

    if (my_rank == 0) {
        int* matrix = malloc(N * sizeof(int));

        // Preenche a matriz com valores simples (0 a N-1)
        for (int i = 0; i < N; i++)
            matrix[i] = i;

        printf("Processo 0 enviando triangular superior:\n");

        for (int i = 0; i < N; i++) {
            printf("%d ", matrix[i]);
            if ((i+1) % n == 0) printf("\n");
        }

        // Envia a matriz inteira, mas o tipo derivado define o QUE será enviado
        MPI_Send(matrix, 1, upper_type, 1, 0, MPI_COMM_WORLD);

        free(matrix);
    } 
    else if (my_rank == 1) {
        // Quantidade total de elementos da triangular superior
        int total = n*(n+1)/2;

        int *recvbuf = malloc(total * sizeof(int));

        MPI_Recv(recvbuf, total, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        printf("Processo 1 recebeu os elementos da triangular superior:\n");
        for (int i = 0; i < total; i++)
            printf("%d ", recvbuf[i]);
        printf("\n");

        free(recvbuf);
    }

    MPI_Type_free(&upper_type);
    free(blocklengths);
    free(displs);

    MPI_Finalize();
    return 0;
}
