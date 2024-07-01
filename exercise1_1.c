#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h> // Needed for close()
#include "my_rand.h"

pthread_mutex_t mutex;
struct timespec start1, end1, start2, end2;
int total_hits = 0;

typedef struct {
    long int throws;
    int num_threads;
    int current_thread;
} ThreadArguments;

void* piEstimation(void* args) {
    ThreadArguments *thread_args = (ThreadArguments*)args;
    unsigned int seed = rand();

    int local_m = thread_args->throws / thread_args->num_threads;
    int first_i = local_m * thread_args->current_thread;
    int last_i = first_i + local_m;

    int local_hits = 0;
    for (int i = first_i; i < last_i; i++) {
        double x = my_drand(&seed) * 2 - 1;
        double y = my_drand(&seed) * 2 - 1;
        double sq_distance = x * x + y * y;
        if (sq_distance <= 1)
            local_hits++;
    }

    pthread_mutex_lock(&mutex);
    total_hits += local_hits;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <throws> <num_threads>\n", argv[0]);
        return 1;
    }

    long long throws = strtol(argv[1], NULL, 10);
    int num_threads = strtol(argv[2], NULL, 10);

    if (errno == ERANGE) {
        perror("Conversion error: Number out of range.\n");
        return 1;
    }

    printf("We will throw %lld darts.\n\n", throws);

    int fd = open("data1_1.txt", O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (fd == -1) {
        perror("File couldn't be opened or created.\n");
        return 2;
    }

    FILE* fp = fdopen(fd, "w");
    if (fp == NULL) {
        perror("File stream couldn't be created.\n");
        close(fd);  // Close the file descriptor on failure
        return 2;
    }

    printf("We will start with sequential code.\n");

    clock_gettime(CLOCK_MONOTONIC, &start1);

    long long int hits = 0;
    unsigned seed = time(NULL);

    for (int i = 0; i < throws; i++) {
        double x = my_drand(&seed) * 2 - 1;
        double y = my_drand(&seed) * 2 - 1;
        double sq_distance = x * x + y * y;
        if (sq_distance <= 1)
            hits++;
    }

    clock_gettime(CLOCK_MONOTONIC, &end1);

    double elapsed_time = (end1.tv_sec - start1.tv_sec) + (end1.tv_nsec - start1.tv_nsec) / 1e9;
    double pi_estimation = 4 * (hits / (double)throws);

    printf("The estimation of pi is %lf and it took %lf seconds.\n\n", pi_estimation, elapsed_time);

    printf("Now we will do the same using %d threads.\n", num_threads);

    fprintf(fp, "%lld %d %lf\n", throws, num_threads, elapsed_time);

    pthread_t threads[num_threads];
    pthread_mutex_init(&mutex, NULL);

    clock_gettime(CLOCK_MONOTONIC, &start2);
    ThreadArguments thread_args[num_threads]; // Create an array for thread arguments

    for (int i = 0; i < num_threads; i++) {
        thread_args[i].throws = throws;
        thread_args[i].num_threads = num_threads;
        thread_args[i].current_thread = i;
        if (pthread_create(&threads[i], NULL, piEstimation, (void*)&thread_args[i]) != 0) {
            perror("Something went wrong with creating the threads.\n");
            return 3;
        }
    }

    for (int i = 0; i < num_threads; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Something went wrong with joining.\n");
            return 4;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end2);
    elapsed_time = (end2.tv_sec - start2.tv_sec) + (end2.tv_nsec - start2.tv_nsec) / 1e9;

    fprintf(fp, "%lld %d %lf\n", throws, num_threads, elapsed_time);

    pi_estimation = 4 * (total_hits / (double)throws);

    printf("The estimation of pi is %lf and it took %.6f seconds.\n\n", pi_estimation, elapsed_time);

    pthread_mutex_destroy(&mutex);
    fclose(fp);

    return 0;
}