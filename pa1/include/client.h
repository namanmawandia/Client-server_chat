#ifndef CLIENT_H
#define CLIENT_H


// application startup
void client__init();
int c_reg_list();

//command execution
void execute_command(char command[], int requesting_client_fd);
void c_exec_comm(char command[]);

#endif