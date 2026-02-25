#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <semaphore.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define ITERATIONS 10

struct shared_data {
    int buffer[BUFFER_SIZE];
    int in;
    int out;
    bool produce_sucess;

    sem_t mutex;
    sem_t empty;
    sem_t full;
};

struct shared_data shared;

void* producer(void* arg) {

    int produced_buffer[ITERATIONS];

    for (int i = 0; i < ITERATIONS; i++) {

        int next_produced = i + 1;

        // entry section
        sem_wait(&shared.empty);
        sem_wait(&shared.mutex);

        shared.buffer[shared.in] = next_produced;
        shared.in = (shared.in + 1) % BUFFER_SIZE;

        // exit section
        sem_post(&shared.mutex);
        sem_post(&shared.full);

        produced_buffer[i] = next_produced;
        printf("Produced: %d\n", next_produced);
    }

    sleep(1);
    
    printf("Produced Buffer: ");
    for (int i = 0; i < ITERATIONS; i++) {
        printf("%d ", produced_buffer[i]);
    }
    printf("\n");

    shared.produce_sucess = true;

    pthread_exit(NULL);
}

void* consumer(void* arg) {

    int consumed_buffer[ITERATIONS];

    for (int i = 0; i < ITERATIONS; i++) {

        // entry section
        sem_wait(&shared.full);
        sem_wait(&shared.mutex);

        int next_consumed = shared.buffer[shared.out];
        shared.out = (shared.out + 1) % BUFFER_SIZE;

        // exit section
        sem_post(&shared.mutex);
        sem_post(&shared.empty);

        consumed_buffer[i] = next_consumed;
        printf("Consumed: %d\n", next_consumed);
    }

    while (!shared.produce_sucess) {
        usleep(1000);
    }

    printf("Consumed Buffer: ");
    for (int i = 0; i < ITERATIONS; i++) {
        printf("%d ", consumed_buffer[i]);
    }
    printf("\n");

    pthread_exit(NULL);
}

int main() {

    shared.in = 0;
    shared.out = 0;
    shared.produce_sucess = false;

    // IMPORTANT: pshared = 0 (thread)
    sem_init(&shared.mutex, 0, 1);
    sem_init(&shared.empty, 0, BUFFER_SIZE);
    sem_init(&shared.full, 0, 0);

    pthread_t producer_thread, consumer_thread;

    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    sem_destroy(&shared.mutex);
    sem_destroy(&shared.empty);
    sem_destroy(&shared.full);

    return 0;
}
