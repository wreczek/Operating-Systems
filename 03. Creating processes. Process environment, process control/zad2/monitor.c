#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>

struct fileInfo{
    char * content;
    long size;
    char * time;
};

void error(char *msg){
    printf("%s", msg);
    exit(EXIT_FAILURE);
}

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

int monitorArch(char * fileName, int seconds, char * path, int programSeconds){
    char filePath[100];
    strcpy(filePath, path);
    strcat(filePath, "/");
    strcat(filePath, fileName);

    FILE * file = fopen(filePath, "r");
    if ( !file ) error("monitorArch fopen");

    struct stat statbuf;

    int howMany = 0;
    struct fileInfo * info = copyFileToMemory(filePath);
    int i = 0;
    while(++i){
        if (stat(filePath, &statbuf) < 0)
            error("monitorArch stat");

        char cmp[21];
        strftime (cmp, 21 ,"_%F_%T\n", localtime((const time_t *)&statbuf.st_mtime));
        if (strcmp(info->time, cmp) != 0){
            howMany++;
            char archPathWithName[120];// = calloc(120, sizeof(char));
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
        if (i * seconds > programSeconds) break;
    }
    fclose(file);
    return howMany;
}

void monitorArchHelper(char * listFileName, int programSeconds){
    FILE* file = fopen(listFileName, "r");
    if ( !file ) error("monitorArchHelper fopen");

    pid_t pids[30];
    int howMany = 0;
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
            int returned = monitorArch(name, seconds, path, programSeconds);
            exit(returned); // mozna _exit() jakby byly bledy
        }
        else{
            pids[howMany] = pid;
        }
        howMany++;
    }
    fclose(file);

    int i;
    for (i = 0; i < howMany; ++i){
        pid_t pid = pids[i];
        wait(&pids[i]);
        if (WIFEXITED(pids[i])){
            pid_t ret = WEXITSTATUS(pids[i]);
            printf("Proces %d utworzyl %d kopii pliku.\n", pid, ret);
        }
    }
}

int monitorExec(char * fileName, int seconds, char * path, int programSeconds){
    char filePath[100];
    strcpy(filePath, path);
    strcat(filePath, "/");
    strcat(filePath, fileName);

    FILE * file = fopen(filePath, "r");
    if ( !file ) error("monitorArch fopen");

    struct stat statbuf;
    if (stat(filePath, &statbuf) < 0)
        error("exec stat");
    char lastModTime[21];// = calloc(21, sizeof(char));
    strftime (lastModTime, 21 ,"_%F_%T\n", localtime((const time_t *)&statbuf.st_mtime));

    char archPathWithName[120];// = calloc(120, sizeof(char));
    strcpy(archPathWithName, path);
    strcat(archPathWithName, "/archiwum/");
    strcat(archPathWithName, fileName);
    strcat(archPathWithName, lastModTime);

    pid_t pid;
    int howMany = 0;
    pid = fork();
    if (pid < 0) error("monitorExec fork");
    if (pid == 0){
        execlp("cp", "-T", filePath, archPathWithName, NULL);
    }
    else{
        wait(&pid);
        howMany++;
    }
    //free(archPathWithName);

    int i = 0;
    while(++i){
        if (stat(filePath, &statbuf) < 0)
            error("monitorArch stat");

        char cmp[21];
        strftime (cmp, 21 ,"_%F_%T\n", localtime((const time_t *)&statbuf.st_mtime));

        if (strcmp(lastModTime, cmp) != 0){
            strcpy(lastModTime, cmp);
            char archPathWithName[120];// = calloc(120, sizeof(char));
            strcpy(archPathWithName, path);
            strcat(archPathWithName, "/archiwum/");
            strcat(archPathWithName, fileName);
            strcat(archPathWithName, lastModTime);

            FILE * newFile = fopen(archPathWithName, "w");
            if ( !file ) error("archPathWithName exec fopen");

            pid = fork();

            if (pid < 0) error("monitorExec fork");
            if (pid == 0){
                execlp("cp", "-T", filePath, archPathWithName, NULL);
            }
            else{
                wait(&pid);
                howMany++;
            }

//            free(archPathWithName);
            fclose(newFile);
        }

        sleep(seconds);
        if (i * seconds > programSeconds) break;
    }

//    free(lastModTime);
    fclose(file);
    return howMany;
}

void monitorExecHelper(char * listFileName, int programSeconds){
    FILE* file = fopen(listFileName, "r");
    if ( !file ) error("monitorArchHelper fopen");

    pid_t pids[30];
    int howMany = 0;
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
            int returned = monitorExec(name, seconds, path, programSeconds);
            exit(returned);
        }
        else{
            pids[howMany] = pid;
        }
        howMany++;
    }
    fclose(file);

    int i;
    for (i = 0; i < howMany; ++i){
        pid_t pid = pids[i];
        wait(&pids[i]);
        if (WIFEXITED(pids[i])){
            pid_t ret = WEXITSTATUS(pids[i]);
            printf("Proces %d utworzyl %d kopii pliku.\n", pid, ret);
        }
    }
}

int main(int argc, char * argv[]){
/// argv[1] = nazwa pliku z listą
/// argv[2] = czas_monitorowania
/// argv[3] = tryb kopiowania:
    if (argc != 4)
        error("Wrong arguments number");

    if ( strcmp(argv[3], "arch") == 0 )
        monitorArchHelper(argv[1], atoi(argv[2]));
    else if ( strcmp(argv[3], "exec") == 0 )
        monitorExecHelper(argv[1], atoi(argv[2]));
    else
        error("Bad argv[3]");

    return 0;
}
