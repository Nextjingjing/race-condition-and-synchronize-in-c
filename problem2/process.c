#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include <fcntl.h>

#define BUFFER_SIZE 5
#define ITERATIONS 50

// Parent process is Consumer
// Child process is Producer

// Shared data structure
struct shared_data {
    int buffer[BUFFER_SIZE];
    int in;
    int out;
    bool produce_sucess;
    // Semaphores for synchronization
    sem_t mutex;
    sem_t empty;
    sem_t full;
};

int main() {
    struct shared_data *shared = mmap(NULL, sizeof(struct shared_data),
                                     PROT_READ | PROT_WRITE,
                                     MAP_SHARED | MAP_ANONYMOUS,
                                     -1, 0);

    if (shared == MAP_FAILED) {
        perror("mmap failed");
        exit(1);
    }

    shared->in = 0; // next free position in buffer
    shared->out = 0; // next full position in buffer
    shared->produce_sucess = false;

    // Initialize semaphores
    sem_init(&shared->mutex, 1, 1);
    sem_init(&shared->empty, 1, BUFFER_SIZE);
    sem_init(&shared->full, 1, 0);

    pid_t pid = fork();

    // Check for mmap failure
    if (pid < 0) {
        perror("Fork failed");
        return 1;
    }
    else if (pid == 0) {
        // Child - Producer
        int produced_buffer[ITERATIONS];
        for (int i = 0; i < ITERATIONS; i++) {
            // produce an item in next_produced
            int next_produced = i + 1;

            // entry section for producer
            sem_wait(&shared->empty);
            sem_wait(&shared->mutex);

            // add next_produced to the buffer
            shared->buffer[shared->in] = next_produced;
            shared->in = (shared->in + 1) % BUFFER_SIZE;

            // exit section for producer
            sem_post(&shared->mutex);
            sem_post(&shared->full);

            produced_buffer[i] = next_produced;
            printf("Produced: %d\n", next_produced);
        }
        
        printf("Produced Buffer: ");
        for (int i = 0; i < ITERATIONS; i++) {
            printf("%d ", produced_buffer[i]);
        }
        printf("\n");
        shared->produce_sucess = true;
    }
    else {
        // Parent - Consumer
        int consumed_buffer[ITERATIONS];
        for (int i = 0; i < ITERATIONS; i++) {
            // entry section for consumer
            sem_wait(&shared->full);
            sem_wait(&shared->mutex);

            // remove an item from the buffer to next_consumed
            int next_consumed = shared->buffer[shared->out];
            shared->out = (shared->out + 1) % BUFFER_SIZE;

            // exit section for consumer
            sem_post(&shared->mutex);
            sem_post(&shared->empty);

            // consume the item in next_consumed
            consumed_buffer[i] = next_consumed;
            printf("Consumed: %d\n", next_consumed);
        }

        // print consumed buffer
        while (!shared->produce_sucess) {
            usleep(1000);
        }

        printf("Consumed Buffer: ");
        for (int i = 0; i < ITERATIONS; i++) {
            printf("%d ", consumed_buffer[i]);
        }
        printf("\n");

        // Clean up semaphores and shared memory
        sem_destroy(&shared->mutex);
        sem_destroy(&shared->empty);
        sem_destroy(&shared->full);
        munmap(shared, sizeof(struct shared_data));
    }

    return 0;
}