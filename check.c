/*
	Developed for habrahabr.ru article
    Copyright 2011 Korisk (korisk(at)yandex.ru)

    This file is part of Pinc.

	Pinc is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published 
	by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. Pinc is distributed in 
	the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
	See the GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along with Pinc. If not, see http://www.gnu.org/licenses/.
*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> //_SC_NPROCESSORS_ONLN defined here
#include <sys/time.h>

#include <sched.h>

void usage(char *name){
	printf("This test measures time spended for Parallel INCrement of one variable with using of different lock mechanism\n");
	printf("Usage: %s [-t addititional threads][-f big number factor][-h|-?]\n",basename(name));
	printf("\t-t addititional theads - number of threads which added to number of processors\n");
	printf("\t-f factor - factor which difine big number. Number of increments = factorial(factor)\n");
	printf("\t-h,-? - this help\n");
}


//#define _GNU_SOURCE   

/*
:.!bash
A10="obase=10;";A16="obase=16;"; while read a;do echo $A10$a|bc|tr -d '\n'; echo -n ' 0x'; echo $A16$a|bc|tr -d '\n'; echo " $a";    done < <(X="2*3*4"; for i in $(seq 5 17);do echo -e "$X";X="$X*$i";done)

4!	24				0x18			2*3*4
5!	120				0x78			2*3*4*5
6!	720				0x2D0 			2*3*4*5*6
7!	5040			0x13B0 			2*3*4*5*6*7
8!	40320			0x9D80 			2*3*4*5*6*7*8
9!	362880			0x58980			2*3*4*5*6*7*8*9
10!	3628800			0x375F00		2*3*4*5*6*7*8*9*10
11!	39916800		0x2611500 		2*3*4*5*6*7*8*9*10*11
12!	479001600		0x1C8CFC00 		2*3*4*5*6*7*8*9*10*11*12
13!	6227020800 		0x17328CC00 	2*3*4*5*6*7*8*9*10*11*12*13
14!	87178291200 	0x144C3B2800 	2*3*4*5*6*7*8*9*10*11*12*13*14
15!	1307674368000 	0x13077775800	2*3*4*5*6*7*8*9*10*11*12*13*14*15
16!	20922789888000	0x130777758000	2*3*4*5*6*7*8*9*10*11*12*13*14*15*16
*/

#ifndef BIG_NUMBER
	#define BIG_NUMBER ((unsigned long)(0x1C8CFC00))
#endif

unsigned long factor(unsigned long N){
	if(N <= 0)return 1;
	return (N*factor(N-1));
}




//Current GMT time in ms since the Epoch
unsigned long epochmsec() {
 	struct timeval tv;
	if (gettimeofday(&tv, 0) == 0) {
		return tv.tv_sec * 1000 + tv.tv_usec / 1000;
	}else{
		return 0L;
	}
}

typedef struct {
	int nprocs;
	int threads;
	int my_number;
	unsigned long big_number;
} position;

volatile unsigned long int incremented;

/************* spinlock test ***********************/

pthread_spinlock_t spin;

int spin_init(int nums){
	incremented = 0;
	pthread_spin_init(&spin,0);
	return 0;
}

void *spin_thread(void *_data){
	position *data = (position*)(_data);
	cpu_set_t cpuset;
	CPU_ZERO((&cpuset));
	CPU_SET((data->my_number % data->nprocs), (&cpuset));
	int ret;
	ret = pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset);
	if(ret != 0){
		fprintf(stdout,"Set affinity error: %s \n",strerror(ret));
		exit(EXIT_FAILURE);
	}

	unsigned long N = (unsigned long)((data->big_number)/(data->threads));
	unsigned long i = 0;
 	for(i=0; i< N; i++){
		pthread_spin_lock(&spin);
		incremented++;
		pthread_spin_unlock(&spin);
	}
	return NULL;
}

int spin_fini(int nums){
	if(incremented == 0)
		printf("alarm! wrong result\n");
//	printf("incremented: %ld\n",incremented);
}
/***********************************************/

/************* mutex test ***********************/

pthread_mutex_t mut;

int mutex_init(int nums){
	incremented = 0;
	pthread_mutex_init(&mut,NULL);
	return 0;
}

void *mutex_thread(void *_data){
	position *data = (position*)(_data);
	cpu_set_t cpuset;
	CPU_ZERO((&cpuset));
	CPU_SET((data->my_number % data->nprocs), (&cpuset));
	int ret;
	ret = pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset);
	if(ret != 0){
		fprintf(stdout,"Set affinity error: %s \n",strerror(ret));
		exit(EXIT_FAILURE);
	}

	unsigned long N = (unsigned long)((data->big_number)/(data->threads));
	unsigned long i = 0;
 	for(i=0; i< N; i++){
		pthread_mutex_lock(&mut);
		incremented++;
		pthread_mutex_unlock(&mut);
	}
	return NULL;
}

int mutex_fini(int nums){
	if(incremented == 0)
		printf("alarm! wrong result\n");
//	printf("incremented: %ld\n",incremented);
}
/***********************************************/

/************* lock test ***********************/

int lock_init(int nums){
	incremented = 0;
	return 0;
}

void *lock_thread(void *_data){
	position *data = (position*)(_data);
	cpu_set_t cpuset;
	CPU_ZERO((&cpuset));
	CPU_SET((data->my_number % data->nprocs), (&cpuset));
	int ret;
	ret = pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset);
	if(ret != 0){
		fprintf(stdout,"Set affinity error: %s \n",strerror(ret));
		exit(EXIT_FAILURE);
	}

	unsigned long N = (unsigned long)((data->big_number)/(data->threads));
	unsigned long i = 0;
 	for(i=0; i< N; i++){
		__sync_fetch_and_add(&incremented,1);
	}
	return NULL;
}

int lock_fini(int nums){
	if(incremented == 0)
		printf("alarm! wrong result\n");
//	printf("incremented: %ld\n",incremented);
}
/***********************************************/


/************* lock test ***********************/

int lock2_init(int nums){
	incremented = 0;
	return 0;
}

void *lock2_thread(void *_data){
	position *data = (position*)(_data);
	cpu_set_t cpuset;
	CPU_ZERO((&cpuset));
	CPU_SET((data->my_number % data->nprocs), (&cpuset));
	int ret;
	ret = pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset);
	if(ret != 0){
		fprintf(stdout,"Set affinity error: %s \n",strerror(ret));
		exit(EXIT_FAILURE);
	}

	unsigned long N = (unsigned long)((data->big_number)/(data->threads));
	unsigned long i = 0;
	unsigned long x = 0;
	
 	for(i=0; i< N; i++){
		do{
			x = incremented;
		}while(!__sync_bool_compare_and_swap(&incremented,x , x + 1));
	}
	return NULL;
}

int lock2_fini(int nums){
	if(incremented == 0)
		printf("alarm! wrong result\n");
}
/***********************************************/

/************* dumb test ***********************/

int dumb_init(int nums){
	incremented = 0;
	return 0;
}

void *dumb_thread(void *_data){
	position *data = (position*)(_data);
//	printf("%d -\n",data->my_number);
	if(data->my_number != 0)return NULL;
	unsigned long i = 0;
 	for(i=0; i< data->big_number; i++){
		incremented++;
	}
	return NULL;
}

int dumb_fini(int nums){
	if(incremented == 0)
		printf("alarm! wrong result\n");
//	printf("incremented: %ld\n",incremented);
}
/***********************************************/


#define MAX_TESTS 0x40
int main(int argc, char** argv, char **env){
	


	//массивы функций
	void * (*per_thread[MAX_TESTS])(void *) = {0}; 	//выполняемая в каждой из нитей
	int (*init_test[MAX_TESTS])(int) = {0};			//инициализирующиая
	int (*fini_test[MAX_TESTS])(int) = {0};			//финализация
	char *titles[MAX_TESTS] = {0};
	
	int tests = 0;
	titles[tests] = "pure increment test";
	init_test[tests]	= &dumb_init;
	per_thread[tests]	= &dumb_thread;
	fini_test[tests]	= &dumb_fini;
	tests++;
		
	titles[tests] = "mutexed test";
	init_test[tests]	= &mutex_init;
	per_thread[tests]	= &mutex_thread;
	fini_test[tests]	= &mutex_fini;
	tests++;
	
	titles[tests] = "lock increment test";
	init_test[tests]	= &lock_init;
	per_thread[tests]	= &lock_thread;
	fini_test[tests]	= &lock_fini;
	tests++;

	titles[tests] = "lock compare&swap test";
	init_test[tests]	= &lock2_init;
	per_thread[tests]	= &lock2_thread;
	fini_test[tests]	= &lock2_fini;
	tests++;

	titles[tests] = "spinlock test";
	init_test[tests]	= &spin_init;
	per_thread[tests]	= &spin_thread;
	fini_test[tests]	= &spin_fini;
	tests++;

	//default
	unsigned int	big_factor = 11;
	unsigned int 	additition_threads = 4;

	int opt = 0;
	while ((opt = getopt(argc, argv, "?hf:t:")) != -1) {
		switch (opt) {
			case '?':
			case 'h':
				usage(argv[0]);
				exit(EXIT_SUCCESS);
				break;
			case 't':
				additition_threads = atoi(optarg);
				break;
			case 'f':
				big_factor = atoi(optarg);
				break;
			default: /* '?' */
				break;
		}
	}



	printf("CPU ");
	fflush(stdout);
	int sys = system("cat /proc/cpuinfo |grep 'model name'|head -n1");
	printf("Thread libs: ");
	fflush(stdout);
	sys = system("getconf GNU_LIBPTHREAD_VERSION");



#ifdef _SC_NPROCESSORS_ONLN
	long nprocs = -1, nprocs_max = -1;
	nprocs = sysconf(_SC_NPROCESSORS_ONLN);
	if (nprocs < 1)
	{
		fprintf(stdout, "Could not determine number of CPUs online:\n%s\n", strerror (errno));
		exit (EXIT_FAILURE);
	}
	nprocs_max = sysconf(_SC_NPROCESSORS_CONF);
	if (nprocs_max < 1)
	{
		fprintf(stdout, "Could not determine number of CPUs configured:\n%s\n", strerror (errno));
	}
	printf ("%ld of %ld cores online\n",nprocs, nprocs_max);
#else
	fprintf(stdout, "Could not determine number of CPUs\n");
	exit (EXIT_FAILURE);
#endif
		
	int test_number = 0;
	position	*positions = NULL;
	position next;
	next.nprocs = nprocs;
	next.my_number = 0;
	next.big_number = factor(big_factor);
//	next.big_number = 0xFFFFFF;
	printf("Additition threads: %d\n",additition_threads);
	printf("Big number: %d! = 0x%lx\n", big_factor, next.big_number);
	nprocs += additition_threads;
	int thread_number = 0;
	int threads_number = 0;
	int ret = 0;
	int my_number = 0;
	pthread_t *threads = NULL;
	unsigned long before = 0;
	unsigned long after  = 0;
	threads = malloc(nprocs * sizeof(pthread_t));
	positions = malloc(nprocs * sizeof(position));
			
	printf("Thrds:\t");	
	for(threads_number = 1; threads_number <= nprocs; threads_number++){
		printf("%d\t",threads_number);			
	}
	printf("\n");	
	while( MAX_TESTS > test_number && NULL != init_test[test_number] && NULL != per_thread[test_number] ){
		fprintf(stdout,"%s \n",titles[test_number]);
		printf("*\t");
		fflush(stdout);
		for(threads_number = 1; threads_number <= nprocs; threads_number++){

			next.my_number = 0;
			next.threads = threads_number;
			my_number = 0;
			memset(threads, 0, nprocs*sizeof(pthread_t));
			memset(positions,0,nprocs*sizeof(positions));
			
			
			(*init_test[test_number])(threads_number);
			before = epochmsec();
			for(thread_number = 0; thread_number < threads_number; thread_number++){
				next.my_number = my_number;
				positions[thread_number] = next;
				pthread_create(threads+thread_number,NULL,per_thread[test_number],positions+thread_number);
				if(ret != 0){
					fprintf(stdout,"Thread %d create error: %s\n",thread_number, strerror(ret));
					exit (EXIT_FAILURE);
				}
//				printf("%d ",next.my_number);
				my_number++;
			}

			for(thread_number = 0; thread_number < threads_number; thread_number++){
				ret = pthread_join(*(threads+thread_number),NULL);
				if(ret != 0){
					fprintf(stdout,"Thread %d join error: %s\n",thread_number, strerror(ret));
					exit (EXIT_FAILURE);
				}
			}
			(*fini_test[test_number])(threads_number);
			after = epochmsec();
			printf("%ld\t", after - before);
			fflush(stdout);
//			printf("-------------------------------\n");
		}
		printf("\n");
			
		test_number++;
	}
	fprintf(stdout, "Done. %d tests at all.\n", test_number);
	free(threads);
	return 0;
}

