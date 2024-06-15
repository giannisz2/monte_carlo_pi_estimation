#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

struct timespec start,end;

int main(void) {

    int my_rank;
    int comm_sz;
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    
    // 2nd solution method
    // MPI_Status status;

    int total_throws = 0;
    if(my_rank == 0) {
        printf("I am process %d. Give the number of total throws:\n", my_rank);
        scanf("%d", &total_throws);
        clock_gettime(CLOCK_MONOTONIC, &start);
        // 2nd solution method
        // for(int i = 0; i < comm_sz; i++) 
        //     MPI_Send(&total_throws, sizeof(int), MPI_INT, i, 1, MPI_COMM_WORLD);
    } 


    // 1st solution method
    MPI_Bcast(&total_throws, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // 2nd solution method
    // if(my_rank != 0) 
    //     MPI_Recv(&total_throws, sizeof(int), MPI_INT, 0, 1, MPI_COMM_WORLD, &status);

    srand(time(NULL));

    int local_throws = total_throws/comm_sz;
    int local_hits = 0;

    for(int i = 0; i < local_throws; i++) {
        double x = (double)rand() / RAND_MAX;
        double y = (double)rand() / RAND_MAX;
        double sq_distance = x*x + y*y;
        if(sq_distance <= 1)
            local_hits++;
    }

    int total_hits;
    MPI_Reduce(&local_hits, &total_hits, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // 1st solution method
    if(my_rank == 0) {
        double pi_estimation = 4*(total_hits/ (double)total_throws);
        clock_gettime(CLOCK_MONOTONIC, &end);
        double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        printf("The estimation of pi is %lf and it took %.6f seconds in total with %d processes.\n", pi_estimation, elapsed_time, comm_sz);
    }

    // 2nd solution method
    // if(status.MPI_SOURCE != 0) {
    //     double pi_estimation = 4*(total_hits/ (double)total_throws);
    //     clock_gettime(CLOCK_MONOTONIC, &end);
    //     double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    //     printf("The estimation of pi is %lf and it took %.6f seconds with %d cores.\n", pi_estimation, elapsed_time, comm_sz);
    // }

    MPI_Finalize();

    return 0;
}