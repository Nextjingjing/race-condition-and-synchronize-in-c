#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdbool.h>

#define BUFFER_SIZE 5

// Parent process is Consumer
// Child process is Producer

// Shared data structure
struct shared_data {
    int buffer[BUFFER_SIZE];
    int in;
    int out;
    int count
};

int main() {
    struct shared_data *shared = mmap(NULL, sizeof(struct shared_data),
                                     PROT_READ | PROT_WRITE,
                                     MAP_SHARED | MAP_ANONYMOUS,
                                     -1, 0);

    shared->in = 0;
    shared->out = 0;
    shared->count = 0;

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
        while (1) {

        }
        
    }
    else {
        // Parent - Consumer
        while (1) {
            
        }
    }

    return 0;
}
