
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "commands.h"

#define MAX_MSG_LEN 256
#define MAX_USR_CNT 10

int sqid = -1;
key_t sqkey = -1;

int cqid = -1;
key_t cqkey = -1;

int q_desc = -1;
int sessionID = -1;

void recognize_and_proceed(char * , char * );
void echo(char * string){
    MyMsg msg;
    msg.cqid = cqid;
    msg.cqkey = cqkey;
    msg.mtype = ECHO;
    sprintf(msg.mtext, "%s", string);
    if (msgsnd(q_desc, &msg, MSG_SIZE, 0) < 0)      error("client echo msgsnd failed");
    if (msgrcv(cqid, &msg, MSG_SIZE, 0, 0) == -1)   error("server response");
    printf("Client received: %s\n", msg.mtext);
}

void list(){
    MyMsg msg;
    msg.cqid = cqid;
    msg.cqkey = cqkey;
    msg.mtype = LIST;
    sprintf(msg.mtext, "%s", "List");
    if (msgsnd(q_desc, &msg, MSG_SIZE, 0) < 0)      error("client list msgsnd failed");
}

void to_all(char * string){
    MyMsg msg;
    msg.cqid = cqid;
    msg.cqkey = cqkey;
    msg.mtype = TOALL;
    sprintf(msg.mtext, "%s", string);
    if (msgsnd(q_desc, &msg, MSG_SIZE, 0) < 0)      error("client to_all msgsnd failed");
}

void to_one(int ID, char * string){
    MyMsg msg;
    msg.cqid = cqid;
    msg.cqkey = ID;
    msg.mtype = TOONE;
    sprintf(msg.mtext, "%s", string);
    if (msgsnd(q_desc, &msg, MSG_SIZE, 0) < 0)      error("client to_one msgsnd failed");
}

void stop(){
    MyMsg msg;
    msg.cqid = cqid;
    msg.cqkey = cqkey;
    msg.mtype = STOP;
    sprintf(msg.mtext, "%s", "Stop");
    if (msgsnd(q_desc, &msg, MSG_SIZE, 0) < 0)      error("client echo msgsnd failed");
    exit(0);
}

void delete_queue(){
    if ( msgctl(cqid, IPC_RMID, NULL) < 0){
        printf("main queue remove");
    }
    else{
        printf("Deleted successfully.\n");
    }
}

void handle_sigint(int signum){
    printf("Odebrano sygnal SIGINT\n");
    stop();
}

void read_f(char * filename){
    char * buff;
    size_t len = 0;
    ssize_t bufsize = 1;

    printf("filename: %s\n", filename);

    FILE * file = fopen(filename, "r");
    if (!file)        error("READ fopen");
    while ((bufsize = getline(&buff, &len, file)) > 0){
        char * rest;
        char * cmd = strtok_r(buff, delims, &rest);

        recognize_and_proceed(cmd, rest);
    }

    if (fclose(file) < 0){
        error("READ fclose");
    }
}

void recognize_and_proceed(char * cmd, char * rest){
    rest = strtok_r(rest, "\n", &rest); // ucinam enter na koncu
    if (strcmp(cmd, "ECHO") == 0){
        echo(rest);
    }
    else if (strcmp(cmd, "LIST") == 0){
        list();
    }
    else if (strcmp(cmd, "2ALL") == 0){
        to_all(rest);
    }
    else if (strcmp(cmd, "2ONE") == 0){
        char * num = strtok_r(rest, delims, &rest);
        to_one(atoi(num), rest);
    }
    else if (strcmp(cmd, "STOP") == 0){
        stop();
    }
    else if (strcmp(cmd, "READ") == 0){
        read_f(rest);
    }
    else {
        printf("Bad command\n");
    }
}

int create_queue(char * path, int ID){
    int key = ftok(path, ID);
    if (key == -1)    error("ftok in create_queue");

    int QID = msgget(key, 0);
    if (QID < 0)     error("msgget in create_queue");

    return QID;
}

void register_client(){
    MyMsg msg;
    msg.mtype = LOGIN;
    msg.cqid = cqid;
    msg.cqkey = cqkey;
    sprintf(msg.mtext, "%s", "Login");

    if ( msgsnd(q_desc, &msg, MSG_SIZE, 0) < 0)      error("client login failed");
    if ( msgrcv(cqid, &msg, MSG_SIZE, 0, 0) < 0)     error("server login response failed");
    if (sscanf(msg.mtext, "%d", &sessionID) < 1)     error("sscanf login failed");
    if (sessionID < 0)                               error("servers queue full");

    printf("Client registered with session ID %d.\n", sessionID);
}

int main(int argc, char ** argv){
    if ( atexit(delete_queue) < 0)                  error("atexit");
    if ( signal(SIGINT, handle_sigint) == SIG_ERR)  error("signal");

    char * path = getenv("HOME");
    if (!path)                                      error("getenv");

    q_desc = create_queue(path, PROJECT_ID);

    cqkey = ftok(path, getpid());
    if (cqkey == -1)              error("cannot generate clients queue key");

    cqid = msgget(cqkey, IPC_CREAT | IPC_EXCL | 0666);
    if (cqid < 0)                 error("clients msgget");


    register_client();

    char * buff;
    size_t len = 0;
    while(1){
        printf("\nType command:\n> ");
        getline(&buff, &len, stdin); // ssize_t bufsize =
        char * rest;
        char * cmd = strtok_r(buff, delims, &rest);
        if (!cmd){
            printf("empty string\n");
            continue;
        }
        recognize_and_proceed(cmd, rest);
    }

    return 0;
}
