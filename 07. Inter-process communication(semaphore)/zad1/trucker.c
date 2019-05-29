#include "shared_utils.h"

key_t key;
int shmid;
int semid;
int X, K, M;
int current;
timeval tm;

void SIGINT_handler(int signum){
    printf("Finished unloading.\n");
    exit(EXIT_SUCCESS);
}

void arrival(){      // ARRIVAL
    sleep(1);
    tm = current_time();
    printf("Truck just arrived: %ld.%ld\n", tm.tv_sec, tm.tv_usec);
    conveyor->t_status = WAITING;
}

void waiting(){      // WAITING
    sleep(1);
    tm = current_time();
    printf("Current remove = %d, pids[..] = %d\n", conveyor->current_remove, conveyor->pids[conveyor->current_remove]);
    printf("Truck is ready for loading: %ld.%ld\n\n", tm.tv_sec, tm.tv_usec);
    if (conveyor->pids[conveyor->current_remove] > 0)
        conveyor->t_status = LOADING;
}

void take_package(){ // LOADING
    sleep(1);
    int current = conveyor->current_remove;
    if (conveyor->pids[current] < 0){
        printf("Current remove %d is less than zero: %d.\n", current, (int)conveyor->pids[current]);
        return;
    }

    int weight = conveyor->weights[current];
    pid_t client_pid = conveyor->pids[current];
    timeval c_time = conveyor->times[current];
    tm = gettime();
    if (conveyor->truck_left_space >= weight){
        conveyor->curr_k -= 1;
        conveyor->curr_m -= weight;
        conveyor->pids[current] = -1;   // zebym wiedzial ze puste
        conveyor->truck_left_space -= weight;
        conveyor->current_remove = (current + 1) % K;

        time_t secs;
        suseconds_t usecs;

        if (tm.tv_usec - c_time.tv_usec < 0){
            secs = tm.tv_sec - c_time.tv_sec;
            usecs = c_time.tv_usec - tm.tv_usec;
        }
        else{
            secs = tm.tv_sec - c_time.tv_sec;
            usecs = tm.tv_usec - c_time.tv_usec;
        }

        printf("Package %dkg from %d was being processed for %ld.%lds. %d/%d and %d/%d left.\n",
                weight,
                client_pid,
                secs, usecs,
                M - conveyor->curr_m, M,
                K - conveyor->curr_k, K
        );
    }
    if (conveyor->truck_left_space >= weight){
        conveyor->t_status = WAITING;
    }
    else {
        conveyor->t_status = DEPARTURE;
    }
}

void empty_the_truck(){   // DEPARTURE
    trucker_acquire(semid); // blokujemy wstawianie na tasme
    sleep(1);
    tm = current_time();
    printf("Truck is full %d/%d: %ld.%ld\n",
            X-conveyor->truck_left_space,
            X,
            tm.tv_sec, tm.tv_usec
    );
    conveyor->truck_left_space = X;
    conveyor->t_status = ARRIVAL;
    trucker_release(semid);
}

void clean_memory() {
    conveyor->t_status = ARRIVAL; // blokuje przez petle while
    trucker_acquire(semid);
    while (!is_conveyor_empty()){
        if (conveyor->t_status == DEPARTURE)
            empty_the_truck();
        else
            take_package();
    }
    if(shmdt(conveyor) < 0) error("Detach shared memory.\n");
    if(semid != 0)          semctl(semid, 0, IPC_RMID);
    if(shmid != 0)          shmctl(shmid, IPC_RMID, NULL);
}

void init_trucker(){
    if (signal(SIGINT, SIGINT_handler) == SIG_ERR)  error("signal");
    if (atexit(clean_memory) != 0)     error("atexit");
    if ((key    = ftok(PROJECT_PATH, PROJECT_ID)) == -1)    /* --> */     error("ftok");
    if ((shmid  = shmget(key, sizeof(Conveyor), S_IRWXU|IPC_CREAT)) < 0)  error("shmget");
    if ((conveyor = shmat(shmid, NULL, 0)) == (void*) -1)   /* --> */     error("shmat");
    if ((semid = semget(key, 4, S_IRWXU|IPC_CREAT)) < 0)  /* --> */       error("semget");

    semun arg2;
    semun arg3;

    arg2.val = K;
    arg3.val = M;

    if (semctl(semid, 0, SETVAL, 0) < 0)  /* --> */                       error("semtctl 1");
    if (semctl(semid, 1, SETVAL, 0) < 0)  /* --> */                       error("semtctl 2");
    if (semctl(semid, 2, SETVAL, arg2) < 0)  /* --> */                    error("semtctl 3");
    if (semctl(semid, 3, SETVAL, arg3) < 0)  /* --> */                    error("semtctl 4");

    conveyor->t_status = ARRIVAL;
    conveyor->K = K;
    conveyor->curr_k = 0;
    conveyor->M = M;
    conveyor->curr_m = 0;
    conveyor->current_insert = 0;
    conveyor->current_remove = 0;
    conveyor->truck_left_space = X;

    for (size_t i = 0; i < K; ++i){
        conveyor->pids[i] = (pid_t) -1;
    }
}

int main(int argv, char ** argc){
    if (argv != 4)  error("Bad args number");
    X = atoi(argc[1]);
    K = atoi(argc[2]);
    M = atoi(argc[3]);
    init_trucker();

    conveyor->s_time = gettime();

    release_conveyor(semid);
    trucker_release(semid);

    while(1){
        switch(conveyor->t_status){
        case ARRIVAL:
            arrival();
            break;
        case WAITING:
            waiting();
            break;
        case LOADING:
            take_package();
            break;
        case DEPARTURE:
            empty_the_truck();
            break;
        }
    }

    return 0;
}
