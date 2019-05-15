#include "shared_utils.h"

int X, K, M;

int t_shmfd;
int l_shmfd;

sem_t * t_sem;
sem_t * l_sem;

timeval tm;

void arrival(){      // ARRIVAL
    sleep(1);
    tm = current_time();
    printf("\nTruck just arrived: %ld.%ld.\n", tm.tv_sec, tm.tv_usec);
    conveyor->t_status = WAITING;
}

void waiting(){      // WAITING
    sleep(1);
    tm = current_time();
    printf("\nconveyor->pids[%d] = %d\n", conveyor->current_remove, conveyor->pids[conveyor->current_remove]);
    printf("Truck is ready for loading: %ld.%ld.\n\n", tm.tv_sec, tm.tv_usec);
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
    truckers_acquire(t_sem); // blokujemy wstawianie na tasme ...
    sleep(1);
    tm = current_time();
    printf("Truck is full %d/%d. Time: %ld.%ld.\n",
            X-conveyor->truck_left_space,
            X,
            tm.tv_sec, tm.tv_usec
    );
    conveyor->truck_left_space = X;
    conveyor->t_status = ARRIVAL;
    truckers_release(t_sem); // ... i odblokowujemy
}

void clean_memory(){
    conveyor->t_status = ARRIVAL; // blokuje przez petle while
    truckers_acquire(t_sem);
    while (!is_conveyor_empty()){
        if (conveyor->t_status == DEPARTURE)
            empty_the_truck();
        else
            take_package();
    }

    kill(conveyor->loaders_pid, SIGUSR1);

    // zamykanie/usuwanie zasobow systemowych
    if (sem_close(t_sem) == -1) /* -----------> */ error("sem_close(t_sem)");
    if (sem_close(l_sem) == -1) /* -----------> */ error("sem_close(l_sem)");

    if (munmap(conveyor, sizeof(conveyor)) == -1)  error("munmap T");
    if (munmap(acc,      sizeof(acc))      == -1)  error("munmap L");
}

void init_trucker(){
    // handlers and exit
    if (signal(SIGINT, SIGINT_handler) < 0) /* -> */ error("signal");
    if (atexit(clean_memory) < 0) /* -----------> */ error("atexit");

    // shared memory
    if ((t_shmfd = shm_open(T_SEM_NAME, RDWR_CREAT_EXCL, S_IRWXU_G)) == -1) /* -----------------> */ error("shm_open");
    if ((l_shmfd = shm_open(L_SEM_NAME, RDWR_CREAT_EXCL, S_IRWXU_G)) == -1) /* -----------------> */ error("shm_open");

    if (ftruncate(t_shmfd, sizeof(*conveyor)) == -1) /* ----------------------------------------> */ error("ftruncate");
    if (ftruncate(l_shmfd, sizeof(*acc))      == -1) /* ----------------------------------------> */ error("ftruncate");

    if ((conveyor = mmap(NULL, sizeof(*conveyor), PROT_RD_WR, MAP_SHARED, t_shmfd, 0)) == (void*)-1) error("mmap");
    if ((acc = mmap(NULL, sizeof(*acc),      PROT_RD_WR, MAP_SHARED, l_shmfd, 0)) == (void*)-1) error("mmap");

    // semaphores
    if ((t_sem = sem_open(T_SEM_NAME, RDWR_CREAT_EXCL, S_IRWXU_G, 0)) == (void*)-1) /* ---------> */ error("sem_open");
    if ((l_sem = sem_open(L_SEM_NAME, RDWR_CREAT_EXCL, S_IRWXU_G, 0)) == (void*)-1) /* ---------> */ error("sem_open");

    // initialization
    conveyor->t_status = ARRIVAL;
    conveyor->K = K;
    conveyor->curr_k = 0;
    conveyor->M = M;
    conveyor->curr_m = 0;
    conveyor->current_insert = 0;
    conveyor->current_remove = 0;
    conveyor->truck_left_space = X;

    for (int i = 0; i < K; ++i){
        conveyor->pids[i] =  -1;
    }
}

int main(int argc, char ** argv){
    if (argc != 4)  error("bad args number");

    X = atoi(argv[1]);
    K = atoi(argv[2]);
    M = atoi(argv[3]);

    init_trucker();
    conveyor->s_time = gettime();

    truckers_release(t_sem);
    loaders_release(l_sem);

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
