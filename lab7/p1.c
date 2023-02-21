#include<unistd.h>
#include<stdio.h>
#include<pthread.h>
#include<errno.h>
#include<stdlib.h>
#define MAX_RESOURCES 5
int available_resources = MAX_RESOURCES;

pthread_mutex_t mtx;
pthread_t *threads;

int decrease_count(int count){
	pthread_mutex_lock(&mtx);
	if(available_resources>=count){
		available_resources = available_resources - count;
		printf("Got %d resources,%d remaining \n",count,available_resources);
		pthread_mutex_unlock(&mtx);
	}
	else{
		printf("Not enough resources. Got :%d, Remaining resources: %d\n",count,available_resources);
		pthread_mutex_unlock(&mtx);
		return -1;
	}
	return 0;
}
int increase_count(int count){
	pthread_mutex_lock(&mtx);
	available_resources+=count;
	printf("Released  %d resources %d remaining\n",count,available_resources);
	pthread_mutex_unlock(&mtx);
	return 0;
}

void* thread_routine(void *arg){
	int* argument=(int *)arg;
	int count=*argument;
	if(!decrease_count(count)){
		increase_count(count);
	}
	free(argument);
	return NULL;
}
int main(){
	printf("MAX_RESOURCES = %d\n",MAX_RESOURCES);
	threads = (pthread_t*)malloc(MAX_RESOURCES*sizeof(pthread_t));
	
	if(pthread_mutex_init(&mtx,NULL)){
		perror(NULL);
		return errno;
	}
	
	for(int i=0;i<5;i++){
		int* argument;
		argument=(int*) malloc(sizeof(int));
		*argument=i;
		pthread_create(&threads[i], NULL, thread_routine, argument);
	}
	
	for(int i=0;i<5;i++){
		pthread_join(threads[i],NULL);
	}
	pthread_mutex_destroy(&mtx);
	free(threads);
	return 0;
}
