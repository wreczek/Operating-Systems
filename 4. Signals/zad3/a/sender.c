#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

volatile pid_t catchersPid;
volatile int signalsSent;
volatile int signalsReceived = 0;
volatile int msgs[100];
volatile int counter = 0;


void error(char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

/// ---------- HANDLERS -------------
void sigusrHandler(int signum){
    sigset_t mask;
    sigfillset(&mask);
    if (signum == SIGUSR1){
        signalsReceived++;
        sigdelset(&mask, SIGUSR1);
        sigdelset(&mask, SIGUSR2);
        sigsuspend(&mask);
    }
    else if (signum == SIGUSR2){
        sigprocmask(SIG_SETMASK, &mask, NULL);
    }
    else error("Bad signalin sigusr handler");
}

void sigusrHandler2(int signum, siginfo_t * info, void * context){
    if (signum == SIGUSR1){
        signalsReceived++;

        struct sigaction action;

        sigfillset(&action.sa_mask);
        sigdelset(&action.sa_mask, SIGUSR1);
        sigdelset(&action.sa_mask, SIGUSR2);

        action.sa_flags |= SA_SIGINFO;
        action.sa_sigaction = sigusrHandler2;

        if (sigaction(SIGUSR1, &action, NULL) != 0) error("sigaction1 in sigusrHandler2");

        msgs[counter++] = info->si_value.sival_int;

        sigsuspend(&action.sa_mask);
    }
    else if (signum == SIGUSR2){
        sigset_t mask;
        sigfillset(&mask);
        sigprocmask(SIG_SETMASK, &mask, NULL);
    }
    else error("Bad signal in sigusr2 handler");
}

void sigRTHandler(int signum){
    sigset_t mask;
    sigfillset(&mask);
    if (signum == SIGRTMIN){
        signalsReceived++;
        sigdelset(&mask, SIGRTMIN);
        sigdelset(&mask, SIGRTMAX);
        sigsuspend(&mask);
    }
    else if (signum == SIGRTMAX){
        sigprocmask(SIG_SETMASK, &mask, NULL);
    }
    else error("Bad signal in sigrt handler");
}


/// ----------- RECEIVE -------------
void receiveSignals(){
    struct sigaction action;

    sigfillset(&action.sa_mask);
    sigdelset(&action.sa_mask, SIGUSR1);
    sigdelset(&action.sa_mask, SIGUSR2);

    action.sa_flags = 0;
    action.sa_handler= sigusrHandler;

    if (sigaction(SIGUSR1, &action, NULL) != 0) error("sigaction1 in receiver");
    if (sigaction(SIGUSR2, &action, NULL) != 0) error("sigaction2 in receiver");

    pause(); 
}

void receiveSignalsQueue(){
    struct sigaction action;

    sigfillset(&action.sa_mask);
    sigdelset(&action.sa_mask, SIGUSR1);
    sigdelset(&action.sa_mask, SIGUSR2);

    action.sa_flags |= SA_SIGINFO;
    action.sa_sigaction = sigusrHandler2;

    if (sigaction(SIGUSR1, &action, NULL) != 0) error("sigaction1 in receiver");
    if (sigaction(SIGUSR2, &action, NULL) != 0) error("sigaction2 in receiver");

    pause();
}

void receiveSignalsRT(){
    struct sigaction action;

    sigfillset(&action.sa_mask);
    sigdelset(&action.sa_mask, SIGRTMIN);
    sigdelset(&action.sa_mask, SIGRTMAX);

    action.sa_flags = 0;
    action.sa_handler= sigRTHandler;

    if (sigaction(SIGRTMIN, &action, NULL) != 0) error("sigaction1 in receiver");
    if (sigaction(SIGRTMAX, &action, NULL) != 0) error("sigaction2 in receiver");

    pause();
}

/// ------------- SEND --------------
void killMode(){
    int i;
    for (i = 0; i < signalsSent; ++i){
        kill(catchersPid, SIGUSR1);
    }
    sleep(0.2);
    kill(catchersPid, SIGUSR2);
    kill(catchersPid, SIGUSR2);

    receiveSignals();
}

void queueMode(){
    int i;
    for (i = 0; i < signalsSent; ++i){
        kill(catchersPid, SIGUSR1);
    }
    sleep(0.2);
    kill(catchersPid, SIGUSR2);
    kill(catchersPid, SIGUSR2);

    receiveSignalsQueue();
}

void rtMode(){
    int i;
    for (i = 0; i < signalsSent; ++i){
        kill(catchersPid, SIGRTMIN);
    }
    sleep(1);
    kill(catchersPid, SIGRTMAX);
    kill(catchersPid, SIGRTMAX);

    receiveSignalsRT();
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
