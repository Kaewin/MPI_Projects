#include <iostream>
#include <vector>
#include <mpi.h>
#include <unistd.h>


/*
 * print_full_grid() — LLM-generated (Claude Sonnet 4.6, Anthropic)
 * Generated via claude.ai on 2026-03-04
 * Prompt: "I have some code for a game of life simulation, but I need a fancy
 *          function that will print out what is happening inside so it can be
 *          watched. Generate that for this code."
 * Model output used with minor integration edits (parameter passing, call site).
 */
void print_full_grid(char** local_grid, int local_rows, int local_cols,
                     int N, int P, int Q, int rank, int row, int col, int iter) {

    // Pack interior cells (no ghost rows/cols) into a flat send buffer
    std::vector<char> send_buf(local_rows * local_cols);
    for (int i = 0; i < local_rows; i++)
        for (int j = 0; j < local_cols; j++)
            send_buf[i * local_cols + j] = local_grid[i + 1][j + 1];

    // Rank 0 allocates a receive buffer for all processes' data
    std::vector<char> recv_buf;
    if (rank == 0)
        recv_buf.resize(N * N);  // Total cells across all processes

    MPI_Gather(
        send_buf.data(), local_rows * local_cols, MPI_CHAR,
        recv_buf.data(), local_rows * local_cols, MPI_CHAR,
        0, MPI_COMM_WORLD
    );

    if (rank == 0) {
        // Reconstruct the full N×N grid from gathered blocks.
        // MPI_Gather deposits process r's block at recv_buf[r * local_rows * local_cols].
        // Each process r occupies grid row-block (r/Q) and col-block (r%Q).
        std::vector<std::vector<char>> full(N, std::vector<char>(N, ' '));

        for (int r = 0; r < P * Q; r++) {
            int proc_row = r / Q;   // which row-band this process owns
            int proc_col = r % Q;   // which col-band this process owns
            int row_offset = proc_row * local_rows;
            int col_offset = proc_col * local_cols;

            for (int i = 0; i < local_rows; i++)
                for (int j = 0; j < local_cols; j++)
                    full[row_offset + i][col_offset + j] =
                        recv_buf[r * local_rows * local_cols + i * local_cols + j];
        }

        // Clear terminal and print header
        std::cout << "\033[2J\033[H";  // ANSI: clear screen, cursor to top-left
        std::cout << "╔";
        for (int j = 0; j < N; j++) std::cout << "═";
        std::cout << "╗\n";

        for (int i = 0; i < N; i++) {
            std::cout << "║";
            for (int j = 0; j < N; j++)
                std::cout << full[i][j];
            std::cout << "║\n";
        }

        std::cout << "╚";
        for (int j = 0; j < N; j++) std::cout << "═";
        std::cout << "╝\n";
        std::cout << " Iteration: " << iter + 1
                  << "  Grid: " << N << "×" << N
                  << "  Processes: " << P << "×" << Q << "\n";
        std::cout.flush();

        usleep(150000);  // 150ms pause so the animation is watchable
    }

    // All processes must wait here — rank 0 can't start the next iteration's
    // gather until everyone has finished computing their local_grid_next swap.
    MPI_Barrier(MPI_COMM_WORLD);
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 5) {
        if (rank == 0)
            std::cout << "Usage: mpirun -n <procs> ./hw_1_7 P Q N" << std::endl;
        MPI_Finalize();
        return 1;
    }

    // Here, atoi is used to convert the command line argument to an integer.
    int P = atoi(argv[1]);
    int Q = atoi(argv[2]);
    int N = atoi(argv[3]);
    int iterations = atoi(argv[4]);

    // Validation checks for P, Q, and N:
    if (P <= 0 || Q <= 0) {
        if (rank == 0)
            std::cout << "P and Q must be positive integers." << std::endl;
        MPI_Finalize();
        return 1;
    }

    if (P * Q != size) {
        if (rank == 0)
            std::cout << "The product of P and Q must equal the number of processes." << std::endl;
        MPI_Finalize();
        return 1;
    }

    if (N % P != 0 || N % Q != 0) {
        if (rank == 0)
            std::cout << "N must be evenly divisible by both P and Q." << std::endl;
        MPI_Finalize();
        return 1;
    }

    // Calculate row, column, and rank within the row and column communicators.
    int row = rank / Q; // Determine the row based on the rank and number of columns
    int col = rank % Q; // Determine the column based on the rank and number of columns
    // Print the rank, row, and column for each process.
    // std::cout << "World rank " << rank
    //           << " | row " << row
    //           << " | col " << col
    //           << std::endl;

    // Calculate all 8 neighbors in the 2D grid, considering wrap-around (toroidal) topology.
    int up = ((row - 1 + P) % P) * Q + col;
    int down = ((row + 1) % P) * Q + col;
    int left = row * Q + (col - 1 + Q) % Q;
    int right = row * Q + (col + 1) % Q;
    int up_left = ((row - 1 + P) % P) * Q + (col - 1 + Q) % Q;
    int up_right = ((row - 1 + P) % P) * Q + (col + 1) % Q;
    int down_left = ((row + 1) % P) * Q + (col - 1 + Q) % Q;
    int down_right = ((row + 1) % P) * Q + (col + 1) % Q;
    // Neighborhood print statements:
    // std::cout << "Process " << rank << " (row " << row << ", col " << col << ") neighbors: "
    //           << "up: " << up << ", down: " << down
    //           << ", left: " << left << ", right: " << right
    //           << ", up-left: " << up_left << ", up-right: " << up_right
    //           << ", down-left: " << down_left << ", down-right: " << down_right
    //           << std::endl;

    int local_rows = N / P;
    int local_cols = N / Q;
    // std::cout << "Process " << rank << " has local grid size: " << local_rows << " x " << local_cols << std::endl;

    // Allocate the local grid:
    char** local_grid = new char*[local_rows + 2];
    for (int i = 0; i < local_rows + 2; ++i) {
        local_grid[i] = new char[local_cols + 2];
    }
    for (int i = 0; i < local_rows + 2; i++) {
        for (int j = 0; j < local_cols + 2; j++) {
            local_grid[i][j] = ' '; // Initialize to ' ' (dead cell)
        }
    }

    srand(rank); // Seed the random number generator with the rank for different random values in each process
    // Initialize the local grid (for example, set all values to 0.0):
    for (int i = 1; i <= local_rows; i++) {
        for (int j = 1; j <= local_cols; j++) {
            local_grid[i][j] = (rand() % 2) ? '*' : ' '; // Randomly set to '*' or ' '
        }
    }

    // Create second grid, local_grid_next
    char** local_grid_next = new char*[local_rows + 2];
    for (int i = 0; i < local_rows + 2; ++i) {
        local_grid_next[i] = new char[local_cols + 2];
    }
    for (int i = 0; i < local_rows + 2; i++) {
        for (int j = 0; j < local_cols + 2; j++) {
            local_grid_next[i][j] = ' '; // Initialize to ' ' (dead cell)
        }
    }

    MPI_Comm row_comm, col_comm;
    MPI_Comm_split(MPI_COMM_WORLD, row, col, &row_comm);
    MPI_Comm_split(MPI_COMM_WORLD, col, row, &col_comm);

    // Columns are not continuous in memory, so we need to pack and unpack them into temporary buffers for communication.
    // Left to right communication:
    std::vector<char> send_buf(local_rows+2);
    std::vector<char> recv_buf(local_rows+2);

    // Wrap halo exchange in a loop for multiple iterations:
    for (int iter = 0; iter < iterations; iter++) {


        // Top to bottom communication:
        MPI_Sendrecv(
            &local_grid[1][0],           // Send buffer: first interior cell of row 1
            local_cols+2,                  // Send count: interior columns only (no ghost cols)
            MPI_CHAR,                    // Send type: each element is a char
            up,                          // Destination: send to up neighbor's rank
            0,                           // Send tag: label for this message (just using 0)
            &local_grid[local_rows+1][0],// Recv buffer: interior cells of bottom ghost row
            local_cols+2,                  // Recv count: how many elements to receive
            MPI_CHAR,                    // Recv type: each element is a char
            down,                        // Source: receive from down neighbor's rank
            0,                           // Recv tag: must match sender's tag
            MPI_COMM_WORLD,              // Communicator
            MPI_STATUS_IGNORE            // Status: we don't need message details so ignore it
        );

        // Bottom to top communication:
        MPI_Sendrecv(
            &local_grid[local_rows][0],  // Send buffer: first interior cell of last row
            local_cols+2,                  // Send count: interior columns only (no ghost cols)
            MPI_CHAR,                    // Send type: each element is a char
            down,                        // Destination: send to down neighbor's rank
            0,                           // Send tag: label for this message (just using 0)
            &local_grid[0][0],           // Recv buffer: interior cells of top ghost row
            local_cols+2,                  // Recv count: how many elements to receive
            MPI_CHAR,                    // Recv type: each element is a char
            up,                          // Source: receive from up neighbor's rank
            0,                           // Recv tag: must match sender's tag
            MPI_COMM_WORLD,              // Communicator
            MPI_STATUS_IGNORE            // Status: we don't need message details so ignore it
        );

        // Pack left interior column (col 1)
        for (int i = 0; i < local_rows+2; i++)
            send_buf[i] = local_grid[i][1];

        MPI_Sendrecv(
            send_buf.data(), local_rows+2, MPI_CHAR, left, 0,
            recv_buf.data(), local_rows+2, MPI_CHAR, right, 0,
            MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Unpack into right ghost column (col local_cols+1)
        for (int i = 0; i < local_rows+2; i++)
            local_grid[i][local_cols+1] = recv_buf[i];


        // Right to left communication:
        // Pack right interior column (col local_cols)
        for (int i = 0; i < local_rows+2; i++)
            send_buf[i] = local_grid[i][local_cols];

        MPI_Sendrecv(
            send_buf.data(), local_rows+2, MPI_CHAR, right, 0,
            recv_buf.data(), local_rows+2, MPI_CHAR, left, 0,
            MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Unpack into left ghost column (col 0)
        for (int i = 0; i < local_rows+2; i++)
            local_grid[i][0] = recv_buf[i];

        // Bottom-right corner communication:
        // Send bottom-right interior cell to down_right neighbor, receive from up_left into top-left ghost corner
        MPI_Sendrecv(
            &local_grid[local_rows][local_cols],        // Send buffer: bottom-right interior cell
            1, MPI_CHAR, down_right, 0,                 // Count, type, destination, tag
            &local_grid[0][0],                          // Recv buffer: top-left ghost corner
            1, MPI_CHAR, up_left, 0,                    // Count, type, source, tag
            MPI_COMM_WORLD, MPI_STATUS_IGNORE
        );
        // Bottom-left corner communication:
        // Send bottom-left interior cell to down_left neighbor, receive from up_right into top-right ghost corner
        MPI_Sendrecv(
            &local_grid[local_rows][1],        // Send buffer: bottom-left interior cell
            1, MPI_CHAR, down_left, 0,                 // Count, type, destination, tag
            &local_grid[0][local_cols+1],                          // Recv buffer: top-right ghost corner
            1, MPI_CHAR, up_right, 0,                    // Count, type, source, tag
            MPI_COMM_WORLD, MPI_STATUS_IGNORE
        );
        
        // Top-left corner communication:
        // Send top-left interior cell to up_left neighbor, receive from down_right into bottom-right ghost corner
        MPI_Sendrecv(
            &local_grid[1][1],                          // Send buffer: top-left interior cell
            1, MPI_CHAR, up_left, 0,                    // Count, type, destination, tag
            &local_grid[local_rows+1][local_cols+1],    // Recv buffer: bottom-right ghost corner
            1, MPI_CHAR, down_right, 0,                 // Count, type, source, tag
            MPI_COMM_WORLD, MPI_STATUS_IGNORE
        );

        // Top-right corner communication:
        // Send top-right interior cell to up_right neighbor, receive from down_left into bottom-left ghost corner
        MPI_Sendrecv(
            &local_grid[1][local_cols],                 // Send buffer: top-right interior cell
            1, MPI_CHAR, up_right, 0,                   // Count, type, destination, tag
            &local_grid[local_rows+1][0],               // Recv buffer: bottom-left ghost corner
            1, MPI_CHAR, down_left, 0,                  // Count, type, source, tag
            MPI_COMM_WORLD, MPI_STATUS_IGNORE
        );

        // TODO: Apply Life rules to compute local_grid_next based on local_grid (including ghost cells)
        for (int i = 1; i <= local_rows; i++) {
            for (int j = 1; j <= local_cols; j++) {
                int neighbors = 0;
                for (int di = -1; di <= 1; di++) {
                    for (int dj = -1; dj <= 1; dj++) {
                        if (di == 0 && dj == 0) continue;
                        neighbors += (local_grid[i+di][j+dj] == '*') ? 1 : 0;;
                    }
                }
                if (local_grid[i][j] == '*') {
                    if (neighbors < 2 || neighbors > 3) {
                        local_grid_next[i][j] = ' ';
                    } else {
                        local_grid_next[i][j] = '*';
                    }
                } else {
                    if (neighbors == 3) {
                        local_grid_next[i][j] = '*';
                    } else {
                        local_grid_next[i][j] = ' ';
                    }
                }
            }
        }

        // Swap grids for the next iteration:
        std::swap(local_grid, local_grid_next);

        // PUT FUNCTION HERE
        print_full_grid(local_grid, local_rows, local_cols, N, P, Q, rank, row, col, iter);

    } // End of iterations loop

    for (int i = 0; i < local_rows + 2; ++i) {
        delete[] local_grid[i];
        delete[] local_grid_next[i];
    }
    delete[] local_grid;
    delete[] local_grid_next;

    MPI_Comm_free(&row_comm);
    MPI_Comm_free(&col_comm);

    MPI_Finalize();
    return 0;
}