#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include "sem.h"

#define N 3        // chairs in the waiting room
#define VISITS 2   // help sessions each student needs before going home
#define TICK 100000 // one time unit = 100ms, keeps a run to a few seconds


/*
This program is the solution for the Teaching assistant problem where there is a TA who helps students but only one at time.
There are 3 chairs in which the students can wait and if more students come then can revisit after sometime for the help.
If no students are there in the waiting chairs then the TA goes into sleeping mode.

For synchronization semaphores have been used and each individual student and TA are created by seperate threads.

  mutex    - protects numberOfSeatsWR (and the TA's asleep flag)
  students - number of students sitting in the waiting room; the TA sleeps on it
  ta       - TA calls the next student in (starts at 0: nobody is called until the TA is ready)
  done     - TA tells the student the session is over
*/
sem mutex,students,ta,done;
int numberOfSeatsWR = N; //The number of chairs empty that are empty
int asleep = 0;
int sessions = 0; // help sessions given, only touched by the TA thread

void *ta_p(void *arg); //function for teaching assistent
void *student_p(void *arg); //function for student

int main(int argc,char *argv[])
{
	int std = argc > 1 ? atoi(argv[1]) : 5;
	if (std < 1)
	{
		printf("Usage: %s [number of students >= 1]\n", argv[0]);
		return 1;
	}
	srand(42);
	pthread_t TA,student[std];

	sem_make(&mutex,1);
	sem_make(&students,0);
	sem_make(&ta,0);
	sem_make(&done,0);

	int total = std*VISITS;
	if (pthread_create(&TA,NULL,ta_p,&total) != 0)
		perror("Thread creation failed");
	for(long i=0;i<std;i++)
		if (pthread_create(&student[i],NULL,student_p,(void*)i) != 0)
			perror("Thread creation failed");

	for(int i=0;i<std;i++)
		pthread_join(student[i],NULL);
	pthread_join(TA,NULL);

	assert(sessions == total && numberOfSeatsWR == N);
	printf("OK: %d students, %d help sessions, waiting room empty\n", std, sessions);

	sem_free(&mutex);
	sem_free(&students);
	sem_free(&ta);
	sem_free(&done);
	return 0;
}

//teaching assistent process
void *ta_p(void *arg)
{
	int total = *(int *)arg;
	while(sessions < total)
	{
		sem_down(&mutex);
		if(numberOfSeatsWR == N)
		{
			printf("TA goes to sleep.\n");
			asleep = 1;
		}
		sem_up(&mutex);

		sem_down(&students);   // sleep until a student sits down
		sem_down(&mutex);
		if(asleep)
		{
			printf("TA has been woken up\n");
			asleep = 0;
		}
		numberOfSeatsWR++;     // student leaves the chair for the TA's desk
		assert(numberOfSeatsWR <= N);
		printf("TA tutoring, No of empty seats :%d.\n",numberOfSeatsWR);
		sem_up(&mutex);

		sem_up(&ta);           // call the student in
		usleep(3*TICK);        // tutoring
		sessions++;
		printf("TA tutoring completed.\n");
		sem_up(&done);         // let the student go
	}
	return NULL;
}

//student process
void *student_p(void *arg)
{
	long id = (long)arg+1;
	unsigned int seed = (unsigned int)id;
	int helped = 0;
	while(helped < VISITS)
	{
		int t = rand_r(&seed)%10 + 1;
		printf("Student %ld is studying on own for %d units\n",id,t);
		usleep(t*TICK);
		sem_down(&mutex);
		if(numberOfSeatsWR>0)
		{
			numberOfSeatsWR--;
			assert(numberOfSeatsWR >= 0);
			printf("Student %ld arrived in waiting room, No of empty seats :%d\n",id,numberOfSeatsWR);
			sem_up(&students);
			sem_up(&mutex);
			sem_down(&ta);     // wait to be called in
			sem_down(&done);   // wait until tutoring is over
			helped++;
		}
		else
		{
			sem_up(&mutex);
			printf("Student %ld sees that there is no empty chair and leaves.\n",id);
		}
	}
	return NULL;
}
