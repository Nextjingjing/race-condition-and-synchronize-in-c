#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdbool.h>

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
};

int main() {
    struct shared_data *shared = mmap(NULL, sizeof(struct shared_data),
                                     PROT_READ | PROT_WRITE,
                                     MAP_SHARED | MAP_ANONYMOUS,
                                     -1, 0);

    shared->in = 0; // next free position in buffer
    shared->out = 0; // next full position in buffer
    shared->produce_sucess = false;

    // Check for mmap failure
    if (shared == MAP_FAILED) {
        perror("mmap failed");
        exit(1);
    }

    pid_t pid = fork();

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

            // add next_produced to the buffer
            shared->buffer[shared->in] = next_produced;
            shared->in = (shared->in + 1) % BUFFER_SIZE;

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
            // remove an item from the buffer to next_consumed
            int next_consumed = shared->buffer[shared->out];
            shared->out = (shared->out + 1) % BUFFER_SIZE;
            
            // consume the item in next_consumed
            consumed_buffer[i] = next_consumed;
            printf("Consumed: %d\n", next_consumed);
        }

        // print consumed buffer
        while (!shared->produce_sucess)
        {
            // wait for producer to finish
        }
        printf("Consumed Buffer: ");
        for (int i = 0; i < ITERATIONS; i++) {
            printf("%d ", consumed_buffer[i]);
        }
        printf("\n");

        // Clean up shared memory
        munmap(shared, sizeof(struct shared_data));
    }

    return 0;
}
