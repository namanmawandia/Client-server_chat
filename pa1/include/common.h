#ifndef COMMON_H
#define COMMON_H


#define min(a, b)(((a) < (b)) ? (a) : (b))
#define MSIZE 500
#define MSIZEBACK 500 * 200
#define STDIN 0

// Structs
struct message {
    char text[MSIZEBACK];
    struct host * from_client;
    struct message * next_message;
    bool is_broadcast;
};

struct host {
    char hostname[MSIZE];
    char ip_addr[MSIZE];
    char port_num[MSIZE];
    int num_msg_sent;
    int num_msg_rcv;
    char status[MSIZE];
    int fd;
    struct host * blocked;
    struct host * next_host;
    bool is_logged_in;
    bool is_server;
    struct message * queued_messages;
};


// declare global variables
extern struct host * new_client;
extern struct host * clients;
extern struct host * localhost;
extern struct host * server; 
extern int yes;

// functions
bool h_valid_ip(char ip_addr[MSIZE]);
void h_send_com(int fd, char msg[]);
void h_exec_command(char command[], int requesting_client_fd);


void h_author();
void h_ip_address();
void h_port();
void h_list();

#endif