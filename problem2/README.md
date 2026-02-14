# Producer-Consumer Problem
## Producer
```c
while (true) {
   ...
   /* produce an item in next_produced */
   ...
   wait(empty);
   wait(mutex);
   ...
   /* add next produced to the buffer */
   ...
   signal(mutex);
   signal(full);
 }
```

## Consumer
```c
while (true) {
   wait(full);
   wait(mutex);
   ...
   /* remove an item from buffer to next_consumed */
   ...
   signal(mutex);
   signal(empty);
   ...
   /* consume the item in next consumed */
   ...
 }
```