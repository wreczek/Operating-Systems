#ifndef LIB_H
#define LIB_G

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>
#include <poll.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <time.h>

// general settings:
#define SOCKET_PROTOCOL         SOCK_STREAM

// server settings:
#define SERVER_NAME             "cluster_server"
#define SERVER_CLIENTS_LIMIT    16
#define PING_INTERVAL           5
#define PING_TIMEOUT            5

// client settings:
#define CLIENT_MAX_NAME_LENGTH  32

// socket modes:
#define MODE_INET               1
#define MODE_UNIX               2

// task types:
#define ALL_WORDS                1
#define ONE_WORD                 2

// client representation
struct client {
    char name[CLIENT_MAX_NAME_LENGTH + 1];
    int socket_fd;
    int client_id;
    int registered;
    int active;
};

// message settings:
#define RAW_MESSAGE_SIZE 64
#define CONTENT_SIZE 1024

// message types:
#define MSG_LOGIN       1  // format: "%d %s" -> msg_type, message
#define MSG_TASK        2  // format: "%d %s" -> msg_type, task_id, path
#define MSG_PING        3  // format: "%d %d" -> msg_type

struct login_message {
    char string[CLIENT_MAX_NAME_LENGTH];
};

struct task_message {
    int task_id;
    int task_type;
    char path_or_content[CONTENT_SIZE];
};

struct ping_message {
    int ping_id;
};

void error(char * msg);
// lib API:
int send_message(int socket_fd, const char message[RAW_MESSAGE_SIZE]);
int receive_message(int socket_fd, char message[RAW_MESSAGE_SIZE]);
int check_message_type(const char message[RAW_MESSAGE_SIZE]);

int send_login_message(int socket_fd, const char * string);
int send_task_message(int socket_fd, int task_id, int task_type, char * path_or_content);
int send_ping_message(int socket_fd, int ping_id);

struct login_message get_login_message(const char raw_message[RAW_MESSAGE_SIZE]);
struct task_message get_task_message(const char raw_message[RAW_MESSAGE_SIZE]);
struct ping_message get_ping_message(const char raw_message[RAW_MESSAGE_SIZE]);

#endif
