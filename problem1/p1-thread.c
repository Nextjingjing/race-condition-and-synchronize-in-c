#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>

#define BUFFER_SIZE 5

int buffer[BUFFER_SIZE] = {0};
int count = 0;
int in = 0;
int out = 0;

long total_produced = 0;
long total_consumed = 0;

sem_t display_mutex;

void check_and_print(int snap_in,
                     int snap_out,
                     int snap_count,
                     int snap_buffer[],
                     long snap_prod,
                     long snap_cons)
{
    sem_wait(&display_mutex);

    int expected_count = snap_prod - snap_cons;

    printf("in:%d out:%d | Count:%d Exp:%d | [ ",
            snap_in, snap_out, snap_count, expected_count);
    for (int i = 0; i < BUFFER_SIZE; i++) printf("%d ", snap_buffer[i]);
    printf("]\n");

    if (snap_count != expected_count || snap_count < 0 || snap_count > BUFFER_SIZE) {
        printf("\n>>> RACE CONDITION DETECTED (Data Corrupted!) <<<\n\n");
        exit(1);
    }

    sem_post(&display_mutex);
}

void *producer(void *arg) {
    while (1) {
        while (count == BUFFER_SIZE) usleep(100);

        buffer[in] = 1;
        in = (in + 1) % BUFFER_SIZE;
        total_produced++;
        count++;

        // snapshot
        int snap_buffer[BUFFER_SIZE];
        for (int i = 0; i < BUFFER_SIZE; i++) snap_buffer[i] = buffer[i];

        check_and_print(in, out, count, snap_buffer,
                        total_produced, total_consumed);
    }
}

void *consumer(void *arg) {
    while (1) {
        while (count == 0) usleep(100);

        buffer[out] = 0;
        out = (out + 1) % BUFFER_SIZE;
        total_consumed++;

        count--;

        // snapshot
        int snap_buffer[BUFFER_SIZE];
        for (int i = 0; i < BUFFER_SIZE; i++) snap_buffer[i] = buffer[i];

        check_and_print(in, out, count, snap_buffer,
                        total_produced, total_consumed);
    }
}

int main() {
    pthread_t t1, t2;

    sem_init(&display_mutex, 0, 1);

    printf("Simulation: Snapshot Printing.\n\n");

    pthread_create(&t1, NULL, producer, NULL);
    pthread_create(&t2, NULL, consumer, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    sem_destroy(&display_mutex);
    return 0;
}