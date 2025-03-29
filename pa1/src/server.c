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

//Refresh
void s_refresh(int requesting_client_fd);

// SEND
void s_send(char client_ip[], char msg[], int requesting_client_fd);

// BROADCAST
void s_broadcast(char msg[], int requesting_client_fd);

//block and unblock
void s_block_unblock(char command[], bool is_a_block, int requesting_client_fd);

// BLOCKED
void s_blocked(char blocker_ip_addr[]);

// LOGOUT
void s_logout(int requesting_client_fd);

// EXIT
void s_exit(int requesting_client_fd);

// STATISTICS
void s_stats();
