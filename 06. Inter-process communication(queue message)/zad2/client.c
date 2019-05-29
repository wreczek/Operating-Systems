#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <mqueue.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>

#include "commands.h"

int cqid = -1;

mqd_t q_desc = -1;
mqd_t sqid = -1;
int sessionID = -1;
char myPath[10];

void recognize_and_proceed(char * , char * );
void echo(char * string){
    MyMsg msg;
    msg.cqid = cqid;
    msg.pid = getpid();
    msg.mtype = ECHO;
    sprintf(msg.mtext, "%s", string);
    if (mq_send(q_desc, (char*)&msg, MSG_SIZE, 1) < 0)       error("client echo msgsnd failed");
    if (mq_receive(cqid, (char*)&msg, MSG_SIZE, NULL) == -1) error("server response");
    printf("Client received: %s\n", msg.mtext);
}

void list(){
    MyMsg msg;
    msg.cqid = cqid;
    msg.pid = getpid();
    msg.mtype = LIST;
    sprintf(msg.mtext, "%s", "List");
    if (mq_send(q_desc, (char*)&msg, MSG_SIZE, 1) < 0)      error("client list msgsnd failed");
}

void to_all(char * string){
    MyMsg msg;
    msg.cqid = cqid;
    msg.pid = getpid();
    msg.mtype = TOALL;
    sprintf(msg.mtext, "%s", string);
    if (mq_send(q_desc, (char*)&msg, MSG_SIZE, 1) < 0)      error("client to_all msgsnd failed");
}

void to_one(int ID, char * string){
    MyMsg msg;
    msg.cqid = cqid;
    msg.pid = ID;
    msg.mtype = TOONE;
    sprintf(msg.mtext, "%s", string);
    if (mq_send(q_desc, (char*)&msg, MSG_SIZE, 1) < 0)      error("client to_one msgsnd failed");
}

void stop(){
    MyMsg msg;
    msg.cqid = cqid;
    msg.pid = getpid();
    msg.mtype = STOP;
    sprintf(msg.mtext, "%s", "Stop");
    if (mq_send(q_desc, (char*)&msg, MSG_SIZE, 1) < 0)      error("client echo msgsnd failed");
    exit(0);
}

void delete_queue(){
    if (cqid > -1){
        if (sessionID >= 0){
            printf("Need to close all\n");
        }

        if (mq_close(q_desc) == -1){
            printf("error on closing server's queue\n");
        }
        else {
            printf("closed server's queue successfully\n");
        }

        if (mq_close(cqid) == -1) {
            printf("error on closing clients's queue\n");
        }
        else {
            printf("closed client's queue successfully\n");
        }

        if (mq_unlink(myPath) == -1) {
            printf("cannot delete client's queue\n");
        }
        else {
            printf("deleted client's queue successfully\n");
        }
    }
    else {
        printf("queue doesn't exist\n");
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

void register_client(){
    MyMsg msg;
    msg.mtype = LOGIN;
    msg.cqid = cqid;
    msg.pid = getpid();
    sprintf(msg.mtext, "%s", "Login");

    if (mq_send(q_desc, (char*)&msg, MSG_SIZE, 1) < 0)     error("client login failed");
    if (mq_receive(cqid, (char*)&msg, MSG_SIZE, NULL) < 0) error("server login response failed");
    if (sscanf(msg.mtext, "%d", &sessionID) < 1)           error("sscanf login failed");
    if (sessionID < 0)                                     error("servers queue full");

    printf("Client registered with session ID %d.\n", sessionID);
}

int main(int argc, char ** argv){
    printf("My PID = %d\n", getpid());
    if ( atexit(delete_queue) == -1)                error("atexit");
    if ( signal(SIGINT, handle_sigint) == SIG_ERR)  error("signal");

    sprintf(myPath, "/%d", getpid());

    q_desc = mq_open(server_path, O_WRONLY);
    if (q_desc == -1)   error("cannot open server's queue");

    struct mq_attr posixAttr;
    posixAttr.mq_maxmsg = MAX_MSG_Q_SZ;
    posixAttr.mq_msgsize = MSG_SIZE;

    cqid = mq_open(myPath, O_RDONLY | O_CREAT | O_EXCL,
                   0666, &posixAttr);
    if (cqid == -1)     error("cannot create client's queue");

    printf("client cqid = %d\n", cqid);
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
