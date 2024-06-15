#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <omp.h>
#include <errno.h>
#include "my_rand.h"

struct timespec start1, end1, start2, end2;


int main(int argc, char* argv[]) {

    if(argc < 2) {
        perror("Give 2 arguments when you execute the program.\n");
        return 1;
    }

    long long int throws = atoi(argv[1]);
    int thread_count = atoi(argv[2]);

    unsigned int seed_a = 1;


    printf("We will throw %lld darts.\n\n", throws);

    printf("We will start with sequential code.\n");

    clock_gettime(CLOCK_MONOTONIC, &start1);

    long long int hits = 0;
    for(int i = 0; i < throws; i++) {
            double x =  my_drand(&seed_a) * 2 - 1;
            double y =  my_drand(&seed_a) * 2 - 1;
            double sq_distance = x*x + y*y;
            if(sq_distance <= 1)
                hits++;
    }
  
    clock_gettime(CLOCK_MONOTONIC, &end1);
    
    double elapsed_time = (end1.tv_sec - start1.tv_sec) + (end1.tv_nsec - start1.tv_nsec) / 1e9;

    double pi_estimation = 4*(hits/(double)throws);

    printf("The estimation of pi is %lf and it took %lf seconds.\n\n", pi_estimation, elapsed_time);

    printf("Now we will do the same using %d threads with OpenMP.\n", thread_count);
    hits = 0;
    clock_gettime(CLOCK_MONOTONIC, &start2);

    double x;
    double y;
    double sq_distance;
    # pragma omp parallel private(x,y,sq_distance) reduction(+:hits) 
    {
            unsigned int seed_b = 1 + omp_get_thread_num();

            # pragma omp for 
            for(int i = 0; i < throws; i++) {
            x =  my_drand(&seed_b) * 2 - 1;
            y =  my_drand(&seed_b) * 2 - 1;
            sq_distance = x*x + y*y;
            if(sq_distance <= 1)
                hits++;
        }
    }       
    

    clock_gettime(CLOCK_MONOTONIC, &end2);
    elapsed_time = (end2.tv_sec - start2.tv_sec) + (end2.tv_nsec - start2.tv_nsec) / 1e9;

    pi_estimation = 4*(hits/(double)throws);

    printf("The estimation of pi is %lf and it took %.6f seconds.\n\n", pi_estimation, elapsed_time);

    return 0;
}