```c
Semaphore mutex = 1; // mutual exclusion to shared buffers

Semaphore empty = N; // count of empty
buffers(all empty to start)

    // producer
    while (1)
    {

        produce_new_resource();

        P(mutex); // lock buffer list

        add_resource_to_an_empty_buffer();

        P(empty); // wait for empty buffer

        V(mutex); // unlock buffer list
    }


    // consumer
    while (1)
    {

        P(mutex); // lock buffer list

        remove_resource_from_a_full_buffer();

        V(mutex); // unlock buffer list

        V(empty); // note an empty buffer

        Consume resource;
    }
```