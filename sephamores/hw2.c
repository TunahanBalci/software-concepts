#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

#define MAX_PATH 1024

sem_t active_threads;

// thread arguments 
typedef struct {
    int thread_id;
    char filename[MAX_PATH];
    char filepath[MAX_PATH];
} ThreadArgs;

// function to check if a number is deficient
// a number is deficient if  sum of its proper divisors less than the number
int is_deficient(int n) {
    if (n <= 0) return 0;
    int sum = 0;
    for (int i = 1; i <= n / 2; i++) {
        if (n % i == 0) {
            sum += i;
        }
    }
    return sum < n;
}

// thread function that processes a single file
void* process_file(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    
    FILE* file = fopen(args->filepath, "r");
    if (!file) {
        perror("Error opening file");
        // release semaphore
        sem_post(&active_threads);
        free(args);
        pthread_exit(NULL);
    }

    int num;
    int count = 0;
    // read integers from file
    while (fscanf(file, "%d", &num) == 1) {
        if (is_deficient(num)) {
            count++;
        }
    }

    fclose(file);

    printf("Thread %d has found %d deficient numbers in %s\n", args->thread_id, count, args->filename);

    // release the semaphore when thread has finished
    sem_post(&active_threads);
    free(args);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    // check command args
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <directoryName> <threadNumber>\n", argv[0]);
        return 1;
    }

    char* dir_name = argv[1];
    int thread_number = atoi(argv[2]);

    if (thread_number <= 0) {
        fprintf(stderr, "Thread number must be a positive integer.\n");
        return 1;
    }

    // initialize the semaphore with the max number of threads
    if (sem_init(&active_threads, 0, thread_number) != 0) {
        perror("Semaphore initialization failed");
        return 1;
    }

    // open directory
    DIR* dir = opendir(dir_name);
    if (!dir) {
        perror("Error opening directory");
        sem_destroy(&active_threads);
        return 1;
    }

    struct dirent* entry;
    int thread_id = 1;
    
    int capacity = 100;
    int file_count = 0;
    pthread_t* thread_arr = malloc(capacity * sizeof(pthread_t));

    // iterate files in the directory
    while ((entry = readdir(dir)) != NULL) {
        // skip current and parent directory pointers
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        if (strstr(entry->d_name, ".txt") != NULL) { // only txt


            // wait for the semaphore to ensure thread limit is not exceedded
            sem_wait(&active_threads);

            ThreadArgs* args = malloc(sizeof(ThreadArgs));
            args->thread_id = thread_id++;
            strncpy(args->filename, entry->d_name, MAX_PATH - 1);
            snprintf(args->filepath, MAX_PATH, "%s/%s", dir_name, entry->d_name);
            
            // if needed expand thread array
            if (file_count >= capacity) {
                capacity *= 2;
                thread_arr = realloc(thread_arr, capacity * sizeof(pthread_t));
            }

            // create new thread to process fileas
            if (pthread_create(&thread_arr[file_count], NULL, process_file, args) != 0) {
                perror("Failed to create thread");
                sem_post(&active_threads);
                free(args);
                continue;
            }
            
            file_count++;
        }
    }

    closedir(dir);

    // wait all created threads to finish
    for (int i = 0; i < file_count; i++) {
        pthread_join(thread_arr[i], NULL);
    }

    // cleanup
    free(thread_arr);
    sem_destroy(&active_threads);

    return 0;
}
