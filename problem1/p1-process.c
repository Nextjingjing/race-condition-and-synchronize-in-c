#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define BUFFER_SIZE 5

typedef struct {
    int buffer[BUFFER_SIZE];
    int count;
    int in;
    int out;
    long total_produced;
    long total_consumed;
    sem_t display_mutex;
} shared_data;

shared_data *sh;

void check_and_print(int snap_in,
                     int snap_out,
                     int snap_count,
                     int snap_buffer[],
                     long snap_prod,
                     long snap_cons)
{
    sem_wait(&sh->display_mutex);

    int expected_count = snap_prod - snap_cons;

    printf("in:%d out:%d | Count:%d Exp:%d | [ ",
            snap_in, snap_out, snap_count, expected_count);
    for (int i = 0; i < BUFFER_SIZE; i++) printf("%d ", snap_buffer[i]);
    printf("]\n");

    if (snap_count != expected_count || snap_count < 0 || snap_count > BUFFER_SIZE) {
        printf("\n>>> RACE CONDITION DETECTED (Data Corrupted!) <<<\n\n");
        exit(1);
    }

    sem_post(&sh->display_mutex);
}

void producer() {
    while (1) {
        while (sh->count == BUFFER_SIZE) usleep(100);

        sh->buffer[sh->in] = 1;
        sh->in = (sh->in + 1) % BUFFER_SIZE;
        sh->total_produced++;
        sh->count++;

        // snapshot
        int snap_buffer[BUFFER_SIZE];
        for (int i = 0; i < BUFFER_SIZE; i++) snap_buffer[i] = sh->buffer[i];

        check_and_print(sh->in, sh->out, sh->count, snap_buffer,
                        sh->total_produced, sh->total_consumed);
    }
}

void consumer() {
    while (1) {
        while (sh->count == 0) usleep(100);

        sh->buffer[sh->out] = 0;
        sh->out = (sh->out + 1) % BUFFER_SIZE;
        sh->total_consumed++;
        sh->count--;

        // snapshot
        int snap_buffer[BUFFER_SIZE];
        for (int i = 0; i < BUFFER_SIZE; i++) snap_buffer[i] = sh->buffer[i];

        check_and_print(sh->in, sh->out, sh->count, snap_buffer,
                        sh->total_produced, sh->total_consumed);
    }
}

int main() {

    sh = mmap(NULL, sizeof(shared_data),
              PROT_READ | PROT_WRITE,
              MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (sh == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // init shared data
    for (int i = 0; i < BUFFER_SIZE; i++) sh->buffer[i] = 0;
    sh->count = 0;
    sh->in = 0;
    sh->out = 0;
    sh->total_produced = 0;
    sh->total_consumed = 0;

    sem_init(&sh->display_mutex, 1, 1);

    printf("Simulation: fork once Snapshot Printing.\n\n");

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(1);
    }
    else if (pid == 0) {
        // child = consumer
        consumer();
    }
    else {
        // parent = producer
        producer();
        wait(NULL);
    }

    sem_destroy(&sh->display_mutex);
    munmap(sh, sizeof(shared_data));
    return 0;
}