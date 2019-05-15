#ifndef SHARED_UTILS_H
#define SHARED_UTILS_H

#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <semaphore.h>


#define MAX_PCKGS_NUM 256
#define T_SEM_NAME "/xAAAN"
#define L_SEM_NAME "/xAABN"

typedef struct timeval timeval;

/** Arguments:
    - trucker.c:
        X to pojemnosc tira
        K to max liczba paczek na tasmie
        M to max masa paczek na tasmie
    - loader.c:
        N to maksymalna masa paczki
        loaders_num to liczba loaderow
        C to liczba cykli
*/

int * acc;
int RDWR_CREAT_EXCL =  O_RDWR | O_CREAT | O_EXCL;
mode_t S_IRWXU_G = S_IRWXU | S_IRWXG|0666;
int PROT_RD_WR = PROT_READ | PROT_WRITE;

enum trucker_status{
    ARRIVAL,
    WAITING,
    LOADING,
    DEPARTURE
};
typedef enum trucker_status trucker_status;

enum loader_status{
    SHIPING,
    AWAITING
};
typedef enum loader_status loader_status;

struct Conveyor{
    trucker_status t_status;
    pid_t loaders_pid;
    int K;       // max liczba sztuk
    int curr_k;  // aktualna liczba sztuk
    int M;       // max waga
    int curr_m;  // aktualna waga
    int current_insert; // miejsce pod ktorym mozemy (po weryfikacji) wsadzic loadera
    int current_remove; // stad pierwsza paczka do wziecia
    pid_t pids[MAX_PCKGS_NUM];  // kolejka pidow loaderow
    int weights[MAX_PCKGS_NUM]; // waga poszczegolnych paczek
    timeval times[MAX_PCKGS_NUM];  // czasy polozenia na tasmie
    int truck_left_space;
    timeval s_time;
} *conveyor;
typedef struct Conveyor Conveyor;

void SIGINT_handler(int signum){
    printf("Finished unloading.\n");
    exit(EXIT_SUCCESS);
}

struct timeval gettime(){
    struct timeval time;
    gettimeofday(&time, NULL);
    return time;
}

struct timeval current_time(){
    struct timeval time, ret_time;
    gettimeofday(&time, NULL);
    if (time.tv_usec - conveyor->s_time.tv_usec < 0){
        ret_time.tv_sec = time.tv_sec - conveyor->s_time.tv_sec-1;
        ret_time.tv_usec = conveyor->s_time.tv_usec - time.tv_usec;
    }
    else {
        ret_time.tv_sec = time.tv_sec - conveyor->s_time.tv_sec-1;
        ret_time.tv_usec = time.tv_usec - conveyor->s_time.tv_usec;
    }
    return ret_time;
}

void error(char * msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

/// ^^^^^ SEMAPHORES ^^^^^
void loaders_acquire(sem_t * sem){
    if (sem_wait(sem) == -1) /* --> */ error("sem_wait");
}

void loaders_release(sem_t * sem){
    if (sem_post(sem) == -1) /* --> */ error("sem_post");
}

// te ponizej robia to samo, ale latwiej w kodzie sie odnalezc
void truckers_acquire(sem_t * sem){
    if (sem_wait(sem) == -1) /* --> */ error("sem_wait");
}

void truckers_release(sem_t * sem){
    if (sem_post(sem) == -1) /* --> */ error("sem_post");
}
/// $$$$$ SEMAPHORES $$$$$

int is_conveyor_available(){
    return conveyor->t_status == WAITING || conveyor->t_status == LOADING;
}

int is_conveyor_empty(){
    return conveyor->pids[conveyor->current_remove] < 0;
}

#endif // SHARED_UTILS_H
