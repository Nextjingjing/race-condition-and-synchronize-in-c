#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include <fcntl.h>

// Parent process is Consumer
// Child process is Producer

// Shared data structure
struct shared_data {
    int buffer[5];
    int in;
    int out;
    bool produce_sucess;
    // Semaphores for synchronization
    sem_t mutex;
    sem_t empty;
    // sem_t full;
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
    sem_init(&shared->empty, 1, 5);
    // sem_init(&shared->full, 1, 0);

    pid_t pid = fork();

    // Check for mmap failure
    if (pid < 0) {
        perror("Fork failed");
        return 1;
    }
    else if (pid == 0) {
        // Child - Producer
        int i = 0;
        while (true) {
            // produce an item in next_produced
            int next_produced = i + 1;

            // entry section for producer
            sem_wait(&shared->mutex);

            // add next_produced to the buffer
            shared->buffer[shared->in] = next_produced;
            shared->in = (shared->in + 1) % 5;

            // exit section for producer
            sem_wait(&shared->empty);
            sem_post(&shared->mutex);
            // sem_post(&shared->full);

        }
        printf("\n");
        shared->produce_sucess = true;
    }
    else {
        // Parent - Consumer
        sleep(1);
        while (true) {
            // entry section for consumer
            // sem_wait(&shared->full);
            sem_wait(&shared->mutex);

            // remove an item from the buffer to next_consumed
            int next_consumed = shared->buffer[shared->out];
            shared->out = (shared->out + 1) % 5;

            // exit section for consumer
            sem_post(&shared->mutex);
            sem_post(&shared->empty);

            // consume the item in next_consumed
            printf("Consumed: %d\n", next_consumed);
        }

        // Clean up semaphores and shared memory
        sem_destroy(&shared->mutex);
        sem_destroy(&shared->empty);
        // sem_destroy(&shared->full);
        munmap(shared, sizeof(struct shared_data));
    }

    return 0;
}