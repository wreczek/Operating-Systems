#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>

#define MAX_TROLLEYS_NUM 64

struct timeval s_time;

int current_trolley = -1; // potrzebne do pthread_cond_t
int * c_num;
int finished = -1;
int closed = -1;
int is_empty = 1;
int can_entry = 0;
int started = 0;
int p = 0; // liczba pasazerow
int t = 0; // liczba trolley'ow
int c = 0; // pojemnosc wagonika
int n = 0; // liczba przejazdow
int curr_t;

pthread_mutex_t in_mutex;
pthread_mutex_t out_mutex;

pthread_mutexattr_t mutexattr;

pthread_cond_t entry_cond = PTHREAD_COND_INITIALIZER; // trolley
pthread_cond_t start_cond = PTHREAD_COND_INITIALIZER; // client
pthread_cond_t empty_cond = PTHREAD_COND_INITIALIZER; // door
pthread_cond_t finish_cond = PTHREAD_COND_INITIALIZER; // empty
pthread_cond_t trolley_cond = PTHREAD_COND_INITIALIZER; // exit
pthread_cond_t close_cond = PTHREAD_COND_INITIALIZER; // exit
pthread_t * p_threads;
pthread_t * t_threads;

void error(char * msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

struct timeval gettime(){
    struct timeval time;
    gettimeofday(&time, NULL);
    return time;
}

struct timeval diff_time(struct timeval e_time){
    struct timeval diff;
    diff.tv_sec = e_time.tv_sec - s_time.tv_sec;
    diff.tv_usec = e_time.tv_usec - s_time.tv_usec;

    if (e_time.tv_usec < s_time.tv_usec){
        --diff.tv_sec;
        diff.tv_usec = - diff.tv_usec;
    }
    return diff;
}

void sig_hanlder(int signum) {
    for (int i = 0; i < p; i++)
        pthread_cancel(p_threads[i]);
    exit(EXIT_SUCCESS);
}

void print_time(char * msg, int tid){
    struct timeval timeout = diff_time(gettime());
    printf("%s", msg);
    printf("%ld.%ld for thread %d.\n", timeout.tv_sec, timeout.tv_usec, tid);
}

void print_finish(){
    print_time("4. Client - finished work: ", -1);
}

void init(){
    signal(SIGINT, sig_hanlder);
    c_num = calloc((size_t)t, sizeof(int));
    // pthread_cond_init // ??

    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_ERRORCHECK);

    pthread_mutex_init(&in_mutex, &mutexattr);
    pthread_mutex_init(&out_mutex, &mutexattr);

    p_threads = calloc((size_t)p, sizeof(pthread_t));
    t_threads = calloc((size_t)t, sizeof(pthread_t));
}

void join_threads(){
    for (int i = 0; i < t; ++i){
        pthread_join(t_threads[i], NULL);
    }
    for (int i = 0; i < p; ++i){
        pthread_join(p_threads[i], NULL);
    }
}

void free_and_destroy(){
    pthread_mutex_destroy(&in_mutex);
    pthread_mutex_destroy(&out_mutex);

    pthread_mutexattr_destroy(&mutexattr);

    free(c_num);
    free(p_threads);
    free(t_threads);
}

void * passenger(void * p_num){
    atexit(print_finish);
    if (!can_entry)
        pthread_cond_wait(&entry_cond, &in_mutex);
    while (1){
        pthread_mutex_lock(&in_mutex);
        int end = curr_t;
        c_num[end]++;
        print_time("1. Client - entered: ", p_num);
        if (c_num[end] == c){
            print_time("3. Client - started: ", p_num);
            started = 1;
            pthread_cond_broadcast(&start_cond);
            if (!closed)
                pthread_cond_wait(&close_cond, &in_mutex);
            while (c_num[curr_t] != 0)
                pthread_cond_wait(&empty_cond, &in_mutex);
        }
        pthread_mutex_unlock(&in_mutex);
        while (finished != end)
            pthread_cond_wait(&finish_cond, &in_mutex);
        pthread_mutex_lock(&out_mutex);
        c_num[end]--;
        print_time("2. Client - left the trolley: ", p_num);
        pthread_mutex_unlock(&out_mutex);
        if (c_num[end] == 0)
            pthread_cond_broadcast(&empty_cond);
    }
    return NULL;
}

void * trolley(void * t_nu){
    int t_num = (int)t_nu;
    can_entry = 1;
    if (t_num == 0){
        print_time("4. Trolley - open: ", t_num);
        pthread_cond_broadcast(&entry_cond);
    }

    for (int i = 0; i < n; ++i){
        while (t_num != curr_t)
            pthread_cond_wait(&trolley_cond, &in_mutex);
        if (started != 1)
            pthread_cond_wait(&start_cond, &in_mutex);

        started = 0;
        closed = 1;
        curr_t = (curr_t + 1) % t;
        pthread_cond_broadcast(&close_cond);
        print_time("1. Trolley - closed the door: ", t_num);
        print_time("2. Trolley - started: ", t_num);
        pthread_cond_broadcast(&trolley_cond);
        usleep(5);
        while (t_num != curr_t)
            pthread_cond_wait(&trolley_cond, &in_mutex);
        print_time("3. Trolley - finished: ", t_num);
        print_time("4. Trolley - opened: ", t_num);
        finished = t_num;
        pthread_cond_broadcast(&finish_cond);
        while (c_num[t_num] != 0)
            pthread_cond_wait(&empty_cond, &out_mutex);
    }

    if (t_num == t-1){ // end passenger threads
        for (int i = 0; i < p; ++i){
            pthread_cancel(p_threads[i]);
        }
    }
    return NULL;
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

    s_time = gettime();

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
