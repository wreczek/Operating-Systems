#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define MAX_TROLLEYS_NUM 64

int actual_trolley = -1; // potrzebne do pthread_cond_t
int p = 0; // liczba pasazerow
int t = 0; // liczba trolley'ow
int c = 0; // pojemnosc wagonika
int n = 0; // liczba przejazdow
int array[MAX_TROLLEYS_NUM];
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_t * p_threads;
pthread_t * t_threads;

void error(char * msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

void print_time(){
    pid_t tid = syscall(__NR_gettid);

    struct timespec timeout;
    clock_gettime(CLOCK_REALTIME, &timeout);
    printf("Current time is %ld.%ld for thread %d.\n", timeout.tv_sec, timeout.tv_nsec, tid);
}

void init(){
    int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
    // pthread_cond_init

    pthread_mutex_t mutex;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);		/* inicjalizacja atrybutów mutexa */
    pthread_mutex_init(&mutex, &attr);	/* inicjalizacja mutexa */
    ...
    pthread_mutex_destroy(&mutex);		/* zwolnienie zasobów zwi¹zanych z mutexem */

    p_threads = calloc((size_t)p, sizeof(pthread_t));
    t_threads = calloc((size_t)t, sizeof(pthread_t));
}

void start_threads(){
    for (int i = 0; i < p; ++i){
        pthread_create(&p_threads[i], NULL, passenger, (void*)i);
    }
    for (int i = 0; i < t; ++i){
        pthread_create(&t_threads[i], NULL, trolley, (void*)i);
    }
}

void join_threads(){
    // watek glowny czeka na ich koniec
}

void destroy_threads(){

    int pthread_mutex_destroy(pthread_mutex_t *mutex)
}

void del(){


    free(p_threads);
    free(t_threads);
}

void * passenger(void * t_num){
    // cos z warunkami na jego koniec pracy
}

void * trolley(void * t_num){


    for (int i = 0; i < n; ++i){
        // wykonanie programu trolleya
        pthread_mutex_lock(&t_threads[t_num]);
        while (x != numer_troleja){
            pthread_cond_wait(&cond, &mutex);
        }
    }
    // zmieniamy wartosc
    // start
    // odjazd
    // odblokowujemy
    pthread_mutex_unlock(&t_threads[t_num]);

    // broadcast
    int pthread_cond_broadcast(pthread_cond_t *cond) // powiadamia o aktualnej wartosci
}

int main(int argc, char ** argv){
    if (argc != 5) error("bad args number");
    p = atoi(argv[1]);
    t = atoi(argv[2]);
    c = atoi(argv[3]);
    n = atoi(argv[4]);

    init();

    del();

    return 0;
}
