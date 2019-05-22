#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

int passenger_threads_num = 0;
int trolley_threads_num = 0;
int c = 0; // pojemnosc wagonika
int n = 0; // liczba przejazdow
pthread_t * p_threads;
pthread_t * t_threads;

void error(char * msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char ** argv){
    if (argc != 2) error("bad args number");



    return 0;
}
