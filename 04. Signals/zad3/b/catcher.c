#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

volatile pid_t sendersPid;
volatile int signalsReceived = 0;
volatile int whichSent = 0;

void error(char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

/// --------- KILL ----------
void killHandler(int signum){
    if (signum == SIGUSR1){
        signalsReceived++;

        sigset_t mask;
        sigfillset(&mask);
        sigdelset(&mask, SIGUSR1);
        sigdelset(&mask, SIGUSR2);
        kill(sendersPid, SIGUSR1);
        sigsuspend(&mask);

    }
    else if (signum == SIGUSR2){
        kill(sendersPid, SIGUSR1);
    }
    else error("Bad signal in handler");
}

void killHandlerSA(int signum, siginfo_t * info, void * context){
    if (signum == SIGUSR1){
        sendersPid = info->si_pid;
        signalsReceived++;

        struct sigaction action;
        action.sa_handler = killHandler;
        action.sa_flags = 0;
        sigfillset(&action.sa_mask);
        sigdelset(&action.sa_mask, SIGUSR1);
        sigdelset(&action.sa_mask, SIGUSR2);

        sigaction(SIGUSR1, &action, NULL);
        sigaction(SIGUSR2, &action, NULL);

        kill(sendersPid, SIGUSR1);

        sigsuspend(&action.sa_mask);
    }
    else if (signum == SIGUSR2){
        kill(sendersPid, SIGUSR1);
    }
    else error("Bad signal in handler");
}

void killMode(){
    printf("My pid is %d.\n", getpid());

    struct sigaction action;
    action.sa_sigaction = killHandlerSA;
    action.sa_flags |= SA_SIGINFO;
    sigfillset(&action.sa_mask);
    sigdelset(&action.sa_mask, SIGUSR1);
    sigdelset(&action.sa_mask, SIGUSR2);

    sigaction(SIGUSR1, &action, NULL);
    sigaction(SIGUSR2, &action, NULL);

    sigsuspend(&action.sa_mask);
}

/// -------- QUEUE ----------
/**
void queueHandler(int signum){
    if (signum == SIGUSR1){
        signalsReceived++;

        union sigval value;
        value.sival_int = whichSent++;

        sigset_t mask;
        sigfillset(&mask);
        sigdelset(&mask, SIGUSR1);
        sigdelset(&mask, SIGUSR2);
        kill(sendersPid, SIGUSR1);

        sigqueue(sendersPid, SIGUSR1, value);

        sigsuspend(&mask);
    }
    else if (signum == SIGUSR2){
        union sigval value;
        value.sival_int = whichSent;
        sigqueue(sendersPid, SIGUSR1, value);
    }
    else error("Bad signal in handler");
}
*/
void queueHandlerSA(int signum, siginfo_t * info, void * context){ // mozna dodac pozniej zwykly handler
    if (signum == SIGUSR1){
        sendersPid = info->si_pid;
        signalsReceived++;

        union sigval value;
        value.sival_int = whichSent++;

        struct sigaction action;
        action.sa_sigaction = queueHandlerSA;
        action.sa_flags = SA_SIGINFO;
        sigfillset(&action.sa_mask);
        sigdelset(&action.sa_mask, SIGUSR1);
        sigdelset(&action.sa_mask, SIGUSR2);

        sigaction(SIGUSR1, &action, NULL);
        sigaction(SIGUSR2, &action, NULL);

        sigqueue(sendersPid, SIGUSR1, value);

        sigsuspend(&action.sa_mask);
    }
    else if (signum == SIGUSR2){
        union sigval value;
        value.sival_int = whichSent;
        sigqueue(sendersPid, SIGUSR1, value);
    }
    else error("Bad signal in handler");
}

void queueMode(){
    printf("My pid is %d.\n", getpid());

    struct sigaction action;
    action.sa_sigaction = queueHandlerSA;
    action.sa_flags |= SA_SIGINFO;
    sigfillset(&action.sa_mask);
    sigdelset(&action.sa_mask, SIGUSR1);
    sigdelset(&action.sa_mask, SIGUSR2);

    sigaction(SIGUSR1, &action, NULL);
    sigaction(SIGUSR2, &action, NULL);

    sigsuspend(&action.sa_mask);
}

/// --------- REAL TIME -----------
void rtHandler(int signum){

}

void rtHandlerSA(int signum, siginfo_t * info, void * context){
    if (signum == SIGRTMIN){
        sendersPid = info->si_pid;
        signalsReceived++;

        struct sigaction action;

        sigfillset(&action.sa_mask);
        sigdelset(&action.sa_mask, SIGRTMIN);
        sigdelset(&action.sa_mask, SIGRTMAX);

        action.sa_flags = SA_SIGINFO;
        action.sa_sigaction = rtHandlerSA;

        if (sigaction(SIGRTMIN, &action, NULL) != 0) error("sigaction1 in self handler");
        if (sigaction(SIGRTMAX, &action, NULL) != 0) error("sigaction1 in self handler");

        kill(sendersPid, SIGRTMIN);

        sigsuspend(&action.sa_mask);
    }
    else if (signum == SIGRTMAX){
        sendersPid = info->si_pid;
        kill(sendersPid, SIGRTMIN);
    }
    else error("Bad signal");
}

void rtMode(){
    printf("My pid is %d.\n", getpid());

    struct sigaction action;
    action.sa_sigaction = rtHandlerSA;
    action.sa_flags |= SA_SIGINFO;
    sigfillset(&action.sa_mask);
    sigdelset(&action.sa_mask, SIGRTMIN);
    sigdelset(&action.sa_mask, SIGRTMAX);

    sigaction(SIGRTMIN, &action, NULL);
    sigaction(SIGRTMAX, &action, NULL);

    sigsuspend(&action.sa_mask);
}

int main(int argc, char ** argv){
    if (argc != 2) error("Bad args number");
    char * mode = argv[1];

    if (strcmp(mode, "KILL") == 0){
        killMode();
        printf("Got %d SIGUSR1 signals.\n", signalsReceived);
    }
    else if (strcmp(mode, "SIGQUEUE") == 0){
        queueMode();
        printf("Got %d SIGUSR1 signals from %d.\n", signalsReceived, sendersPid);
    }
    else if (strcmp(mode, "SIGRT") == 0){
        rtMode();
        printf("Got %d SIGRTMIN signals from %d.\n", signalsReceived, sendersPid);
    }
    else error("Bad first arg");

    return 0;
}
