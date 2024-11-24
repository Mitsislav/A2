/* * * * * * * * * * * * * * * * * * * * * * * * * *
 *						   *
 *   	   IOANNIS CHATZIANTONIOU CSD5193	   *
 *						   *
 * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>

#define N 12
#define T 10

/* mutex to ensure that only one thread join to critical zone and
 * semaphore to ensure bus starts after initialization of threads */

sem_t semaphore;
pthread_barrier_t barrier;
pthread_mutex_t mutex;

/* global queues that we store students - threads to check their
 * position that indepents from student->positon */

struct student ** stopAQueue;
struct student ** BusQueue;
struct student ** UniversityQueue;
struct student ** stopBQueue;

/* global counters to check how many students at each position and 
 * works as top pointer at each queue */

int stopA_count = 0, bus_count = 0, university_count = 0, stopB_count = 0;

/* counters that ensure us the N/4 rule that bus takes 3 students from each
 * department and counters becomes zero after a bus delivered */

int math_count = 0, physics_count = 0, chemistry_count = 0, csd_count = 0;

/* global thread for our bus and volatile variable bus_stopB to checks if
 * the bus is on stop b */

pthread_t bus_t;
volatile int terminate_bus=0;
volatile int bus_stopB=0;

/* enums that contains every possible place that any student could be */

enum place{ 
	stopA=0,
       	Bus=1,
       	University=2,
       	stopB=3
};

/* the student department of every student - thread */

enum science{
	Math=0,
	Physics=1,
	Chemistry=2,
	CSD=3
};

/* struct student is basically a struct that contains information 
 * of every student - thread */

struct student{
	pthread_t thread;
	int am;
	int time;
	enum place position;
	enum science department;
	int index;
};

/* random number generator that generates a random number
 * at takes the limits of the number (min,max) */

int random_number(int min,int max){
	
	if (min > max) {
        	perror("Failed to generate random number\n");
        	return 1;
	}
	return (rand() % (max - min + 1)) + min;
}

/* function that initialize the student threads and their information */

void initialize_students(struct student * Student, pthread_t thread, int i){
	Student->thread=thread;
	Student->am=random_number(1000,6000);
	Student->time=random_number(5,15);
	Student->position=stopA;
	Student->department=random_number(0,3);
	Student->index=i;
	return;
}

/* takes the number that coresponse to a specific place-position that
 * thread could be */

char * getPlace(enum place position){
	
	switch (position) {
        	case stopA:
            		return "Stop A";
        	case Bus:
            		return "Bus";
       		case University:
            		return "University";
        	case stopB:
           		return "Stop B";
        	default:
            		return "Unknown";
    	}	
}

/* takes the number that coresponse to a specific department that 
 * student thread could has */

char * getScience(enum place department){

	switch (department){
		case Math:
			return "Math";
		case Physics:
			return "Physics";
		case Chemistry:
			return "Chemistry";
		case CSD:
			return "CSD";
		default:
			return "Unknown";
	}
}

/* prints the current situation that our threads */

void printInfo(){
	
	printf("\nStop A: ");
	for(int i=0;i<stopA_count;i++){
		printf("[%d , %s] ", stopAQueue[i]->am, getScience(stopAQueue[i]->department));
	}

	printf("\nBus: ");
	for(int i=0;i<bus_count;i++){
		printf("[%d , %s] ", BusQueue[i]->am, getScience(BusQueue[i]->department));
	}

	printf("\nUniversity: ");
	for(int i=0;i<university_count;i++){
		printf("[%d , %s] ", UniversityQueue[i]->am, getScience(UniversityQueue[i]->department));	
	}

	printf("\nStop B: ");
	for(int i=0;i<stopB_count;i++){
		printf("[%d , %s] ", stopBQueue[i]->am, getScience(stopBQueue[i]->department));	
	}

	printf("\n");
	
}

/* add a student thread to a queue that defines from his position */

void addQueue(struct student * s){
	
	switch (s->position) {
		case stopA:
			stopAQueue[stopA_count++] = s;
			break;
		case Bus:
			BusQueue[bus_count++] = s;
			break;
		case University:
			UniversityQueue[university_count++] = s;
			break;
		case stopB:
			stopBQueue[stopB_count++] = s;
			break;
	}
	printInfo();
}

/* here i dequeue a thread student from the front of the queue */

struct student * popQueue(struct student ** queue, int * count){

	if (*count == 0) 
		return NULL;	

	struct student *s = queue[0];
    	for(int i = 0; i < *count - 1; i++){
     		queue[i] = queue[i + 1];
    	}

    	(*count)--;

    	return s;
}

/* here i remove a student from the univeristy and shift all students thread 
 * one position to left */

void removeUni(struct student * s){
	int index=-1;

	for (int i = 0; i < university_count; i++) {
        	if (UniversityQueue[i]==s){
            		index = i;
            		break;
        	}
    	}

	if(index==-1) return;

	for (int i = index; i < university_count - 1; i++) {
       		UniversityQueue[i]=UniversityQueue[i + 1];
	}

	university_count--;
}

/* every student thread has to study for a certified time depending
 * at student->time and make sure that threads goes to stop b 
 * base on their time */

void * studyEvent(void * arg){

	struct student* s = (struct student*) arg;

	sleep(s->time);

	s->position=stopB;

	pthread_mutex_lock(&mutex);

	printf("\nStudent %d (%s) studied for %d seconds, and now heading to stop B.\n", s->am, getScience(s->department), s->time);

	sleep(1);

	removeUni(s);
	addQueue(s);

	sleep(1);

	pthread_mutex_unlock(&mutex);
}

/* here removes the n-th student from stop A*/

void removeStopA(int n){
	 for (int i = n; i < stopA_count - 1; i++) {
        	stopAQueue[i] =stopAQueue[i + 1];
    	}
    	stopA_count--;
}

/* here removes the n-th student from stop B*/

void removeStopB(int n){
	for(int i=n;i<stopB_count-1;i++){
		stopBQueue[i]=stopBQueue[i+1];
	}
	stopB_count--;
}

/* bus event */

void * busEvent(){

	while(1){

		pthread_mutex_lock(&mutex);
		
		/* counter that has how many students has the bus */
		
		int boarded_count = 0;
		int i=0;	/* tries to take i-th student */
		while(boarded_count < N && stopA_count > 0 && i<N){
			struct student *s=stopAQueue[i];	/* current student */
			
			int allowBoard = 0;			/* flag that ensures us that student can ride the bus */
           	 	switch (s->department) {
                		case Math: 			/* checks if the counter of each department not greater than N/4 */
					if (math_count < N/4){ 
						math_count++; 
						allowBoard = 1; 
					} 
					break;
                		case Physics: 
					if (physics_count < N/4){ 
						physics_count++; 
						allowBoard = 1; 
					} 
					break;
                		case Chemistry: 
					if (chemistry_count < N/4){ 
						chemistry_count++; 
						allowBoard = 1; 
					} 
					break;
                		case CSD: 
					if (csd_count < N/4){ 
						csd_count++; 
						allowBoard = 1; 
					} 
					break;
            		}

			if(allowBoard){		/* if can join then remove from stop A and put him to bus */
				removeStopA(i);
				s->position = Bus;
				//addQueue(s);
				sem_post(&semaphore); /* communicates with sem_wait at startEvent and checks that every student initialized */
				boarded_count++;
				printf("\nStudent %d (%s) boarded to the bus.\n", s->am, getScience(s->department));
				sleep(1);
				addQueue(s);
			}else{
				/* if cant join print desired message and */
				printf("\nStudent %d (%s) cannot enter the bus.\n",s->am,getScience(s->department));
				i++;
				if(s==stopAQueue[stopA_count]) break;	/* that ensures us that there are still students at stop A*/
			}						/* if does not exist then break and the bus goes to uni */
		
			sleep(1);
		}


		if (boarded_count > 0) {
			printf("\nThe Bus is on the way to University...!\n");
			pthread_mutex_unlock(&mutex);
						/* bus starts to going to university and sleeps T time */
			sleep(T);

			printf("\nBus arrived at University!\n");
									/* counters set to zero cause now dequeue the students */
			math_count = 0, physics_count = 0, chemistry_count = 0, csd_count = 0;

			pthread_mutex_lock(&mutex);
			while (bus_count > 0) {		/* there are still  students at bus continue the dequeue process*/
				struct student* s = popQueue(BusQueue, &bus_count);

				s->position = University;
							/* student get of the bus and sleep 1 for better printing */
				printf("\nStudent %d (%s) got off the bus.\n", s->am, getScience(s->department));
				sleep(1);
				addQueue(s);
				sleep(1);
							/* here i create every time an independant thread for each student for study */
				pthread_t study_thread;
                		pthread_create(&study_thread, NULL, &studyEvent, (void*) s);
                		pthread_detach(study_thread);

			}
			pthread_mutex_unlock(&mutex);

			
			pthread_mutex_lock(&mutex);		/* if thera are still students at stop b then sent signal to bus to keep going*/
            		if (stopB_count > 0) {
                		bus_stopB=1; 
            		}else{
				printf("\nThe Bus is returning to Stop A \n");	/* if there are not then go stop A */
				sleep(T);
			}

            		pthread_mutex_unlock(&mutex);

		} else {
			pthread_mutex_unlock(&mutex);
		}
		
		/* going from stop B to stop A */

		if (bus_stopB) {
			
			pthread_mutex_lock(&mutex);

			int returnCount=0;	/* boarding students at stop B to return them to stop A */
			int i=0;
			while(stopB_count>0 && returnCount<N && i<N){
				struct student *s = stopBQueue[i];
				
				int allowBoard = 0;
            			switch (s->department) {
                			case Math: 
						if (math_count < N/4) {
						       	math_count++; 
							allowBoard = 1; 
						} 
						break;
                			case Physics: 
						if (physics_count < N/4) { 
							physics_count++; 
							allowBoard = 1; 
						} 
						break;
                			case Chemistry: 
						if (chemistry_count < N/4) { 
							chemistry_count++;
						       	allowBoard= 1;
					       	}
					       	break;
                			case CSD: 
						if (csd_count < N/4) {
						       	csd_count++; 
							allowBoard = 1; 
						} 
						break;
            			}
					/* checks if students is allowed to be boarded */
				if(allowBoard){
					removeStopB(i);
					s->position=Bus;
					sem_post(&semaphore);
					printf("\nStudent %d (%s) boarded to the bus.\n", s->am, getScience(s->department));
					returnCount++;
					sleep(1);
					addQueue(s);
				}else{
					printf("\nStudent %d (%s) cannot enter the bus.\n",s->am,getScience(s->department));
					sleep(1);	
					i++;
				}
				
				sleep(1);
			}
				/* bus has students to return and print the desired msg */
			if(returnCount>0){
				printf("\nThe Bus is returning to Stop A with students from Stop B.\n");
				pthread_mutex_unlock(&mutex);
				sleep(T);
				math_count = 0, physics_count = 0, chemistry_count = 0, csd_count = 0;
				/* counters becomes zero because bus arrived to the stop A */
				pthread_mutex_lock(&mutex);
                		while (bus_count > 0) {
                    			struct student* s = popQueue(BusQueue, &bus_count);
                    			s->position = stopA;
                   	 		printf("\nStudent %d (%s) went home.\n", s->am, getScience(s->department));
					sleep(1);
                    			printInfo();
					sleep(1);
                    			
                		}
                		pthread_mutex_unlock(&mutex);
				bus_stopB=0;
			}else{
				pthread_mutex_unlock(&mutex);	
			}				
		}
		
		pthread_mutex_unlock(&mutex);

		pthread_mutex_lock(&mutex);	/* if there are no students at stop A bus there are at stop B goes to uni again */
		if(bus_stopB==0 && stopA_count==0 && stopB_count>0){
			printf("\nBus is on the way to University...!\n");
			sleep(T);
			printf("\nBus arrived at University!\n");
			math_count = 0, physics_count = 0, chemistry_count = 0, csd_count = 0; /* counters becomes zero again cause bus empty */
			bus_stopB=1;
		}
		pthread_mutex_unlock(&mutex);

		sleep(1);

		pthread_mutex_lock(&mutex);	/* if there are not students at any position that means bus has to stop */
		if(stopA_count == 0 && bus_count == 0 && university_count == 0 && stopB_count == 0){
			return NULL;
		}
		pthread_mutex_unlock(&mutex);
	}
}

/* initialize every student - thread and add to the queue */

void * startEvent(void * args){
	
	struct student * s = (struct student*)args;

	pthread_mutex_lock(&mutex);
	
	printf("\nStudent %d [%s] created\n",s->am,getScience(s->department));

	sleep(1);

	addQueue(s);

	sleep(1);

	pthread_mutex_unlock(&mutex);

	/* board to bus */
	
	sem_wait(&semaphore);

}

int main(){

	int NUM_THREADS;
	int i;

	/* checks if input is 12-200 if not then exit */

	printf("Please enter the number of students :\n");
   	scanf("%d", &NUM_THREADS);
    	if (NUM_THREADS < 12 || NUM_THREADS > 200) {
        	perror("Invalid number of students. It should be between 12 and 200.\n");
        	return EXIT_FAILURE;
    	}

	/* memory allocate for the table with threads */

	pthread_t * thread=malloc(NUM_THREADS * sizeof(pthread_t));

	srand(time(NULL));
	
	/* memory allocate for every student - thread and their info */

	struct student * Students=malloc(NUM_THREADS * sizeof(struct student));
	
	/* coresponse every thread to the desire student position of struct */

	for(i=0;i<NUM_THREADS;i++){
		initialize_students((Students+i),thread[i],i);
	}

	/* Allocates memory for Queues */
	
	stopAQueue = malloc(NUM_THREADS * sizeof(struct student*));
	BusQueue = malloc(NUM_THREADS * sizeof(struct student*));
	UniversityQueue = malloc(NUM_THREADS * sizeof(struct student*));
	stopBQueue = malloc(NUM_THREADS * sizeof(struct student*));

	/* init the semaphore for bus-threads and mutex */

	sem_init(&semaphore,0,0);
	pthread_mutex_init(&mutex,NULL);
	pthread_barrier_init(&barrier,NULL,NUM_THREADS);

	if(pthread_create(&bus_t,NULL,&busEvent,NULL) != 0 )
			perror("Error Trying to create the bus\n");

	for(i=0;i<NUM_THREADS;i++){

		if( pthread_create(&Students[i].thread,NULL,&startEvent,&Students[i]) != 0 ){
			perror("Error trying to create a thread\n");
		}

	}
	
	for(i=0;i<NUM_THREADS;i++){
		//pthread_barrier_wait(&barrier);
		if( pthread_join(Students[i].thread,NULL) != 0 ){
			perror("Error to join the thread\n");
		}
	}


	
	if(pthread_join(bus_t,NULL) != 0 )
		perror("Error to join the bus\n");

	printf("\nFinished\n\n");

	/* destroy the semaphore ,mutex , barrier */

	pthread_mutex_destroy(&mutex);
	pthread_barrier_destroy(&barrier);
	sem_destroy(&semaphore);

	/* free the memory */

	free(thread);
    	free(Students);
    	free(stopAQueue);
    	free(BusQueue);
    	free(UniversityQueue);
    	free(stopBQueue);

	return  0;
}

