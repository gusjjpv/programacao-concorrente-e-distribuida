#include <stdio.h>
#include <stdlib.h>

void prefix_sum(int x[], int y[], int n) {
    int soma = 0;

    for (int i = 0; i < n; i++) {
        soma += x[i];
        y[i] = soma;
    }
}

int main() {
    int n, i;
    int *x, *y;

    printf("Digite o tamanho do vetor: ");
    scanf("%d", &n);

    x = (int*) malloc(n * sizeof(int));
    y = (int*) malloc(n * sizeof(int));

    printf("Digite os %d elementos do vetor:\n", n);
    for (i = 0; i < n; i++) {
        scanf("%d", &x[i]);
    }

    prefix_sum(x, y, n);

    printf("\nSomas de prefixos:\n");
    for (i = 0; i < n; i++) {
        printf("%d ", y[i]);
    }
    printf("\n");

    free(x);
    free(y);

    return 0;
}