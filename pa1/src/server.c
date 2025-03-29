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

// starting of the server
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

    // assign listener to localhost fd
    localhost -> fd = listener;

    freeaddrinfo(localhost_ai);

    fd_set master; // master file descriptor list
    fd_set read_fds; // temporary file descriptor list
    FD_ZERO( & master); // initialize all the list to zero or clear old values
    FD_ZERO( & read_fds);
    FD_SET(listener, & master); // listener added to master
    FD_SET(STDIN, & master); // STDIN is also added to list, so that commands from user input is considered
    int fdmax = listener > STDIN ? listener : STDIN;    
    
    //Initilization
    int new_client_fd; // newly socket descriptor
    struct sockaddr_storage new_client_addr; // client address
    socklen_t addrlen; 
    char data_buffer[MSIZEBACK]; // buffer for client data
    int data_buffer_bytes; 
    char newClientIP[INET6_ADDRSTRLEN]; // ip of the new client
    int fd;

    while (true) {
        read_fds = master;
        if (select(fdmax + 1, & read_fds, NULL, NULL, NULL) == -1) {
            exit(EXIT_FAILURE);
        }

        for (fd = 0; fd <= fdmax; fd++) {
            if (FD_ISSET(fd, & read_fds)) {
                if (fd == listener) {
                    addrlen = sizeof new_client_addr;
                    new_client_fd = accept(listener, (struct sockaddr * ) & new_client_addr, & addrlen);

                    if (new_client_fd != -1) {
                        // new client to be added
                        new_client = malloc(sizeof(struct host));
                        FD_SET(new_client_fd, & master); 
                        if (new_client_fd > fdmax) { 
                            fdmax = new_client_fd;
                        }
                        memcpy(new_client -> ip_addr,
                            inet_ntop(
                                new_client_addr.ss_family,
                                host__get_in_addr((struct sockaddr * ) & new_client_addr), 
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
                    // standard input data
                    char * command = (char * ) malloc(sizeof(char) * MSIZEBACK);
                    memset(command, '\0', MSIZEBACK);
                    if (fgets(command, MSIZEBACK - 1, stdin) == NULL) { 
                    } else {
                        execute_command_server(command, fd);
                    }
                    fflush(stdout);
                } else {
                    // client data 
                    data_buffer_bytes = recv(fd, data_buffer, sizeof data_buffer, 0);
                    if (data_buffer_bytes == 0) {
                        close(fd); // Close
                        FD_CLR(fd, & master); // fd removed from list
                    } else if (data_buffer_bytes == -1) {
                        close(fd); // Close
                        FD_CLR(fd, & master); // fd removed from list
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

// get the IP v6 or v4
void * host__get_in_addr(struct sockaddr * sa) {
    if (sa -> sa_family == AF_INET) {
        return &(((struct sockaddr_in * ) sa) -> sin_addr);
    }
    return &(((struct sockaddr_in6 * ) sa) -> sin6_addr);
}

// executing the commands
void s_exec_command(char command[], int requesting_client_fd) {
    if (strstr(command, "LIST") != NULL) {
        h_list();
    } else if (strstr(command, "STATISTICS") != NULL) {
        s_stats();
    } else if (strstr(command, "BLOCKED") != NULL) {
        char client_ip[MSIZE];
        sscanf(command, "BLOCKED %s", client_ip);
        s_blocked(client_ip);
    } else if (strstr(command, "LOGIN") != NULL) {
        char client_hostname[MSIZE], client_port[MSIZE], client_ip[MSIZE];
        sscanf(command, "LOGIN %s %s %s", client_ip, client_port, client_hostname);
        h_login(client_ip, client_port, client_hostname, requesting_client_fd);
    } else if (strstr(command, "BROADCAST") != NULL) {
        char message[MSIZEBACK];
        int cmdi = 10;
        int msgi = 0;
        while (command[cmdi] != '\0') {
            message[msgi] = command[cmdi];
            cmdi += 1;
            msgi += 1;
        }
        message[msgi - 1] = '\0';
        s_broadcast(message, requesting_client_fd);
    } else if (strstr(command, "REFRESH") != NULL) {
        s_refresh(requesting_client_fd);
    } else if (strstr(command, "SEND") != NULL) {
        char client_ip[MSIZE], message[MSIZE];
        int cmdi = 5;
        int ipi = 0;
        while (command[cmdi] != ' ') {
            client_ip[ipi] = command[cmdi];
            cmdi += 1;
            ipi += 1;
        }
        client_ip[ipi] = '\0';
        cmdi++;
        int msgi = 0;
        while (command[cmdi] != '\0') {
            message[msgi] = command[cmdi];
            cmdi += 1;
            msgi += 1;
        }
        message[msgi - 1] = '\0'; // Remove new line
        s_send(client_ip, message, requesting_client_fd);
    } else if (strstr(command, "UNBLOCK") != NULL) {
        s_block_unblock(command, false, requesting_client_fd);
    } else if (strstr(command, "BLOCK") != NULL) {
        s_block_unblock(command, true, requesting_client_fd);
    } else if (strstr(command, "LOGOUT") != NULL) {
        s_logout(requesting_client_fd);
    } else if (strstr(command, "EXIT") != NULL) {
        s_exit(requesting_client_fd);
    }
    fflush(stdout);
}

// handling login from the side of server, adding in list etc
void h_login(char client_ip[], char client_port[], char client_hostname[], int requesting_client_fd) {
    char client_return_msg[MSIZEBACK] = "REFRESHRESPONSE FIRST\n";
    struct host * temp = clients;
    bool is_new = true;
    struct host * requesting_client = malloc(sizeof(struct host));

    while (temp != NULL) {
        if (temp -> fd == requesting_client_fd) {
            requesting_client = temp;
            is_new = false;
            break;
        }
        temp = temp -> next_host;
    }

    if (is_new) {
        memcpy(new_client -> hostname, client_hostname, sizeof(new_client -> hostname));
        memcpy(new_client -> port_num, client_port, sizeof(new_client -> port_num));
        requesting_client = new_client;
        int client_port_value = atoi(client_port);
        if (clients == NULL) {
            clients = malloc(sizeof(struct host));
            clients = new_client;
        } else if (client_port_value < atoi(clients -> port_num)) {
            new_client -> next_host = clients;
            clients = new_client;
        } else {
            struct host * temp = clients;
            while (temp -> next_host != NULL && atoi(temp -> next_host -> port_num) < client_port_value) {
                temp = temp -> next_host;
            }
            new_client -> next_host = temp -> next_host;
            temp -> next_host = new_client;
        }

    } else {
        requesting_client -> is_logged_in = true;
    }

    temp = clients;
    while (temp != NULL) {
        if (temp -> is_logged_in) {
            char clientString[MSIZE * 4];
            sprintf(clientString, "%s %s %s\n", temp -> ip_addr, temp -> port_num, temp -> hostname);
            strcat(client_return_msg, clientString);
        }
        temp = temp -> next_host;
    }

    strcat(client_return_msg, "ENDREFRESH\n");
    struct message * temp_message = requesting_client -> queued_messages;
    char receive[MSIZEBACK * 3];

    while (temp_message != NULL) {
        requesting_client -> num_msg_rcv++;
        sprintf(receive, "RECEIVE %s %s    ", temp_message -> from_client -> ip_addr, temp_message -> text);
        strcat(client_return_msg, receive);

        if (!temp_message -> is_broadcast) {
            cse4589_print_and_log("[RELAYED:SUCCESS]\n");
            cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", temp_message -> from_client -> ip_addr, requesting_client -> ip_addr, temp_message -> text);
            cse4589_print_and_log("[RELAYED:END]\n");
        }
        temp_message = temp_message -> next_message;
    }
    h_send_com(requesting_client_fd, client_return_msg);
    requesting_client -> queued_messages = temp_message;
}

// refresh request from client side, handling frm server side
void s_refresh(int requesting_client_fd) {
    char clientListString[MSIZEBACK] = "REFRESHRESPONSE NOTFIRST\n";
    struct host * temp = clients;
    while (temp != NULL) {
        if (temp -> is_logged_in) {
            char clientString[MSIZE * 4];
            sprintf(clientString, "%s %s %s\n", temp -> ip_addr, temp -> port_num, temp -> hostname);
            strcat(clientListString, clientString);
        }
        temp = temp -> next_host;
    }
    strcat(clientListString, "ENDREFRESH\n");
    h_send_com(requesting_client_fd, clientListString);
}

// handling send request from server side
void s_send(char client_ip[], char msg[], int requesting_client_fd) {

    char receive[MSIZE * 4];
    struct host * temp = clients;
    struct host * from_client = malloc(sizeof(struct host)), * to_client = malloc(sizeof(struct host));;
    while (temp != NULL) {
        if (strstr(client_ip, temp -> ip_addr) != NULL) {
            to_client = temp;
        }
        if (requesting_client_fd == temp -> fd) {
            from_client = temp;
        }
        temp = temp -> next_host;
    }

    from_client -> num_msg_sent++;
    // if client is blocked by the server or other client

    bool is_blocked = false;

    temp = to_client -> blocked;
    while (temp != NULL) {
        if (strstr(from_client -> ip_addr, temp -> ip_addr) != NULL) {
            is_blocked = true;
            break;
        }
        temp = temp -> next_host;
    }
    h_send_com(from_client -> fd, "SUCCESSSEND\n");
    if (is_blocked) {
        cse4589_print_and_log("[RELAYED:SUCCESS]\n");
        cse4589_print_and_log("msg from:%s, to:%s\n[msg]:%s\n", from_client -> ip_addr, to_client -> ip_addr, msg);
        cse4589_print_and_log("[RELAYED:END]\n");
        h_send_com(from_client -> fd, "SUCCESSSEND\n");
        return;
    }

    if (to_client -> is_logged_in) {
        to_client -> num_msg_rcv++;
        sprintf(receive, "RECEIVE %s %s\n", from_client -> ip_addr, msg);
        h_send_com(to_client -> fd, receive);
        
    } else {
        struct message * new_message = malloc(sizeof(struct message));
        memcpy(new_message -> text, msg, sizeof(new_message -> text));
        new_message -> from_client = from_client;
        new_message -> is_broadcast = false;
        if (to_client -> queued_messages == NULL) {
            to_client -> queued_messages = new_message;
        } else {
            struct message * temp_message = to_client -> queued_messages;
            while (temp_message -> next_message != NULL) {
                temp_message = temp_message -> next_message;
            }
            temp_message -> next_message = new_message;
        }
    }

}

// Handling broadcast request from client on server side
void s_broadcast(char msg[], int requesting_client_fd) {
    struct host * temp = clients;
    struct host * from_client = malloc(sizeof(struct host));
    while (temp != NULL) {
        if (requesting_client_fd == temp -> fd) {
            from_client = temp;
        }
        temp = temp -> next_host;
    }
    struct host * to_client = clients;
    int id = 1;
    from_client -> num_msg_sent++;
    while (to_client != NULL) {
        if (to_client -> fd == requesting_client_fd) {
            to_client = to_client -> next_host;
            continue;
        }

        bool is_blocked = false;

        struct host * temp_blocked = to_client -> blocked;
        while (temp_blocked != NULL) {
            if (temp_blocked -> fd == requesting_client_fd) {
                is_blocked = true;
                break;
            }
            temp_blocked = temp_blocked -> next_host;
        }

        if (is_blocked) {
            to_client = to_client -> next_host;
            continue;
        }

        char receive[MSIZE * 4];

        if (to_client -> is_logged_in) {
            to_client -> num_msg_rcv++;
            sprintf(receive, "RECEIVE %s %s\n", from_client -> ip_addr, msg);
            h_send_com(to_client -> fd, receive);
        } else {
            struct message * new_message = malloc(sizeof(struct message));
            memcpy(new_message -> text, msg, sizeof(new_message -> text));
            new_message -> from_client = from_client;
            new_message -> is_broadcast = true;
            if (to_client -> queued_messages == NULL) {
                to_client -> queued_messages = new_message;
            } else {
                struct message * temp_message = to_client -> queued_messages;
                while (temp_message -> next_message != NULL) {
                    temp_message = temp_message -> next_message;
                }
                temp_message -> next_message = new_message;
            }
        }
        to_client = to_client -> next_host;
    }

    cse4589_print_and_log("[RELAYED:SUCCESS]\n");
    cse4589_print_and_log("msg from:%s, to:255.255.255.255\n[msg]:%s\n", from_client -> ip_addr, msg);
    cse4589_print_and_log("[RELAYED:END]\n");
    h_send_com(from_client -> fd, "SUCCESSBROADCAST\n");
}

// handling block and unblock from the server side
void s_block_unblock(char command[], bool is_a_block, int requesting_client_fd) {
    char client_ip[MSIZE], client_port[MSIZE];;
    if (is_a_block) {
        sscanf(command, "BLOCK %s %s\n", client_ip, client_port);
    } else {
        sscanf(command, "UNBLOCK %s %s\n", client_ip, client_port);
    }
    struct host * temp = clients;
    struct host * requesting_client = malloc(sizeof(struct host));
    struct host * blocked_client = malloc(sizeof(struct host));

    while (temp != NULL) {
        if (temp -> fd == requesting_client_fd) {
            requesting_client = temp;
        }
        if (strstr(client_ip, temp -> ip_addr) != NULL) {
            blocked_client = temp;
        }
        temp = temp -> next_host;
    }

    if (blocked_client != NULL) {
        if (is_a_block) {
            struct host * new_blocked_client = malloc(sizeof(struct host));
            memcpy(new_blocked_client -> ip_addr, blocked_client -> ip_addr, sizeof(new_blocked_client -> ip_addr));
            memcpy(new_blocked_client -> port_num, blocked_client -> port_num, sizeof(new_blocked_client -> port_num));
            memcpy(new_blocked_client -> hostname, blocked_client -> hostname, sizeof(new_blocked_client -> hostname));
            new_blocked_client -> fd = blocked_client -> fd;
            new_blocked_client -> next_host = NULL;
            int new_blocked_client_port_value = atoi(new_blocked_client -> port_num);
            if (requesting_client -> blocked == NULL) {
                requesting_client -> blocked = malloc(sizeof(struct host));
                requesting_client -> blocked = new_blocked_client;
            } else if (new_blocked_client_port_value < atoi(requesting_client -> blocked -> port_num)) {
                new_blocked_client -> next_host = requesting_client -> blocked;
                requesting_client -> blocked = new_blocked_client;
            } else {
                struct host * temp = requesting_client -> blocked;
                while (temp -> next_host != NULL && atoi(temp -> next_host -> port_num) < new_blocked_client_port_value) {
                    temp = temp -> next_host;
                }
                new_blocked_client -> next_host = temp -> next_host;
                temp -> next_host = new_blocked_client;
            }

            h_send_com(requesting_client_fd, "SUCCESSBLOCK\n");
        } else {
            struct host * temp_blocked = requesting_client -> blocked;
            if (strstr(temp_blocked -> ip_addr, blocked_client -> ip_addr) != NULL) {
                requesting_client -> blocked = requesting_client -> blocked -> next_host;
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
            h_send_com(requesting_client_fd, "SUCCESSUNBLOCK\n");
        }
    } else {
        if (is_a_block) {
            h_send_com(requesting_client_fd, "ERRORBLOCK\n");
        } else {
            h_send_com(requesting_client_fd, "ERRORUNBLOCK\n");
        }
    }
}

// handlong the blocked command by printing all the blocked IP for the client
void s_blocked(char blocker_ip_addr[]) {
    struct host * temp = clients;
    while (temp != NULL) {
        if (strstr(blocker_ip_addr, temp -> ip_addr) != NULL) {
            break;
        }
        temp = temp -> next_host;
    }
    if (h_valid_ip(blocker_ip_addr) && temp) {
        cse4589_print_and_log("[BLOCKED:SUCCESS]\n");
        struct host * temp_blocked = clients;
        temp_blocked = temp -> blocked;
        int id = 1;
        while (temp_blocked != NULL) {
            cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", id, temp_blocked -> hostname, temp_blocked -> ip_addr, atoi(temp_blocked -> port_num));
            id = id + 1;
            temp_blocked = temp_blocked -> next_host;
        }
    } else {
        cse4589_print_and_log("[BLOCKED:ERROR]\n");
    }

    cse4589_print_and_log("[BLOCKED:END]\n");
}

// Logout command for logging out from connection
void s_logout(int requesting_client_fd) {
    struct host * temp = clients;
    while (temp != NULL) {
        if (temp -> fd == requesting_client_fd) {
            h_send_com(requesting_client_fd, "SUCCESSLOGOUT\n");
            temp -> is_logged_in = false;
            break;
        }
        temp = temp -> next_host;
    }
    if (temp == NULL) {
        h_send_com(requesting_client_fd, "ERRORLOGOUT\n");
    }
}

// handling exit command for exiting the interface
void s_exit(int requesting_client_fd) {
    struct host * temp = clients;
    if (temp -> fd == requesting_client_fd) {
        clients = clients -> next_host;
    } else {
        struct host * previous = temp;
        while (temp != NULL) {
            if (temp -> fd == requesting_client_fd) {
                previous -> next_host = temp -> next_host;
                temp = temp -> next_host;
                break;
            }
            temp = temp -> next_host;
        }
    }
}

// Statistics command to print the stats of number of message sent and received
void s_stats() {
    cse4589_print_and_log("[STATISTICS:SUCCESS]\n");

    struct host * temp = clients;
    int id = 1;
    while (temp != NULL) {
        cse4589_print_and_log("%-5d%-35s%-8d%-8d%-8s\n", id, temp -> hostname, temp -> num_msg_sent, temp -> num_msg_rcv, temp -> is_logged_in ? "logged-in" : "logged-out");
        id = id + 1;
        temp = temp -> next_host;
    }

    cse4589_print_and_log("[STATISTICS:END]\n");
}

