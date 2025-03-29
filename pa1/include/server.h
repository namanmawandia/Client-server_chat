#ifndef SERVER_H
#define SERVER_H

// Function declaration

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

#endif