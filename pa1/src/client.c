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
        // setsockopt(listener, SOL_SOCKET, SO_REUSEPORT, & yes, sizeof(int));
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