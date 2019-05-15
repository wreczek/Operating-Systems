#include "shared_utils.h"

int N;
int loaders_num;

int t_shmfd;
int l_shmfd;

sem_t * t_sem;
sem_t * l_sem;

timeval tm;

void SIGUSR_handler(int signum){
    printf("I gotta clean up after myself.\n");

    if (sem_close(t_sem) == -1) /* -----------> */ error("sem_close(t_sem)");
    if (sem_close(l_sem) == -1) /* -----------> */ error("sem_close(l_sem)");

    if (sem_unlink(T_SEM_NAME) == -1) /* -----> */ error("sem_unlink T");
    if (sem_unlink(L_SEM_NAME) == -1) /* -----> */ error("sem_unlink L");

    if (munmap(conveyor, sizeof(conveyor)) == -1)  error("munmap T");
    if (munmap(acc,      sizeof(acc))      == -1)  error("munmap L");

    if (shm_unlink(T_SEM_NAME) == -1) /* -----> */ error("shm_unlink T");
    if (shm_unlink(L_SEM_NAME) == -1) /* -----> */ error("shm_unlink L");

    exit(EXIT_SUCCESS);
}

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
    truckers_acquire(t_sem); // sprawdzamy, czy kierowca nie odjechal
    truckers_release(t_sem);
    loaders_acquire(l_sem);

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
        (conveyor->current_insert + 1) % conveyor->K;

    while (conveyor->pids[conveyor->current_insert] != -1){}
    loaders_release(l_sem);
}

void init_loaders(){
    signal(SIGUSR1, SIGUSR_handler);
    if ((t_shmfd = shm_open(T_SEM_NAME, O_RDWR, S_IRWXU_G)) == -1) /* --------------------------> */ error("shmopen T");
    if ((l_shmfd = shm_open(L_SEM_NAME, O_RDWR, S_IRWXU_G)) == -1) /* --------------------------> */ error("shmopen L");

    if ((conveyor = mmap(NULL, sizeof(*conveyor), PROT_RD_WR, MAP_SHARED, t_shmfd, 0)) == (void*)-1) error("mmap T");
    if ((acc      = mmap(NULL, sizeof(*acc),      PROT_RD_WR, MAP_SHARED, l_shmfd, 0)) == (void*)-1) error("mmap L");

    if ((t_sem = sem_open(T_SEM_NAME, O_RDWR)) == (void*)-1) /* --------------------------------> */ error("sem_open T");
    if ((l_sem = sem_open(L_SEM_NAME, O_RDWR)) == (void*)-1) /* --------------------------------> */ error("sem_open L");
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
    conveyor->loaders_pid = getpid();

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
