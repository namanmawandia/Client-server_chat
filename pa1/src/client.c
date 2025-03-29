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

// LOGIN
int c_connect_s(char server_ip[], char server_port[]);
void c_login(char server_ip[], char server_port[]);

// REFRESH
void c_refresh_list(char clientListString[]);

//SEND
void c_send(char command[]);
void c_receive(char client_ip[], char msg[]);

// BLOCK AND UNBLOCK
void c_block_unblock(char command[], bool is_a_block);

//exit
void c_exit();

// SENDFILE (BONUS)
void c_p2p_file_transfer(char peer_ip[], char file_name[]);
void c_recv_file_frm_peer(int peer_fd);

///////////////////////////////////////////////////////////////////////////////
// Function Definitions

/***  EXECUTE COMMANDS ***/
void execute_command(char command[], int requesting_client_fd) {
    h_exec_command(command, requesting_client_fd);
    if (!localhost -> is_server) {
        c_exec_comm(command);
    }
    fflush(stdout);
}
