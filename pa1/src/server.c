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

// #include "../include/client.h"

#define min(a, b)(((a) < (b)) ? (a) : (b))
#define MSIZE 500
#define MSIZEBACK 500 * 200
#define STDIN 0

//////////////////////////////////////////////////////////////////
//function declarations

// Application startup
void server__init();
void * host__get_in_addr(struct sockaddr * sa);

// command execution
void execute_command_server(char command[], int requesting_client_fd);
void s_exec_command(char command[], int requesting_client_fd);

//Login
void h_login(char client_ip[], char client_port[], char client_hostname[], int requesting_client_fd);

//Refresh server
void s_refresh(int requesting_client_fd);

// Send server
void s_send(char client_ip[], char msg[], int requesting_client_fd);

// Broadcast server
void s_broadcast(char msg[], int requesting_client_fd);

//block and unblock server
void s_block_unblock(char command[], bool is_a_block, int requesting_client_fd);

// blocked server
void s_blocked(char blocker_ip_addr[]);

// logout server
void s_logout(int requesting_client_fd);

// exit server
void s_exit(int requesting_client_fd);

// statistics server
void s_stats();

////////////////////////////////////////////////////////////////////////////////////////
// functions implimentation

/***  EXECUTE COMMANDS ***/
void execute_command_server(char command[], int requesting_client_fd) {
    h_exec_command(command, requesting_client_fd);
    if (localhost -> is_server) {
        s_exec_command(command, requesting_client_fd);
    }
    fflush(stdout);
}

/***  SERVER INITIALISATION ***/
void server__init() {
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

    // assign listener to localhost fd
    localhost -> fd = listener;

    freeaddrinfo(localhost_ai);

    // Now we have a listener_fd. We add it to he master list of fds along with stdin.
    fd_set master; // master file descriptor list
    fd_set read_fds; // temp file descriptor list for select()
    FD_ZERO( & master); // clear the master and temp sets
    FD_ZERO( & read_fds);
    FD_SET(listener, & master); // Add listener to the master list
    FD_SET(STDIN, & master); // Add STDIN to the master list
    int fdmax = listener > STDIN ? listener : STDIN; // maximum file descriptor number. initialised to listener    
    // variable initialisations
    int new_client_fd; // newly accept()ed socket descriptor
    struct sockaddr_storage new_client_addr; // client address
    socklen_t addrlen; // address length
    char data_buffer[MSIZEBACK]; // buffer for client data
    int data_buffer_bytes; // holds number of bytes received and stored in data_buffer
    char newClientIP[INET6_ADDRSTRLEN]; // holds the ip of the new client
    int fd;

    // main loop
    while (true) {
        read_fds = master; // make a copy of master set
        if (select(fdmax + 1, & read_fds, NULL, NULL, NULL) == -1) {
            exit(EXIT_FAILURE);
        }

        // run through the existing connections looking for data to read
        for (fd = 0; fd <= fdmax; fd++) {
            if (FD_ISSET(fd, & read_fds)) {
                // if fd == listener, a new connection has come in.
                if (fd == listener) {
                    addrlen = sizeof new_client_addr;
                    new_client_fd = accept(listener, (struct sockaddr * ) & new_client_addr, & addrlen);

                    if (new_client_fd != -1) {
                        // We register the new client onto our system here.
                        // We store the new client details here. We will assign the values later when the 
                        // client sends more information about itself like the hostname
                        new_client = malloc(sizeof(struct host));
                        FD_SET(new_client_fd, & master); // add to master set
                        if (new_client_fd > fdmax) { // keep track of the max
                            fdmax = new_client_fd;
                        }
                        memcpy(new_client -> ip_addr,
                            inet_ntop(
                                new_client_addr.ss_family,
                                host__get_in_addr((struct sockaddr * ) & new_client_addr), // even though new_client_addr is of type sockaddr_storage, they can be cast into each other. Refer beej docs.
                                newClientIP,
                                INET6_ADDRSTRLEN
                            ), sizeof(new_client -> ip_addr));
                        new_client -> fd = new_client_fd;
                        new_client -> num_msg_sent = 0;
                        new_client -> num_msg_rcv = 0;
                        new_client -> is_logged_in = true;
                        new_client -> next_host = NULL;
                        new_client -> blocked = NULL;
                    }
                    fflush(stdout);
                } else if (fd == STDIN) {
                    // handle data from standard input
                    char * command = (char * ) malloc(sizeof(char) * MSIZEBACK);
                    memset(command, '\0', MSIZEBACK);
                    if (fgets(command, MSIZEBACK - 1, stdin) == NULL) { // -1 because of new line
                    } else {
                        execute_command_server(command, fd);
                    }
                    fflush(stdout);
                } else {
                    // handle data from a client
                    data_buffer_bytes = recv(fd, data_buffer, sizeof data_buffer, 0);
                    if (data_buffer_bytes == 0) {
                        close(fd); // Close the connection
                        FD_CLR(fd, & master); // Remove the fd from master set
                    } else if (data_buffer_bytes == -1) {
                        close(fd); // Close the connection
                        FD_CLR(fd, & master); // Remove the fd from master set
                    } else {
                        execute_command_server(data_buffer, fd);
                    }
                    fflush(stdout);
                }
            }
        }
    }
    return;
}


