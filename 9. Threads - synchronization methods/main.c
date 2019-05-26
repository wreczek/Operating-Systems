#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#define MAX_TROLLEYS_NUM 64

int tid = 16;
int current_trolley = -1; // potrzebne do pthread_cond_t
int * number_of_clients;

int closed = -1;
int is_empty = 1;
int p = 0; // liczba pasazerow
int t = 0; // liczba trolley'ow
int c = 0; // pojemnosc wagonika
int n = 0; // liczba przejazdow
pthread_mutex_t * mutex;
pthread_mutex_t entry_mutex;
pthread_mutexattr_t mutexattr;
pthread_cond_t t_cond = PTHREAD_COND_INITIALIZER; // trolley
pthread_cond_t c_cond = PTHREAD_COND_INITIALIZER; // client
pthread_cond_t d_cond = PTHREAD_COND_INITIALIZER; // door
pthread_cond_t e_cond = PTHREAD_COND_INITIALIZER; // empty
pthread_cond_t ex_cond = PTHREAD_COND_INITIALIZER; // exit
pthread_t * p_threads;
pthread_t * t_threads;

void error(char * msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

void sig_hanlder(int signum) {
    for (int i = 0; i < p; i++)
        pthread_cancel(p_threads[i]);
    exit(EXIT_SUCCESS);
}

void print_finish(){
    print_time("4. Client - finished work: ");
}

void print_time(char * msg){
    //pid_t tid = syscall(__NR_gettid);

//    struct timespec timeout;
//    clock_gettime(CLOCK_REALTIME, &timeout);
//    printf(msg);
//    printf("%ld.%ld for thread %d.\n", timeout.tv_sec, timeout.tv_nsec, tid);
}

void init(){
    signal(SIGINT, sig_hanlder);
    number_of_clients = calloc((size_t)t, sizeof(int));
    mutex = calloc((size_t)t, sizeof(pthread_mutex_t));
    // pthread_cond_init // ??

    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_ERRORCHECK);
    for (int i = 0; i < t; ++i){
        pthread_mutex_init(&mutex[i], &mutexattr);
    }

    p_threads = calloc((size_t)p, sizeof(pthread_t));
    t_threads = calloc((size_t)t, sizeof(pthread_t));
}

void join_threads(){
    for (int i = 0; i < t; ++i){
        pthread_join(t_threads[i], NULL);
    }
    is_working = 0;
    for (int i = 0; i < p; ++i){
        pthread_join(p_threads[i], NULL);
    }
}

void free_and_destroy(){
    for (int i = 0; i < t; ++i){
        pthread_mutex_destroy(&mutex[i]);
    }

    pthread_mutex_destroy(&entry_mutex);
    pthread_mutexattr_destroy(&mutexattr);

    free(number_of_clients);
    free(p_threads);
    free(t_threads);
}

void * passenger(void * t_num){
    atexit(print_finish);
    pthread_cond_broadcast(&t_cond);
    while (1){
        pthread_mutex_lock(&entry_mutex);
        print_time("1. Client - entered the trolley: ");
        number_of_clients[current_trolley]++;
        if (number_of_clients[current_trolley] == c ){ // nie moga wchodzic
            print_time("3. Client - clicked start button: ");
            closed = 1;
            pthread_cond_broadcast(&d_cond);
        }
        else{ // moga wchodzic
            pthread_mutex_unlock(&entry_mutex);
        }
        pthread_cond_wait(&ex_cond, &mutex[current_trolley]); // czekaja na koniec tripa
        print_time("2. Client - left the trolley: ");
        number_of_clients[current_trolley]--;
        if (number_of_clients[current_trolley] == 0){
            is_empty = 0;
            pthread_cond_broadcast(&e_cond);
            pthread_mutex_unlock(&entry_mutex);
        }
    }
}

void * trolley(void * t_num){
//    pthread_mutex_lock(&exit_mutex[current_trolley]);
    for (int i = 0; i < n; ++i){
        while (t_num != current_trolley){
            pthread_cond_wait(&t_cond, &mutex[current_trolley]);
        }
        print_time("4. Trolley - Opened the door: ");
        pthread_cond_broadcast(&ex_cond);
//        pthread_mutex_unlock(&exit_mutex[current_trolley]);
        while (is_empty != 1){
            pthread_cond_wait(&e_cond, &mutex[current_trolley]);
        }
        while (number_of_clients != c){
            pthread_cond_wait(&c_cond, &mutex[current_trolley]);
        }
        while (closed != 1) {
            pthread_cond_wait(&d_cond, &mutex[current_trolley]);
        }
        print_time("1. Trolley - closed the door: ");
        print_time("2. Trolley - started the ride: ");
        usleep(5);
        print_time("3. Trolley - finished the ride: ");
        current_trolley = (current_trolley+1)%t;
        pthread_cond_broadcast(t_cond);
    }
    while (t_num != current_trolley){
        pthread_cond_wait(&t_cond, &mutex[current_trolley]);
    }
    while (is_empty != 1){
        pthread_cond_wait(&e_cond, &mutex[current_trolley]);
    }
    print_time("1. Trolley - closed the door: ");
    //pid_t tid = syscall(__NR_gettid);
    printf("5. Trolley - thread %d finished work.\n", tid);

    if (t_num == t-1){ // end passenger threads
        for (int i = 0; i < p; ++i){
//            pthread_kill(p_threads[i], SIGINT);
            pthread_cancel(p_threads[i]);
        }
    }
}

void start_threads(){
    for (int i = 0; i < t; ++i){
        pthread_create(&t_threads[i], NULL, trolley, (void*)i);
    }
    for (int i = 0; i < p; ++i){
        pthread_create(&p_threads[i], NULL, passenger, (void*)i);
    }
}

int main(int argc, char ** argv){
    //if (argc != 5) error("bad args number");
    p = 5;//atoi(argv[1]);
    t = 3;//atoi(argv[2]);
    c = 2;//atoi(argv[3]);
    n = 5;//atoi(argv[4]);

    init();
    start_threads();
    join_threads();
    free_and_destroy();

    return 0;
}
