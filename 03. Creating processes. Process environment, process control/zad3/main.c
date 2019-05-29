#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

struct fileInfo{
    char * content;
    long size;
    char * time;
};

void error(char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

void setLimits(char const * cpu, char const * mem){
    long int cpuLim = strtol(cpu, NULL, 10);
    long int memLim = 1048576 * strtol(mem, NULL, 10);

    struct rlimit cpuRLimit;
    struct rlimit memRLimit;

    cpuRLimit.rlim_max = (rlim_t) cpuLim;
    cpuRLimit.rlim_cur = (rlim_t) cpuLim;
    if(setrlimit(RLIMIT_CPU, &cpuRLimit) != 0)
        error("setLimits cpuRLimit");

    memRLimit.rlim_max = (rlim_t) memLim;
    memRLimit.rlim_cur = (rlim_t) memLim;
    if(setrlimit(RLIMIT_AS, &memRLimit) != 0)
        error("setLimits memRLimitn");
}

long int getUsageTime(struct timeval * t){
    return (long int)t->tv_sec * 1000000 + (long int)t->tv_usec;
}

void reportUsage(struct rusage usage1, struct rusage usage2){
    long int uCpuTime = abs(getUsageTime(&usage2.ru_utime) - getUsageTime(&usage1.ru_stime));
    long int sCpuTime = abs(getUsageTime(&usage2.ru_stime) - getUsageTime(&usage1.ru_stime));
    printf("\nuser CPU time used: %lf\n", (double)uCpuTime / 1000000);
    printf("system CPU time used: %lf\n", (double)sCpuTime / 1000000);
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

void monitorArchHelper(char * listFileName, int programSeconds,
                       char * timeLimit, char * memLimit){
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

        struct rusage usage1;
        struct rusage usage2;
        getrusage(RUSAGE_CHILDREN, &usage1);

        pid_t pid = fork();
        if (pid < 0)    error("fork");
        if (pid == 0){
            setLimits(timeLimit, memLimit);
            int returned = monitorArch(name, seconds, path, programSeconds);
            exit(returned); // mozna _exit() jakby byly bledy
        }
        else{
            pids[howMany] = pid;
        }
        howMany++;
        getrusage(RUSAGE_CHILDREN, &usage2);
        reportUsage(usage1, usage2);
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

            struct rusage usage1;
            struct rusage usage2;
            getrusage(RUSAGE_CHILDREN, &usage1);

            pid = fork();

            if (pid < 0) error("monitorExec fork");
            if (pid == 0){
                /// ograniczenia
                execlp("cp", "-T", filePath, archPathWithName, NULL);
            }
            else{
                wait(&pid);
                howMany++;
            }
            getrusage(RUSAGE_CHILDREN, &usage2);
            reportUsage(usage1, usage2);
            fclose(newFile);
        }

        sleep(seconds);
        if (i * seconds > programSeconds) break;
    }

    fclose(file);
    return howMany;
}

void monitorExecHelper(char * listFileName, int programSeconds,
                       char * timeLimit, char * memLimit){
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

        struct rusage usage1;
        struct rusage usage2;
        getrusage(RUSAGE_CHILDREN, &usage1);

        pid_t pid = fork();
        if (pid < 0)    error("fork");
        if (pid == 0){
            setLimits(timeLimit, memLimit);
            int returned = monitorExec(name
                           ,seconds
                           ,path
                           ,programSeconds
                     //      ,
                       //    ,
                           );
            exit(returned);
        }
        else{
            pids[howMany] = pid;
        }
        getrusage(RUSAGE_CHILDREN, &usage2);
        reportUsage(usage1, usage2);
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
/// argv[4] = avTime
/// argv[5] = avMBytes
    if (argc != 6)
        error("Wrong arguments number");

    if ( strcmp(argv[3], "arch") == 0 )
        monitorArchHelper(argv[1]
                         ,atoi(argv[2])
                         ,argv[4]
                         ,argv[5]
                         );
    else if ( strcmp(argv[3], "exec") == 0 )
        monitorExecHelper(argv[1]
                         ,atoi(argv[2])
                         ,argv[4]
                         ,argv[5]
                         );
    else
        error("Bad argv[3]");

    return 0;
}
