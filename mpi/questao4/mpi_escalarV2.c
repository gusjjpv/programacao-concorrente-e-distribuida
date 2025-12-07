#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(void) {
   int n;
   int local_n; //Tamanho do vetor dividido
   int i; //variavel auxiliar
   int escalar; //Escalar
   int* vetor1 = NULL; //Primeiro vetor
   int* vetor2 = NULL; //Segundo vetor
   int* local_vetor1; //Parte do vetor 1 em um processo
   int* local_vetor2; //Parte do vetor 2 em um processo
   int local_soma = 0; // Soma local
   int resultado = 0; //Resultado final
   int        comm_sz;               /* Numero de processos    */
   int        my_rank;               /* Rank do meu processo       */

   MPI_Init(NULL, NULL); 

   MPI_Comm_size(MPI_COMM_WORLD, &comm_sz); 

   MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); 

   if (my_rank == 0) {
      printf("Entre com o tamanho dos vetores\n");
      scanf("%d", &n);
      printf("Entre com o escalar\n");
      scanf("%d", &escalar);
      local_n = n/comm_sz;
   }

    MPI_Bcast(&local_n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&escalar, 1, MPI_INT, 0, MPI_COMM_WORLD);

   vetor1 = (int*)malloc(n*sizeof(int));
   vetor2 = (int*)malloc(n*sizeof(int));
   local_vetor1 = (int*)malloc(local_n*sizeof(int));
   local_vetor2 = (int*)malloc(local_n*sizeof(int));

   if(my_rank == 0){
      printf("Digite o primeiro vetor\n");
	for(i=0;i<n;i++)
	 scanf("%d",&vetor1[i]);
      printf("Digite o segundo vetor\n");
	for(i=0;i<n;i++)
	 scanf("%d",&vetor2[i]);
      MPI_Scatter(vetor1, local_n, MPI_INT, 
            local_vetor1, local_n, MPI_INT, 0, MPI_COMM_WORLD);
      MPI_Scatter(vetor2, local_n, MPI_INT, 
            local_vetor2, local_n, MPI_INT, 0, MPI_COMM_WORLD);
      free(vetor1);
      free(vetor2);
   }else{
      MPI_Scatter(vetor1, local_n, MPI_INT, 
            local_vetor1, local_n, MPI_INT, 0, MPI_COMM_WORLD);
      MPI_Scatter(vetor2, local_n, MPI_INT, 
            local_vetor2, local_n, MPI_INT, 0, MPI_COMM_WORLD);
   }

   for(i=0;i<local_n;i++){
      local_vetor1[i] *= escalar;           // vetor1 * escalar
        local_soma += local_vetor2[i] * local_vetor2[i];
   }

   printf("Processo %d -> vetor1[%d] após multiplicar: %d\n", my_rank, i, local_vetor1[i]);

   MPI_Reduce(&local_soma, &resultado, 1, MPI_INT, MPI_SUM, 0,
         MPI_COMM_WORLD);

    int* vetor1_final = NULL;
    if (my_rank == 0)
    vetor1_final = (int*)malloc(n*sizeof(int));

    MPI_Gather(local_vetor1, local_n, MPI_INT, vetor1_final, local_n, MPI_INT, 0, MPI_COMM_WORLD);


   if(my_rank == 0){
    printf("Norma do vetor2 (sem raiz): %d\n", resultado);
    printf("Vetor1 multiplicado pelo escalar:\n");
    for (i = 0; i < n; i++)
        printf("%d ", vetor1_final[i]);
    printf("\n");

    free(vetor1_final);
    free(local_vetor1);
    free(local_vetor2);
   }

   MPI_Finalize(); 

   return 0;
}