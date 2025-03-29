#ifndef SERVER_H
#define SERVER_H

// Function declaration

// Application startup
void server__init();
void * host__get_in_addr(struct sockaddr * sa);

// command execution
void execute_command_server(char command[], int requesting_client_fd);
void s_exec_command(char command[], int requesting_client_fd);

#endif