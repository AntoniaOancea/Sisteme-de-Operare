#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <semaphore.h>

#define NTHREADS 5
sem_t semaphore;
pthread_mutex_t mutex;
pthread_t *threads;
int nr;
void barrier_point(){
    pthread_mutex_lock(&mutex);
    nr--;
    if(nr == 0){
        pthread_mutex_unlock(&mutex);
        sem_post(&semaphore);
        return;
    }
    pthread_mutex_unlock(&mutex);
    sem_wait(&semaphore);
    sem_post(&semaphore);

}

void* thread_routine(void* arg){
    int* argument = (int*) arg;
    int number = *argument;
    printf("%d reached the barrier\n", number);

    barrier_point();

    printf("%d passed the barrier\n", number);
    free(argument);
    return NULL;
}

int main(){
    nr = NTHREADS;
    threads = (pthread_t*) malloc(NTHREADS * sizeof(pthread_t));
    int semaphore_value = 0;
    if(sem_init(&semaphore, 0, semaphore_value)){
        perror("Error at sem_init\n");
        return errno;
    }

    if(pthread_mutex_init(&mutex, NULL)){
        perror("Error at pthread_mutex_init\n");
        return errno;
    }

    for(int i = 0; i < NTHREADS; ++i){
        int* argument;
        argument = (int*) malloc(sizeof(int));
        *argument = i;
        if(pthread_create(&threads[i], NULL, thread_routine, argument)){
            perror(NULL);
            return errno;
        }
    }

    for(int i = 0; i < NTHREADS; ++i){
        if(pthread_join(threads[i], NULL)){
            perror(NULL);
            return errno;
        }
        
    }

    if(sem_destroy(&semaphore)){
        perror(NULL);
        return errno;
    }

    if(pthread_mutex_destroy(&mutex)){
        perror(NULL);
        return errno;
    }
    free(threads);
    return 0;


}
