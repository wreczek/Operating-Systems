#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>

int flag;
int howMany;
int parentFlag;
pid_t pids[30];
int backups[30];
int isRunning[30];

struct fileInfo{
    char * content;
    long size;
    char * time;
};

void initPidList(){
    int i;
    for (i = 0; i < 30; ++i){
        pids[i] = 0;
    }
}

/// ----------- COMMANDS -------------
void LIST();
void STOPPID(pid_t); // SIGUSR1
void STOPALL();
void STARTPID(pid_t); // SIGUSR2
void STARTALL();
void END(); // SIGRTMAX for all pids

void error(char *msg){
    perror(msg);
    END();
    exit(EXIT_FAILURE);
}

/// ----------- HANDLERS ------------
void sigintHandler(int signum){ // SIGINT
    END();
}

void sigrtmaxHandler(int signum){
    flag = 0; // to je zakonczy, bo koniec petli
}

void sigusr1Handler(int signum){
    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGINT);
    sigdelset(&mask, SIGUSR1);
    sigdelset(&mask, SIGUSR2);
    sigdelset(&mask, SIGRTMAX);
    sigsuspend(&mask);
}

void sigusr2Handler(int signum){
    // unsuspends
}

/// ---------- PROPER FUNCTIONS -------------

struct fileInfo * copyFileToMemory(char * filePath){
    FILE *f = fopen(filePath, "rb");
    if ( !f ) error("copyFileToMemory fopen");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

    char *string = malloc(fsize + 1);
    fread(string, fsize, 1, f);
    fclose(f);

    string[fsize] = 0;

    struct fileInfo * info = malloc(sizeof(struct fileInfo*));
    info->content = string;
    info->size = fsize + 1;

    struct stat statbuf;
    if (stat(filePath, &statbuf) < 0)
        error("stat");
    char * buffer = calloc(21, sizeof(char));
    strftime (buffer, 21 ,"_%F_%T\n", localtime((const time_t *)&statbuf.st_mtime));
    info->time = buffer;
    return info;
}

int monitorArch(char * fileName, char * path, int seconds){
    char filePath[100];
    strcpy(filePath, path);
    strcat(filePath, "/");
    strcat(filePath, fileName);

    FILE * file = fopen(filePath, "r");
    if ( !file ) error("monitorArch fopen");

    struct stat statbuf;

    int howManyChanges = 0;
    struct fileInfo * info = copyFileToMemory(filePath);

    signal(SIGRTMAX, sigrtmaxHandler);
    signal(SIGUSR1, sigusr1Handler);
    signal(SIGUSR2, sigusr2Handler);

    flag = 1;
    while(flag){
        if (stat(filePath, &statbuf) < 0)
            error("monitorArch stat");

        char cmp[21];
        strftime (cmp, 21 ,"_%F_%T\n", localtime((const time_t *)&statbuf.st_mtime));
        if (strcmp(info->time, cmp) != 0){
            howManyChanges++;
            char archPathWithName[120];
            strcpy(archPathWithName, path);
            strcat(archPathWithName, "/archiwum/");
            strcat(archPathWithName, fileName);
            strcat(archPathWithName, info->time);

            FILE * newFile = fopen(archPathWithName, "w");
            if ( !file ) error("archPathWithName fopen");

            fprintf(newFile, "%s", info->content);
            fclose(newFile);

            info = copyFileToMemory(filePath);
        }
        sleep(seconds);
    }
    fclose(file);
    return howManyChanges;
}

void monitorArchHelper(char * listFileName){
    FILE* file = fopen(listFileName, "r");
    if ( !file ) error("monitorArchHelper fopen");

    howMany = 0;
    char line[80];
    while (fgets(line, sizeof(line), file)) {
        char * name;
        char * path;
        int seconds;

        char * ptr = strtok(line, " ");
        name = ptr;
        ptr = strtok(NULL, " ");
        path = ptr;
        ptr = strtok(NULL, " ");
        seconds = atoi(ptr);

        pid_t pid = fork();
        if (pid < 0)    error("fork");
        if (pid == 0){
            int returned = monitorArch(name, path, seconds);
            exit(returned);
        }
        else {
            printf("Process %d monitors %s.\n", pid, name);
            pids[howMany] = pid;
            isRunning[howMany] = 1;
        }
        howMany++;
    }
    fclose(file);

    parentFlag = 1;

    signal(SIGINT, sigintHandler);
    while(parentFlag){
        char command[50];
        if (fgets(command, 50, stdin) == NULL){
            error("Bad fgets: probably too long string");
        }

        char * cmd;   // 6
        char * arg;   // 4
        if ((cmd = strtok(command, " \n")) == NULL){
            cmd = "";
        }
        arg = strtok(NULL, "\n");

        if (strcmp(cmd, "LIST") == 0){
            LIST();
        } else
        if (strcmp(cmd, "STOP") == 0){
            if (strcmp(arg, "ALL") == 0){
                STOPALL();
            }
            else{
                STOPPID(atoi(arg));
            }
        } else
        if (strcmp(cmd, "START") == 0){
            if (strcmp(arg, "ALL") == 0){
                STARTALL();
            }
            else{
                STARTPID(atoi(arg));
            }
        } else
        if (strcmp(cmd, "END") == 0){
            END();
        }
        else printf("BAD COMMAND\n");
        cmd = "";
        arg = "";
    }
}

int main(int argc, char * argv[]){
/// argv[1] = nazwa pliku z listą
    if (argc != 2)
        error("Wrong arguments number");

    initPidList();
    monitorArchHelper(argv[1]);

    return 0;
}

/** ----------- COMMANDS -------------- */

void LIST(){
    int i;
    for (i = 0; i < howMany; ++i){
        if (isRunning[i])
            printf("Process %d is running.\n", pids[i]);
        else
            printf("Process %d is blocked.\n", pids[i]);
    }
}

void STOPPID(pid_t pid){ // SIGUSR1
    int i;
    for (i = 0; i < howMany; ++i){
        if (pids[i] == pid){
            isRunning[i] = 0;
            break;
        }
    }
    kill(pid, SIGUSR1);
}

void STOPALL(){
    int i;
    for (i = 0; i < howMany; ++i){
        isRunning[i] = 0;
        STOPPID(pids[i]);
    }
}

void STARTPID(pid_t pid){ // SIGUSR2
    int i;
    for (i = 0; i < howMany; ++i){
        if (pids[i] == pid){
            isRunning[i] = 1;
            break;
        }
    }
    kill(pid, SIGUSR2);
}

void STARTALL(){
    int i;
    for (i = 0; i < howMany; ++i){
        isRunning[i] = 1;
        STARTPID(pids[i]);
    }
}

void END(){ // SIGRTMAX
    int i;
    for (i = 0; i < howMany; ++i){
        pid_t pid = pids[i];
        kill(pid, SIGRTMAX);
    }
    parentFlag = 0;

    for (i = 0; i < howMany; ++i){
        pid_t pid = pids[i];
        wait(&pids[i]);
        if (WIFEXITED(pids[i])){
            pid_t ret = WEXITSTATUS(pids[i]);
            printf("Process %d created %d file copies.\n", pid, ret);
        }
    }
    exit(0);
}
