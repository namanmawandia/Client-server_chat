#ifndef CLIENT_H
#define CLIENT_H


void client__init();
int c_reg_list();

//command execution client
void execute_command(char command[], int requesting_client_fd);
void c_exec_comm(char command[]);

// login client
int c_connect_s(char server_ip[], char server_port[]);
void c_login(char server_ip[], char server_port[]);

// refresh client
void c_refresh_list(char clientListString[]);

//send client
void c_send(char command[]);
void c_receive(char client_ip[], char msg[]);

// block and unblock client
void c_block_unblock(char command[], bool is_a_block);

//exit client
void c_exit();

#endif