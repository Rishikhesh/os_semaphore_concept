#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "sem.h"

/*
This problem is the implementation of producer consumer problem where a producer can't produce over a limit and
a consumer can't consume of there are no products in the buffer.

Here we have implemented 5 counsumers and 5 producers and the buffer size as 5.

Semaphore have been used for synchronization and seperate threads have been created for each individual customers and producers.

  empty  - counts free slots   (starts at BufferSize). Producer waits on it: blocks when the buffer is full.
  full   - counts filled slots (starts at 0).          Consumer waits on it: blocks when the buffer is empty.
  mutex  - only one thread touches buffer/in/out at a time.
*/

#define MaxItems 5 // Maximum items a producer can produce or a consumer can consume
#define BufferSize 5 // Size of the buffer where the production is stored or from where it is consumed
#define Threads 5 // Number of producers and of consumers

sem empty;
sem full;

int in = 0;
int out = 0;
int buffer[BufferSize];
int count = 0;  // items currently in the buffer, only used to check the invariant
long produced_sum = 0, consumed_sum = 0;

pthread_mutex_t mutex;

void *producer(void *pno)
{
    int item;
    unsigned int seed = *((int *)pno); // rand() is not thread-safe, so each thread keeps its own seed
    for(int i = 0; i < MaxItems; i++) {
        item = rand_r(&seed) % 100;
        sem_down(&empty);
        pthread_mutex_lock(&mutex);
        buffer[in] = item;
        printf("Producer %d: Insert Item %d at %d\n", *((int *)pno),buffer[in],in);
        in = (in+1)%BufferSize;
        count++;
        produced_sum += item;
        assert(count <= BufferSize); // never overfills
        pthread_mutex_unlock(&mutex);
        sem_up(&full);
    }
    return NULL;
}
void *consumer(void *cno)
{
    for(int i = 0; i < MaxItems; i++) {
        sem_down(&full);
        pthread_mutex_lock(&mutex);
        int item = buffer[out];
        printf("Consumer %d: Remove Item %d from %d\n",*((int *)cno),item, out);
        out = (out+1)%BufferSize;
        count--;
        consumed_sum += item;
        assert(count >= 0); // never reads an empty slot
        pthread_mutex_unlock(&mutex);
        sem_up(&empty);
    }
    return NULL;
}

int main()
{
    pthread_t pro[Threads],con[Threads];
    pthread_mutex_init(&mutex, NULL);
    sem_make(&empty,BufferSize);
    sem_make(&full,0);

    int a[Threads] = {1,2,3,4,5};
    for(int i = 0; i < Threads; i++)
    {
        pthread_create(&pro[i], NULL, producer, (void *)&a[i]);
    }
    for(int i = 0; i < Threads; i++) {
        pthread_create(&con[i], NULL, consumer, (void *)&a[i]);
    }

    for(int i = 0; i < Threads; i++) {
        pthread_join(pro[i], NULL);
    }
    for(int i = 0; i < Threads; i++) {
        pthread_join(con[i], NULL);
    }

    // every item produced was consumed exactly once
    assert(count == 0 && produced_sum == consumed_sum);
    printf("OK: %d items produced and consumed, sum %ld\n", Threads*MaxItems, consumed_sum);

    pthread_mutex_destroy(&mutex);
    sem_free(&empty);
    sem_free(&full);

    return 0;

}
