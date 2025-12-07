#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(void) {
   int n;
   int i;
   int escalar;
   int *vetor1 = NULL, *vetor2 = NULL;
   int *local_vetor1, *local_vetor2;
   int local_soma = 0, resultado = 0;
   int comm_sz, my_rank;

   MPI_Init(NULL, NULL);
   MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
   MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

   int *sendcounts = malloc(comm_sz * sizeof(int));
   int *displs = malloc(comm_sz * sizeof(int));

   if (my_rank == 0) {
      printf("Entre com o tamanho dos vetores:\n");
      scanf("%d", &n);

      printf("Entre com o escalar:\n");
      scanf("%d", &escalar);

      // calcula distribuição como na questão da regra trapezoidal
      int base = n / comm_sz;
      int resto = n % comm_sz;

      int offset = 0;
      for (i = 0; i < comm_sz; i++) {
         sendcounts[i] = (i < resto) ? base + 1 : base;
         displs[i] = offset;
         offset += sendcounts[i];
      }
   }

   // Broadcast de n e escalar
   MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
   MPI_Bcast(&escalar, 1, MPI_INT, 0, MPI_COMM_WORLD);

   // Broadcast de sendcounts e displs
   MPI_Bcast(sendcounts, comm_sz, MPI_INT, 0, MPI_COMM_WORLD);
   MPI_Bcast(displs, comm_sz, MPI_INT, 0, MPI_COMM_WORLD);

   int local_n = sendcounts[my_rank];

   local_vetor1 = malloc(local_n * sizeof(int));
   local_vetor2 = malloc(local_n * sizeof(int));

   if (my_rank == 0) {
      vetor1 = malloc(n * sizeof(int));
      vetor2 = malloc(n * sizeof(int));

      printf("Digite o primeiro vetor:\n");
      for (i = 0; i < n; i++)
         scanf("%d", &vetor1[i]);

      printf("Digite o segundo vetor:\n");
      for (i = 0; i < n; i++)
         scanf("%d", &vetor2[i]);
   }

   // SCATTERV corrigido
   MPI_Scatterv(vetor1, sendcounts, displs, MPI_INT,
                local_vetor1, local_n, MPI_INT, 0, MPI_COMM_WORLD);

   MPI_Scatterv(vetor2, sendcounts, displs, MPI_INT,
                local_vetor2, local_n, MPI_INT, 0, MPI_COMM_WORLD);

   // Cálculo local
   for (i = 0; i < local_n; i++) {
      local_vetor1[i] *= escalar;
      local_soma += local_vetor2[i] * local_vetor2[i];
      printf("Processo %d -> vetor1[%d] apos multiplicar: %d\n",
             my_rank, i, local_vetor1[i]);
   }

   MPI_Reduce(&local_soma, &resultado, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

   int *vetor1_final = NULL;
   if (my_rank == 0) vetor1_final = malloc(n * sizeof(int));

   // GATHERV corrigido
   MPI_Gatherv(local_vetor1, local_n, MPI_INT,
               vetor1_final, sendcounts, displs, MPI_INT,
               0, MPI_COMM_WORLD);

   if (my_rank == 0) {
      printf("Norma do vetor2 (sem raiz): %d\n", resultado);
      printf("Vetor1 multiplicado pelo escalar:\n");
      for (i = 0; i < n; i++)
         printf("%d ", vetor1_final[i]);
      printf("\n");

      free(vetor1_final);
      free(vetor1);
      free(vetor2);
   }

   free(local_vetor1);
   free(local_vetor2);
   free(sendcounts);
   free(displs);

   MPI_Finalize();
   return 0;
}
