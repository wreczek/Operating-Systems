#include "lib.h"

int send_message(int socket_fd, const char message[RAW_MESSAGE_SIZE]) {
    return write(socket_fd, message, RAW_MESSAGE_SIZE);
}

int receive_message(int socket_fd, char message[RAW_MESSAGE_SIZE]) {
    int result = read(socket_fd, message, RAW_MESSAGE_SIZE);
    return result;
}

int check_message_type(const char message[RAW_MESSAGE_SIZE]) {
    int type;
    sscanf(message, "%d", &type);
    return type;
}

int send_login_message(int socket_fd, const char * string) {
    char buffer[RAW_MESSAGE_SIZE];
    sprintf(buffer, "%d %s", MSG_LOGIN, string);
    return send_message(socket_fd, buffer);
}

int send_task_message(int socket_fd, int task_id, int task_type, int operand1, int operand2, int result) {
    char buffer[RAW_MESSAGE_SIZE];
    sprintf(buffer, "%d %d %d %d %d %d", MSG_TASK, task_id, task_type, operand1, operand2, result);
    return send_message(socket_fd, buffer);
}

int send_ping_message(int socket_fd, int ping_id) {
    char buffer[RAW_MESSAGE_SIZE];
    sprintf(buffer, "%d %d", MSG_PING, ping_id);
    return send_message(socket_fd, buffer);
}

struct login_message get_login_message(const char raw_message[RAW_MESSAGE_SIZE]) {
    int tmp;
    struct login_message lm;
    sscanf(raw_message, "%d %s", &tmp, lm.string);
    return lm;
}

struct task_message get_task_message(const char raw_message[RAW_MESSAGE_SIZE]) {
    int tmp;
    struct task_message tm;
    sscanf(raw_message, "%d %d %d %d %d %d", &tmp,
           &tm.task_id, &tm.task_type, &tm.operand1,
           &tm.operand2, &tm.result
    );
    return tm;
}

struct ping_message get_ping_message(const char raw_message[RAW_MESSAGE_SIZE]) {
    int tmp;
    struct ping_message pm;
    sscanf(raw_message, "%d %d", &tmp, &pm.ping_id);
    return pm;
}
