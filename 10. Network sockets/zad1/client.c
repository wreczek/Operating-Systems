#include "lib.h"

char * client_name;
short socket_mode;
int server_socket_fd = -1;

char * server_IP_address;
in_port_t server_inet_port;
char * server_unix_socket_path_name;

void error(char * msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

void sig_int(int signum) {
    printf("got SIGINT - closing the client\n");
    if(server_socket_fd > 0)
        send_login_message(server_socket_fd, "logout");
    exit(EXIT_SUCCESS);
}

void print_usage() {
    printf("example usage: ./client <name> <socket_mode> <serwer_address>\n"
        "name: client's name (string)\n"
        "socket_mode: [ unix | inet ]\n"
        "server_address:\n"
        "\tif socket_mode == unix -> <serwer_socket_path>\n"
        "\tif socket_mode == inet -> <serwer_IPv4_address> <port>\n"
    );
}

void connect_to_server(){
    if(socket_mode == MODE_UNIX){
        // laczymy do serwera, UNIX socket
        struct sockaddr_un sa;
        strcpy(sa.sun_path, server_unix_socket_path_name);
        sa.sun_family = AF_UNIX;

        if((server_socket_fd = socket(AF_UNIX, SOCKET_PROTOCOL, 0)) == -1)
            error("socket");

        if(connect(server_socket_fd, (struct sockaddr*) &sa, sizeof(sa)) == -1)
            error("connect");

        printf("sent request to server's UNIX socket\n");
    }
    else{
        // lacyzmz do serwera, INET socket
        struct sockaddr_in sa_inet;
        sa_inet.sin_family = AF_INET;
        sa_inet.sin_addr.s_addr = inet_addr(server_IP_address);
        sa_inet.sin_port = htons(server_inet_port);

        if((server_socket_fd = socket(AF_INET, SOCKET_PROTOCOL, 0)) == -1)
            error("socket");

        if(connect(server_socket_fd, (struct sockaddr*) &sa_inet, sizeof(sa_inet)) == -1)
            error("connect");

        printf("sent request to server's INET socket\n");
    }
}

// odczytaj wejscie socketa i przetworz wiadomosc do odczytu
void process_message() {
    char buffer[RAW_MESSAGE_SIZE];
    receive_message(server_socket_fd, buffer);

    int msg_type = check_message_type(buffer);
    if(msg_type == MSG_PING) {
        struct ping_message pm = get_ping_message(buffer);
        printf("got ping request, id: %d\n", pm.ping_id);
        send_ping_message(server_socket_fd, pm.ping_id);
    }
    else if(msg_type == MSG_TASK) { /// TODO: tu bedzie liczenie slow
        struct task_message tm = get_task_message(buffer);
        printf("got task, id: %d, type: %d, operands: %d, %d\n", tm.task_id, tm.task_type, tm.operand1, tm.operand2);

        if(tm.task_type == TASK_ADD)
            tm.result = tm.operand1 + tm.operand2;
        else if(tm.task_type == TASK_SUB)
            tm.result = tm.operand1 - tm.operand2;
        else if(tm.task_type == TASK_MUL)
            tm.result = tm.operand1 * tm.operand2;
        else if(tm.task_type == TASK_DIV)
            tm.result = tm.operand1 / tm.operand2;

        send_task_message(server_socket_fd, tm.task_id, tm.task_type, tm.operand1, tm.operand2, tm.result);
    }
}

int main(int argc, char **argv) {
    struct sigaction act_int;
    act_int.sa_handler = sig_int;
    sigemptyset(&act_int.sa_mask);
    act_int.sa_flags = 0;

    // Setting handler for SIGINT
    if ((sigaction(SIGINT, &act_int, NULL)) < 0) error("sigaction");

    client_name = argv[1];

    if(strcmp(argv[2], "unix") == 0) {
        if(argc != 4) {
            error("bad args number");
            print_usage();
        }
        socket_mode = MODE_UNIX;
        server_unix_socket_path_name = argv[3];
    }
    else if(strcmp(argv[2], "inet") == 0) {
        if(argc != 5) {
            error("bad args number");
            print_usage();
        }
        socket_mode = MODE_INET;
        server_IP_address = argv[3];
        server_inet_port = (in_port_t) atoi(argv[4]);
    }
    else {
        print_usage();
        error("bad args number");
    }

    connect_to_server();

    char message_buffer[RAW_MESSAGE_SIZE];
    receive_message(server_socket_fd, message_buffer);
    struct login_message lm = get_login_message(message_buffer);

    if(strcmp(lm.string, "pending") == 0)
        printf("connection established\n");
    else {
        error("server rejected connection\n");
    }

    send_login_message(server_socket_fd, client_name);

    receive_message(server_socket_fd, message_buffer);
    lm = get_login_message(message_buffer);

    if(strcmp(lm.string, "accepted") == 0)
        printf("logged in successfully\n");
    else
        error("login rejected");

    while(1) {
        process_message();
    }
}
