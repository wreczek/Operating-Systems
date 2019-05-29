#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_CONT_SIZE 256

void error(char * msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char ** argv){
/// argv[1] = sciezka do potoku nazwanego
    if (argc != 2){
        error("Bad args count");
    }

    int fd;

    char buffer[MAX_CONT_SIZE];

    if (mkfifo(argv[1], 0666) < 0) error("mkfifo");

    if ((fd = open(argv[1], O_RDONLY)) < 0) error("open");
    while(read(fd, buffer, sizeof(buffer)) > 0){
        printf("%s\n", buffer);
    }
    if (close(fd) < 0) error("close");
    return 0;
}
