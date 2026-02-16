# ภฤศ ตัณฑ์วรกุล 6601012610083
# การบ้าน OS
- Problem 1 การเกิด Race condition ในปัญหา Producer-Consumer
  - ไม่มีการทำ Synchronize
- Problem 2 การทำ Synchronize ในปัญหา Producer-Consumer
- Problem 3 แสดงการเกิด Deadlock ของ
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
