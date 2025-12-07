#include <stdio.h>
#include <mpi.h>

double f(double x) {
    return x * x;   // exemplo, substitua pela sua função
}

double Trap(double left_endpt, double right_endpt, int trap_count, double h) {
    double estimate, x;
    int i;

    estimate = (f(left_endpt) + f(right_endpt)) / 2.0;
    for (i = 1; i <= trap_count - 1; i++) {
        x = left_endpt + i * h;
        estimate += f(x);
    }
    estimate *= h;

    return estimate;
}

int main() {
    int my_rank, comm_sz;
    double a = 0.0, b = 1.0;
    int n = 1000;              
    double h, local_a, local_b;
    int base_n, resto, local_n;

    MPI_Init(NULL,NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    h = (b - a) / n;

    base_n = n / comm_sz; 
    resto = n % comm_sz;      

    if (my_rank < resto) {
        local_n = base_n + 1;
        local_a = a + my_rank * local_n * h;
    } else {
        local_n = base_n;
        local_a = a + (resto * (base_n + 1) 
                      + (my_rank - resto) * base_n) * h;
    }

    local_b = local_a + local_n * h;

    double local_integral = Trap(local_a, local_b, local_n, h);

    double total_integral;
    MPI_Reduce(&local_integral, &total_integral, 1, MPI_DOUBLE,
               MPI_SUM, 0, MPI_COMM_WORLD);

    if (my_rank == 0)
        printf("Integral = %.15f\n", total_integral);

    MPI_Finalize();
    return 0;
}
