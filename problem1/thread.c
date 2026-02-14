#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

#define BUFFER_SIZE 5
#define ITERATIONS 50

struct shared_data {
    int buffer[BUFFER_SIZE];
    int in;
    int out;
    bool produce_sucess;
};

struct shared_data shared;

void* producer(void* arg) {
    int produced_buffer[ITERATIONS];

    for (int i = 0; i < ITERATIONS; i++) {

        int next_produced = i + 1;

        shared.buffer[shared.in] = next_produced;
        shared.in = (shared.in + 1) % BUFFER_SIZE;

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

        int next_consumed = shared.buffer[shared.out];
        shared.out = (shared.out + 1) % BUFFER_SIZE;

        consumed_buffer[i] = next_consumed;

        printf("Consumed: %d\n", next_consumed);
    }

    while (!shared.produce_sucess) {
        // wait for producer to finish
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

    pthread_t producer_thread, consumer_thread;

    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    return 0;
}
