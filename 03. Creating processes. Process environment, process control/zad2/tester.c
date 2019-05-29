#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>

#define ITERS 5

void error(char *msg){
    printf("%s", msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char * argv[]){
    /// argv[1] = plik
    /// argv[2] = pmin
    /// argv[3] = pmax
    /// argv[4] = bytes
    srand(time(NULL));
    if (argc != 6) error("Bad arguments number");


    int pmin = atoi(argv[2]);
    int pmax = atoi(argv[3]);
    char *  bytes = argv[4];//atoi(argv[4]);
    int progSeconds = atoi(argv[5]);

    time_t rawtime;
    struct tm * timeinfo;

    int i;
    while(++i){
//    for (i = 0; i < ITERS; ++i){
        FILE * file = fopen(argv[1], "a");
        if (!file) error("fopen");

        pid_t pid = getpid();
        int waitSeconds = rand() % (pmax - pmin) + pmin;
        time ( &rawtime );
        timeinfo = localtime ( &rawtime );
        char string[atoi(bytes)+1];
        string[atoi(bytes)] = '\0';
        int j;
        for (j = 0; j < atoi(bytes); j++){
            string[j] = '0' + rand() % 72;
        }

        fprintf(file, "%d %d %s %s\n"
                ,pid
                ,waitSeconds
                ,strtok(asctime(timeinfo), "\n")
                ,string
        );
        fclose(file);
        sleep(waitSeconds);
        if (i * waitSeconds > progSeconds) break;
    }

    return 0;
}
