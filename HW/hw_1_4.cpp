/* dotprod_llm.c  --  LLM-generated (Claude Sonnet 4.6), Problem 4
 * Compile: mpicc -O2 -o dotprod_llm dotprod_llm.c -lm
 * Run:     mpirun -np <P> ./dotprod_llm <N>
 *          e.g. mpirun -np 4 ./dotprod_llm 1024
 */
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char *argv[]) {
    int rank, size, N;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 2) {
        if (rank == 0) fprintf(stderr, "Usage: %s <N>\n", argv[0]);
        MPI_Finalize(); return 1;
    }
    N = atoi(argv[1]);

    /* Data partitioning: base elements each, first (N%P) ranks get +1 */
    int base      = N / size;
    int remainder = N % size;
    int local_n   = (rank < remainder) ? base + 1 : base;
    int offset    = rank * base + (rank < remainder ? rank : remainder);

    double *a = (double *)malloc(local_n * sizeof(double));
    double *b = (double *)malloc(local_n * sizeof(double));
    if (!a || !b) { fprintf(stderr, "malloc failed\n"); MPI_Abort(MPI_COMM_WORLD, 1); }

    /* a[i] = global_index+1, b[i] = 1  =>  dot product = N*(N+1)/2 */
    for (int i = 0; i < local_n; i++) {
        a[i] = (double)(offset + i + 1);
        b[i] = 1.0;
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double t_start = MPI_Wtime();

    /* Local partial sum */
    double local_sum = 0.0;
    for (int i = 0; i < local_n; i++)
        local_sum += a[i] * b[i];

    /* Tree reduction: step doubles each round */
    double received;
    for (int step = 1; step < size; step *= 2) {
        if (rank % (2 * step) == 0) {
            int partner = rank + step;
            if (partner < size) {
                MPI_Recv(&received, 1, MPI_DOUBLE, partner, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                local_sum += received;
            }
        } else if (rank % (2 * step) == step) {
            MPI_Send(&local_sum, 1, MPI_DOUBLE, rank - step, 0, MPI_COMM_WORLD);
            break;
        }
    }

    double t_end = MPI_Wtime();

    if (rank == 0) {
        double expected = (double)N * (N + 1) / 2.0;
        printf("N=%d P=%d | dot=%.0f expected=%.0f %s | time=%.6f s\n",
               N, size, local_sum, expected,
               fabs(local_sum - expected) < 0.5 ? "CORRECT" : "WRONG",
               t_end - t_start);
    }
    free(a); free(b);
    MPI_Finalize();
    return 0;
}
