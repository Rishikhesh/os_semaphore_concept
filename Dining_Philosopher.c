#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "sem.h"

/*
The solution for this problem is the act of grabbing the left and the right fork is atomic.

The first version did that by holding one global mutex while waiting for both forks. That can't
deadlock, but a hungry philosopher blocked on a busy fork kept the mutex, so nobody else at the
table could even try to eat. This is the state-based version (Dijkstra / Tanenbaum):

  - mutex guards the states array.
  - A hungry philosopher only starts EATING when neither neighbour is EATING (test()).
  - If it can't eat yet it waits on its own semaphore, without holding the mutex.
  - When a philosopher puts its forks down it tests both neighbours and wakes them if they can now eat.

No philosopher ever holds one fork while waiting for the other, so there is no circular wait,
and two non-neighbours can eat at the same time.
*/

#define TICK 100000 // one time unit = 100ms

enum State
{
	EATING,
	THINKING,
	HUNGRY
};

int num = 0;
int meals = 3;   // meals each philosopher eats before leaving, so the program ends
int *states;
int *eaten;
sem *self;       // one per philosopher: "you may eat now"
sem mutex;

#define LEFT(n)  (((n) + num - 1) % num)
#define RIGHT(n) (((n) + 1) % num)

char getStateName(int state)
{
	switch(state)
	{
		case EATING: return 'E';
		case THINKING: return 'T';
		case HUNGRY: return 'H';
	}
	return '?';
}

// call with mutex held
void test(int n)
{
	if (states[n] == HUNGRY && states[LEFT(n)] != EATING && states[RIGHT(n)] != EATING)
	{
		states[n] = EATING;
		sem_up(&self[n]);
	}
}

void take_forks(int n)
{
	sem_down(&mutex);
	states[n] = HUNGRY;
	test(n);                 // try to grab both forks at once
	sem_up(&mutex);
	sem_down(&self[n]);      // block here (not inside the mutex) until test() lets us eat
}

void put_forks(int n)
{
	sem_down(&mutex);
	states[n] = THINKING;
	test(LEFT(n));           // our forks are free now, maybe a neighbour can eat
	test(RIGHT(n));
	sem_up(&mutex);
}

void* philosopher (void* arg)
{
	int n = *((int *)arg);
	unsigned int seed = n + 1;
	free(arg);
	for (int i = 0; i < meals; i++)
	{
		usleep((rand_r(&seed)%10+1)*TICK);   // think
		take_forks(n);
		eaten[n]++;
		usleep((rand_r(&seed)%10+1)*TICK);   // eat
		put_forks(n);
	}
	return NULL;
}


int main(int argc, char *argv[ ])
{
	if (argc > 1)
	{
		num = atoi(argv[1]);
	}
	if (argc > 2)
	{
		meals = atoi(argv[2]);
	}
	if (num < 2 || meals < 1)
	{
		printf("Usage: %s <philosophers >= 2> [meals each, default 3]\n", argv[0]);
		return 1;
	}

	int i;
	states = malloc(sizeof(int)*num);
	eaten = calloc(num, sizeof(int));
	self = malloc(sizeof(sem)*num);
	for (i=0; i<num; i++)
	{
		states[i] = THINKING;
		sem_make(&self[i], 0);
	}
	sem_make(&mutex, 1);

	pthread_t philosophers[num];
	for(i=0; i<num;i++)
	{
		int *arg = malloc(sizeof(int));
		*arg = i;
		pthread_create(&philosophers[i], NULL, philosopher, arg);
	}

	// print the table every time unit until everyone has finished
	int finished = 0;
	while(!finished)
	{
		usleep(TICK);
		finished = 1;
		sem_down(&mutex);
		for (i=0; i<num; i++)
		{
			// the safety property: two neighbours never eat at once
			assert(!(states[i] == EATING && states[RIGHT(i)] == EATING));
			printf(i != num-1 ? " %c - " : " %c \n", getStateName(states[i]));
			if (eaten[i] < meals || states[i] != THINKING)
				finished = 0;
		}
		sem_up(&mutex);
	}

	for (i = 0; i < num; i++)
	{
		pthread_join(philosophers[i], NULL);
	}
	printf("OK: %d philosophers each ate %d meals, no deadlock\n", num, meals);

	for (i = 0; i < num; i++)
		sem_free(&self[i]);
	sem_free(&mutex);
	free(states); free(eaten); free(self);
	return 0;
}
