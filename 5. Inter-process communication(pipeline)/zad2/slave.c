#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_CONT_SIZE 256

void error(char * msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char ** argv){
/// argv[1] = sciezka do potoku nazwanego
/// argv[2] = liczba N
    if (argc != 3){
        error("Bad args count");
    }

    int fd;
    int pid = getpid();
    srand(time(NULL));

    printf("Moj PID to %d.\n", pid);

    int i;
    char buffer[MAX_CONT_SIZE];
    char output[MAX_CONT_SIZE];

    if ((fd = open(argv[1], O_WRONLY)) < 0) error("open");
    for (i = 0; i < atoi(argv[2]); ++i){
        FILE * pipe = popen("date", "r");
        if (!pipe)  error("popen");
        if (!fgets(buffer, MAX_CONT_SIZE, pipe)) error("fgets");
        char * buffer2 = strtok(buffer, "\n");
        if (pclose(pipe) < 0) error("pclose");
        if (sprintf(output, "%d %s", pid, buffer2) < 0) error("sprintf");
        if (write(fd, output, strlen(output)) < 0) error("write");
        sleep((rand() % 3 + 1));
    }

    if (close(fd) < 0) error("close");
    return 0;
}
