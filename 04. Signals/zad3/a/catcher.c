#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

volatile pid_t sendersPid;
volatile int signalsReceived = 0;

void error(char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

void sigusrHandler(int signum){
    if (signum == SIGUSR1){
        signalsReceived++;

        struct sigaction action;

        sigfillset(&action.sa_mask);
        sigdelset(&action.sa_mask, SIGUSR1);
        sigdelset(&action.sa_mask, SIGUSR2);

        action.sa_flags = 0;
        action.sa_handler = sigusrHandler;

        sigsuspend(&action.sa_mask);
    }
    else if (signum == SIGUSR2){
        sigset_t mask;
        sigfillset(&mask);
        sigprocmask(SIG_SETMASK, &mask, NULL);
    }
    else error("Bad signal");
}

void sigusrHandlerSA(int signum, siginfo_t * info, void * context){
    if (signum == SIGUSR1){
        sendersPid = info->si_pid;
        signalsReceived++;

        struct sigaction action;

        sigfillset(&action.sa_mask);
        sigdelset(&action.sa_mask, SIGUSR1);
        sigdelset(&action.sa_mask, SIGUSR2);

        action.sa_flags = 0;
        action.sa_handler= sigusrHandler;

        if (sigaction(SIGUSR1, &action, NULL) != 0) error("sigaction1 in self handler");
        if (sigaction(SIGUSR2, &action, NULL) != 0) error("sigaction1 in self handler");

        sigsuspend(&action.sa_mask);
    }
    else if (signum == SIGUSR2){
        sendersPid = info->si_pid;
        sigset_t mask;
        sigfillset(&mask);
        sigprocmask(SIG_SETMASK, &mask, NULL);
    }
    else error("Bad signal");
}

/// ---------- REAL TIME ------------
void sigRTHandler(int signum){
    if (signum == SIGRTMIN){
        signalsReceived++;

        struct sigaction action;

        sigfillset(&action.sa_mask);
        sigdelset(&action.sa_mask, SIGRTMIN);
        sigdelset(&action.sa_mask, SIGRTMAX);

        action.sa_flags = 0;
        action.sa_handler= sigRTHandler;

        //if (sigaction(SIGRTMIN, &action, NULL) != 0) error("sigaction1 in self handler");
        //if (sigaction(SIGRTMAX, &action, NULL) != 0) error("sigaction1 in self handler");

        sigsuspend(&action.sa_mask);
    }
    else if (signum == SIGRTMAX){
        sigset_t mask;
        sigfillset(&mask);
        sigprocmask(SIG_SETMASK, &mask, NULL);
    }
    else error("Bad signal");
}

void sigRTHandlerSA(int signum, siginfo_t * info, void * context){
     if (signum == SIGRTMIN){
        sendersPid = info->si_pid;
        signalsReceived++;

        struct sigaction action;

        sigfillset(&action.sa_mask);
        sigdelset(&action.sa_mask, SIGRTMIN);
        sigdelset(&action.sa_mask, SIGRTMAX);

        action.sa_flags = 0;
        action.sa_handler= sigRTHandler;

        if (sigaction(SIGRTMIN, &action, NULL) != 0) error("sigaction1 in self handler");
        if (sigaction(SIGRTMAX, &action, NULL) != 0) error("sigaction1 in self handler");

        sigsuspend(&action.sa_mask);
    }
    else if (signum == SIGRTMAX){
        sendersPid = info->si_pid;
        sigset_t mask;
        sigfillset(&mask);
        sigprocmask(SIG_SETMASK, &mask, NULL);
    }
    else error("Bad signal");
}

/// ----------- MODES -------------

void killMode(){
    int i;
    for (i = 0; i < signalsReceived; ++i){
        kill(sendersPid, SIGUSR1);
    }
    sleep(0.2);
    kill(sendersPid, SIGUSR2);
    kill(sendersPid, SIGUSR2);
    kill(sendersPid, SIGUSR2);
}

void queueMode(){
    int i;
    union sigval value;
    for (i = 0; i < signalsReceived; ++i){
        value.sival_int = i;
        sigqueue(sendersPid, SIGUSR1, value);
    }
    sleep(0.2);
    kill(sendersPid, SIGUSR2);
    kill(sendersPid, SIGUSR2);
    kill(sendersPid, SIGUSR2);
}

void rtMode(){
    int i;
    sleep(1);
    for (i = 0; i < signalsReceived; ++i){
        kill(sendersPid, SIGRTMIN);
    }
    sleep(1);

    kill(sendersPid, SIGRTMAX);
    kill(sendersPid, SIGRTMAX);
}

/// ---------- RECEIVE -----------
void receiveSignals(){
    printf("My pid is %d.\n", getpid());

    struct sigaction action;

    sigfillset(&action.sa_mask);
    sigdelset(&action.sa_mask, SIGUSR1);
    sigdelset(&action.sa_mask, SIGUSR2);

    action.sa_flags |= SA_SIGINFO;
    action.sa_sigaction = sigusrHandlerSA;

    if (sigaction(SIGUSR1, &action, NULL) != 0) error("sigaction1 in receiver");
    if (sigaction(SIGUSR2, &action, NULL) != 0) error("sigaction2 in receiver");

    pause();
}

void receiveRTSignals(){
    printf("My pid is %d.\n", getpid());

    struct sigaction action;

    sigfillset(&action.sa_mask);
    sigdelset(&action.sa_mask, SIGRTMIN);
    sigdelset(&action.sa_mask, SIGRTMAX);

    action.sa_flags |= SA_SIGINFO;
    action.sa_sigaction = sigRTHandlerSA;

    if (sigaction(SIGRTMIN, &action, NULL) != 0) error("sigaction1 in receiver");
    if (sigaction(SIGRTMAX, &action, NULL) != 0) error("sigaction2 in receiver");

    pause();
}

int main(int argc, char ** argv){
    if (argc != 2) error("Bad args number");
    char * mode = argv[1];

    if (strcmp(mode, "KILL") == 0){
        receiveSignals();
        killMode();
        printf("Got %d SIGUSR1 signals from %d.\n", signalsReceived, sendersPid);
    }
    else if (strcmp(mode, "SIGQUEUE") == 0){
        receiveSignals();
        queueMode();
        printf("Got %d SIGUSR1 signals from %d.\n", signalsReceived, sendersPid);
    }
    else if (strcmp(mode, "SIGRT") == 0){
        receiveRTSignals();
        rtMode();
        printf("Got %d SIGRTMIN signals from %d.\n", signalsReceived, sendersPid);
    }
    else error("Bad first arg");

    return 0;
}
