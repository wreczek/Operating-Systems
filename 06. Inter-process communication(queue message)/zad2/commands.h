#ifndef commands_h
#define commands_h

#define MAX_CLIENTS  10
#define PROJECT_ID 0x099
#define MAX_MSG_SIZE 4096
#define MSG_SIZE sizeof(MyMsg)
#define MAX_MSG_Q_SZ 9

const char delims[3] = {' ', '\n', '\t'};

int error(char * msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

typedef enum mtype {
    LOGIN = 1, STOP = 2, LIST = 3, ECHO = 4,
    TOALL = 5, TOONE = 6
} mtype;

typedef struct MyMsg{
    long mtype;
    int cqid;
    pid_t pid;
    char mtext[MAX_MSG_SIZE];
} MyMsg;

const char server_path[] = "/server";

#endif
