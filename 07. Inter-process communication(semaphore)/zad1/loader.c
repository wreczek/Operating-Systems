#include "shared_utils.h"

timeval tm;
int loaders_num;
int semid;
key_t key;
int shmid;
int N;

void awaiting_for_conveyor(){ // AWAITING
    sleep(3);
    tm = current_time();
    printf("%d is waiting for the production tape to be released: %ld.%ld\n", getpid(), tm.tv_sec, tm.tv_usec);
}

void put_package_on_conveyor(int w){ // SHIPING
    sleep(3);
    while (!is_conveyor_available()){
        awaiting_for_conveyor();
    }

    trucker_acquire(semid);
    trucker_release(semid);

    acquire_conveyor(semid);
    acquire_K(semid);
    acquire_M(semid, w);

    tm = gettime();

    while (conveyor->curr_m + w > conveyor->M || conveyor->curr_k + 1 > conveyor->K){}
    int current = conveyor->current_insert;
    conveyor->times[current] = tm;
    conveyor->weights[current] = w;
    conveyor->pids[current] = getpid();
    conveyor->curr_k += 1;
    conveyor->curr_m += w;

    tm = current_time();
    printf("Placed %dkg by %d. %d/%dkg, %d/%dunits occupied. Time: %ld.%ld.\n\n",
            w, getpid(),
            conveyor->curr_m,
            conveyor->M,
            conveyor->curr_k,
            conveyor->K,
            tm.tv_sec, tm.tv_usec
    );

    conveyor->current_insert = // insert inkrementujemy na koncu
        (current + 1) % conveyor->K;

    while (conveyor->pids[conveyor->current_insert] != -1){}
    release_K(semid);
    release_M(semid, w);
    release_conveyor(semid);
}

void init_loaders(){
    if ((key      = ftok(PROJECT_PATH, PROJECT_ID)) == -1) /* -> */ error("ftok");
    if ((shmid    = shmget(key, sizeof(Conveyor), S_IRWXU)) < 0)    error("shmget");
    if ((conveyor = shmat(shmid, NULL, 0)) == (void*) -1) /* --> */ error("shmat");
    if ((semid    = semget(key, 0, 0)) < 0) /* ----------------> */ error("semget");
}

void limited_action(int C, int w){
    for (int i = C-1; i >= 0; --i){
        printf("W petelce, i = %d, pid = %d\n", i, getpid());
        put_package_on_conveyor(w);
    }
    exit(0);
}

void infinite_action(int w){
    while(1){
        printf("PID = %d\n", getpid());
        put_package_on_conveyor(w);
    }
}

int main(int argc, char ** argv){
    if (argc > 4 || argc <= 2)  error("Bad args number");
    N = atoi(argv[1]);

    loaders_num = atoi(argv[2]);
    init_loaders();

    if (argc == 4){ // arg C provided
        int C = atoi(argv[3]);
        for(int i = 0; i < loaders_num; ++i){
            if (fork() == 0){
        /// Pracowników uruchamiamy fork i exec - argument programu
                limited_action(C, (i+1)%N);
            }
        }
    }
    else { // arg C not provided
        for(int i = 0; i < loaders_num; ++i){
            if (fork() == 0){
                infinite_action((i+1)%N);
            }
        }
    }

    while(wait(0)){
        if(errno != ECHILD)
            break;
    }
    return 0;
}
