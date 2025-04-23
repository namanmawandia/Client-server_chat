#define _POSIX_C_SOURCE 200112L

#include <stdio.h>      
#include <stdlib.h>     
#include <string.h>     
#include <stdbool.h>    
#include <unistd.h>     
#include <arpa/inet.h>  
#include <netdb.h>      
#include <sys/types.h>  
#include <sys/socket.h>

#include "../include/global.h"

#include "../include/logger.h"

#include "../include/common.h"

#define min(a, b)(((a) < (b)) ? (a) : (b))
#define MSIZE 500
#define MSIZEBACK 500 * 200
#define STDIN 0



// global variables init
struct host * new_client = NULL;
struct host * clients = NULL;
struct host * localhost = NULL;
struct host * server = NULL; 
int yes = 1; 


// function definitions
/////////////////////////////////////////////////////////////////////

// check ip address valid or not
bool h_valid_ip(char ip_addr[MSIZE]) {
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ip_addr, & (sa.sin_addr));
    return result != 0;
}


void h_send_com(int fd, char msg[]) {
    int rv;
    if (rv = send(fd, msg, strlen(msg) + 1, 0) == -1) {
        // printf("ERROR")
    }
}

// to execute some common commands from the host side
void h_exec_command(char command[], int requesting_client_fd) {
    if (strstr(command, "AUTHOR") != NULL) {
        h_author();
    } else if (strstr(command, "IP") != NULL) {
        h_ip_address();
    } else if (strstr(command, "PORT") != NULL) {
        h_port();
    }
    fflush(stdout);
}

// author print
void h_author() {
    cse4589_print_and_log("[AUTHOR:SUCCESS]\n");
    cse4589_print_and_log("I, have read and understood the course academic integrity policy.\n");
    cse4589_print_and_log("[AUTHOR:END]\n");
}

// IP print
void h_ip_address() {
    cse4589_print_and_log("[IP:SUCCESS]\n");
    cse4589_print_and_log("IP:%s\n", localhost -> ip_addr);
    cse4589_print_and_log("[IP:END]\n");
}

//Port print
void h_port() {
    cse4589_print_and_log("[PORT:SUCCESS]\n");
    cse4589_print_and_log("PORT:%s\n", localhost -> port_num);
    cse4589_print_and_log("[PORT:END]\n");
}

// list of clients loggedin print
void h_list() {
    cse4589_print_and_log("[LIST:SUCCESS]\n");

    struct host * temp = clients;
    int id = 1;
    while (temp != NULL) {
        
        if (temp -> is_logged_in) {
            cse4589_print_and_log("%-5d%-35s%-20s%-8s\n", id, temp -> hostname, temp -> ip_addr, (temp -> port_num));
            id = id + 1;
        }
        temp = temp -> next_host;
    }

    cse4589_print_and_log("[LIST:END]\n");
}
