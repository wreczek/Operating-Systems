#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

int arg;
pid_t pid;
int waiting = 0;

void error(const char * msg){
	perror(msg);
	exit(EXIT_FAILURE);
}

void sigintFunction(int signum){
	printf("Odebrano sygnal SIGINT\n");
	exit(EXIT_SUCCESS);
}

void sigtstpAction(int signum){
    if (waiting == 1){
        waiting = 0;
    }
    else if (waiting == 0){
        if (arg == 2){
            kill(pid, SIGKILL);
        }
        printf("Oczekuje na CTRL+Z - kontynuacja albo CTR+C - zakończenie programu\n");
        waiting = 1;
        sigset_t mask;
        sigfillset(&mask);
        sigdelset(&mask, SIGINT);
        sigdelset(&mask, SIGTSTP);
        sigsuspend(&mask);
    }
    else error("bad waiting");
}

void firstTask() {
    if (signal(SIGINT, sigintFunction) == SIG_ERR){
        error("sigint signal");
    }

    struct sigaction action;
    action.sa_handler = sigtstpAction;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGTSTP, &action, NULL) != 0){
        error("sigaction");
    }

	while(1) {
		system("date");
		sleep(1);
	}
	exit(EXIT_SUCCESS);
}

void secondTask(){
    if (signal(SIGINT, sigintFunction) == SIG_ERR){
        error("second sigint");
    }

    struct sigaction action;
    action.sa_handler = sigtstpAction;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGTSTP, &action, NULL) == -1){
        error("second sigtstp");
    }

    while(1){
        pid = fork();
        if (pid < 0){
            error("fork()");
        }

        if (pid == 0) {
            execlp("./date.sh", "date.sh", NULL);
            error("execlp");
        }
        else {
            sigset_t mask;
            sigfillset(&mask);
            sigdelset(&mask, SIGINT);
            sigdelset(&mask, SIGTSTP);
            sigsuspend(&mask);
        }
    }
	exit(EXIT_SUCCESS);
}

int main(int argc, char * argv[]){
    if (argc != 2) error("Bad arg number");

    if (strcmp(argv[1], "first") == 0) {
        arg = 1;
        firstTask();
    }
    else if (strcmp(argv[1], "second") == 0){
        arg = 2;
        secondTask();
    }
    else{
        error("Bad arg");
    }

    exit(EXIT_SUCCESS);
}

/// w 2gim modyfikujemy 3ci zestaw i git
/// w 3cim zadaniu warto przechwycic CTRL + C
/// za 2 tygodnie kolos tj. 12.04.2019
/// 5 zestaw trzeba zrobic na czas
