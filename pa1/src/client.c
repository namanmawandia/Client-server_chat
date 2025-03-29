#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#include <sys/select.h>


#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>

#include <sys/types.h>

#include <sys/socket.h>

#include <sys/stat.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <netdb.h>

#include <stdbool.h>

#include "../include/global.h"

#include "../include/logger.h"

#include "../include/common.h"

// #include "../include/server.h"

#define min(a, b)(((a) < (b)) ? (a) : (b))
#define MSIZE 500
#define MSIZEBACK 500 * 200
#define STDIN 0

//////////////////////////////////////////////////////////////////////////////////////
// Function declarations

// application startup
void client__init();
int c_reg_list();

//command execution
void c_exec_comm(char command[]);

// login client
int c_connect_s(char server_ip[], char server_port[]);
void c_login(char server_ip[], char server_port[]);

// refresh client
void c_refresh_list(char clientListString[]);

//send client
void c_send(char command[]);
void c_receive(char client_ip[], char msg[]);

// BLOCK AND UNBLOCK
void c_block_unblock(char command[], bool is_a_block);

//exit
void c_exit();

void c_p2p_file_transfer(char peer_ip[], char file_name[]);
void c_recv_file_frm_peer(int peer_fd);

///////////////////////////////////////////////////////////////////////////////
// Function Definitions

// execute command function
void execute_command(char command[], int requesting_client_fd) {
    h_exec_command(command, requesting_client_fd);
    if (!localhost -> is_server) {
        c_exec_comm(command);
    }
    fflush(stdout);
}

// Initialization client side
void client__init() {
    // TODO: modularise
    c_reg_list();
    while (true) {
        // handle data from standard input
        char * command = (char * ) malloc(sizeof(char) * MSIZEBACK);
        memset(command, '\0', MSIZEBACK);
        if (fgets(command, MSIZEBACK, stdin) != NULL) {
            execute_command(command, STDIN);
        }
    }
}

// client side listener initialization
int c_reg_list() {
    int listener = 0, status;
    struct addrinfo hints, * localhost_ai, * temp_ai;

    // get a socket and bind it
    memset( & hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (status = getaddrinfo(NULL, localhost -> port_num, & hints, & localhost_ai) != 0) {
        exit(EXIT_FAILURE);
    }

    for (temp_ai = localhost_ai; temp_ai != NULL; temp_ai = temp_ai -> ai_next) {
        listener = socket(temp_ai -> ai_family, temp_ai -> ai_socktype, temp_ai -> ai_protocol);
        if (listener < 0) {
            continue;
        }
        setsockopt(listener, SOL_SOCKET, SO_REUSEPORT, & yes, sizeof(int));
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, & yes, sizeof(int));
        if (bind(listener, temp_ai -> ai_addr, temp_ai -> ai_addrlen) < 0) {
            close(listener);
            continue;
        }
        break;
    }

    // exit if could not bind
    if (temp_ai == NULL) {
        exit(EXIT_FAILURE);
    }

    // listen
    if (listen(listener, 10) == -1) {
        exit(EXIT_FAILURE);
    }

    localhost -> fd = listener;

    freeaddrinfo(localhost_ai);
}

/***  EXECUTE CLIENT COMMANDS ***/
void c_exec_comm(char command[]) {
    if (strstr(command, "LIST") != NULL) {
        if (localhost -> is_logged_in) {
            h_list();
        } else {
            cse4589_print_and_log("[LIST:ERROR]\n");
            cse4589_print_and_log("[LIST:END]\n");
        }
    } else if (strstr(command, "SUCCESSLOGIN") != NULL) {
        cse4589_print_and_log("[LOGIN:SUCCESS]\n");
        cse4589_print_and_log("[LOGIN:END]\n");
    } else if (strstr(command, "ERRORLOGIN") != NULL) {
        cse4589_print_and_log("[LOGIN:ERROR]\n");
        cse4589_print_and_log("[LOGIN:END]\n");
    } else if (strstr(command, "SUCCESSLOGOUT") != NULL) {
        localhost -> is_logged_in = false;
        cse4589_print_and_log("[LOGOUT:SUCCESS]\n");
        cse4589_print_and_log("[LOGOUT:END]\n");
    } else if (strstr(command, "ERRORLOGOUT") != NULL) {
        cse4589_print_and_log("[LOGOUT:ERROR]\n");
        cse4589_print_and_log("[LOGOUT:END]\n");
    } else if (strstr(command, "SUCCESSBROADCAST") != NULL) {
        cse4589_print_and_log("[BROADCAST:SUCCESS]\n");
        cse4589_print_and_log("[BROADCAST:END]\n");
    } else if (strstr(command, "SUCCESSUNBLOCK") != NULL) {
        cse4589_print_and_log("[UNBLOCK:SUCCESS]\n");
        cse4589_print_and_log("[UNBLOCK:END]\n");
    } else if (strstr(command, "SUCCESSBLOCK") != NULL) {
        cse4589_print_and_log("[BLOCK:SUCCESS]\n");
        cse4589_print_and_log("[BLOCK:END]\n");
    } else if (strstr(command, "ERRORUNBLOCK") != NULL) {
        cse4589_print_and_log("[UNBLOCK:ERROR]\n");
        cse4589_print_and_log("[UNBLOCK:END]\n");
    } else if (strstr(command, "ERRORBLOCK") != NULL) {
        cse4589_print_and_log("[BLOCK:ERROR]\n");
        cse4589_print_and_log("[BLOCK:END]\n");
    } else if (strstr(command, "SUCCESSSEND") != NULL) {
        cse4589_print_and_log("[SEND:SUCCESS]\n");
        cse4589_print_and_log("[SEND:END]\n");
    } else if (strstr(command, "LOGIN") != NULL) { // takes two arguments server ip and server port
        char server_ip[MSIZE], server_port[MSIZE];
        int cmdi = 6;
        int ipi = 0;
        while (command[cmdi] != ' ' && ipi < 256) {
            server_ip[ipi] = command[cmdi];
            cmdi += 1;
            ipi += 1;
        }
        server_ip[ipi] = '\0';

        cmdi += 1;
        int pi = 0;
        while (command[cmdi] != '\0') {
            server_port[pi] = command[cmdi];
            cmdi += 1;
            pi += 1;
        }
        server_port[pi - 1] = '\0'; // REMOVE THE NEW LINE
        c_login(server_ip, server_port);
    } else if (strstr(command, "REFRESHRESPONSE") != NULL) {
        c_refresh_list(command);
    } else if (strstr(command, "REFRESH") != NULL) {
        if (localhost -> is_logged_in) {
            h_send_com(server -> fd, "REFRESH\n");
        } else {
            cse4589_print_and_log("[REFRESH:ERROR]\n");
            cse4589_print_and_log("[REFRESH:END]\n");
        }
    } else if (strstr(command, "SENDFILE") != NULL) {
        if (localhost -> is_logged_in) {
            char peer_ip[MSIZE], file_name[MSIZE];
            sscanf(command, "SENDFILE %s %s\n", peer_ip, file_name);
            c_p2p_file_transfer(peer_ip, file_name);
        } else {
            cse4589_print_and_log("[SENDFILE:ERROR]\n");
            cse4589_print_and_log("[SENDFILE:END]\n");
        }
    } else if (strstr(command, "SEND") != NULL) {
        if (localhost -> is_logged_in) {
            c_send(command);
        } else {
            cse4589_print_and_log("[SEND:ERROR]\n");
            cse4589_print_and_log("[SEND:END]\n");
        }
    } else if (strstr(command, "RECEIVE") != NULL) {
        char client_ip[MSIZE], message[MSIZEBACK];
        int cmdi = 8;
        int ipi = 0;
        while (command[cmdi] != ' ' && ipi < 256) {
            client_ip[ipi] = command[cmdi];
            cmdi += 1;
            ipi += 1;
        }
        client_ip[ipi] = '\0';

        cmdi += 1;
        int msgi = 0;
        while (command[cmdi] != '\0') {
            message[msgi] = command[cmdi];
            cmdi += 1;
            msgi += 1;
        }
        message[msgi - 1] = '\0';
        c_receive(client_ip, message);
    } else if (strstr(command, "BROADCAST") != NULL) {
        if (localhost -> is_logged_in) {
            h_send_com(server -> fd, command);
        } else {
            cse4589_print_and_log("[BROADCAST:ERROR]\n");
            cse4589_print_and_log("[BROADCAST:END]\n");
        }
    } else if (strstr(command, "UNBLOCK") != NULL) {
        if (localhost -> is_logged_in) {
            c_block_unblock(command, false);
        } else {
            cse4589_print_and_log("[UNBLOCK:ERROR]\n");
            cse4589_print_and_log("[UNBLOCK:END]\n");
        }
    } else if (strstr(command, "BLOCK") != NULL) {
        if (localhost -> is_logged_in) {
            c_block_unblock(command, true);
        } else {
            cse4589_print_and_log("[BLOCK:ERROR]\n");
            cse4589_print_and_log("[BLOCK:END]\n");
        }
    } else if (strstr(command, "LOGOUT") != NULL) {
        if (localhost -> is_logged_in) {
            h_send_com(server -> fd, command);
        } else {
            cse4589_print_and_log("[LOGOUT:ERROR]\n");
            cse4589_print_and_log("[LOGOUT:END]\n");
        }
    } else if (strstr(command, "EXIT") != NULL) {
        c_exit();
    }
    fflush(stdout);
}

// connection of client and server from client side
int c_connect_s(char server_ip[], char server_port[]) {
    server = malloc(sizeof(struct host));
    memcpy(server -> ip_addr, server_ip, sizeof(server -> ip_addr));
    memcpy(server -> port_num, server_port, sizeof(server -> port_num));
    int server_fd = 0, status;
    struct addrinfo hints, * server_ai, * temp_ai;

    // get a socket and bind it
    memset( & hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (status = getaddrinfo(server -> ip_addr, server -> port_num, & hints, & server_ai) != 0) {
        return 0;
    }

    for (temp_ai = server_ai; temp_ai != NULL; temp_ai = temp_ai -> ai_next) {
        server_fd = socket(temp_ai -> ai_family, temp_ai -> ai_socktype, temp_ai -> ai_protocol);
        if (server_fd < 0) {
            continue;
        }
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, & yes, sizeof(int));
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, & yes, sizeof(int));
        if (connect(server_fd, temp_ai -> ai_addr, temp_ai -> ai_addrlen) < 0) {
            close(server_fd);
            continue;
        }
        break;
    }

    // exit if not able to bind
    if (temp_ai == NULL) {
        return 0;
    }

    server -> fd = server_fd;

    freeaddrinfo(server_ai);

    // Initalisze a listener
    int listener = 0;
    struct addrinfo * localhost_ai;
    if (status = getaddrinfo(NULL, localhost -> port_num, & hints, & localhost_ai) != 0) {
        return 0;
    }

    for (temp_ai = localhost_ai; temp_ai != NULL; temp_ai = temp_ai -> ai_next) {
        listener = socket(temp_ai -> ai_family, temp_ai -> ai_socktype, temp_ai -> ai_protocol);
        if (listener < 0) {
            continue;
        }
        setsockopt(listener, SOL_SOCKET, SO_REUSEPORT, & yes, sizeof(int));
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, & yes, sizeof(int));

        if (bind(listener, temp_ai -> ai_addr, temp_ai -> ai_addrlen) < 0) {
            close(listener);
            continue;
        }
        break;
    }

    // exit if not able to bind
    if (temp_ai == NULL) {
        return 0;
    }

    // listen
    if (listen(listener, 10) == -1) {
        return 0;
    }

    localhost -> fd = listener;

    freeaddrinfo(localhost_ai);

    return 1;
}

// client login to server
void c_login(char server_ip[], char server_port[]) {

    // register the client to server to store its information
    if (server_ip == NULL || server_port == NULL) {
        cse4589_print_and_log("[LOGIN:ERROR]\n");
        cse4589_print_and_log("[LOGIN:END]\n");
        return;
    }
    if (server == NULL) {
        if (!h_valid_ip(server_ip) || c_connect_s(server_ip, server_port) == 0) {
            cse4589_print_and_log("[LOGIN:ERROR]\n");
            cse4589_print_and_log("[LOGIN:END]\n");
            return;
        }
    } else {
        if (strstr(server -> ip_addr, server_ip) == NULL || strstr(server -> port_num, server_port) == NULL) {
            cse4589_print_and_log("[LOGIN:ERROR]\n");
            cse4589_print_and_log("[LOGIN:END]\n");
            return;
        }
    }

    // client send a info msg to server, containg its all the information
    localhost -> is_logged_in = true;

    char msg[MSIZE * 4];
    sprintf(msg, "LOGIN %s %s %s\n", localhost -> ip_addr, localhost -> port_num, localhost -> hostname);
    h_send_com(server -> fd, msg);

    // add server fd to master list
    fd_set master; 
    fd_set read_fds; 
    FD_ZERO( & master);
    FD_ZERO( & read_fds);
    FD_SET(server -> fd, & master); // server_fd added to master list
    FD_SET(STDIN, & master); 
    FD_SET(localhost -> fd, & master);
    int fdmax = server -> fd > STDIN ? server -> fd : STDIN;    
    fdmax = fdmax > localhost -> fd ? fdmax : localhost -> fd;
    
    char data_buffer[MSIZEBACK]; 
    int data_buffer_bytes;
    int fd;
    struct sockaddr_storage new_peer_addr; // client address
    socklen_t addrlen = sizeof new_peer_addr;

    // main loop
    while (localhost -> is_logged_in) {
        read_fds = master; // make a copy of master set
        if (select(fdmax + 1, & read_fds, NULL, NULL, NULL) == -1) {
            exit(EXIT_FAILURE);
        }

        for (fd = 0; fd <= fdmax; fd++) {
            if (FD_ISSET(fd, & read_fds)) {

                if (fd == server -> fd) {
                    // server data
                    data_buffer_bytes = recv(fd, data_buffer, sizeof data_buffer, 0);
                    if (data_buffer_bytes == 0) {
                        close(fd); // Close
                        FD_CLR(fd, & master); // fd removed from master list
                    } else if (data_buffer_bytes == -1) {
                        close(fd); // Close
                        FD_CLR(fd, & master); // fd removed from master list
                    } else {
                        execute_command(data_buffer, fd);
                    }
                } else if (fd == STDIN) {
                    // data from standard input
                    char * command = (char * ) malloc(sizeof(char) * MSIZEBACK);
                    memset(command, '\0', MSIZEBACK);
                    if (fgets(command, MSIZEBACK - 1, stdin) != NULL) {
                        execute_command(command, STDIN);
                    }
                } else if (fd == localhost -> fd) {

                    int new_peer_fd = accept(fd, (struct sockaddr * ) & new_peer_addr, & addrlen);
                    if (new_peer_fd != -1) {
                        c_recv_file_frm_peer(new_peer_fd);
                    }
                }
            }
        }

        fflush(stdout);
    }

    return;

}

// refresh command
void c_refresh_list(char clientListString[]) {
    char * received = strstr(clientListString, "RECEIVE");
    int rcvi = received - clientListString, cmdi = 0;
    char command[MSIZE];
    int blank_count = 0;
    while (received != NULL && rcvi < strlen(clientListString)) {
        if (clientListString[rcvi] == ' ')
            blank_count++;
        else
            blank_count = 0;
        command[cmdi] = clientListString[rcvi];
        if (blank_count == 4) {
            command[cmdi - 3] = '\0';
            strcat(command, "\n");
            c_exec_comm(command);
            cmdi = -1;
        }
        cmdi++;
        rcvi++;
    }
    bool is_refresh = false;
    clients = malloc(sizeof(struct host));
    struct host * head = clients;
    const char delimmiter[2] = "\n";
    char * token = strtok(clientListString, delimmiter);
    if (strstr(token, "NOTFIRST")) {
        is_refresh = true;
    }
    if (token != NULL) {
        token = strtok(NULL, delimmiter);
        char client_ip[MSIZE], client_port[MSIZE], client_hostname[MSIZE];
        while (token != NULL) {
            if (strstr(token, "ENDREFRESH") != NULL) {
                break;
            }
            struct host * new_client = malloc(sizeof(struct host));
            sscanf(token, "%s %s %s\n", client_ip, client_port, client_hostname);
            token = strtok(NULL, delimmiter);
            memcpy(new_client -> port_num, client_port, sizeof(new_client -> port_num));
            memcpy(new_client -> ip_addr, client_ip, sizeof(new_client -> ip_addr));
            memcpy(new_client -> hostname, client_hostname, sizeof(new_client -> hostname));
            new_client -> is_logged_in = true;
            clients -> next_host = new_client;
            clients = clients -> next_host;
        }
        clients = head -> next_host;
    }
    if (is_refresh) {
        cse4589_print_and_log("[REFRESH:SUCCESS]\n");
        cse4589_print_and_log("[REFRESH:END]\n");
    } else {
        c_exec_comm("SUCCESSLOGIN");
    }
}

// handling send request from client side
void c_send(char command[]) {
    char client_ip[MSIZE];
    int cmdi = 5;
    int ipi = 0;
    while (command[cmdi] != ' ') {
        client_ip[ipi] = command[cmdi];
        cmdi += 1;
        ipi += 1;
    }
    client_ip[ipi] = '\0';
    if (!h_valid_ip(client_ip)) {
        cse4589_print_and_log("[SEND:ERROR]\n");
        cse4589_print_and_log("[SEND:END]\n");
        return;
    }
    struct host * temp = clients;
    while (temp != NULL) {
        if (strstr(temp -> ip_addr, client_ip) != NULL) {
            h_send_com(server -> fd, command);
            break;
        }
        temp = temp -> next_host;
    }
    if (temp == NULL) {
        cse4589_print_and_log("[SEND:ERROR]\n");
        cse4589_print_and_log("[SEND:END]\n");
    }
}

// Handling command if message received from other client
void c_receive(char client_ip[], char msg[]) {
    cse4589_print_and_log("[RECEIVED:SUCCESS]\n");
    cse4589_print_and_log("msg from:%s\n[msg]:%s\n", client_ip, msg);
    cse4589_print_and_log("[RECEIVED:END]\n");
}

// Handling the block or unblock request from the client side
void c_block_unblock(char command[], bool is_a_block) {
    char client_ip[MSIZE];
    if (is_a_block) {
        sscanf(command, "BLOCK %s\n", client_ip);
    } else {
        sscanf(command, "UNBLOCK %s\n", client_ip);
    }

    // if present in the list
    struct host * temp = clients;
    while (temp != NULL) {
        if (strstr(client_ip, temp -> ip_addr) != NULL) {
            break;
        }
        temp = temp -> next_host;
    }
    struct host * blocked_client = temp;

    // check if already blocked, then no need to block
    temp = localhost -> blocked;
    while (temp != NULL) {
        if (strstr(client_ip, temp -> ip_addr) != NULL) {
            break;
        }
        temp = temp -> next_host;
    }
    struct host * blocked_client_2 = temp;

    if (blocked_client != NULL && blocked_client_2 == NULL && is_a_block) {
        struct host * new_blocked_client = malloc(sizeof(struct host));
        memcpy(new_blocked_client -> ip_addr, blocked_client -> ip_addr, sizeof(new_blocked_client -> ip_addr));
        memcpy(new_blocked_client -> port_num, blocked_client -> port_num, sizeof(new_blocked_client -> port_num));
        memcpy(new_blocked_client -> hostname, blocked_client -> hostname, sizeof(new_blocked_client -> hostname));
        new_blocked_client -> fd = blocked_client -> fd;
        new_blocked_client -> next_host = NULL;
        if (localhost -> blocked != NULL) {
            struct host * temp_blocked = localhost -> blocked;
            while (temp_blocked -> next_host != NULL) {
                temp_blocked = temp_blocked -> next_host;
            }
            temp_blocked -> next_host = new_blocked_client;
        } else {
            localhost -> blocked = new_blocked_client;
        }
        h_send_com(server -> fd, command);
    } else if (blocked_client != NULL && blocked_client_2 != NULL && !is_a_block) {
        struct host * temp_blocked = localhost -> blocked;
        if (strstr(blocked_client -> ip_addr, temp_blocked -> ip_addr) != NULL) {
            localhost -> blocked = localhost -> blocked -> next_host;
        } else {
            struct host * previous = temp_blocked;
            while (temp_blocked != NULL) {
                if (strstr(temp_blocked -> ip_addr, blocked_client -> ip_addr) != NULL) {
                    previous -> next_host = temp_blocked -> next_host;
                    break;
                }
                temp_blocked = temp_blocked -> next_host;
            }
        }
        h_send_com(server -> fd, command);

    } else {
        if (is_a_block) {
            cse4589_print_and_log("[BLOCK:ERROR]\n");
            cse4589_print_and_log("[BLOCK:END]\n");
        } else {
            cse4589_print_and_log("[UNBLOCK:ERROR]\n");
            cse4589_print_and_log("[UNBLOCK:END]\n");
        }
    }
}
