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
#include <time.h>

#include "commands.h"

int cqids[10] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
key_t cqkeys[10];
int active = 1;
int counter = 0;
int q_desc = -1;
key_t sqkey = -1;

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
        int tmp = msgctl(q_desc, IPC_RMID, NULL);
        if (tmp == -1){
            printf("error on deleting queue");
        }
        printf("deleted servers queue successfully");
    }
}

void log_in(MyMsg * msg){
    cqids[counter] = msg->cqid;
    cqkeys[counter] = msg->cqkey;
    if (msgget(cqkeys[counter], 0) == -1)    error("msgget");
    if (counter >= MAX_CLIENTS){
        printf("Maximum number of clients exceeded");
        sprintf(msg->mtext, "%d", -1);
    }
    else{
        sprintf(msg->mtext, "%d", counter);
        while (cqids[counter] > 0 && counter < MAX_CLIENTS) counter++;
    }
    if (msgsnd(cqids[counter-1], msg, MSG_SIZE, 0) == -1)   error("login response failed");
}

void echo(MyMsg * msg){
    time_t sekund;
    char napis[100];

    time (&sekund);
    strftime (napis, 100, "%c", localtime(&sekund));

    sprintf(msg->mtext, "%s %s", msg->mtext, napis);
    if (msgsnd(msg->cqid, msg, MSG_SIZE, 0) == -1)      error("echo msgsnd");
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
            if (msgsnd(cqids[i], msg, MSG_SIZE, 0) == -1)
                error("to_all msgsnd");
        }
    }
}

void to_one(MyMsg * msg){
    if (msgsnd(msg->cqkey, msg, MSG_SIZE, 0) == -1)     error("to one msgsnd");
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
            printf("Server sent >>%s<< to %d.\n", msg->mtext, msg->cqkey);
        break;
    }
}

int main(int argc, char ** argv){
    if ( atexit(delete_queue) < 0)                        error("atexit()");
    if ( signal(SIGINT, handle_sigint) == SIG_ERR)        error("signal()");

    struct msqid_ds current_state;

    char * path = getenv("HOME");
    if (!path)              error("getenv");

    sqkey = ftok(path, PROJECT_ID);
    if (sqkey == -1)          error("server ftok");

    q_desc = msgget(sqkey, IPC_CREAT | IPC_EXCL | 0666);
    if (q_desc < 0)         error("server msgget");

    MyMsg msg;
    while(1){
        if (active == 0){
            if (msgctl(q_desc, IPC_STAT, &current_state) < 0)   error("msgctl");
            if (counter == 0)                    break;    // current_state.msg_qnum
        }
        if (msgrcv(q_desc, &msg, MSG_SIZE, 0, 0) < 0)           error("msgrcv");
        recognize_and_proceed(&msg);
    }

    return 0;
}
