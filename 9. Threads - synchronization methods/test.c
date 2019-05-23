#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/syscall.h>

void print_tid(int t_num){
    pid_t x = syscall(__NR_gettid);
    print("Watek %d with t_num = %d.\n", x, t_num);
}

void func(void * t_num){
    print_tid((int)t_num);
}

int main(int argc, char ** argv){
    pthread_t * threads = calloc((size_t)5, sizeof(pthread_t));

    for (int i = 0; i < 5; i++){
        pthread_create(&threads[i], NULL, passenger, (void*)i);
    }

    free(threads);

    return 0;
}

