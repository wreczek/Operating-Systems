#define _XOPEN_SOURCE 500

#include <ftw.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

void error(char *msg){
    printf("%s", msg);
    exit(EXIT_FAILURE);
}

char *type_of_file_nftw(int type){
    switch(type) {
        case FTW_F: return "file";
        case FTW_D: return "dir";
        case FTW_SL: return "link";
        default: return "";
    };
}

int display_info(const char *path, const struct stat *sb, int flagType, struct FTW *tw_buf){
    char *absPath = (char*) calloc(150, sizeof(char));

    realpath(path, absPath);
    pid_t pid = fork();
    if (pid < 0) error("pid fork()");
    if (pid == 0){
        if (flagType == FTW_D){
            printf("\nPID:       %d", pid);
            printf("\nPath:      %s", path);
            printf("\nAbs. path: %s\n", absPath);
            char cmd[80] = "ls -l ";
            strcat(cmd, path);
            system(cmd);
            exit(0);
        }
    }
    else{
        //kill(getpid(), SIGKILL);
        wait(&pid);
    }
    free(absPath);
    return 0;
}

int main(int argc, char **argv) {
    char * path;
    if (argc != 2)
        path = ".";
    else
        path = argv[1];

    nftw(path, display_info, 10, FTW_PHYS);

    return 0;
}
