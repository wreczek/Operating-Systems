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

int cqids[10] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
pid_t pids[10] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
int active = 1;
int counter = 0;
mqd_t q_desc = -1;

void end(){
    active = 0;
}

void handle_sigint(int signum){
    printf("Odebrano sygnal SIGINT\n");
    end();
    exit(EXIT_SUCCESS);
}

void delete_queue(){
    if (q_desc > -1){
        if ( mq_close(q_desc) == -1)      error("mq_close");
        if (mq_unlink(server_path) == -1) error("mq_unlink");
        printf("deleted server's queue successfully\n");
    }
}

void log_in(MyMsg * msg){
    cqids[counter] = msg->cqid;
    pids[counter] = msg->pid;
    char clientPath[10];
    sprintf(clientPath, "/%d", pids[counter]);
    printf("Clients path: %s, pid: %d, cqid: %d\n", clientPath, pids[counter], cqids[counter]);
    printf("Got pid = %d\n", pids[counter]);

    int cqid = mq_open(clientPath, O_WRONLY);
    if (cqid == -1)     error("cannot read client's queue");

    if (counter >= MAX_CLIENTS){
        printf("Maximum number of clients exceeded\n");
        sprintf(msg->mtext, "%d", -1);
        if (mq_send(cqid, (char*)msg, MSG_SIZE, 1) == -1)   error("login response failed");
        if (mq_close(cqid) == -1)       error("cannot close client's queue");
    }
    else{
        sprintf(msg->mtext, "%d", counter);
        while (pids[counter] > 0 && counter < MAX_CLIENTS) counter++;
    }
    if (mq_send(cqids[counter-1], (char*)msg, MSG_SIZE, 1) == -1)   error("login response failed");
}

void echo(MyMsg * msg){
    time_t sekund;
    char napis[100];

    time (&sekund);
    strftime (napis, 100, "%c", localtime(&sekund));

    sprintf(msg->mtext, "%s %s", msg->mtext, napis);
    if (mq_send(msg->cqid, (char*)msg, MSG_SIZE, 1) == -1)    error("echo msgsnd");
}

void stop(MyMsg * msg){
    int i;
    for (i = 0; i < MAX_CLIENTS; ++i){
        if (cqids[i] == msg->cqid){
            cqids[i] = -1;
            counter = i;
        }
    }
    printf("Client %d exited.\n", msg->cqid);
}

void list(){
    int i;
    printf("Active clients:\n");
    for (i = 0; i < MAX_CLIENTS; ++i){
        if (cqids[i] > 0)
            printf(" -\t %d\n", cqids[i]);
    }
}

void to_all(MyMsg * msg){
    int i;
    for (i = 0; i < MAX_CLIENTS; ++i){
        if (cqids[i] > 0){
            if (mq_send(cqids[i], (char*)msg, MSG_SIZE, 1) == -1)
                error("to_all msgsnd");
        }
    }
}

void to_one(MyMsg * msg){
    if (mq_send(msg->cqid, (char*)msg, MSG_SIZE, 1) == -1)     error("to one msgsnd");
}

void recognize_and_proceed(MyMsg * msg){
    if (!msg) return;
    switch(msg->mtype){
        case LOGIN:
            log_in(msg);
            printf("Logged user with cqid %d.\n", msg->cqid);
        break;
        case STOP:
            stop(msg);
            printf("Client %d is gone.\n", msg->cqid);
        break;
        case LIST:
            list();
        break;
        case ECHO:
            printf("Server got >>%s<< from >>%d<<.\n", msg->mtext, msg->cqid);
            echo(msg);
        break;
        case TOALL:
            to_all(msg);
            printf("Server sent >>%s<< to all clients.\n", msg->mtext);
        break;
        case TOONE:
            to_one(msg);
            printf("Server sent >>%s<< to %d.\n", msg->mtext, msg->pid);
        break;
    }
}

int main(int argc, char ** argv){
    if ( atexit(delete_queue) < 0)                    error("atexit()");
    if ( signal(SIGINT, handle_sigint) == SIG_ERR)    error("signal()");

    struct mq_attr current_state;
    struct mq_attr posix_attr;
    posix_attr.mq_maxmsg = MAX_MSG_Q_SZ;
    posix_attr.mq_msgsize = MSG_SIZE;

    q_desc = mq_open(server_path, O_RDONLY |
                    O_CREAT | O_EXCL, 0666, &posix_attr);

    if (q_desc == -1)   error("cannot create server's queue");

    MyMsg msg;
    while(1){
        if (active == 0){
            if (mq_getattr(q_desc, &current_state) == -1) error("cannot read queue params");
            if (current_state.mq_curmsgs == 0) exit(0);
        }
        if (mq_receive(q_desc, (char*)&msg, MSG_SIZE, NULL) == -1)     error("msgrcv");
        recognize_and_proceed(&msg);
    }

    return 0;
}
