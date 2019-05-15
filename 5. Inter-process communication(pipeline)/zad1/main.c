#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_CMDS 5

void error(char * msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

char ** splitCommands(char * line){
    int count = 0;
    char ** args = NULL;
    char delims[2] = {' ', '\n'};
    line = strtok(line, delims);
    while (line){
        count++;
        args = realloc(args, count * sizeof(char*));
        args[count - 1] = line;
        line = strtok(NULL, delims);
    }
    args = realloc(args, (count + 1) * sizeof(char*));
    args[count] = NULL;

    return args;
}

int executeLine(char * line, ssize_t len){
    int i;
    int pipes[2][2];
    int cmdsCount = 0;

    char * cmds[MAX_CMDS];

    line = strtok(line, "|");
    while (line){
        cmds[cmdsCount++] = line;
        line = strtok(NULL, "|");
    }

    for (i = 0; i < cmdsCount; ++i){

        if (i != 0){
            close(pipes[i % 2][0]);
            close(pipes[i % 2][1]);
        }

        if (pipe(pipes[i % 2]) < 0){
            error("pipe");
        }

        pid_t pid = vfork();
        if (pid == 0){ /// DZIECKO
            char ** args = splitCommands(cmds[i]);

            if (i != cmdsCount - 1){
                close(pipes[i % 2][0]);
                if (dup2(pipes[i % 2][1], STDOUT_FILENO) < 0){
                    error("dup2");
                }
            }

            if (i != 0) {
                close(pipes[(i+1) % 2][1]);
                if (dup2(pipes[(i+1) % 2][0], STDIN_FILENO) < 0){
                    error("dup2");
                }
            }

            if (execvp(args[0], args) < 0){
                error("execvp");
            }
            error("not allowed location");
        }
        else if (pid > 0){
            //wait(&pid);
        }
        else {
            error("Bad fork");
        }
    }
    close(pipes[i % 2][0]);
    close(pipes[i % 2][1]);
    wait(NULL);
    exit(0);
}

int main(int argc, char ** argv){
/// argv[1] = file name
    if (argc  != 2){
        error("Bad args number");
    }

    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    fp = fopen(argv[1], "r");
    if (fp == NULL)
        error("fopen");

    while ((read = getline(&line, &len, fp)) != -1) {
        pid_t pid = vfork();
        if (pid == 0){
            executeLine(line, read);
            error("Bad location");
        }
        else if (pid > 0){
            int status;
            wait(&status);
            if (status){
                error("On status");
            }
        }
        else{
            error("main fork()");
        }
    }

    fclose(fp);
    if (line)
        free(line);

    return 0;
}
