#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define BUFFER_SIZE 5

struct shared_data {
    int buffer[BUFFER_SIZE];
    int in;
    int out;
    bool produce_sucess;

    sem_t mutex;
    sem_t empty;
};

struct shared_data shared;

void* producer(void* arg) {

    sleep(1); // Sleep for a while to let the consumer start first

    int i = 0;
    while (1) {

        int next_produced = i + 1;

        // entry section
        sem_wait(&shared.mutex);

        shared.buffer[shared.in] = next_produced;
        shared.in = (shared.in + 1) % BUFFER_SIZE;

        // exit section
        sem_wait(&shared.empty);
        sem_post(&shared.mutex);

        i++;
    }

    shared.produce_sucess = true;
    pthread_exit(NULL);
}

void* consumer(void* arg) {

    int consumed_count = 0;

    while (1) {

        sem_wait(&shared.mutex);

        // Deadlock occurs
        while (shared.buffer[shared.out] == 0) {

            consumed_count++;

            if (consumed_count > 10000000) {
                printf("Deadlock detected. Exiting.\n");
                exit(0);
            }
        }

        consumed_count = 0;

        int next_consumed = shared.buffer[shared.out];
        shared.out = (shared.out + 1) % BUFFER_SIZE;

        sem_post(&shared.mutex);
        sem_post(&shared.empty);

        printf("Consumed: %d\n", next_consumed);
    }

    pthread_exit(NULL);
}

int main() {

    shared.in = 0;
    shared.out = 0;
    shared.produce_sucess = false;

    sem_init(&shared.mutex, 0, 1);
    sem_init(&shared.empty, 0, BUFFER_SIZE);

    pthread_t producer_thread, consumer_thread;

    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    sem_destroy(&shared.mutex);
    sem_destroy(&shared.empty);

    return 0;
}
