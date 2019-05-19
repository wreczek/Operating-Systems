#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

int P, K, N, L, nk, search, verbose;
char ** buffer; // tablica wersow
int last_insert = 0; // tu ostatnio wsadzilismy
int last_remove = 0; // stad ostatnio pobralismy
int current_size; // aktualna liczba elementow w tablicy
char * txt_file_name;
FILE * text_file;
pthread_t * p_threads;
pthread_t * k_threads;

void sigint_handler(int signum){
    printf("Catched SIGINT.\n");
    for (int p = 0; p < P; p++)
        pthread_cancel(p_threads[p]);
    for (int k = 0; k < K; k++)
        pthread_cancel(k_threads[k]);
    exit(EXIT_SUCCESS);
}

void error(char * msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

void initialize(){
    //
}

void configure(char * file_path){
    FILE * config_file;
    if ((config_file = fopen(file_path, "r")) == NULL) error("fopen");

    fscanf(config_file, "%d %d %d %s %d %d %d %d", &P, &K, &N,
           txt_file_name, &search, &verbose, &nk);
    printf("Config:\n> P: %d, K: %d, N: %d, File Name: %s, \
           L: %d,  Search: %d, Verbose: %d, nk: %d\n");
    fclose(config_file);
}

int main(int argc, char ** argv){
    /// tryb opisowy - producent raportuje prace
    if (argc != 2) error("bad args number");

    initialize();
    configure(argv[1]);


    return 0;
}
