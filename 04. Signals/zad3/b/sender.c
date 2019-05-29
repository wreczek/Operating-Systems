#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

volatile pid_t catchersPid;
volatile int signalsSent;
volatile int signalsReceived = 0;
volatile int msgs[2000];
volatile int counter = 0;


void error(char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

/// ------------- KILL --------------
void killHandler(int signum){
    signalsReceived++;
}

void killMode(){
    int i;

    struct sigaction action;
    action.sa_handler = killHandler;
    sigfillset(&action.sa_mask);
    sigdelset(&action.sa_mask, SIGUSR1);
    action.sa_flags = 0;
    sigaction(SIGUSR1, &action, NULL);

    for (i = 0; i < signalsSent; ++i){
        kill(catchersPid, SIGUSR1);
        sigsuspend(&action.sa_mask);
    }

    kill(catchersPid, SIGUSR2);
    sigsuspend(&action.sa_mask);
}

/// --------- QUEUE ----------
void queueHandlerSA(int signum, siginfo_t * info, void * context){
    signalsReceived++;
    msgs[counter++] = info->si_value.sival_int;
}

void queueMode(){
    int i;

    struct sigaction action;
    action.sa_sigaction = queueHandlerSA;
    sigfillset(&action.sa_mask);
    sigdelset(&action.sa_mask, SIGUSR1);
    action.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &action, NULL);

    union sigval value;

    for (i = 0; i < signalsSent; ++i){
        sigqueue(catchersPid, SIGUSR1, value);
        sigsuspend(&action.sa_mask);
    }

    sigqueue(catchersPid, SIGUSR2, value);
    sigsuspend(&action.sa_mask);
}

/// -------- REAL TIME ---------
void rtHandler(int signum){
    signalsReceived++;
}

void rtMode(){
    int i;

    struct sigaction action;
    action.sa_handler = killHandler;
    sigfillset(&action.sa_mask);
    sigdelset(&action.sa_mask, SIGRTMIN);
    action.sa_flags = 0;
    sigaction(SIGRTMIN, &action, NULL);

    for (i = 0; i < signalsSent; ++i){
        kill(catchersPid, SIGRTMIN);
        sigsuspend(&action.sa_mask);
    }

    kill(catchersPid, SIGRTMAX);
    sigsuspend(&action.sa_mask);
}

/// ------------- MAIN ----------------
int main(int argc, char ** argv){
    /// argv[1] = PID catchera
    /// argv[2] = how many signals to send
    /// argv[3] = mode of signal sending

    if (argc != 4) error("Bad args number");
    catchersPid = atoi(argv[1]);
    signalsSent = atoi(argv[2]);
    char * mode = argv[3];

    if (strcmp(mode, "KILL") == 0){
        killMode();
    }
    else if (strcmp(mode, "SIGQUEUE") == 0){
        queueMode();
        int i;
        for (i = 0; i < counter; ++i){
            printf("Received %dth signal number %d.\n", i, msgs[i]);
        }
    }
    else if (strcmp(mode, "SIGRT") == 0){
        rtMode();
    }
    else error("Bad third arg");

    printf("Sent %d signals to %d and received %d.\n", signalsSent, catchersPid, signalsReceived);

    return 0;
}
