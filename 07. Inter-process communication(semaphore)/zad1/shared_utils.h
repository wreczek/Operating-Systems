#ifndef SHARED_UTILS_H
#define SHARED_UTILS_H

#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_PCKGS_NUM 256
#define PROJECT_ID 0xAAAA
#define PROJECT_PATH getenv("HOME")

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

union semun {   /* Used in calls to semctl() */
    int                 val;
    struct semid_ds *   buf;
    unsigned short *    array;
#if defined(__linux__)
    struct seminfo *    __buf;
#endif
};
typedef union semun semun;

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
    int K;       // max liczba sztuk
    int curr_k;  // aktualna liczba sztuk
    int M;       // max waga
    int curr_m;  // aktualna waga
    int current_insert; // miejsce pod ktorym mozemy (po weryfikacji) wsadzic loadera ???
    int current_remove; // stad pierwsza paczka do wziecia
    pid_t pids[MAX_PCKGS_NUM];  // kolejka pidow loaderow
    struct timeval times[MAX_PCKGS_NUM];  // czasy polozenia na tasmie
    int weights[MAX_PCKGS_NUM]; // waga poszczegolnych paczek
    int truck_left_space;
    struct timeval s_time;

} *conveyor;
typedef struct Conveyor Conveyor;

void error(char * msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

struct timeval gettime(){
    struct timeval time;
    gettimeofday(&time,NULL);
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

/// ^^^^^ SEMAPHORES ^^^^^
void acquire_conveyor(int semid){
    struct sembuf sops;
    sops.sem_num = 0;
    sops.sem_op = -1;
    sops.sem_flg = 0;

    if (semop(semid, &sops, 1) < 0)  error("semop1");
}

void release_conveyor(int semid){
    struct sembuf sops;
    sops.sem_num = 0;
    sops.sem_op = 1;
    sops.sem_flg = 0;

    if (semop(semid, &sops, 1) < 0)  error("semop2");
}

void trucker_acquire(int semid){
    struct sembuf sops;
    sops.sem_num = 1;
    sops.sem_op = -1;
    sops.sem_flg = 0;

    if (semop(semid, &sops, 1) < 0)   error("semop3");
}

void trucker_release(int semid){
    struct sembuf sops;
    sops.sem_num = 1;
    sops.sem_op = 1;
    sops.sem_flg = 0;

    if (semop(semid, &sops, 1) < 0)   error("semop");
}

void acquire_K(int semid){
    struct sembuf sops;
    sops.sem_num = 3;
    sops.sem_op = -1;
    sops.sem_flg = 0;

    if (semop(semid, &sops, 1) < 0)   error("semop3");
}

void release_K(int semid){
    struct sembuf sops;
    sops.sem_num = 2;
    sops.sem_op = 1;
    sops.sem_flg = 0;

    if (semop(semid, &sops, 1) < 0)   error("semop");
}

void acquire_M(int semid, int value){
    struct sembuf sops;
    sops.sem_num = 3;
    sops.sem_op = -value;
    sops.sem_flg = 0;

    if (semop(semid, &sops, 1) < 0)   error("semop3");
}

void release_M(int semid, int value){
    struct sembuf sops;
    sops.sem_num = 3;
    sops.sem_op = value;
    sops.sem_flg = 0;

    if (semop(semid, &sops, 1) < 0)  error("semop");
}

/// $$$$$ SEMAPHORES $$$$$

int is_conveyor_available(){
    return conveyor->t_status == WAITING || conveyor->t_status == LOADING;
}

int is_conveyor_empty(){
    return conveyor->pids[conveyor->current_remove] < 0;
}

#endif // SHARED_UTILS_H
