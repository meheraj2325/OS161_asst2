#include <types.h>
#include <lib.h>
#include <synch.h>
#include <test.h>
#include <thread.h>
#include <clock.h>
#include <kern/time.h>
#include "prime_gen.h"
static struct semaphore *sem1;
static struct semaphore *alldone;
struct timespec total_waiting_time[PRIMEGENERATOR + 5],total_running_time[PRIMEGENERATOR + 5];

unsigned long long endTime_sec[PRIMEGENERATOR + 5],startTime_sec[PRIMEGENERATOR + 5],duration_sec[PRIMEGENERATOR + 5],tot_run_sec[PRIMEGENERATOR + 5], tot_wait_sec[PRIMEGENERATOR + 5];
unsigned long endTime_nsec[PRIMEGENERATOR + 5],startTime_nsec[PRIMEGENERATOR + 5],duration_nsec[PRIMEGENERATOR + 5],tot_run_nsec[PRIMEGENERATOR + 5], tot_wait_nsec[PRIMEGENERATOR + 5];
unsigned long int countPrimes[PRIMEGENERATOR + 5];

void prime_generator(void * arg, unsigned long th_id) {
	struct timespec startt,end,duration;
	unsigned long int *args=arg;
	unsigned long int start = args[0]; // Initial number
	unsigned long size = args[1]; // range
	unsigned long int i;
	unsigned long int count = 0l; // keep the count for number of prime generated
	unsigned long int range=start+size*th_id; // actual range for this thread
	// Get start time
	gettime(&startt);
	//generating prime number -- square root method
	for (i = start; i < range + size; i += 1) {
		if (i % 2 != 0) {
			int flag = 1;
			unsigned long int j;
			unsigned long int l=floorSqrt(i);
			for (j = 3; j < l; j += 2) {
				if (i % j == 0) {
					flag = 0;
					break;
				}
			}
			if (flag == 1) {
				count++;
			}
		}
	}
	P(sem1);
	//get end time
	gettime(&end);
	// total time in seconds
	timespec_sub(&end, &startt, &duration);

	/* storing necessary informations */
	countPrimes[th_id] = count;
	duration_sec[th_id] = (unsigned long long)duration.tv_sec;
	duration_nsec[th_id] = (unsigned long)duration.tv_nsec;
	endTime_sec[th_id] = (unsigned long long)end.tv_sec;
	endTime_nsec[th_id] = (unsigned long)end.tv_nsec;
	startTime_sec[th_id] = (unsigned long long)startt.tv_sec;
	startTime_nsec[th_id] = (unsigned long)startt.tv_nsec;
	/* storing necessary informations */
	
	// kprintf("S %ld [%lu-%lu]:%ld took %llu.%09lu(%llu.%09lu-%llu.%09lu) secs\n", th_id, range,range+size,count,(unsigned long long)duration.tv_sec,(unsigned long)duration.tv_nsec,(unsigned long long)end.tv_sec,(unsigned long)end.tv_nsec,(unsigned long long)startt.tv_sec,(unsigned long)startt.tv_nsec);
	V(sem1);
	V(alldone);
}
int runPrimeGenerator(int nargs, char **args) {
	(void) nargs; /* avoid compiler warnings */
	(void) args;
	sem1=sem_create("printer", 1);
	alldone=sem_create("alldone", 0);
	unsigned long int i;
	int result;
	unsigned long int x=(unsigned long int)STARTNUMBER;
	unsigned long int size=(unsigned long int)SIZE;
	for (i=0; i<PRIMEGENERATOR; i++) {
		unsigned long int data[2];
		data[0]=x;
		data[1]=size;
		/**
		 * Thread creator and runner --
		 * you need to modify thread_fork function and thread data structure
		 * to randomly assign the priority
		 */
		result = thread_fork2("Prime Generator", NULL,prime_generator,(void*)data, i,total_running_time,total_waiting_time);
		if (result) {
			panic("runPrimeGenerator: Prime Generator Thread thread_fork failed: %s\n",
					strerror(result));
		}

	}
	//wait while all generators finished
	for(i=0;i<PRIMEGENERATOR; i++){
		P(alldone);
	}
	print_Statistics(x,size);

	return 0;
}

/**
 * Function for sqrt (int only)
 */
int floorSqrt(int x){
	// Base cases
	if (x == 0 || x == 1)
		return x;

	// Staring from 1, try all numbers until
	// i*i is greater than or equal to x.
	int i = 1, result = 1;
	while (result <= x)
	{
		i++;
		result = i * i;
	}
	return i - 1;
}

void print_Statistics(unsigned long int start, unsigned long int size)
{
	unsigned long int th_id;
	for(th_id = 0; th_id<PRIMEGENERATOR; th_id++){
		unsigned long int range = start+size*th_id; // actual range for this thread
		tot_run_sec[th_id] = (unsigned long long)total_running_time[th_id].tv_sec;
		tot_wait_sec[th_id] = (unsigned long long)total_waiting_time[th_id].tv_sec;
		tot_run_nsec[th_id] = (unsigned long) total_running_time[th_id].tv_nsec;
		tot_wait_nsec[th_id] = (unsigned long) total_waiting_time[th_id].tv_nsec;


		kprintf(" S %ld [%lu-%lu]:%ld took %llu.%09lu secs.\n", th_id, range,range+size,countPrimes[th_id],duration_sec[th_id],duration_nsec[th_id]);
		kprintf(" [START TIME to END TIME] : [ %llu.%09lu - %llu.%09lu ]\n",startTime_sec[th_id],startTime_nsec[th_id],endTime_sec[th_id],endTime_nsec[th_id]);
		kprintf(" [TOTAL RUNNING TIME]  : [%llu.%09lu]\n",tot_run_sec[th_id],tot_run_nsec[th_id]);
		kprintf(" [TOTAL WAITING TIME]  : [%llu.%09lu]\n\n",tot_wait_sec[th_id],tot_wait_nsec[th_id]);
	}
}

