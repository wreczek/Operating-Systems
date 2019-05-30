#include "lib.h"

// Threads functions declarations:
void * connection_manager_thread(void * arg);
void * ping_manager_thread(void * arg);
void * input_manager_thread(void * arg);

// Important variables:
in_port_t inet_port;
char * unix_socket_path_name;

// Sockets through which the server is accepting new client's connections.
int server_unix_socket_fd = -1;
int server_inet_socket_fd = -1;

// Array of clients that are currently connected to the server.
struct client registered_clients[SERVER_CLIENTS_LIMIT];
int registered_clients_number;
pthread_mutex_t mut_clients = PTHREAD_MUTEX_INITIALIZER;

// Initializes unix and inet sockets.
void open_server() {
    // UNIX socket setup:
    struct sockaddr_un sa_unix;
    strcpy(sa_unix.sun_path, unix_socket_path_name);
    sa_unix.sun_family = AF_UNIX;

    if ((server_unix_socket_fd = socket(AF_UNIX, SOCKET_PROTOCOL, 0)) == -1)                error("cannot create unix socket");
    if ((bind(server_unix_socket_fd, (struct sockaddr*) &sa_unix, sizeof(sa_unix))) == -1)  error("cannot bind unix socket");
    if ((listen(server_unix_socket_fd, SERVER_CLIENTS_LIMIT)) == -1)                        error("cannot listen on unix socket");

    printf("opened UNIX socket, path: %s\n", unix_socket_path_name);

    // INET socket setup:
    struct hostent * host_entry = gethostbyname("localhost");
    struct in_addr host_address = *(struct in_addr*) host_entry->h_addr;

    struct sockaddr_in sa_inet;
    sa_inet.sin_family = AF_INET;
    sa_inet.sin_addr.s_addr = host_address.s_addr;
    sa_inet.sin_port = htons(inet_port);

    if ((server_inet_socket_fd = socket(AF_INET, SOCKET_PROTOCOL, 0)) == -1)               error("cannot create inet socket");
    if ((bind(server_inet_socket_fd, (struct sockaddr*) &sa_inet, sizeof(sa_inet))) == -1) error("cannot bind inet socket");
    if ((listen(server_inet_socket_fd, SERVER_CLIENTS_LIMIT)) == -1)                       error("cannot listen on inet socket");

    printf("opened INET socket, address: %s : %d\n", inet_ntoa(host_address), inet_port);
}

// Close server's sockets and perform system cleanup
void close_server() {
    if(server_unix_socket_fd != -1) {
        if ((close(server_unix_socket_fd)) == -1)       error("cannot close unix socket");
        if ((unlink(unix_socket_path_name)) == -1)      error("cannot unlink unix socket file");
    }

    if(server_inet_socket_fd != -1) {
        if ((close(server_inet_socket_fd)) == -1)       error("cannot close inet socket");
    }
}

int register_unix_socket_client() {
    if(registered_clients_number >= SERVER_CLIENTS_LIMIT)
        return -1;

    // TODO: register new client

    return 0;
}

void print_usage() {
    printf("example usage: ./server <TCP_port> <UNIX_socket_path>\n");
}

static void exit_fun() {
    close_server();
}

void sig_int(int signum) {
    printf("SIGINT received - closing server\n");
    exit(EXIT_SUCCESS);
}

void sig_pipe(int signum) {
    printf("SIGPIPE received - probably client down\n");
}

// ==================== MAIN =======================

int main(int argc, char **argv) {
    for(int i = 0; i < SERVER_CLIENTS_LIMIT; ++i) {
        registered_clients[i].client_id = i;
        registered_clients[i].socket_fd = -1;
        registered_clients[i].registered = 0;
        registered_clients[i].active = 0;
        registered_clients[i].name[0] = '\0';
    }

    if (atexit(exit_fun) == -1)                      error("cannot set atexit function");

    struct sigaction act_int;
    act_int.sa_handler = sig_int;
    sigemptyset(&act_int.sa_mask);
    act_int.sa_flags = 0;
    if ((sigaction(SIGINT, &act_int, NULL)) == -1)   error("cannot set SIGINT signal handler");

    struct sigaction act_pipe;
    act_pipe.sa_handler = sig_pipe;
    sigemptyset(&act_pipe.sa_mask);
    act_pipe.sa_flags = 0;
    if ((sigaction(SIGPIPE, &act_pipe, NULL)) == -1) error("cannot set SIGPIPE signal handler");

    if(argc != 3) {
        print_usage();
        error("bad args number");
    }

    inet_port = (in_port_t) atoi(argv[1]);
    unix_socket_path_name = argv[2];

    open_server();

    pthread_t connection_manager;
    pthread_create(&connection_manager, NULL, connection_manager_thread, NULL);

    pthread_t ping_manager;
    pthread_create(&ping_manager, NULL, ping_manager_thread, NULL);

    pthread_t input_manager;
    pthread_create(&input_manager, NULL, input_manager_thread, NULL);

    pthread_join(connection_manager, NULL);
    pthread_join(ping_manager, NULL);
    pthread_join(input_manager, NULL);

    return 0;
}


// =========================================================================================================
// ===================================== THREADS FUNCTIONS =================================================
// =========================================================================================================

// HELPER - iterates through array of pollfd structures and finds first descriptor that is ready to read from.
int get_input_ready_fd(struct pollfd sockets[], int sockets_length) {
    for(int i = 0; i < sockets_length; i++)
        if(((sockets[i].revents) & POLLIN) != 0)
            return sockets[i].fd;

    return -1; // TODO: losuje
}

// ================================= MANAGING INCOMING MESSAGES ============================================

// Removes client from registered_clients array
int remove_client(int client_fd, int lock) {
    if(lock == 1)
        pthread_mutex_lock(&mut_clients);

    for(int i = 0; i < SERVER_CLIENTS_LIMIT; ++i)
        if(registered_clients[i].socket_fd == client_fd) {
            printf("removing client from slot %d\n", registered_clients[i].client_id);
            registered_clients[i].socket_fd = -1;
            registered_clients[i].registered = 0;
            registered_clients[i].name[0] = '\0';
            close(client_fd);

            if(lock == 1)
                pthread_mutex_unlock(&mut_clients);

            return 1;
        }

    if(lock == 1)
        pthread_mutex_unlock(&mut_clients);

    return 0;
}

// Registers new client
int register_client(int client_fd, const char * name) {
    pthread_mutex_lock(&mut_clients);

    for(int i = 0; i < SERVER_CLIENTS_LIMIT; ++i){
        if(strcmp(name, registered_clients[i].name) == 0) {
            printf("client's name %s already in use\n", name);

            send_login_message(client_fd, "rejected");
            remove_client(client_fd, 0); // potrzebne? tak jest w tresci?
            pthread_mutex_unlock(&mut_clients);
            return 0;
        }
    }
    for(int i = 0; i < SERVER_CLIENTS_LIMIT; ++i){
        if(registered_clients[i].socket_fd == client_fd) {
            strcpy(registered_clients[i].name, name);
            registered_clients[i].registered = 1;
            registered_clients[i].active = 1;

            printf("client's name %s successfully registered\n", name);

            send_login_message(client_fd, "accepted");
            pthread_mutex_unlock(&mut_clients);
            return 1;
        }
    }
    pthread_mutex_unlock(&mut_clients);
    return -1;
}

// Sets active field in proper client struct as a response to ping message.
int set_active(int client_fd) {
    pthread_mutex_lock(&mut_clients);

    for(int i = 0; i < SERVER_CLIENTS_LIMIT; ++i){
        if(registered_clients[i].socket_fd == client_fd) {
            registered_clients[i].active = 1;
            pthread_mutex_unlock(&mut_clients);
            return 1;
        }
    }
    pthread_mutex_unlock(&mut_clients);
    return 0;
}

void process_message(int client_fd, const char message[RAW_MESSAGE_SIZE]) {
    int msg_type = check_message_type(message);
    if (msg_type == MSG_LOGIN) {
        struct login_message lm = get_login_message(message);
        if(strcmp(lm.string, "logout") == 0)
            remove_client(client_fd, 1);
        else
            register_client(client_fd, lm.string);
    }
    else if(msg_type == MSG_PING) {
        struct ping_message pm = get_ping_message(message);
        printf("ping response, id: %d\n", pm.ping_id);
        set_active(client_fd);
    }
    else if(msg_type == MSG_TASK) {
        struct task_message tm = get_task_message(message);
        printf("result for task %d is: %s\n", tm.task_id, tm.path_or_content);
    }
}

// ================================== MANAGING OPENED SOCKETS ===========================================

// Function run by thread responsible for establishing new connections and receiving messages.
void * connection_manager_thread(void * arg) {
    struct pollfd sockets[2 + SERVER_CLIENTS_LIMIT];

    sockets[SERVER_CLIENTS_LIMIT + 1].fd = server_inet_socket_fd;
    sockets[SERVER_CLIENTS_LIMIT + 1].events = POLLIN;

    sockets[SERVER_CLIENTS_LIMIT].fd = server_unix_socket_fd;
    sockets[SERVER_CLIENTS_LIMIT].events = POLLIN;

    while(1) {
        for(int i = 0; i < SERVER_CLIENTS_LIMIT; ++i) {
            sockets[i].fd = registered_clients[i].socket_fd;
            sockets[i].events = POLLIN;
            sockets[i].revents = 0;
        }

        for(int i = 0; i < SERVER_CLIENTS_LIMIT + 2; ++i)
            sockets[i].revents = 0;

        poll(sockets, SERVER_CLIENTS_LIMIT + 2, -1);

        for(int i = 0; i < SERVER_CLIENTS_LIMIT; i++){
            if(sockets[i].revents & POLLHUP){
                remove_client(sockets[i].fd, 1);
                sockets[i].revents = 0;
            }
        }

        int socket = get_input_ready_fd(sockets, SERVER_CLIENTS_LIMIT + 2);

        if(socket == -1)
            continue;

        if(socket == server_unix_socket_fd)
            printf("new connection request on UNIX socket\n");
        else if(socket == server_inet_socket_fd)
            printf("new connection request on INET socket\n");
        else {
            char msg_buffer[RAW_MESSAGE_SIZE];
            receive_message(socket, msg_buffer);
            process_message(socket, msg_buffer);
            continue;
        }

        pthread_mutex_lock(&mut_clients);

        int empty_slot = -1;
        for(int i = 0; i < SERVER_CLIENTS_LIMIT; ++i){
            if(registered_clients[i].socket_fd == -1) {
                empty_slot = i;
                break;
            }
        }

        int client_fd = accept(socket, NULL, 0);
        if(empty_slot == -1) {
            printf("no empty slots found - rejecting connection\n");
            send_login_message(client_fd, "rejected");
            close(client_fd);
        }
        else {
            printf("new connection saved to slot %d\n", empty_slot);
            registered_clients[empty_slot].socket_fd = client_fd;
            send_login_message(client_fd, "pending");
        }

        pthread_mutex_unlock(&mut_clients);
    }

    return (void*) 0;
}

// ====================================== CLIENTS PINGING ===============================================

void * ping_manager_thread(void * arg) {
    while(1) {
        sleep(PING_INTERVAL);
        pthread_mutex_lock(&mut_clients);

        for (int i = 0; i < SERVER_CLIENTS_LIMIT; ++i){
            if(registered_clients[i].registered == 1) {
                registered_clients[i].active = 0;
                send_ping_message(registered_clients[i].socket_fd, i);
            }
        }

        pthread_mutex_unlock(&mut_clients);

        printf("waiting for responses\n");
        sleep(PING_TIMEOUT);

        pthread_mutex_lock(&mut_clients);

        for(int i = 0; i < SERVER_CLIENTS_LIMIT; ++i){
            if(registered_clients[i].registered == 1 && registered_clients[i].active == 0) {
                printf("client at slot %d did not respond to ping message (removing client)\n", i);
                registered_clients[i].registered = 0;
                registered_clients[i].name[0] = '\0';
                close(registered_clients[i].socket_fd);
                registered_clients[i].socket_fd = -1;
            }
        }
        pthread_mutex_unlock(&mut_clients);
    }

    return (void*) 0;
}

// ==================================== HANDLING USER INPUT ============================================

int get_next_registered_client(int starting_point) {
    pthread_mutex_lock(&mut_clients);
    int iter = (starting_point + 1) % SERVER_CLIENTS_LIMIT;

    for(int i = 0; i < SERVER_CLIENTS_LIMIT - 1; ++i) {
        if(registered_clients[iter].registered == 1) {
            pthread_mutex_unlock(&mut_clients);
            return iter;
        }
        iter = (iter + 1) % SERVER_CLIENTS_LIMIT;
    }

    pthread_mutex_unlock(&mut_clients);
    return -1;
}

// Processes string that contains calc request
int get_calc_task(char * message, struct task_message * task) {
    char * token_ptr;

    token_ptr = strtok(message, " \t\n");

    if(token_ptr == NULL)
        return -1;

    strcpy(task->path_or_content, token_ptr);

    token_ptr = strtok(NULL, " \n\t");

    if(token_ptr == NULL)
        return -1;

    task->task_type = atoi(token_ptr);

    if(strtok(NULL, " \n") != NULL)
        return -1;

    return 0;
}

// Reads input on server's terminal and sends tasks to clients.
void * input_manager_thread(void * arg) {
    int previous_client = -1;
    int task_id_counter = 0;

    char * buffer = NULL;
    size_t buffer_size = 0;
    size_t bytes_read;

    while ((bytes_read = getline(&buffer, &buffer_size, stdin)) != -1) {
        struct task_message task;
        if (get_calc_task(buffer, &task) == -1) {
            printf("invalid input (usage: path)\n");
            continue;
        }
        int target_client = get_next_registered_client(previous_client);
        if (target_client != -1)
            send_task_message(registered_clients[target_client].socket_fd,
                task_id_counter++, task.task_type, task.path_or_content);
        else {
            printf("no computing clients available\n");
            continue;
        }

        previous_client = target_client;
    }

    return (void*) 0;
}
